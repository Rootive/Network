#ifndef ROOTIVE_EVENTLOOPTHREAD_H
#define ROOTIVE_EVENTLOOPTHREAD_H

#include <mutex>
#include <functional>
#include <condition_variable>
#include <string>

namespace std
{
class thread;
}

namespace Rootive
{
class EventLoop;
class EventLoopThread
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitFunc;
private:
    EventLoop *eventLoop_;
    std::mutex eventLoopMutex_;
    std::condition_variable eventLoopCond_;
    ThreadInitFunc threadInitFunc_;
    std::thread *thread_;
    std::string name_;

    void threadFunc();
public:
    EventLoopThread(const std::string &name);
    ~EventLoopThread();

    inline EventLoop *getEventLoop() { return eventLoop_; }
    inline void setThreadInitFunc(ThreadInitFunc threadInitFunc) { threadInitFunc_ = threadInitFunc; }

    void run();
};

}

#endif