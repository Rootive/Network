#ifndef ROOTIVE_THISTHREAD_H
#define ROOTIVE_THISTHREAD_H

#include <unistd.h>

namespace Rootive
{
namespace ThisThread
{
    const __pid_t pid_ = getpid();
    thread_local const __pid_t tid_ = gettid();;
}
}

#endif