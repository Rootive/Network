#ifndef ROOTIVE_PATHSEGMENTVIEW_H
#define ROOTIVE_PATHSEGMENTVIEW_H

#include "Basics/StringView.h"
#include <map>

namespace Rootive
{
struct PathSegmentView
{
    StringView path_;
    std::map<StringView, StringView> paramsMap_;
    std::map<StringView, StringView> queryMap_;

    inline void clear() { path_.clear(); paramsMap_.clear(); queryMap_.clear(); }
    bool set(const StringView &data);
};
}

#endif