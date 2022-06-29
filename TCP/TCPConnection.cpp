#include "TCPConnection.h"

#include "Basics/StringView.h"
#include "Thread/EventLoop.h"
#include "Thread/Channel.h"
#include "Log/LogLine.h"
#include <assert.h>
#include <iostream>

namespace Rootive
{
void TCPConnection::_send(const void* message, int size)
{
    if (state_ == EState::Disconnected) 
    { 
        ROOTIVE_LOG_WARN << name_ << " id: " << socket_->getFD() << " disconnected";
        return; 
    }
    ssize_t written = 0;
    bool fatalError = false;
    if (!channel_->bWrite() && !outputBuffer_.readableLength())
    {
        written = ::write(channel_->getFD(), message, size);
        if (written >= 0)
        {
            size -= written;
            if (!size && writeFinishedCallback_)
            {
                eventLoop_->queue(std::bind(writeFinishedCallback_, shared_from_this()));
            }
        }
        else
        {
            written = 0;
            if (errno != EWOULDBLOCK)
            {
                ROOTIVE_LOG_SYSTEMERROR;
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    fatalError = true;
                }
            }
        }      
    }
    if (!fatalError && size > 0)
    {
        size_t oldSize = outputBuffer_.readableLength();
        if (oldSize + size >= hwm_ && oldSize < hwm_ && hwmCallback_)
        {
            eventLoop_->queue(std::bind(hwmCallback_, shared_from_this(), oldSize + size));
        }
        outputBuffer_.write(static_cast<const char*>(message) + written, size);
        if (!channel_->bWrite()) { channel_->enableWrite(); }
    }
}
void TCPConnection::_send(const StringView& message)
{
    _send(message.begin(), message.getLength());
}
void TCPConnection::_forceClose() { handleClose(); }
void TCPConnection::_shutdown() 
{ 
    if (!channel_->bWrite())
    {
        socket_->shutdown(SHUT_WR);
    }
}
std::string TCPConnection::getTCPInfoString() const
{
    char buffer[1024]; buffer[0] = '\0';
    socket_->getTCPInfoString(buffer, sizeof(buffer));
    return buffer;
}
void TCPConnection::handleRead(std::chrono::system_clock::time_point time)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.write(channel_->getFD(), 65536, &savedErrno);
    if (n > 0) { messageCallback_(shared_from_this(), time); }
    else if (n == 0) 
    { 
        ROOTIVE_LOG_TRACE << getName() << " read 0";
        handleClose(); 
    }
    else 
    { 
        errno = savedErrno;
        ROOTIVE_LOG_SYSTEMERROR;
        handleError(); 
    }
}
void TCPConnection::handleWrite()
{
    if (channel_->bWrite()) 
    { 
        ssize_t n = ::write(channel_->getFD(), outputBuffer_.readBegin(), outputBuffer_.readableLength());
        if (n > 0)
        {
            outputBuffer_.hasRead(n);
            if (!outputBuffer_.readableLength())
            {
                channel_->disableWrite();
                if (writeFinishedCallback_) 
                { 
                    eventLoop_->queue(std::bind(writeFinishedCallback_, shared_from_this())); 
                }
                if (state_ == Disconnecting) { _shutdown(); }
            }
        }
        else 
        {
            ROOTIVE_LOG_SYSTEMERROR;
        }
    }
    else 
    {
        ROOTIVE_LOG_TRACE << name_ << " down";
    }
}
void TCPConnection::handleClose() //BUG
{
    ROOTIVE_LOG_TRACE << name_ << " state: " << state_;
    assert(state_ == Connected || state_ == Disconnecting);
    state_ = Disconnected;
    channel_->disableAll();
    std::shared_ptr<TCPConnection> guard(shared_from_this());
    connectionCallback_(guard);
    closeCallback_(guard);
}
void TCPConnection::handleError()
{
    auto res = socket_->getError();
    ROOTIVE_LOG_ERROR << name_ << " fd: " << socket_->getFD() << " SO_ERROR: " << res << " " << LogLine::errorString(res);
}
TCPConnection::TCPConnection(EventLoop* eventLoop, int fd, const InetAddress &localAddr, const InetAddress &peerAddr, const std::string& name) :
eventLoop_(eventLoop), name_(name), state_(EState::Connecting), 
socket_(new Socket(fd)), channel_(new Channel(eventLoop, fd)),
localAddr_(localAddr), peerAddr_(peerAddr), hwm_(64 * 1024 * 1024), 
context_(nullptr)
{
    channel_->setReadCallback(std::bind(&TCPConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TCPConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TCPConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TCPConnection::handleError, this));
    socket_->setKeepAlive(true);
    ROOTIVE_LOG_INFO << name_ << " fd: " << fd;
}
TCPConnection::~TCPConnection()
{
    ROOTIVE_LOG_DEBUG << name_ << " state: " << state_;
    assert(state_ == Disconnected);
}
void TCPConnection::enableRead() 
{ 
    eventLoop_->run([this]()->void{ if (!channel_->bRead()) { channel_->enableRead(); } }); 
}
void TCPConnection::disableRead()
{ 
    eventLoop_->run([this]()->void{ if (channel_->bRead()) { channel_->disableRead(); } }); 
}
void TCPConnection::forceClose()
{
    if (state_ == Connected || state_ == Disconnecting)
    {
        state_ = Disconnecting;
        eventLoop_->queue(std::bind(&TCPConnection::_forceClose, shared_from_this()));
    }
}
void TCPConnection::shutdown()
{
    if (state_ = Connected)
    {
        state_ = Disconnecting;
        eventLoop_->run(std::bind(&TCPConnection::_shutdown, this));
    }
}
void TCPConnection::run()
{
    state_ = Connected;
    channel_->tie(shared_from_this());
    channel_->enableRead();
    connectionCallback_(shared_from_this());
}
void TCPConnection::close()
{
    if (state_ == Connected)
    {
        state_ = Disconnected;
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}
void TCPConnection::send(const void* message, int size) 
{
    if (state_ == Connected)
    {
        if (eventLoop_->bThread()) { _send(message, size); }
        else 
        { 
            eventLoop_->run(std::bind(static_cast<void(TCPConnection::*)(const void*, int)>(&TCPConnection::_send), this, message, size)); 
        }
    }
}
void TCPConnection::send(const StringView& message) { send(message.begin(), message.getLength()); }
void TCPConnection::send(IOBuffer &message)
{
    if (state_ == Connected)
    {
        if (eventLoop_->bThread())
        {
            _send(message.readBegin(), message.readableLength());
            message.hasReadAll();
        }
        else 
        { 
            eventLoop_->run(std::bind(static_cast<void(TCPConnection::*)(const StringView&)>(&TCPConnection::_send), this, message.readAllAsString())); 
        }
    }
}

}