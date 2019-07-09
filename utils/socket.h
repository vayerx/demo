#pragma once

#include "generic_io.h"
#include "file_handle.h"

#if __cplusplus < 201703L
#   include <string>
#else
#   include <string_view>
#endif

#include <vector>

struct sockaddr_in;

namespace net {
    //--------------------------------------------------------------------------------------------------------
    class HostAddress {
    public:
#if __cplusplus < 201703L
        HostAddress(const std::string &a_host, unsigned a_port)
            : m_host(a_host)
            , m_port(a_port)
        {}
#else
        HostAddress(const std::string_view &a_host, unsigned a_port)
            : m_host(a_host)
            , m_port(a_port)
        {}
#endif
        const std::string & GetHost() const { return m_host; }
        unsigned            GetPort() const { return m_port; }
        std::string         GetHostPort() const;

        sockaddr_in         GetSockAddr() const;

    private:
        std::string m_host;
        unsigned    m_port;
    };

    //--------------------------------------------------------------------------------------------------------
    class BaseSocket
        : public FileDescriptor
    {
    public:
        using FileDescriptor::FileDescriptor;

        void    Connect(const HostAddress &address);
    };

    //--------------------------------------------------------------------------------------------------------
    class Tcp4Socket
        : public BaseSocket
    {
    public:
        Tcp4Socket();
    };
}
