#include "Poller.h"

#include "Thread/Channel.h"
#include "Log/LogLine.h"

namespace Rootive
{

std::chrono::system_clock::time_point Poller::poll(int timeout)
{
    availableVector_.clear();
    int numActive = ::poll(pollFDVector_.data(), pollFDVector_.size(), timeout);
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    if (numActive > 0)
    {
        ROOTIVE_LOG_TRACE << numActive << " events";
        for (std::vector<pollfd>::const_iterator pfd = pollFDVector_.cbegin(); pfd != pollFDVector_.cend() && numActive > 0; ++pfd)
        {
            if (pfd->revents > 0)
            {
                --numActive;
                Channel *ch = channelMap_.find(pfd->fd)->second;
                ch->setRevents(pfd->revents);
                availableVector_.push_back(ch);
            }
        }
    }
    else if (!numActive) 
    {
        ROOTIVE_LOG_TRACE << "nothing";
    }
    else if (errno != EINTR)
    {
        ROOTIVE_LOG_SYSTEMERROR;
    }
    return now;
}
void Poller::updateChannel(Channel *ch)
{
    ROOTIVE_LOG_TRACE << "fd: " << ch->getFD() << " events: " << ch->getEvents();
    if (ch->index_ < 0)
    {
        ch->index_ = pollFDVector_.size();
        pollfd pfd = { ch->getFD(), static_cast<short int>(ch->getEvents()), 0 };
        pollFDVector_.push_back(pfd);
        channelMap_[pfd.fd] = ch;
    }
    else
    {
        pollfd &pfd = pollFDVector_[ch->index_];
        pfd.fd = ch->getFD();
        pfd.events = ch->getEvents();
        pfd.revents = 0;
        if (ch->bNoneEvent()) { pfd.fd = -pfd.fd - 1; }
    }
}
void Poller::removeChannel(Channel *ch)
{
    ROOTIVE_LOG_TRACE << "To remove fd: " << ch->getFD();
    channelMap_.erase(ch->getFD());
    ROOTIVE_LOG_TRACE << "pollFDVector_.size: " << pollFDVector_.size();
    if (ch->index_ != pollFDVector_.size() - 1)
    {
        int endFD = pollFDVector_.back().fd;
        iter_swap(pollFDVector_.begin() + ch->index_, pollFDVector_.end() - 1);
        if (endFD < 0)
        {   
            endFD = -endFD - 1;
        }
        ROOTIVE_LOG_TRACE << "swap fd: " << endFD;
        channelMap_[endFD]->index_ = ch->index_;
    }
    pollFDVector_.pop_back();
}

}