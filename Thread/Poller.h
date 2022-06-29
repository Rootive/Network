#ifndef ROOTIVE_POLLER_H
#define ROOTIVE_POLLER_H

#include <poll.h>
#include <vector>
#include <map>
#include <chrono>

#include "Basics/Noncopyable.h"

namespace Rootive
{

class Channel;
class EventLoop;

class Poller : Noncopyable
{
    std::vector<pollfd> pollFDVector_;
    std::map<int, Channel *> channelMap_;
    std::vector<Channel *> availableVector_;
public:
    std::chrono::system_clock::time_point poll(int timeout);
    void updateChannel(Channel *ch);
    void removeChannel(Channel *ch);
    const std::vector<Channel *> &getAvailableVector() const { return availableVector_; }
};

}

#endif