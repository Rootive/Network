#include "IOBuffer.h"

#include <sys/uio.h>

namespace Rootive
{

ssize_t IOBuffer::write(int fd, const size_t max, int* out_errno)
{
    char *extra = (char *)malloc(max);
    iovec vec[2];
    const size_t writable = writableLength();
    vec[0].iov_base = writeBegin();
    vec[0].iov_len = writable;
    vec[1].iov_base = extra;
    vec[1].iov_len = max;
    const int iovcnt = (writable >= max) ? 1 : 2;
    const ssize_t ret = ::readv(fd, vec, iovcnt);
    if (ret < 0) { *out_errno = errno; }
    else if (static_cast<size_t>(ret) <= writable) { hasWritten(ret); }
    else { writeIndex_ = buffer_.size(); write(extra, ret - writable); }
    delete extra;
    return ret;
}
void IOBuffer::makeSpace(size_t len)
{
    size_t _readableLength = readableLength();
    int _i = constexpr_initialPrependSize + _readableLength + len;
    if (_i > buffer_.size()) { buffer_.resize(_i); }
    std::copy(begin() + readIndex_, writeBegin(), begin() + constexpr_initialPrependSize);
    readIndex_ = constexpr_initialPrependSize;
    writeIndex_ = readIndex_ + _readableLength;
}

}


