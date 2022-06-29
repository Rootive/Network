#ifndef ROOTIVE_LOGFILE_H
#define ROOTIVE_LOGFILE_H

#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include "IO/OutputFileInterface.h"
#include "Log/LogLine.h"

namespace Rootive
{
class LogFile : public OutputFileInterface
{
    int fd_;
    char filename_[128];
    size_t filenameSize_;

    time_t time_;
    struct tm tm_;

    int fileSize_;
    const int fileSizeExpected_;

    void _roll()
    {
        fileSize_ = 0;
        sprintf(filename_ + filenameSize_, "%lld\0", std::chrono::system_clock::now().time_since_epoch().count());
        do { fd_ = open(filename_, O_WRONLY | O_APPEND | O_CREAT); } 
        while (fd_ == -1);
        ROOTIVE_LOG_INFO << filename_;
    }
public:
    static constexpr size_t constexpr_maxFilenameSize = sizeof (filename_) - 20 - 1;

    LogFile(const char *filename, int fileSizeExpected);
    ~LogFile() { ::close(fd_); }
    virtual void write(const void *data, size_t size) override
    {
        if (fileSize_ > fileSizeExpected_) { roll(); }
        fileSize_ += ::write(fd_, data, size);
    }
    virtual void flush() override { ::fsync(fd_); }
    void roll()
    { close(fd_); _roll(); }
};
}

#endif