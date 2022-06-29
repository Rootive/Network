#include "HTTPServerBoot.h"
#include <cstring>
#include "Database/SQLQuery.h"
#include "Log/LogLine.h"

#include <iostream>

namespace Rootive
{
void HTTPServerBoot::defaultUnauthorizedCallback(const std::shared_ptr<TCPConnection> &conn)
{
    cold(conn, "401");
    HTTPServer::getHTTPParse(conn).pause();
}
void HTTPServerBoot::onConnection(const std::shared_ptr<TCPConnection> &conn)
{
    HTTPServer::ConnectionContext *context = HTTPServer::getContext(conn);
    if (conn->bConnected() && !context->context_)
    {
        context->context_ = new ConnectionContext;
    }
    if (conn->bDisconnected() && context->context_)
    {
        delete static_cast<ConnectionContext *>(context->context_);
        context->context_ = nullptr;
    }
}
void HTTPServerBoot::onStartLineFinished(const std::shared_ptr<TCPConnection> &conn)
{
    bool bFinished = false;
    auto context = getContext(conn);
    auto &parse = HTTPServer::getHTTPParse(conn);
    StringView url(parse.url_.first + conn->inputBuffer()->readBegin(), parse.url_.second - parse.url_.first);
    int ll = 1;
    int lr = url.indexOf('/', 1);
    int rl = lr + 1;
    int rr = url.indexOf('/', rl);
    if (ll < lr && rl < rr)
    {
        if (url.mid(ll, lr) == getName())
        {
            const auto it = functionMap2_.find(url.mid(rl, rr).toString());
            if (it != functionMap2_.cend())
            {
                if (context->url_.find("/..") == std::string::npos)
                { 
                    context->it_ = it;
                    context->url_ = url.mid(std::min(rr + 1, url.getLength())).toString();  
                    bFinished = true; 
                }
            }
        }
    }
    if (!bFinished)
    {
        cold(conn, "404"); 
        parse.pause();
        ROOTIVE_LOG_INFO << conn->getName() << ": 404";
    }
}
void HTTPServerBoot::onHeadersChanged(const std::shared_ptr<TCPConnection> &conn)
{
    auto context = getContext(conn);
    if (!context->it_->second.type_.empty())
    {
        auto &parse = HTTPServer::getHTTPParse(conn);
        const auto &readBegin = conn->inputBuffer()->readBegin();
        const auto &keyOffset = parse.headersList_.back().first;
        const auto &valueOffset = parse.headersList_.back().second;
        StringView key(keyOffset.first + readBegin, keyOffset.second - keyOffset.first);
        if (key == "username")
        {
            context->account_.set(valueOffset.first + readBegin, valueOffset.second - valueOffset.first);
        }
        else if (key == "password")
        {
            context->password_.set(valueOffset.first + readBegin, valueOffset.second - valueOffset.first);
        }
    }
}
void HTTPServerBoot::onHeadersFinished(const std::shared_ptr<TCPConnection> &conn)
{
    auto context = getContext(conn);
    if (!context->it_->second.type_.empty() && authentication_ && !authentication_(conn))
    {
        if (unauthorizedCallback_) { unauthorizedCallback_(conn); }
        HTTPServer::getHTTPParse(conn).pause();
        ROOTIVE_LOG_INFO << conn->getName() << ": unauthorized";
    }
}
void HTTPServerBoot::onFinished(const std::shared_ptr<TCPConnection> &conn)
{
    auto context = getContext(conn);
    auto &parse = HTTPServer::getHTTPParse(conn);
    const auto &function = context->it_->second;
    const auto &_entityBody = parse.getEntityBody();
    StringView entityBody(conn->inputBuffer()->readBegin() + _entityBody.first, _entityBody.second - _entityBody.first);
    std::vector<StringView> argumentVector;
    if (!context->url_.empty()) { argumentVector.push_back(context->url_); }
    int index;
    while (function.argument_ - argumentVector.size() > 1 && (index = entityBody.indexOfCRLF()) < entityBody.getLength())
    {
        argumentVector.push_back(entityBody.cutPrefix(index));
        entityBody.removePrefix(2);
    }
    if (!entityBody.bEmpty()) { argumentVector.push_back(entityBody); }
    if (function.argument_ <= argumentVector.size()) { function.function_(conn, argumentVector); }
    else 
    { 
        cold(conn, "400");
        parse.pause();
        ROOTIVE_LOG_INFO << conn->getName() << " give " << argumentVector.size() << " instead of " << function.argument_;
    }
    getContext(conn)->clear();
}
void HTTPServerBoot::onErrorOccurred(const std::shared_ptr<TCPConnection> &conn, HTTPParse::EError _errno, int begin, int end)
{
    auto &parse = HTTPServer::getHTTPParse(conn);
    ROOTIVE_LOG_INFO << conn->getName();
    parse.pause();
    cold(conn, "400");
}
HTTPServerBoot::HTTPServerBoot(EventLoop *eventLoop, const InetAddress &listenAddr, 
const std::string &name, int numThread, TCPServer::EOption option) :
server_(eventLoop, listenAddr, name, numThread, option), unauthorizedCallback_(defaultUnauthorizedCallback)
{
    server_.setConnectionCallback(std::bind(&HTTPServerBoot::onConnection, this, std::placeholders::_1));
    server_.setStartLineFinishedCallback(std::bind(&HTTPServerBoot::onStartLineFinished, this, std::placeholders::_1));
    server_.setHeadersChangedCallback(std::bind(&HTTPServerBoot::onHeadersChanged, this, std::placeholders::_1));
    server_.setHeadersFinishedCallback(std::bind(&HTTPServerBoot::onHeadersFinished, this, std::placeholders::_1));
    server_.setFinishedCallback(std::bind(&HTTPServerBoot::onFinished, this, std::placeholders::_1));
    server_.setErrorOccurredCallback(std::bind(&HTTPServerBoot::onErrorOccurred, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}
void HTTPServerBoot::cold(const std::shared_ptr<TCPConnection> &conn, 
const StringView &status, const StringView &reason, const StringView &version)
{
    ROOTIVE_LOG_TRACE << conn->getName();
    HTTPBuilder builder(64);
    builder.writeStartLine(version, status, reason);
    builder.finish();
    conn->send(builder.getBuffer());
    conn->forceClose();
}
}
