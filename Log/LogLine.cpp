#include "LogLine.h"

#include <thread>
#include "Basics/StringView.h"
#include "Thread/ThisThread.h"

namespace Rootive
{
std::unique_ptr<OutputInterface> LogLine::output_(new DefaultOutput);
const char *LogLine::levelToStringArray[] = { "Trace", "Debug", "Info", "Warn", "Error", "Fatal" };
thread_local char LogLine::strerror_[512];
LogLine::LogLine(Level level, const char *function, bool bSystem)
{   
    data_ += std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
                                                data_ += "|";
    data_ += levelToStringArray[level];         data_ += "|";
    data_ += std::to_string(ThisThread::pid_);  data_ += "|";
    data_ += std::to_string(ThisThread::tid_);  data_ += "|";
    data_ += function;                          data_ += "|";
    if (bSystem) { data_ += "errno: "; data_ += std::to_string(errno); }
}
LogLine::~LogLine() 
{
    data_ += "\n";
    output_->write(data_.data(), data_.size());
}
}