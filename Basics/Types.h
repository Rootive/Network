#ifndef ROOTIVE_TYPES_H
#define ROOTIVE_TYPES_H

#include <memory>

namespace Rootive
{

namespace Types
{

template <typename To, typename From>
inline To implicitCast(const From &arg)
{
    return arg;
}

template <typename To, typename From>
inline To downCast(const From &arg)
{
    if (false)
    {
        implicitCast<From *, To *>(0);
    }
    return static_cast<To>(arg);
}

template <typename To, typename From>
inline ::std::shared_ptr<To> down_pointer_cast(const ::std::shared_ptr<From> &arg)
{
    if (false)
    {
        implicitCast<From *, To *>(0);
    }
    return ::std::static_pointer_cast<To>(arg);
}

}

};

#endif
