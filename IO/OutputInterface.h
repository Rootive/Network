#ifndef ROOTIVE_OUTPUTINTERFACE_H
#define ROOTIVE_OUTPUTINTERFACE_H

#include <unistd.h>
#include "IO/OutputFileInterface.h"

namespace Rootive
{
class OutputInterface
{
public:
    virtual ~OutputInterface() = default;
    virtual void write(const void *data, size_t size) = 0;
};
class DefaultOutput : public OutputInterface
{
    std::unique_ptr<OutputFileInterface> file_;
public:
    DefaultOutput(OutputFileInterface *file = new DefaultOutputFile) :
    file_(file)
    {}
    virtual void write(const void *data, size_t size) override
    { file_->write(data, size); }
};
}

#endif