#ifndef ROOTIVE_NETWORKBUFFER_H
#define ROOTIVE_NETWORKBUFFER_H

#include <endian.h>
#include "IO/IOBuffer.h"

namespace Rootive
{
class NetworkBuffer : public IOBuffer
{
public:
    NetworkBuffer(size_t initialSize = 1024) :
    IOBuffer(initialSize)
    {}

    inline void hasReadInt64() { hasRead(sizeof(int64_t)); }
    inline void hasReadInt32() { hasRead(sizeof(int32_t)); }
    inline void hasReadInt16() { hasRead(sizeof(int16_t)); }
    inline void hasReadInt8() { hasRead(sizeof(int8_t)); }
    int64_t peekInt64() const
    {
        int64_t net64;
        ::memcpy(&net64, readBegin(), sizeof(net64));
        return be64toh(net64);
    }
    int32_t peekInt32() const
    {
        int32_t net32;
        ::memcpy(&net32, readBegin(), sizeof(net32));
        return be32toh(net32);
    }
    int16_t peekInt16() const
    {
        int16_t net16;
        ::memcpy(&net16, readBegin(), sizeof(net16));
        return be16toh(net16);
    }
    int8_t peekInt8() const
    {
        return *(readBegin());
    }
    int64_t readInt64()
    {
        int64_t ret = peekInt64();
        hasReadInt64();
        return ret;
    }
    int32_t readInt32()
    {
        int32_t ret = peekInt32();
        hasReadInt32();
        return ret;
    }
    int16_t readInt16()
    {
        int16_t ret = peekInt16();
        hasReadInt16();
        return ret;
    }
    int8_t readInt8()
    {
        int8_t ret = peekInt8();
        hasReadInt8();
        return ret;
    }
    inline void writeInt64(int64_t data)
    {
        int64_t net64 = htole64(data);
        write(&net64, sizeof(net64));
    }
    inline void writeInt32(int32_t data)
    {
        int32_t net32 = htobe32(data);
        write(&net32, sizeof(net32));
    }
    inline void writeInt16(int16_t data)
    {
        int16_t net16 = htobe16(data);
        write(&net16, sizeof(net16));
    }
    inline void writeInt8(int8_t data)
    {
        write(&data, sizeof(data));
    }

    void prependInt64(int64_t data)
    {
        int64_t net64 = htobe64(data);
        prepend(&net64, sizeof(net64));
    }
    void prependInt32(int32_t data)
    {
        int32_t net32 = htobe32(data);
        prepend(&net32, sizeof(net32));
    }
    void prependInt16(int16_t data)
    {
        int16_t net16 = htobe16(data);
        prepend(&net16, sizeof(net16));
    }
    void prependInt8(int8_t data)
    {
        prepend(&data, sizeof(data));
    }
};
}

#endif