#include "http.h"
#include <utils/logger.h>

#include <cstring>
#include <strings.h>    // strncasecmp
#include <charconv>


//------------------------------------------------------------------------------------------------------------
/// Split @arg a_source into two parts by @arg a_pattern. Put prefix to @arg a_dest and @return suffix.
//------------------------------------------------------------------------------------------------------------
static std::string_view find_substr(
    const std::string_view &a_source,
    const std::string_view &a_pattern,
    std::string_view &a_dest
) {
    if (const auto pos = a_source.find(a_pattern); pos != std::string::npos) {
        a_dest = std::string_view{a_source.data(), pos};
        return a_source.substr(pos + a_pattern.size());
    } else {
        return a_source;
    }
}

//------------------------------------------------------------------------------------------------------------
/// Parse data in @arg source as @arg a_data data-type -- require full-match
//------------------------------------------------------------------------------------------------------------
template <typename Data>
void parse_field(const std::string_view &a_source, Data &a_data, const char *a_error_msg)
{
    if (auto [p, ec] = std::from_chars(a_source.data(), a_source.data() + a_source.size(), a_data);
        ec != std::errc() || p != a_source.data() + a_source.size()
    ) {
        throw InvalidUrlError(a_error_msg);
    }
}

//------------------------------------------------------------------------------------------------------------
enum class ResponseState {
    HEADER,
    BODY,
};

//------------------------------------------------------------------------------------------------------------
/// Receive HTTP-response headers.
/// Call @arg a_handler for each header with HEADER argument.
/// Call @arg a_handler for remaining data passing BODY argument.
//------------------------------------------------------------------------------------------------------------
template <typename Handler>
void receive_headers(GenericIO &a_source, Handler a_handler)
{
    char buffer[4096];
    size_t buf_pos = 0, buf_size = 0;
    bool end_of_headers = false;
    while (!end_of_headers) {
        const size_t read = a_source.Read(&buffer[buf_size], sizeof(buffer) - buf_size);
        if (!read) {
            throw HttpRuntimeError("connection has been unexpectedly closed");
        }
        buf_size += read;

        while (buf_pos < buf_size) {
            std::string_view sbuf(&buffer[buf_pos], buf_size - buf_pos);

            if (size_t pos = sbuf.find('\n'); pos != std::string::npos) {
                buf_pos += pos + 1;

                if (pos != 0 && (pos != 1 || sbuf[0] != '\r')) {
                    std::string_view header = sbuf.substr(0, pos);
                    if (*header.rbegin() == '\r') {
                        header.remove_suffix(1);
                    }

                    a_handler(header, ResponseState::HEADER);
                } else {
                    end_of_headers = true;
                    break;
                }
            } else {
                // need more data
                break;
            }
        }

        // shift data to the beginning of the buffer
        if (buf_pos > sizeof(buffer) / 2 || buf_size == sizeof(buffer)) {
            if (buf_pos == 0) {
                throw HttpRuntimeError("run out of buffer (too long headers?)");
            }

            // this should not occure in most cases
            std::memmove(buffer, &buffer[buf_pos], buf_size - buf_pos);
            buf_size -= buf_pos;
            buf_pos = 0;
        }
    }

    if (buf_size != buf_pos) {
        a_handler({&buffer[buf_pos], buf_size - buf_pos}, ResponseState::BODY);
    }
}

//------------------------------------------------------------------------------------------------------------
/// Fetch @arg a_url using socket from @arg a_socket_factory and write content to @arg a_output stream.
//------------------------------------------------------------------------------------------------------------
void http_fetch(
    const std::string &a_url,
    SocketFactory a_socket_factory,
    GenericIO &a_output
) {
    std::string_view schema("http"), host, path;
    uint16_t port = 80;

    // parse schema
    std::string_view unparsed = find_substr(a_url, "://", schema);
    if (schema != "http") {
        throw InvalidUrlError("unsupported schema: " + std::string(schema));
    }

    // parse host
    const size_t end_of_host_pos = unparsed.find_first_of(":/");
    if (end_of_host_pos < 1) {
        throw InvalidUrlError("missing host in url: " + a_url);
    }

    if (end_of_host_pos == std::string::npos) {
        host = unparsed;
        path = "/";
    } else {
        // parse port
        const bool has_port = (unparsed[end_of_host_pos] == ':');
        host = std::string_view{unparsed.data(), end_of_host_pos};
        unparsed.remove_prefix(end_of_host_pos + (has_port ? 1 : 0) /* leave first slash */);

        if (has_port) {
            const size_t end_of_port_pos = unparsed.find('/');
            const bool has_path = (end_of_port_pos != std::string::npos);
            if (end_of_port_pos < 1) {
                throw InvalidUrlError("missing port in url: " + a_url);
            }

            parse_field(unparsed.substr(0, end_of_port_pos), port, "invalid port in url");

            unparsed.remove_prefix(has_path ? end_of_port_pos /* leave first slash */ : unparsed.size());
        }

        // "parse" path
        path = unparsed.empty() ? "/" : unparsed;
    }

    // construct socket
    std::unique_ptr<GenericIO> socket = a_socket_factory(schema, host, port);

    // send request
    const std::string request =
        "GET " + std::string{path} + " HTTP/1.1\r\n"
        "Host: " + std::string{host} + "\r\n"
        "Connection: Close\r\n\r\n";
    socket->Write(request.data(), request.size());

    // get response
    size_t content_length = 0;
    receive_headers(*socket.get(), [&](const std::string_view &header, ResponseState state) {
        switch (state) {
        // response headers
        case  ResponseState::HEADER:
            if (header.starts_with("HTTP/")) {
                // get HTTP-code

                const size_t code_pos = header.find(' ');
                if (code_pos == std::string::npos) {
                    throw HttpRuntimeError("invalid header (no code): " + std::string(header));
                }
                const size_t code_end_pos = header.find(' ', code_pos + 1);
                if (code_end_pos == std::string::npos) {
                    throw HttpRuntimeError("invalid header (no message): " + std::string(header));
                }

                int http_code;
                parse_field(
                    header.substr(code_pos + 1, code_end_pos - code_pos - 1),
                    http_code,
                    "invalid response code"
                );

                if (http_code != 200) {
                    /// @todo support redirects
                    throw HttpRuntimeError(
                        "http request is unsuccessful: " + std::to_string(http_code) +
                        " (" + std::string(header.substr(code_end_pos + 1)) + ")"
                    );
                }
            } else {
                // parse generic headers
                std::string_view key;
                std::string_view value = find_substr(header, ": ", key);    ///< @todo support non-standard servers
                DBGLOG("Header {" + std::string(key) + "}={" + std::string(value) + "}");

                // process Content-Length header
                constexpr std::string_view cl_header = "content-length";
                if (key.size() == cl_header.size()
                    && strncasecmp(key.data(), cl_header.data(), cl_header.size()) == 0
                ) {
                    parse_field(value, content_length, "invalid Content-Length header");
                }
            }
            return;

        // remaining part of content
        case ResponseState::BODY:
            const size_t my_consume = std::min(header.size(), content_length);
            a_output.Write(header.data(), std::min(header.size(), content_length));
            content_length -= my_consume;
            return;
        }

        std::abort();   // unexpected state (compiler should make warning in the previous switch)
    });

    // receive remaining contents using greater buffer
    if (content_length) {
        const size_t buffer_size = std::min<size_t>(0x100000, content_length);
        std::unique_ptr<void, decltype(&free)> buffer(malloc(buffer_size), free);
        while (content_length) {
            const size_t read = socket->Read(buffer.get(), std::min(buffer_size, content_length));
            if (!read) {
                throw HttpRuntimeError("connection has been unexpectedly closed");
            }
            a_output.Write(buffer.get(), read);
            content_length -= read;
        }
    }
}
