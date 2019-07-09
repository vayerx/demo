#include "socket.h"
#include "exception.h"
#include "logger.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

using namespace net;


//------------------------------------------------------------------------------------------------------------
std::string HostAddress::GetHostPort() const
{
    return m_host + ":" + std::to_string(m_port);
}

//------------------------------------------------------------------------------------------------------------
sockaddr_in HostAddress::GetSockAddr() const {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    if (m_host.empty() || m_host == "0.0.0.0") {
        // either no address or any-host IP
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_family = AF_INET;
    } else {
        hostent *hp = ::gethostbyname(m_host.c_str());

        if (hp == nullptr)
            throw std::runtime_error("SocketAddress: unknown host: " + m_host);

        (hp->h_length == sizeof(addr.sin_addr));      // Flawfinder: ignore
        memcpy(&addr.sin_addr, hp->h_addr, std::min<size_t>(hp->h_length, sizeof(addr.sin_addr)));
        addr.sin_family = static_cast<unsigned short>(hp->h_addrtype);
    }

    addr.sin_port = htons(static_cast<decltype(addr.sin_port)>(m_port));
    return addr;
}

//------------------------------------------------------------------------------------------------------------
void BaseSocket::Connect(const HostAddress &a_address) {
    const sockaddr_in server = a_address.GetSockAddr();
    if (::connect(m_handle, reinterpret_cast<const sockaddr*>(&server), sizeof(server)) == -1) {
        throw TSystemError(
            "Socket: can't connect to " + a_address.GetHostPort()
        );
    }

    DBGLOG("Connected to " << a_address.GetHostPort());
}

//------------------------------------------------------------------------------------------------------------
Tcp4Socket::Tcp4Socket()
    : BaseSocket(::socket(AF_INET, SOCK_STREAM, 0))
{
}
