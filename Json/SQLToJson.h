#ifndef ROOTIVE_SQLTOJSON_H
#define ROOTIVE_SQLTOJSON_H

#include <string>
#include "Database/SQLQuery.h"

namespace Rootive
{
namespace SQLToJson
{
inline std::string recordToJson(SQLQuery &query, int record = 0)
{
    std::string ret = "{ ";
    if (query)
    {
        for (int _i = 0; _i < query.numCol(); ++_i)
        {
            ret += '\"';
            ret += query.colName(_i);
            ret += "\": \"";
            ret += query[record][_i] ? query[record][_i] : "";
            ret += "\", ";
        }
        ret.pop_back();
        ret.pop_back();
    }
    ret += " }";
    return ret;
}
inline std::string recordsToJson(SQLQuery &query)
{
    std::string ret = "[ ";
    if (query)
    {
        for (int _i = 0; _i < query.numRow(); ++_i)
        {
            ret += recordToJson(query, _i);
            ret += ", ";
        }
        ret.pop_back();
        ret.pop_back();
    }
    ret += " ]";
    return ret;
}
inline std::string columnToJson(SQLQuery &query, int column = 0)
{
    std::string ret = "[ ";
    if (query)
    {
        for (int _i = 0; _i < query.numRow(); ++_i)
        {
            ret += "\"";
            ret += query[_i][column] ? query[_i][column] : "";
            ret += "\", ";
        }
        ret.pop_back();
        ret.pop_back();
    }
    ret += " ]";
    return std::move(ret);
}
inline std::string columnToJson(SQLQuery &query, int limit, int offset = 0, int column = 0)
{
    std::string ret = "[ ";
    for (int _i = 0; offset < query.numRow() && _i < limit; ++_i, ++offset)
    {
        ret += "\"";
        ret += query[offset][column] ? query[offset][column] : "";
        ret += "\", ";
    }
    if (ret.size() > 2)
    {
        ret.pop_back();
        ret.pop_back();
    }
    ret += " ]";
    return std::move(ret);
}
void escapeCharacter(::std::string &arg);
}
}

#endif