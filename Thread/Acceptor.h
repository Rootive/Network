#ifndef ROOTIVE_ACCEPTOR_H
#define ROOTIVE_ACCEPTOR_H

#include <functional>

#include "Basics/Noncopyable.h"
#include "Network/Socket.h"
#include "Thread/Channel.h"

namespace Rootive
{

class EventLoop;
class InetAddress;

class Acceptor : Noncopyable
{
public:
    typedef std::function<void(int, const InetAddress&)> NewConnectionCallback;
private:
    Socket socket_;
    Channel channel_;
    NewConnectionCallback newConnectionCallback_;
    int idleFD_;

    void handleRead();
public: 
    Acceptor(EventLoop *eventLoop, const InetAddress &listenAddr, bool reusePort);
    ~Acceptor();

    inline void setNewConnectionCallback(NewConnectionCallback callback) { newConnectionCallback_ = callback; }
    void listen();
};

}

#endif