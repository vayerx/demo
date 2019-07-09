#include "http.h"

#include <utils/socket.h>
#include <utils/logger.h>

#include <iostream>
#include <cstdlib>

//------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[]) try {
    if (argc < 2) {
        // can't use boost::program_options for "--help" -- suffer
        throw std::invalid_argument("url hasn't been specified");
    }

    // can't use boost::program_options -- suffer
    const std::string url = argv[1];
    DBGLOG("Fetching " + url);

    http_fetch(
        url,
        [](const std::string_view &/*schema*/,
            const std::string_view &host,
            uint16_t port
        ) {
            auto sock = std::make_unique<net::Tcp4Socket>();
            sock->Connect({host, port});
            return sock;
        },
        GetStdoutIO()
    );

    return EXIT_SUCCESS;
} catch (std::exception &x) {
    std::cerr << "Error: " << x.what() << std::endl;
    return EXIT_FAILURE;
}
