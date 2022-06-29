#include "HTTPServer.h"
#include "Log/LogLine.h"

#include <iostream>

namespace Rootive
{
void HTTPServer::onConnection(const std::shared_ptr<TCPConnection> &conn)
{
    if (conn->bConnected() && !conn->context_)
    {
        conn->setTCPNodelay(true);
        conn->context_ = new ConnectionContext;
        auto &parse = getContext(conn)->parse_;
        parse.setStartLineFinishedCallback(std::bind(&HTTPServer::onStartLineFinished, this, conn, std::placeholders::_1));
        parse.setHeadersChangedCallback(std::bind(&HTTPServer::onHeadersChanged, this, conn, std::placeholders::_1));
        parse.setHeadersFinishedCallback(std::bind(&HTTPServer::onHeadersFinished, this, conn, std::placeholders::_1));
        parse.setFinishedCallback(std::bind(&HTTPServer::onFinished, this, conn, std::placeholders::_1));
        parse.setUnparsedLengthIncreaseCallback(std::bind(&HTTPServer::onUnparsedLengthIncrease, this, conn, std::placeholders::_1));
        parse.setErrorOccurredCallback(std::bind(&HTTPServer::onErrorOccurred, this, conn, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        if (connectionCallback_) { connectionCallback_(conn); }
    }
    if (conn->bDisconnected() && conn->context_)
    {
        if (connectionCallback_) { connectionCallback_(conn); }
        delete getContext(conn);
        conn->context_ = nullptr;
    }
}
void HTTPServer::onMessage(const std::shared_ptr<TCPConnection> &conn, std::chrono::system_clock::time_point time)
{
    auto buffer = conn->inputBuffer();
    ConnectionContext *context = getContext(conn);
    auto &parse = context->parse_;
    while (buffer->readableLength())
    {
        context->time_ = time;
        parse.update(buffer->readBegin(), buffer->readableLength());
        if (parse.bFinished())
        {
            buffer->hasRead(parse.getOffset());
            context->clear();
        }
        else { break; }
    }
}
void HTTPServer::onWriteFinished(const std::shared_ptr<TCPConnection> &conn)
{
    ROOTIVE_LOG_INFO << conn->getName();
}
void HTTPServer::onStartLineFinished(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder)
{
    if (startLineFinishedCallback_) 
    { startLineFinishedCallback_(conn); }
}
void HTTPServer::onHeadersChanged(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder)
{
    if (headersChangedCallback_) 
    { headersChangedCallback_(conn); }
}
void HTTPServer::onHeadersFinished(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder)
{
    if (headersFinishedCallback_) 
    { headersFinishedCallback_(conn); }
}
void HTTPServer::onFinished(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder)
{
    if (finishedCallback_) 
    { finishedCallback_(conn); }
}
void HTTPServer::onUnparsedLengthIncrease(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder)
{
    if (unparsedLengthIncreaseCallback_) 
    { unparsedLengthIncreaseCallback_(conn); }
}
void HTTPServer::onErrorOccurred(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder, HTTPParse::EError _errno, int begin, int end)
{
    if (errorOccurredCallback_) 
    { errorOccurredCallback_(conn, _errno, begin, end); }
}
HTTPServer::HTTPServer(EventLoop *eventLoop, const InetAddress &listenAddr, 
const std::string &name, int numThread, TCPServer::EOption option) :
server_(eventLoop, listenAddr, name, numThread, option)
{
    server_.setConnectionCallback(std::bind(&HTTPServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&HTTPServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    server_.setWriteFinishedCallback(&HTTPServer::onWriteFinished);
}
}