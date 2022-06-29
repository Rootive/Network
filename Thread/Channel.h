#ifndef ROOTIVE_CHANNEL_H
#define ROOTIVE_CHANNEL_H

#include <functional>
#include <memory>
#include <cstdio>
#include <chrono>

#include "Thread/Poller.h"
#include "Basics/Noncopyable.h"

namespace Rootive
{

class EventLoop;

class Channel : Noncopyable
{
public:
    typedef std::function<void()> EventCallback;
    typedef std::function<void(std::chrono::system_clock::time_point)> ReadCallback;
private:
    static constexpr int constexpr_noneEvent = 0;
    static constexpr int constexpr_readEvent = POLLIN | POLLPRI;
    static constexpr int constexpr_writeEvent = POLLOUT;

    EventLoop *eventLoop_;
    const int fd_;
    int events_;
    int revents_;

    ReadCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

    std::weak_ptr<void> tie_;
    bool bTied_;

    void update();
    void _handleEvent(std::chrono::system_clock::time_point time) const;
public:
    int index_;

    Channel(EventLoop *eventLoop, int fd);

    inline int getFD() const { return fd_; }
    inline int getEvents() const { return events_; }
    inline int getRevents() const { return revents_; }
    inline bool bNoneEvent() const { return events_ == constexpr_noneEvent; }
    inline bool bWrite() const { return events_ & constexpr_writeEvent; }
    inline bool bRead() const { return events_ & constexpr_readEvent; }
    void enableRead();
    void enableWrite();
    void disableRead();
    void disableWrite();
    void disableAll();
    inline void setReadCallback(ReadCallback callback) { readCallback_ = std::move(callback); }
    inline void setWriteCallback(EventCallback callback) { writeCallback_ = std::move(callback); }
    inline void setCloseCallback(EventCallback callback) { closeCallback_ = std::move(callback); }
    inline void setErrorCallback(EventCallback callback) { errorCallback_ = std::move(callback); }
    inline void setRevents(int revents) { revents_ = revents; }
    void handleEvent(std::chrono::system_clock::time_point time) const;
    void remove();
    void tie(const std::shared_ptr<void> &obj);

};
}

#endif