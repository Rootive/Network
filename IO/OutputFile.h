#ifndef ROOTIVE_OUTPUTFILE_H
#define ROOTIVE_OUTPUTFILE_H

#include "IO/OutputFileInterface.h"
#include "Basics/StringView.h"
#include "Log/LogLine.h"
#include <string>
#include <fcntl.h>
#include <unistd.h>

namespace Rootive
{
class OutputFile : public OutputFileInterface
{
    std::string filename_;
    int fd_;
    int writtenSize_;
public:
    OutputFile(const StringView &filename) : 
    filename_(filename.begin(), filename.getLength()), 
    fd_(::open(filename_.c_str(), O_WRONLY | O_APPEND | O_CREAT)), writtenSize_(0)
    {
        if (fd_ < 0)
        {
            ROOTIVE_LOG_SYSTEMERROR;
        }
    }
    ~OutputFile() { ::close(fd_); }
    int getWrittenSize() const { return writtenSize_; }
    virtual void write(const void *data, size_t size) override
    {
        auto res = ::write(fd_, data, size); 
        if (res < 0) { ROOTIVE_LOG_SYSTEMERROR; }
        else { writtenSize_ += res; }
        
    }
    virtual void flush() override
    { ::fsync(fd_); }
};
}

#endif
