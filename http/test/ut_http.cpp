#define BOOST_TEST_MODULE http
#include <boost/test/unit_test.hpp>

#include <client/http.h>
#include <test/test_io.h>

struct HttpFixture
{
    test::BufferIO output;
    SocketFactory socket_factory;

    size_t read_chunk_size = 16;
    std::string request;
    std::string response;

    uint16_t    required_port = 80;
    std::string required_host = "local.host";
    std::string required_schema = "http";

    HttpFixture() {
        response =
            "HTTP/1.1 200 OK\n"
            "Content-Length: 2\n"
            "\n"
            "OK";

        socket_factory = [this](
            const std::string_view &schema,
            const std::string_view &host,
            uint16_t port
        ) {
            BOOST_CHECK_EQUAL(schema, required_schema);
            BOOST_CHECK_EQUAL(host, required_host);
            BOOST_CHECK_EQUAL(port, required_port);

            auto io = std::make_unique<test::ChunkIo>();

            io->SetReadChunkSize(read_chunk_size);
#if 0
            io->RegisterOnRead([&](void *, [[maybe_unused]] size_t a_size) {
                BOOST_TEST_MESSAGE("READ " + std::to_string(a_size) + " bytes");
            });
#endif

            io->RegisterOnWrite([&]([[maybe_unused]] const void *data, [[maybe_unused]] size_t a_size) {
                request += std::string_view(static_cast<const char *>(data), a_size);
            });

            io->SetInput(response);
            return std::unique_ptr<GenericIO>(io.release());
        };
    }

    void CheckValidUrl(const std::string &a_url, const std::string &a_path, uint16_t a_port = 80)
    {
        request = "";       // several requests are made in the same test-case -- too hard to make lots of TCs
        required_port = a_port;

        http_fetch(a_url, socket_factory, output);

        const std::string expect_query = "GET " + a_path + " HTTP/1.1";
        BOOST_CHECK_MESSAGE(
            request.find(expect_query) != std::string::npos,
            '"'+ expect_query + "\" not found in request \"" + request + '"'
        );

        const std::string content = output.AquireOutput();  // content is moved -- don't pass call to macro
        BOOST_CHECK_EQUAL(content, "OK");
    }
};


BOOST_FIXTURE_TEST_SUITE(http, HttpFixture)

//------------------------------------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(query_file)
{
    CheckValidUrl("http://local.host/index.html", "/index.html");
    CheckValidUrl("local.host/index.html", "/index.html");
    CheckValidUrl("http://local.host:8080/index.html", "/index.html", 8080);
}

//------------------------------------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(query_root)
{
    CheckValidUrl("http://local.host", "/");
    CheckValidUrl("http://local.host/", "/");
    CheckValidUrl("http://local.host:8080", "/", 8080);
    CheckValidUrl("http://local.host:8080/", "/", 8080);
}

//------------------------------------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(invalid_ports)
{
    BOOST_CHECK_THROW(http_fetch("http://local.host:1z//index.html", socket_factory, output), InvalidUrlError);
    BOOST_CHECK_THROW(http_fetch("http://local.host:99999/index.html", socket_factory, output), InvalidUrlError);
}

//------------------------------------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(empty_parts)
{
    BOOST_CHECK_THROW(http_fetch("http://local.host:/index.html", socket_factory, output), InvalidUrlError);
    BOOST_CHECK_THROW(http_fetch("http:///index.html", socket_factory, output), InvalidUrlError);
    BOOST_CHECK_THROW(http_fetch("http://:123/index.html", socket_factory, output), InvalidUrlError);
}

//------------------------------------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(unsuported_schema)
{
    BOOST_CHECK_THROW(http_fetch("https://local.host", socket_factory, output), InvalidUrlError);
    BOOST_CHECK_THROW(http_fetch("ftp://local.host/index.html", socket_factory, output), InvalidUrlError);
}

//------------------------------------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(response_crlf)
{
    response =
        "HTTP/1.1 200 OK\r\n"
        "Server: nginx/1.17.1\r\n"
        "Date: Sat, 06 Jul 2019 17:47:13 GMT\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 11\r\n"
        "Last-Modified: Sat, 06 Jul 2019 17:45:00 GMT\r\n"
        "\r\n"
        "XXX-content"
    ;
    http_fetch("http://local.host/index.html", socket_factory, output);
    const std::string content = output.AquireOutput();
    BOOST_CHECK_EQUAL(content, "XXX-content");
}

//------------------------------------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(response_lf)
{
    response =
        "HTTP/1.1 200 OK\n"
        "Server: nginx/1.17.1\n"
        "Date: Sat, 06 Jul 2019 17:47:13 GMT\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 11\n"
        "Last-Modified: Sat, 06 Jul 2019 17:45:00 GMT\n"
        "\n"
        "XXX-content"
    ;
    http_fetch("http://local.host/index.html", socket_factory, output);
    const std::string body = output.AquireOutput();
    BOOST_CHECK_EQUAL(body, "XXX-content");
}

//------------------------------------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(broken_connection_in_content)
{
    response =
        "HTTP/1.1 200 OK\n"
        "Server: nginx/1.17.1\n"
        "Date: Sat, 06 Jul 2019 17:47:13 GMT\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 100\n"
        "Last-Modified: Sat, 06 Jul 2019 17:45:00 GMT\n"
        "\n"
        "Ooops"
    ;
    BOOST_CHECK_THROW(http_fetch("http://local.host/index.html", socket_factory, output), HttpConnectionError);
}

//------------------------------------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(broken_connection_in_headers)
{
    response =
        "HTTP/1.1 200 OK\n"
        "Server: nginx/1.17.1\n"
        "Date: Sat, 06 Jul 2019 17:47:13 GMT\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 100\n"
    ;
    BOOST_CHECK_THROW(http_fetch("http://local.host/index.html", socket_factory, output), HttpConnectionError);
}

BOOST_AUTO_TEST_SUITE_END()
