#ifndef ROOTIVE_IOBUFFER_H
#define ROOTIVE_IOBUFFER_H

#include <vector>
#include <algorithm>
#include <string>

#include "Basics/StringView.h"

namespace Rootive
{

class IOBuffer
{
    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;

    inline const char *begin() const { return &*buffer_.begin(); }
    inline char *begin() { return &*buffer_.begin(); }
public:
    static constexpr size_t constexpr_initialPrependSize = 8;
    explicit IOBuffer(size_t initialSize)
        : buffer_(constexpr_initialPrependSize + initialSize), readIndex_(constexpr_initialPrependSize), writeIndex_(constexpr_initialPrependSize) {}

    inline void swap(IOBuffer &another)
    {
        buffer_.swap(another.buffer_);
        std::swap(readIndex_, another.readIndex_);
        std::swap(writeIndex_, another.writeIndex_);
    }
    inline size_t readableLength() const { return writeIndex_ - readIndex_; }
    inline size_t writableLength() const { return buffer_.size() - writeIndex_; }
    inline size_t prependableLength() const { return readIndex_; }

    inline const char *readBegin() const { return begin() + readIndex_; }
    inline void hasRead(size_t len)
    {
        if (len < readableLength()) { readIndex_ += len; }
        else { hasReadAll(); }
    }
    inline void hasReadAll() { readIndex_ = constexpr_initialPrependSize; writeIndex_ = constexpr_initialPrependSize; }
    inline StringView toStringView(int length) const 
    { 
        if (length < readableLength()) 
        { 
            return StringView(readBegin(), length); 
        }
        else 
        {
            return StringView(readBegin(), static_cast<int>(readableLength())); 
        }
    }
    inline StringView toStringView() const { return StringView(readBegin(), static_cast<int>(readableLength())); }
    inline std::string readAsString(size_t len) { std::string ret(readBegin(), len); hasRead(len); return ret; }
    inline std::string readAllAsString() { return readAsString(readableLength()); }

    inline char *writeBegin() { return begin() + writeIndex_; }
    inline const char *writeBegin() const { return begin() + writeIndex_; }
    inline void hasWritten(size_t len) { writeIndex_ += len; }
    inline void unWrite(size_t len) { writeIndex_ -= len; }
    void makeSpace(size_t len);
    inline void ensureWritableLength(size_t len) { if (writableLength() < len) { makeSpace(len); } }
    inline void write(const char *data, size_t len)
    {
        ensureWritableLength(len);
        std::copy(data, data + len, writeBegin());
        hasWritten(len);
    }
    inline void write(const void *data, size_t len) { write(static_cast<const char *>(data), len); }
    inline void write(const StringView &str) { write(str.begin(), str.getLength()); }
    ssize_t write(int fd, const size_t max, int* out_errno);
    inline void write(char ch) { write(&ch, 1); }

    void prepend(const void *data, size_t len)
    {
        readIndex_ -= len;
        const char *_data = static_cast<const char *>(data);
        std::copy(_data, _data + len, begin() + readIndex_);
    }

    void shrink(size_t reserve)
    {
        makeSpace(reserve);
        buffer_.shrink_to_fit();
    }
    size_t capacity() const { return buffer_.capacity(); }

};

}

#endif
