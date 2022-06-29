#ifndef ROOTIVE_HTTPSERVER_H
#define ROOTIVE_HTTPSERVER_H

#include <chrono>
#include "Basics/Noncopyable.h"
#include "TCP/TCPServer.h"
#include "HTTP/HTTPParse.h"

namespace Rootive
{
class HTTPServer : Noncopyable
{
public:
    class ConnectionContext
    {
        friend class HTTPServer;
        HTTPParse parse_;
        std::chrono::system_clock::time_point time_;
    public:
        void *context_;
        ConnectionContext() : 
        parse_(HTTPParse::EType::Request), context_(nullptr) {}
        void clear()
        {
            parse_.clear(HTTPParse::EType::Request);
        }
    };
    typedef std::function<void(const std::shared_ptr<TCPConnection> &)> Callback;
    typedef std::function<void(const std::shared_ptr<TCPConnection> &, HTTPParse::EError, int, int)> ErrorOccurredCallback;
private:
    TCPServer server_;
    Callback connectionCallback_;
    Callback startLineFinishedCallback_;
    Callback headersChangedCallback_;
    Callback headersFinishedCallback_;
    Callback finishedCallback_;
    Callback unparsedLengthIncreaseCallback_;
    ErrorOccurredCallback errorOccurredCallback_;
    void onConnection(const std::shared_ptr<TCPConnection> &conn);
    void onMessage(const std::shared_ptr<TCPConnection> &conn, std::chrono::system_clock::time_point time);
    static void onWriteFinished(const std::shared_ptr<TCPConnection> &conn);
    void onStartLineFinished(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder);
    void onHeadersChanged(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder);
    void onHeadersFinished(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder);
    void onFinished(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder);
    void onUnparsedLengthIncrease(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder);
    void onErrorOccurred(const std::shared_ptr<TCPConnection> &conn, const HTTPParse *placeholder, HTTPParse::EError _errno, int begin, int end);
public:
    HTTPServer(EventLoop *eventLoop, const InetAddress &listenAddr, 
    const std::string &name = "HTTPServer", int numThread = 0, TCPServer::EOption option = TCPServer::EOption::NonReusePort);
    
    static inline ConnectionContext *getContext(const std::shared_ptr<TCPConnection> &conn)
    { return static_cast<ConnectionContext *>(conn->context_); }
    static inline HTTPParse &getHTTPParse(const std::shared_ptr<TCPConnection> &conn) 
    { return getContext(conn)->parse_; }
    static inline std::chrono::system_clock::time_point getTime(const std::shared_ptr<TCPConnection> &conn) 
    { return getContext(conn)->time_; }

    inline const std::string &getName() const { return server_.getName(); }
    void setConnectionCallback(const Callback &callback) { connectionCallback_ = callback; }
    void setStartLineFinishedCallback(const Callback &callback) { startLineFinishedCallback_ = callback; }
    void setHeadersChangedCallback(const Callback &callback) { headersChangedCallback_ = callback; }
    void setHeadersFinishedCallback(const Callback &callback) { headersFinishedCallback_ = callback; }
    void setFinishedCallback(const Callback &callback) { finishedCallback_ = callback; }
    void setUnparsedLengthIncreaseCallback_(const Callback &callback) { unparsedLengthIncreaseCallback_ = callback; };
    void setErrorOccurredCallback(const ErrorOccurredCallback &callback) { errorOccurredCallback_ = callback; }
    inline void run() { server_.run(); }
};
}


#endif