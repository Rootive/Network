#ifndef ROOTIVE_HTTPBUILDER_H
#define ROOTIVE_HTTPBUILDER_H

#include <fcntl.h>
#include <unistd.h>
#include "Basics/Noncopyable.h"
#include "IO/IOBuffer.h"
#include "Basics/StringView.h"

#include <iostream>

namespace Rootive
{
class HTTPBuilder : Noncopyable
{
    IOBuffer buffer_;
public:
    HTTPBuilder(size_t initialSize = 1024) : buffer_(initialSize) {}
    IOBuffer &getBuffer() { return buffer_; }
    void writeStartLine(const StringView &l, const StringView &m, const StringView &r)
    {
        buffer_.write(l); buffer_.write(' ');
        buffer_.write(m); buffer_.write(' ');
        buffer_.write(r); buffer_.write("\r\n");
    }
    void writeHeader(const StringView &l, const StringView &r)
    {
        buffer_.write(l); buffer_.write(": ");
        buffer_.write(r); buffer_.write("\r\n");
    }
    void finish() { buffer_.write("\r\n"); }
    void writeEntityBody(const StringView &l)
    {
        char sizeBuffer[16];
        sprintf(sizeBuffer, "%d", l.getLength());
        writeHeader("Content-Length", sizeBuffer);
        buffer_.write("\r\n");
        buffer_.write(l);
    }
    int writeEntityBody(const char *file, size_t fileSize)
    {
        char sizeBuffer[16];
        sprintf(sizeBuffer, "%lu", fileSize);
        writeHeader("Content-Length", sizeBuffer);

        buffer_.write("\r\n");

        int fd = open(file, O_RDONLY);
        int _errno = 0;
        buffer_.write(fd, fileSize, &_errno);
        close(fd);
        return _errno;
    }
};
}

#endif