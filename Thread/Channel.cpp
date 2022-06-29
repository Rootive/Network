#include "Channel.h"
#include "Thread/EventLoop.h"
#include "Log/LogLine.h"

namespace Rootive
{
void Channel::update() { eventLoop_->updateChannel(this); }
void Channel::_handleEvent(std::chrono::system_clock::time_point time) const
{
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        ROOTIVE_LOG_WARN << "fd: " << fd_ << " POLLHUP";
        if (closeCallback_) closeCallback_();
    }
    if (revents_ & POLLNVAL)
    {
        ROOTIVE_LOG_WARN << "fd: " << fd_ << " POLLNVAL";
    }
    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (errorCallback_) { errorCallback_(); }
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (readCallback_) { readCallback_(time); }
    }
    if (revents_ & POLLOUT)
    {
        if (writeCallback_) { writeCallback_(); }
    }
}
void Channel::enableRead() { events_ |= constexpr_readEvent; update(); }
void Channel::enableWrite() { events_ |= constexpr_writeEvent; update(); }
void Channel::disableRead() { events_ &= ~constexpr_readEvent; update(); }
void Channel::disableWrite() { events_ &= ~constexpr_writeEvent; update(); }
void Channel::disableAll() { events_ = constexpr_noneEvent; update(); }
Channel::Channel(EventLoop *eventLoop, int fd) : 
eventLoop_(eventLoop), fd_(fd), events_(0), revents_(0), bTied_(false), index_(-1) {}
void Channel::handleEvent(std::chrono::system_clock::time_point time) const
{
    if (bTied_)
    {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard) { _handleEvent(time); }
    }
    else { _handleEvent(time); }
}
void Channel::remove(){ eventLoop_->removeChannel(this); }
void Channel::tie(const std::shared_ptr<void> &obj) { tie_ = obj; bTied_ = true; }

}