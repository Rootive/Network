#ifndef ROOTIVE_OUTPUTFILEINTERFACE_H
#define ROOTIVE_OUTPUTFILEINTERFACE_H

#include <cstdio>
#include <unistd.h>

namespace Rootive
{
class OutputFileInterface
{
public:
    virtual void write(const void *data, size_t size) = 0;
    virtual void flush() = 0;
    virtual ~OutputFileInterface() = default;
};
class DefaultOutputFile : public OutputFileInterface
{
public:
    virtual void write(const void *data, size_t size) override
    { ::write(1, data, size); }
    virtual void flush() override { ::fsync(1); }
};
}

#endif