#include "Acceptor.h"

#include <sys/socket.h>
#include <fcntl.h>

#include "Network/InetAddress.h"
#include "Log/LogLine.h"

namespace Rootive
{

void Acceptor::handleRead()
{
    InetAddress addr;
    int fd = socket_.accept(&addr);
    if (fd >= 0)
    {
        ROOTIVE_LOG_TRACE << "accpet " << addr.toIPPortString();
        if (newConnectionCallback_) { newConnectionCallback_(fd, addr); }
        else { ::close(fd); }
    }
    else
    {
        ROOTIVE_LOG_SYSTEMERROR;
        if (errno == EMFILE)
        {
            ::close(idleFD_);
            idleFD_ = ::accept(socket_.getFD(), nullptr, nullptr);
            ::close(idleFD_);
            idleFD_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
Acceptor::Acceptor(EventLoop *eventLoop, const InetAddress &listenAddr, bool reusePort) : 
socket_(::socket(listenAddr.getFamily(), SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)), 
channel_(eventLoop, socket_.getFD()), 
idleFD_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    socket_.setReuseAddr(true);
    socket_.setReusePort(reusePort);
    socket_.bind(listenAddr);
    channel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}
Acceptor::~Acceptor()
{
    channel_.remove();
    ::close(idleFD_);
}
void Acceptor::listen()
{
    socket_.listen();
    channel_.enableRead();
}

}