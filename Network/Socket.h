#ifndef ROOTIVE_SOCKET_H
#define ROOTIVE_SOCKET_H

#include <unistd.h>
#include <netinet/tcp.h>


#include "Basics/Noncopyable.h"

namespace Rootive
{

class InetAddress;

class Socket : Noncopyable
{
    const int fd_;
public:
    explicit Socket(int fd) : fd_(fd) {}
    ~Socket() { ::close(fd_); }

    int getFD() { return fd_; }
    void setTCPNodelay(bool bOn);
    void setReuseAddr(bool bOn);
    void setReusePort(bool bOn);
    void setKeepAlive(bool bOn);

    bool getTCPInfo(tcp_info *out) const;
    bool getTCPInfoString(char *buffer, int size) const;
    int getError();

    void bind(const InetAddress &addr);
    void listen();
    int accept(InetAddress *out_addr);
    void shutdown(int how);

};

}

#endif