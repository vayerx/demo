#define BOOST_TEST_MODULE http
#include <boost/test/unit_test.hpp>

#include <client/http.h>
#include <test/test_io.h>

struct HttpFixture
{
    test::BufferIO output;
    SocketFactory socket_factory;

    size_t read_chunk_size = 16;
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

            io->RegisterOnWrite([&]([[maybe_unused]] const void *data, [[maybe_unused]] size_t a_size) {
                BOOST_TEST_MESSAGE("WRITE:\n" + std::string(static_cast<const char *>(data), a_size));
            });
#endif

            io->SetInput(response);
            return std::unique_ptr<GenericIO>(io.release());
        };
    }
};


BOOST_FIXTURE_TEST_SUITE(http, HttpFixture)

//------------------------------------------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE(parse_urls)
{
    http_fetch("http://local.host/index.html", socket_factory, output);
    http_fetch("local.host/index.html", socket_factory, output);

    required_port = 8080;
    http_fetch("http://local.host:8080/index.html", socket_factory, output);
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
    BOOST_CHECK_THROW(http_fetch("http://local.host/", socket_factory, output), InvalidUrlError);
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
BOOST_AUTO_TEST_CASE(broken_connection)
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
    BOOST_CHECK_THROW(http_fetch("http://local.host/index.html", socket_factory, output), HttpRuntimeError);
}


BOOST_AUTO_TEST_SUITE_END()
