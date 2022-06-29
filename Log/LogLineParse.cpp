#include "LogLineParse.h"
#include <iostream>

namespace Rootive
{
LogLine::Level LogLineParse::stringToLevel(const StringView &str)
{
    LogLine::Level ret;
    if (str == "Trace") { ret = LogLine::Level::Trace; }
    else if (str == "Debug") { ret = LogLine::Level::Debug; }
    else if (str == "Info") { ret = LogLine::Level::Info; }
    else if (str == "Warn") { ret = LogLine::Level::Warn; }
    else if (str == "Error") { ret = LogLine::Level::Error; }
    else { ret = LogLine::Level::Fatal; }
    return ret;
}
const int LogLineParse::levelToColorArray_[6]{ 44, 46, 42, 43, 41, 41 };
const char *LogLineParse::pidStyleArray_[6] = { "\e[31m", "\e[32m", "\e[33m", "\e[34m", "\e[35m", "\e[36m" };
void SingleProcessLogLineToConsole::parseLine(std::vector<StringView> &splitResult)
{
    int l = splitResult[4].lastIndexOf(' ', splitResult[4].indexOf('(')) + 1;
    int r = splitResult[4].indexOf('(');
    if (l < r)
    {
        std::string format = "\e[0m%02d:%02d:%02d \e[37;%dm %-5.*s \e[0m %s%.*s\e[0m %s%.*s\e[0m ";
        auto level = LogLineParse::stringToLevel(splitResult[1]);

        int tid = splitResult[3].toInt();
        const char *tidStyle;
        if (tid == pid_)
        {
            tidStyle = "\e[8m";
        }
        else
        {
            auto it = tidStyleMap_.find(tid);
            if (it == tidStyleMap_.cend())
            {
                tidStyleMap_[tid] = LogLineParse::pidStyleArray_[tidCount_];
                tidStyle = LogLineParse::pidStyleArray_[tidCount_];
                tidCount_ = (tidCount_ + 1) % 6;
            }
            else
            {
                tidStyle = it->second;
            }
        }

        auto _time = std::chrono::system_clock::time_point(std::chrono::system_clock::time_point::duration(std::stol(std::string(splitResult[0].begin(), splitResult[0].getLength()))));
        auto time = std::chrono::system_clock::to_time_t(_time);
        struct tm local;
        localtime_r(&time, &local);

        auto functionSplitResult = splitResult[4].mid(l, r).split(':');
        format.append(functionSplitResult[0].begin(), functionSplitResult[0].getLength());
        for (int _i = 1; _i < functionSplitResult.size(); ++_i)
        {
            if (!functionSplitResult[_i].bEmpty())
            {
                format += "\e[4m::\e[0m";
                format.append(functionSplitResult[_i].begin(), functionSplitResult[_i].getLength());
            }
        }
        format += " \e[4m";
        format.append(splitResult[5].begin(), splitResult[5].getLength());
        format += "\e[0m\n";
        printf(format.c_str(), local.tm_hour, local.tm_min, local.tm_sec, 
        LogLineParse::levelToColorArray_[level], splitResult[1].getLength(), splitResult[1].begin(), 
        pidStyle_, splitResult[2].getLength(), splitResult[2].begin(), 
        tidStyle, splitResult[3].getLength(), splitResult[3]);
    }
    else
    {
        std::cout << "function parse failed." << std::endl;
    }
}
void LogLineToConsole::run()
{
    while (std::cin.getline(buffer_, sizeof(buffer_)))
    {
        StringView line(buffer_);
        auto splitResult = line.split('|');
        if (splitResult.size() == 6)
        {
            int pid = splitResult[2].toInt();
            auto it = map_.find(pid);
            if (it == map_.cend())
            {
                it = map_.emplace(std::pair(pid, SingleProcessLogLineToConsole(pid, LogLineParse::pidStyleArray_[count_])) ).first;
                count_ = (count_ + 1) % 6;
            }
            it->second.parseLine(splitResult);
        }
        else
        {
            std::cout << "size parse failed." << std::endl;
        }
    }
}
}