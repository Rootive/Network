#include "EventLoop.h"

#include <sys/eventfd.h>
#include "Log/LogLine.h"
#include <assert.h>

namespace Rootive
{
void EventLoop::wakeup() 
{ 
    uint64_t buffer; 
    auto res = ::write(wakeupChannel_.getFD(), &buffer, sizeof(buffer));
    if (res != sizeof(buffer))
    {
        ROOTIVE_LOG_ERROR << "write " << res << " instead of " << sizeof(buffer);
    } 
}
void EventLoop::availableLogTrace(const std::vector<Channel *> &available)
{
    if constexpr (ROOTIVE_LOG_LEVEL <= LogLine::Level::Trace)
    {
        LogLine stream(LogLine::Level::Trace, __PRETTY_FUNCTION__);
        stream << "[ ";
        int _i = 0;
        int size = available.size();
        while (_i < size)
        {
            stream << "{ " << available[_i]->getFD() << ": " << available[_i]->getRevents() << " }";
            ++_i;
            if (_i < size) { stream << ", "; }
        }
        stream << " ]";
    }
}
EventLoop::EventLoop() : 
bRunning_(false), tid_(ThisThread::tid_), poller_(new Poller), 
wakeupChannel_(this, ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)), bFuncCalling(false)
{
    if (wakeupChannel_.getFD() < 0)
    {
        ROOTIVE_LOG_SYSTEMERROR;
        throw;
    }
    ROOTIVE_LOG_DEBUG << this << " in thread: " << ThisThread::tid_;
    wakeupChannel_.setReadCallback(
    [&](std::chrono::system_clock::time_point time)-> void 
    { 
        uint64_t buffer; 
        auto res = ::read(wakeupChannel_.getFD(), &buffer, sizeof(buffer)); 
        if (res != sizeof(buffer))
        {
            ROOTIVE_LOG_ERROR << "read " << res << " instead of " << sizeof(buffer);
        }
    });
    wakeupChannel_.enableRead();
}
EventLoop::~EventLoop()
{
    ROOTIVE_LOG_DEBUG << this << " of thread: " << tid_ << " in thread: " << ThisThread::tid_;
    wakeupChannel_.disableAll();
    wakeupChannel_.remove();
    ::close(wakeupChannel_.getFD());
}
void EventLoop::run()
{
    assert(bThread());
    bRunning_ = true;
    bquit_ = false;
    ROOTIVE_LOG_TRACE << this << " start";
    while (!bquit_)
    {
        std::chrono::system_clock::time_point time = poller_->poll(constexpr_pollTimeout);
        const auto &available = poller_->getAvailableVector();
        availableLogTrace(available);
        for (Channel *ch : available)
        {
            ch->handleEvent(time);
        }
        std::vector<Func> funcVector;
        {
            std::lock_guard<std::mutex> lock(funcVectorMutex_);
            funcVector.swap(funcVector_);
        }
        bFuncCalling = true;
        for (const Func &func : funcVector) { func(); }
        bFuncCalling = false;
    }
    ROOTIVE_LOG_TRACE << this << " stop";
    bRunning_ = false;
}
void EventLoop::run(Func func)
{
    if (bThread()) { func(); }
    else { queue(func); }
}
void EventLoop::queue(Func func)
{
    {
        std::lock_guard<std::mutex> lock(funcVectorMutex_);
        funcVector_.push_back(func);
    }
    if (!bThread() || bFuncCalling) { wakeup(); }
}

}