#ifndef ROOTIVE_TCPSERVER_H
#define ROOTIVE_TCPSERVER_H

#include <memory>
#include <string>
#include <map>
#include <atomic>

#include "TCP/TCPConnection.h"
#include "Thread/EventLoopThread.h"

namespace Rootive
{

class InetAddress;
class EventLoop;
class Acceptor;
class EventLoopThreadPool;

class TCPServer
{
public:
enum EOption
{
    NonReusePort,
    ReusePort
};
private:
    EventLoop *eventLoop_;
    const std::string ipPort;
    const std::string name_;
    int nextConnID_;
    std::unique_ptr<Acceptor> acceptor_;
    std::map<std::string, std::shared_ptr<TCPConnection> > connectionMap_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    TCPConnection::ConnectionCallback connectionCallback_;
    TCPConnection::MessageCallback messageCallback_;
    TCPConnection::WriteFinishedCallback writeFinishedCallback_;
    EventLoopThread::ThreadInitFunc threadInitFunc_;

    void newConnection(int fd, const InetAddress &peerAddr);
    void removeConnection(const std::shared_ptr<TCPConnection> &conn);
    void _removeConnection(const std::shared_ptr<TCPConnection> &conn);
public:
    TCPServer(EventLoop *eventLoop, const InetAddress &listenAddr, const std::string &name, int numThread = 0, EOption option = NonReusePort);
    ~TCPServer();

    inline const std::string &getName() const { return name_; }
    std::shared_ptr<EventLoopThreadPool> getEventLoopThreadPool() { return threadPool_; }
    void setConnectionCallback(const TCPConnection::ConnectionCallback &callback) { connectionCallback_ = callback; }
    void setMessageCallback(const TCPConnection::MessageCallback &callback) { messageCallback_ = callback; }
    void setWriteFinishedCallback(const TCPConnection::WriteFinishedCallback &callback) { writeFinishedCallback_ = callback; }
    void setThreadInitFunc(const EventLoopThread::ThreadInitFunc &callback) { threadInitFunc_ = callback; }

    void run();
};
}

#endif