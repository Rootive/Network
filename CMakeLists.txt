cmake_minimum_required(VERSION 3.5)

project(Rootive LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(.)
add_executable(Rootive
    main.cpp
    Basics/Types.h Basics/StringView.h Basics/Noncopyable.h Network/PathSegmentView.h Network/PathSegmentView.cpp
    IO/IOBuffer.h IO/IOBuffer.cpp
    Thread/EventLoop.h Thread/EventLoop.cpp Thread/EventLoopThread.h Thread/EventLoopThread.cpp
    Thread/Channel.h Thread/Channel.cpp Thread/Poller.h Thread/Poller.cpp
    Thread/Acceptor.h Thread/Acceptor.cpp Thread/EventLoopThreadPool.h Thread/EventLoopThreadPool.cpp
    Network/InetAddress.h Network/InetAddress.cpp
    Network/Socket.h Network/Socket.cpp Network/NetworkBuffer.h
    TCP/TCPServer.h TCP/TCPServer.cpp TCP/TCPConnection.h TCP/TCPConnection.cpp
    Database/SQLConnection.h Database/SQLQuery.h Database/SQLConnectionPool.h
    HTTP/HTTPServer.h HTTP/HTTPServer.cpp HTTP/HTTPServerBoot.h HTTP/HTTPServerBoot.cpp
    HTTP/HTTPBuilder.h 
    #Example/SportsServer.h Example/SportsServer.cpp
    Json/SQLToJson.h Json/SQLToJson.cpp
    ForumServer/ForumServer.h ForumServer/ForumServer.cpp
    IO/DoubleBufferingOutput.h IO/DoubleBufferingOutput.cpp Log/LogFile.h IO/OutputFileInterface.h IO/OutputInterface.h
    Log/LogLine.h Log/LogLine.cpp
    Thread/ThisThread.h Log/LogFile.cpp IO/OutputFile.h
    Log/LogLineParse.h Log/LogLineParse.cpp
    HTTP/HTTPParse.h HTTP/HTTPParse.cpp 
)
target_link_libraries(Rootive pthread)
target_link_libraries(Rootive mariadbclient)


