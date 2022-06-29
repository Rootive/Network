#ifndef ROOTIVE_LOGSTREAM_H
#define ROOTIVE_LOGSTREAM_H

#ifndef ROOTIVE_LOG_LEVEL
#define ROOTIVE_LOG_LEVEL ::Rootive::LogLine::Level::Trace
#endif

#include <string>
#include <memory>
#include <cstring>
#include "IO/OutputInterface.h"
#include "Basics/StringView.h"

namespace Rootive
{
class StringView;
class LogLine
{
public:
    enum Level
    {
        Trace, Debug, Info, Warn, Error, Fatal
    };
private:
    std::string data_;
    static std::unique_ptr<OutputInterface> output_;
    static const char *levelToStringArray[];
    static thread_local char strerror_[512];
public:
    LogLine(Level level, const char *function, bool bSystem = false);
    ~LogLine();

    static inline const char *errorString(int _errno)
    { return strerror_r(_errno, strerror_, sizeof(strerror_)); }

    static inline void setOutput(OutputInterface *output)
    { output_.reset(output); }
    
    inline LogLine &operator<<(const StringView &data)
    { data_.append(data.begin(), data.getLength()); return *this; }

    inline LogLine &operator<<(const char *data)
    { return operator<<(StringView(data)); }

    inline LogLine &operator<<(char data)
    { data_ += data; return *this; }

    inline LogLine &operator<<(int data)
    { data_ += std::to_string(data); return *this; }

    inline LogLine &operator<<(long data)
    { data_ += std::to_string(data); return *this; }

    inline LogLine &operator<<(unsigned long data)
    { data_ += std::to_string(data); return *this; }

    inline LogLine &operator<<(const void *data)
    { data_ += std::to_string(reinterpret_cast<uintptr_t>(data)); return *this; }


};


#define ROOTIVE_LOG_TRACE \
if constexpr (ROOTIVE_LOG_LEVEL <= ::Rootive::LogLine::Level::Trace) \
::Rootive::LogLine(::Rootive::LogLine::Level::Trace, __PRETTY_FUNCTION__)
#define ROOTIVE_LOG_DEBUG \
if constexpr (ROOTIVE_LOG_LEVEL <= ::Rootive::LogLine::Level::Debug) \
::Rootive::LogLine(::Rootive::LogLine::Level::Debug, __PRETTY_FUNCTION__)
#define ROOTIVE_LOG_INFO \
if constexpr (ROOTIVE_LOG_LEVEL <= ::Rootive::LogLine::Level::Info) \
::Rootive::LogLine(::Rootive::LogLine::Level::Info, __PRETTY_FUNCTION__)
#define ROOTIVE_LOG_WARN \
if constexpr (ROOTIVE_LOG_LEVEL <= ::Rootive::LogLine::Level::Warn) \
::Rootive::LogLine(::Rootive::LogLine::Level::Warn, __PRETTY_FUNCTION__)
#define ROOTIVE_LOG_ERROR \
if constexpr (ROOTIVE_LOG_LEVEL <= ::Rootive::LogLine::Level::Error) \
::Rootive::LogLine(::Rootive::LogLine::Level::Error, __PRETTY_FUNCTION__)
#define ROOTIVE_LOG_FATAL \
if constexpr (ROOTIVE_LOG_LEVEL <= ::Rootive::LogLine::Level::Fatal) \
::Rootive::LogLine(::Rootive::LogLine::Level::Fatal, __PRETTY_FUNCTION__)

#define ROOTIVE_LOG_SYSTEMERROR \
if constexpr (ROOTIVE_LOG_LEVEL <= ::Rootive::LogLine::Level::Error) \
::Rootive::LogLine(::Rootive::LogLine::Level::Error, __PRETTY_FUNCTION__, true);
#define ROOTIVE_LOG_SYSTEMFATAL \
if constexpr (ROOTIVE_LOG_LEVEL <= ::Rootive::LogLine::Level::Fatal) \
::Rootive::LogLine(::Rootive::LogLine::Level::Fatal, __PRETTY_FUNCTION__, true);
}

#endif