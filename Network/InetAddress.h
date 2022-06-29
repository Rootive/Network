#ifndef ROOTIVE_INETADDRESS_H
#define ROOTIVE_INETADDRESS_H

#include <netinet/in.h>
#include <string>
#include <type_traits>

#include "Basics/Types.h"

namespace Rootive
{

class InetAddress
{
private:
    union { sockaddr_in addr_; sockaddr_in6 addr6_; };
public:
    template<typename To, typename From> 
    inline static To *sockaddrPtrCast(From *addr) 
    { return static_cast<To *>(Types::implicitCast<typename std::conditional< std::is_const<From>::value, const void *, void *>::type >(addr)); }

    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);
    InetAddress(const char *ip, uint16_t port, bool ipv6 = false);
    explicit InetAddress(const sockaddr_in &addr) : addr_(addr) {}
    explicit InetAddress(const sockaddr_in6 &addr) : addr6_(addr) {}

    static bool resolve(const char *hostname, InetAddress *result);

    inline sa_family_t getFamily() const { return addr_.sin_family; }
    inline const sockaddr *getSockaddr() const { return sockaddrPtrCast<const sockaddr>(&addr_); }
    inline uint16_t getPort_NetEndian() const { return addr_.sin_port; }
    inline uint16_t getPort_HostEndian() const { return be16toh(addr_.sin_port); }
    inline uint32_t getIP_NetEndian() const { return addr_.sin_addr.s_addr; }

    std::string toIPString() const;
    std::string toIPPortString() const;

    inline void setSockaddrIn6(const sockaddr_in6 &addr6) { addr6_ = addr6; }
    inline void setScopeID(uint32_t scopeID) { if (addr_.sin_family == AF_INET6) { addr6_.sin6_scope_id = scopeID; } }
};

}

#endif 
