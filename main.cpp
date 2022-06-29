#include "Thread/EventLoop.h"
#include "Network/InetAddress.h"
#include "Log/LogLine.h"
#include "Log/LogFile.h"
#include "IO/DoubleBufferingOutput.h"
#include "Log/LogLineParse.h"
#include "IO/OutputFile.h"
#include "ForumServer/ForumServer.h"

using namespace Rootive;

int main()
{
    LogLine::setOutput(new DoubleBufferingOutput(new OutputFile("/home/dmtsai/bak/Test")));
    EventLoop eventLoop;
    const char user[] = "Rootive";
    const char pw[] = ""; // BUG
    ForumServer server(&eventLoop, InetAddress(11116), 
    InetAddress("127.0.0.1", 3306), user, pw, "Forum"); //  47.111.114.171
    server.setContextPath("/home/dmtsai/Sports/");
    server.run();
    eventLoop.run();
    return 0;
    // LogLineToConsole c;
    // c.run();
}
