#include "EventLoopThreadPool.h"

namespace Rootive
{
EventLoopThreadPool::EventLoopThreadPool(EventLoop *eventLoop, int numThread, const std::string &name) :
eventLoop_(eventLoop), numThread_(numThread), name_(name), next_(0)
{}
    
void EventLoopThreadPool::run()
{
    for (int _i = 0; _i < numThread_; ++_i)
    {
        const int bufferLength = name_.size() + 8;
        char *buffer = new char[bufferLength];
        snprintf(buffer, bufferLength * sizeof(char), "%sThread%d", name_.c_str(), _i);
        threadVector_.push_back(std::unique_ptr<EventLoopThread>(new EventLoopThread(buffer)));
        std::unique_ptr<EventLoopThread> &_thread(threadVector_.back());
        _thread->setThreadInitFunc(threadInitFunc_);
        _thread->run();
        eventLoopVector_.push_back(_thread->getEventLoop());
    }
    if (!numThread_ && threadInitFunc_)
    {
        threadInitFunc_(eventLoop_);
    }
}
EventLoop *EventLoopThreadPool::getEventLoop()
{
    EventLoop *ret = eventLoop_;
    if (!eventLoopVector_.empty())
    {
        ret = eventLoopVector_[next_++];
        next_ %= eventLoopVector_.size();
    }
    return ret;
}
EventLoop *EventLoopThreadPool::getEventLoop(int hashcode)
{
    EventLoop *ret = eventLoop_;
    if (!eventLoopVector_.empty())
    {
        ret = eventLoopVector_[hashcode % eventLoopVector_.size()];
    }
    return ret;
}
std::vector<EventLoop *> EventLoopThreadPool::getEventLoopVector()
{
    if (eventLoopVector_.empty())
    {
        return std::vector<EventLoop *>(1, eventLoop_);
    }
    else
    {
        return eventLoopVector_;
    }
}
}