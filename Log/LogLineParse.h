#ifndef ROOTIVE_LOGLINEPARSE_H
#define ROOTIVE_LOGLINEPARSE_H

#include <chrono>
#include <map>
#include "Basics/StringView.h"
#include "Log/LogLine.h"

namespace Rootive
{
namespace LogLineParse
{
LogLine::Level stringToLevel(const StringView &str);
extern const int levelToColorArray_[6];
extern const char *pidStyleArray_[6];
}
class SingleProcessLogLineToConsole
{
    std::map<__pid_t, const char *> tidStyleMap_;
    int tidCount_;
    int pid_;
    const char *pidStyle_;
public:
    SingleProcessLogLineToConsole(int pid, const char *pidStyle) :
    tidCount_(0), pid_(pid), pidStyle_(pidStyle)
    {}
    void parseLine(std::vector<StringView> &splitResult);
};
class LogLineToConsole
{
    char buffer_[1024];
    int count_ = 0;
    std::map<__pid_t, SingleProcessLogLineToConsole> map_;
public:
    void run();
};
}

#endif