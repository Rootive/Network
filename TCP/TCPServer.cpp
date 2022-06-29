#include "TCPServer.h"

#include <cstring>

#include "Network/InetAddress.h"
#include "Thread/EventLoop.h"
#include "Thread/Acceptor.h"
#include "Thread/EventLoopThreadPool.h"
#include "Log/LogLine.h"

#include <iostream>

namespace Rootive
{
void TCPServer::newConnection(int fd, const InetAddress &peerAddr)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Connection%d", nextConnID_++);
    std::string connName = name_ + buffer;
    ROOTIVE_LOG_INFO << connName << ": " << peerAddr.toIPPortString();
    sockaddr_in6 _localAddr;
    memset(&_localAddr, 0, sizeof(_localAddr));
    socklen_t addrSize = static_cast<socklen_t>(sizeof(_localAddr));
    ::getsockname(fd, InetAddress::sockaddrPtrCast<sockaddr>(&_localAddr), &addrSize);
    EventLoop *eventLoop = threadPool_->getEventLoop();
    std::shared_ptr<TCPConnection> conn(new TCPConnection(eventLoop, fd, InetAddress(_localAddr), peerAddr, connName));
    connectionMap_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeFinishedCallback_);
    conn->setCloseCallback(std::bind(&TCPServer::removeConnection, this, std::placeholders::_1));
    eventLoop->run(std::bind(&TCPConnection::run, conn));
}
void TCPServer::removeConnection(const std::shared_ptr<TCPConnection> &conn)
{
    eventLoop_->run(std::bind(&TCPServer::_removeConnection, this, conn));
}
void TCPServer::_removeConnection(const std::shared_ptr<TCPConnection> &conn)
{
    ROOTIVE_LOG_INFO << conn->getName();
    connectionMap_.erase(conn->getName());
    conn->getEventLoop()->queue(std::bind(&TCPConnection::close, conn));
}
TCPServer::TCPServer(EventLoop *eventLoop, const InetAddress &listenAddr, const std::string &name, int numThread, EOption option) :
eventLoop_(eventLoop), ipPort(listenAddr.toIPPortString()), name_(name), nextConnID_(1), 
acceptor_(new Acceptor(eventLoop, listenAddr, option)), 
threadPool_(new EventLoopThreadPool(eventLoop, numThread, name))
{
    acceptor_->setNewConnectionCallback(std::bind(&TCPServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}
TCPServer::~TCPServer()
{
    ROOTIVE_LOG_TRACE << name_;
    for (std::pair<const std::string, std::shared_ptr<TCPConnection> > &item : connectionMap_)
    {
        std::shared_ptr<TCPConnection> conn(item.second);
        item.second.reset();
        conn->getEventLoop()->run(std::bind(&TCPConnection::close, conn));
    }
}
void TCPServer::run()
{
    threadPool_->setThreadInitFunc(threadInitFunc_);
    threadPool_->run();
    eventLoop_->run(std::bind(&Acceptor::listen, acceptor_.get()));
}

}
