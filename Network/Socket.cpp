#include "Socket.h"

#include <sys/socket.h>
#include <cstring>

#include "Network/InetAddress.h"

namespace Rootive
{

void Socket::setTCPNodelay(bool bOn)
{
    int optval = bOn ? 1 : 0;
    ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval)));
}
void Socket::setReuseAddr(bool bOn)
{
    int optval = bOn ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval)));
}
void Socket::setReusePort(bool bOn)
{
    int optval = bOn ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval)));
}
void Socket::setKeepAlive(bool bOn)
{
    int optval = bOn ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval)));
}
bool Socket::getTCPInfo(tcp_info *out) const
{
    socklen_t size = sizeof(*out);
    memset(out, 0, size);
    return ::getsockopt(fd_, SOL_TCP, TCP_INFO, out, &size) == 0;
}
bool Socket::getTCPInfoString(char *buffer, int size) const
{
    tcp_info info;
    bool ret = getTCPInfo(&info);
    if (ret)
    {
        snprintf(buffer, size, "unrecovered=%u rto=%u ato=%u snd_mss=%u rcv_mss=%u "
        "lost=%u retrans=%u rtt=%u rttvar=%u sshthresh=%u cwnd=%u total_retrans=%u",
        info.tcpi_retransmits, info.tcpi_rto, info.tcpi_ato, info.tcpi_snd_mss,
        info.tcpi_rcv_mss, info.tcpi_lost, info.tcpi_retrans, info.tcpi_rtt,
        info.tcpi_rttvar, info.tcpi_snd_ssthresh, info.tcpi_snd_cwnd, info.tcpi_total_retrans);
    }
    return ret;
}
int Socket::getError()
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if (::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) { return errno; }
    else { return optval; }
}
void Socket::bind(const InetAddress &addr)
{
    ::bind(fd_, addr.getSockaddr(), static_cast<socklen_t>(sizeof(sockaddr_in6)));
}
void Socket::listen()
{
    if (::listen(fd_, SOMAXCONN)) { throw; }
}
int Socket::accept(InetAddress *out_addr)
{
    sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrSize = static_cast<socklen_t>(sizeof(addr));
    int fd = ::accept4(fd_, InetAddress::sockaddrPtrCast<sockaddr>(&addr), &addrSize, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (fd >= 0) { out_addr->setSockaddrIn6(addr); }
    return fd;
}
void Socket::shutdown(int how) { ::shutdown(fd_, how); }


}