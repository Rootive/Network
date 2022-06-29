#include "InetAddress.h"

#include <endian.h>
#include <netdb.h>
#include <cstring>
#include <arpa/inet.h>

namespace Rootive
{

static thread_local char threadLocal_resolveBuffer[64 * 1024];

bool InetAddress::resolve(const char *hostname, InetAddress *out)
{
    hostent hent, *he = nullptr;
    memset(&hent, 0, sizeof(hent));
    int herrno;
    int ret = gethostbyname_r(hostname, &hent, threadLocal_resolveBuffer, sizeof(threadLocal_resolveBuffer), &he, &herrno);
    if (!ret && he != nullptr)
    {
        out->addr_.sin_addr = *reinterpret_cast<in_addr *>(he->h_addr);
        return true;
    }
    return false;
}

InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6)
{
    if (ipv6)
    {
        memset(&addr6_, 0, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = ::htobe16(port);
    }
    else
    {
        memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
        addr_.sin_addr.s_addr = ::htobe32(ip);
        addr_.sin_port = ::htobe16(port);
    }
}

InetAddress::InetAddress(const char *ip, uint16_t port, bool ipv6)
{
    if (ipv6)
    {
        memset(&addr6_, 0, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = htobe16(port);
        ::inet_pton(AF_INET6, ip, &addr6_.sin6_addr);
    }
    else
    {
        memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htobe16(port);
        ::inet_pton(AF_INET, ip, &addr_.sin_addr);
    }
}

std::string InetAddress::toIPPortString() const
{
    char buf[64] = "";
    if (addr_.sin_family == AF_INET) { ::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(sizeof(buf))); }
    else if (addr_.sin_family == AF_INET6) { ::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, static_cast<socklen_t>(sizeof(buf))); }
    size_t end = ::strlen(buf);
    snprintf(buf + end, sizeof(buf) - end, ":%u", be16toh(addr_.sin_port));
    return buf;
}

std::string InetAddress::toIPString() const
{
    char buf[64] = "";
    if (addr_.sin_family == AF_INET) { ::inet_ntop(AF_INET, &addr_.sin_addr, buf, static_cast<socklen_t>(sizeof(buf))); }
    else if (addr_.sin_family == AF_INET6) { ::inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, static_cast<socklen_t>(sizeof(buf))); }
    return buf;
}

}
