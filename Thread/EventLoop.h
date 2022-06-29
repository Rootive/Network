#ifndef ROOTIVE_EVENTLOOP_H
#define ROOTIVE_EVENTLOOP_H

#include <atomic>
#include <memory>
#include <functional>
#include <vector>
#include <mutex>
#include <unistd.h>
#include <signal.h>

#include "Basics/Noncopyable.h"
#include "Thread/Poller.h"
#include "Thread/Channel.h"
#include "Thread/ThisThread.h"

namespace Rootive
{

class EventLoop : Noncopyable
{
    class IgnoreSIGPIPE
    {
    public:
        IgnoreSIGPIPE() 
        { ::signal(SIGPIPE, SIG_IGN); }
    };
public:
    typedef std::function<void()> Func;
private:
    static const IgnoreSIGPIPE ignore_;
    static constexpr int constexpr_pollTimeout = 10000;
    
    const __pid_t tid_;
    std::atomic<bool> bRunning_;
    std::atomic<bool> bquit_;
    std::unique_ptr<Poller> poller_;
    Channel wakeupChannel_;
    std::vector<Func> funcVector_;
    std::mutex funcVectorMutex_;
    bool bFuncCalling;

    void wakeup();
    void availableLogTrace(const std::vector<Channel *> &available);
public:
    EventLoop();
    ~EventLoop();

    inline const __pid_t &getTID() { return tid_; }
    inline bool bThread() { return ThisThread::tid_ == tid_; }
    
    inline void updateChannel(Channel *ch) { poller_->updateChannel(ch); }
    inline void removeChannel(Channel *ch) { poller_->removeChannel(ch); }

    void run();
    void run(Func func);
    void queue(Func func);
    inline void quit() { bquit_ = true; wakeup(); }
};

}

#endif