#include "SQLToJson.h"

namespace Rootive
{
void SQLToJson::escapeCharacter(::std::string &arg)
{
    size_t index, begin = 0;
    while ((index = arg.find('\n', begin)) != std::string::npos)
    {
        arg.replace(index, 1, "\\\\n");
        begin = index + 3;
    }
    begin = 0;
    while ((index = arg.find('\"', begin)) != std::string::npos)
    {
        arg.replace(index, 1, "\\\\\\\"");
        begin = index + 4;
    }
    begin = 0;
    while ((index = arg.find('\'', begin)) != std::string::npos)
    {
        arg.replace(index, 1, "\\\'");
        begin = index + 2;
    }
    
}
}