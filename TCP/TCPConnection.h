#ifndef ROOTIVE_TCPCONNECTION_H
#define ROOTIVE_TCPCONNECTION_H

#include <memory>
#include <string>
#include <atomic>
#include <functional>
#include <chrono>
#include <string>

#include "Basics/Noncopyable.h"
#include "Network/InetAddress.h"
#include "Network/Socket.h"
#include "Network/NetworkBuffer.h"


namespace Rootive
{

class EventLoop;
class Channel;
class StringView;

class TCPConnection : Noncopyable, public std::enable_shared_from_this<TCPConnection>
{
    enum EState { Disconnected, Connecting, Connected, Disconnecting };
public:
    typedef std::function<void(const std::shared_ptr<TCPConnection>&)> ConnectionCallback;
    typedef std::function<void(const std::shared_ptr<TCPConnection>&, std::chrono::system_clock::time_point)> MessageCallback;
    typedef std::function<void(const std::shared_ptr<TCPConnection>&)> WriteFinishedCallback;
    typedef std::function<void(const std::shared_ptr<TCPConnection>&, size_t)> HWMCallback;
    typedef std::function<void(const std::shared_ptr<TCPConnection>&)> CloseCallback;
private:
    EventLoop *eventLoop_;
    std::string name_;
    std::atomic<EState> state_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    InetAddress localAddr_;
    InetAddress peerAddr_;

    NetworkBuffer inputBuffer_;
    NetworkBuffer outputBuffer_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteFinishedCallback writeFinishedCallback_;
    HWMCallback hwmCallback_;
    CloseCallback closeCallback_;
    size_t hwm_;

    void _send(const void* message, int size);
    void _send(const StringView& message);
    void _forceClose();
    void _shutdown();
    void handleRead(std::chrono::system_clock::time_point time);
    void handleWrite();
    void handleClose();
    void handleError();

public:
    void *context_;
    TCPConnection(EventLoop* eventLoop, int fd, const InetAddress &localAddr, const InetAddress &peerAddr, const std::string& name);
    ~TCPConnection();

    inline EventLoop* getEventLoop() const { return eventLoop_; }
    inline const std::string& getName() const { return name_; }
    inline const InetAddress& getLocalAddr() const { return localAddr_; }
    inline const InetAddress& getPeerAddr() const { return peerAddr_; }
    inline bool bConnected() const { return state_ == EState::Connected; }
    inline bool bDisconnected() const { return state_ == EState::Disconnected; }
    inline NetworkBuffer* inputBuffer() { return &inputBuffer_; }
    inline NetworkBuffer* outputBuffer() { return &outputBuffer_; }

    inline void setConnectionCallback(const ConnectionCallback& callback) { connectionCallback_ = callback; }
    inline void setMessageCallback(const MessageCallback& callback) { messageCallback_ = callback; }
    inline void setWriteCompleteCallback(const WriteFinishedCallback& callback) { writeFinishedCallback_ = callback; }
    inline void setHighWaterMarkCallback(const HWMCallback& callback, size_t hwm) { hwmCallback_ = callback; hwm_ = hwm; }
    inline void setCloseCallback(const CloseCallback& callback) { closeCallback_ = callback; }

    bool getTCPInfo(struct tcp_info *out) const { return socket_->getTCPInfo(out); }
    std::string getTCPInfoString() const;
    void setTCPNodelay(bool bOn) { socket_->setTCPNodelay(bOn); }

    void enableRead();
    void disableRead();
    void forceClose();
    void shutdown();
    void run();
    void close();

    void send(const void *message, int size);
    void send(const StringView &message);
    void send(IOBuffer &message);
    
};
}

#endif