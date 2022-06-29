#ifndef ROOTIVE_EVENTLOOPTHREADPOOL_H
#define ROOTIVE_EVENTLOOPTHREADPOOL_H

#include "Basics/Noncopyable.h"

#include <string>
#include <vector>

#include "Thread/EventLoopThread.h"

namespace Rootive
{
class EventLoopThreadPool : Noncopyable
{
    EventLoop *eventLoop_;
    int numThread_;
    std::string name_;
    std::vector<std::unique_ptr<EventLoopThread> > threadVector_;
    std::vector<EventLoop *> eventLoopVector_;
    EventLoopThread::ThreadInitFunc threadInitFunc_;
    int next_;
public:
    EventLoopThreadPool(EventLoop *eventLoop, int numThread, const std::string &name);
    
    inline const std::string &getName() const { return name_; }

    void setThreadInitFunc(EventLoopThread::ThreadInitFunc threadInitFunc) { threadInitFunc_ = threadInitFunc; }
    void run();

    EventLoop *getEventLoop();
    EventLoop *getEventLoop(int hashcode);
    std::vector<EventLoop *> getEventLoopVector();
};
}

#endif