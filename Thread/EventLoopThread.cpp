#include "EventLoopThread.h"
#include <thread>
#include "Thread/EventLoop.h"

namespace Rootive
{

void EventLoopThread::threadFunc()
{
    EventLoop eventLoop;
    if (threadInitFunc_)
    {
        threadInitFunc_(&eventLoop);
    }
    {
        std::lock_guard<std::mutex> lock(eventLoopMutex_);
        eventLoop_ = &eventLoop;
    }
    eventLoopCond_.notify_one();
    eventLoop.run();
    std::lock_guard<std::mutex> lock(eventLoopMutex_);
    eventLoop_ = nullptr;
}
EventLoopThread::EventLoopThread(const std::string &name) : 
eventLoop_(nullptr), thread_(nullptr), name_(name)
{}
EventLoopThread::~EventLoopThread()
{
    if (eventLoop_)
    {
        eventLoop_->quit();
    }
    if (thread_)
    {
        thread_->join();
        delete thread_;
    }
}
void EventLoopThread::run()
{
    thread_ = new std::thread(&EventLoopThread::threadFunc, this);
    std::unique_lock<std::mutex> lock(eventLoopMutex_);
    while (!eventLoop_) { eventLoopCond_.wait(lock); }
}

}