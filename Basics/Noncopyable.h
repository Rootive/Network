#ifndef ROOTIVE_NONCOPYABLE_H
#define ROOTIVE_NONCOPYABLE_H

namespace Rootive
{

class Noncopyable
{
public:
    Noncopyable() = default;
    ~Noncopyable() = default;
    Noncopyable(const Noncopyable &) = delete;
    Noncopyable &operator=(const Noncopyable &) = delete;
};

}

#endif