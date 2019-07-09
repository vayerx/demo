#include <utils/generic_io.h>

#include <stdint.h>
#include <string_view>
#include <functional>
#include <stdexcept>
#include <memory>


class InvalidUrlError: public std::invalid_argument { using std::invalid_argument::invalid_argument; };
class HttpRuntimeError: public std::runtime_error { using std::runtime_error::runtime_error; };

//------------------------------------------------------------------------------------------------------------
typedef
    std::function<
        std::unique_ptr<GenericIO>(
            const std::string_view &schema,
            const std::string_view &host,
            uint16_t port
        )
    > SocketFactory;

//------------------------------------------------------------------------------------------------------------
void http_fetch(
    const std::string &a_url,
    SocketFactory a_socket_factory,
    GenericIO &a_output
);
