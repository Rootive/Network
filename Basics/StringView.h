#ifndef ROOTIVE_STRINGVIEW_H
#define ROOTIVE_STRINGVIEW_H

#include <cstring>
#include <cctype>
#include <string>
#include <vector>

namespace Rootive
{

class StringView
{
protected:
    const char *ptr_;
    int length_;
public:
    StringView() : ptr_(nullptr), length_(0) {}
    StringView(const char *str) : ptr_(str), length_(static_cast<int>(strlen(str))) {}
    StringView(const char *offset, int len) : ptr_(offset), length_(len) {}
    StringView(const std::string &str) : ptr_(str.data()), length_(str.size()) {}

    inline bool operator==(const StringView &another) const
    {
        return (length_ == another.length_) && !memcmp(ptr_, another.ptr_, length_);
    }
    inline bool operator!=(const StringView &another) const { return !(*this == another); }
#define STRINGVIEW_BINARY_PREDICATE(cmp,auxcmp)                                                     \
    bool operator cmp (const StringView& another) const                                             \
    {                                                                                               \
        int ret = memcmp(ptr_, another.ptr_, length_ < another.length_ ? length_ : another.length_);\
        return (ret auxcmp 0) || (ret == 0) && (length_ cmp another.length_);                       \
    }
    STRINGVIEW_BINARY_PREDICATE(<,  <);
    STRINGVIEW_BINARY_PREDICATE(<=, <);
    STRINGVIEW_BINARY_PREDICATE(>=, >);
    STRINGVIEW_BINARY_PREDICATE(>,  >);
#undef STRINGVIEW_BINARY_PREDICATE

    int compare(const StringView& another) const
    {
        int ret = memcmp(ptr_, another.ptr_, length_ < another.length_ ? length_ : another.length_);
        if (!ret) { if (length_ < another.length_) { ret = -1; } else if (length_ > another.length_) { ret = 1; } }
        return ret;
    }
    inline const char *begin() const { return ptr_; }
    inline int getLength() const { return length_; }
    inline bool bEmpty() const { return 0 == length_; }
    inline const char *end() const { return ptr_ + length_; }

    inline void clear() { ptr_ = nullptr, length_ = 0; }
    inline void set(const char *str) { ptr_ = str; length_ = static_cast<int>(strlen(str)); }
    inline void set(const char *offset, int len) { ptr_ = offset; length_ = len; }
    inline char operator[](int index) const { return ptr_[index]; }

    inline void append(int length) { length_ += length; }

    std::string toString() const { return std::string(ptr_, length_); }

    inline StringView mid(int left, int right) const { return StringView(ptr_ + left, right - left); }
    inline StringView mid(int left) const { return mid(left, length_); }

    inline void removePrefix(int length) { ptr_ += length; length_ -= length; }
    inline void removePrefix(char ch) { while (length_ && ptr_[0] == ch) { removePrefix(1); } }
    inline void removeSuffix(int length) { length_ -= length; }
    inline void removeSuffix(char ch) { while (length_ && ptr_[length_ - 1] == ch) { removeSuffix(1); } }
    
    inline bool bPrefix(const StringView &arg) const { return (length_ >= arg.length_) && !memcmp(ptr_, arg.ptr_, arg.length_); }

    inline int indexOf(char ch, const int index = 0) const
    {
        int ret;
        for (ret = index; ret < length_; ++ret)
        {
            if (ptr_[ret] == ch) { break; }
        }
        return ret;
    }
    inline int indexOfNot(char ch, const int index = 0) const
    {
        int ret;
        for (ret = index; ret < length_; ++ret)
        {
            if (ptr_[ret] != ch) { break; }
        }
        return ret;
    }
    inline int indexOfCRLF() const
    {
        int ret;
        for (ret = 1; ret < length_; ++ret)
        {
            if (ptr_[ret - 1] == '\r' && ptr_[ret] == '\n')
            { ret = ret - 1; break; }
        }
        return ret;
    }

    inline int lastIndexOf(char ch) const
    {
        int ret;
        for (ret = length_ - 1; ret >= 0; --ret)
        {
            if (ptr_[ret] == ch) { break; }
        }
        return ret;
    }
    inline int lastIndexOf(char ch, const int index) const
    {
        int ret;
        for (ret = index; ret >= 0; --ret)
        {
            if (ptr_[ret] == ch) { break; }
        }
        return ret;
    }
    inline int lastIndexOfNot(char ch) const
    {
        int ret;
        for (ret = length_ - 1; ret >= 0; --ret)
        {
            if (ptr_[ret] != ch) { break; }
        }
        return ret;
    }
    inline int lastIndexOfNot(char ch, const int index) const
    {
        int ret;
        for (ret = index; ret >= 0; --ret)
        {
            if (ptr_[ret] != ch) { break; }
        }
        return ret;
    }

    inline StringView cutPrefix(int index)
    {
        StringView ret(ptr_, index);
        removePrefix(index);
        return ret;
    }
    inline bool cutPrefix(char ch, StringView &out)
    {
        int index = indexOf(ch);
        if (index == length_) { return false; }
        out = cutPrefix(index);
        return true;
    }
    inline StringView splitPrefix(int index)
    {
        StringView ret = cutPrefix(index);
        removePrefix(1);
        return ret;
    }
    inline bool splitPrefix(char ch, StringView &out)
    {
        bool ret = cutPrefix(ch, out);
        if (ret) { removePrefix(1); }
        return ret;
    }

    std::vector<StringView> split(char ch) const
    {
        StringView cpy(*this);
        int index;
        std::vector<StringView> ret;
        while ((index = cpy.indexOf(ch)) < cpy.getLength())
        {
            ret.push_back(cpy.splitPrefix(index));
        }
        ret.push_back(cpy);
        return ret;
    }
    inline int toInt() const 
    { 
        int ret = 0;
        if (length_ && (isdigit(ptr_[0]) || ptr_[0] == '-' || ptr_[0] == '.'))
        {
            ret = std::stoi(std::string(ptr_, length_));
        }
        return ret;
    }
};

}

#endif
