#ifndef ROOTIVE_HTTPSERVERBOOT_H
#define ROOTIVE_HTTPSERVERBOOT_H

#include <map>
#include "HTTP/HTTPServer.h"
#include "HTTP/HTTPBuilder.h"

namespace Rootive
{
class HTTPServerBoot
{
public:
    typedef std::function<void(const std::shared_ptr<TCPConnection> &, const std::vector<StringView> &)> Function;
    typedef std::function<bool(const std::shared_ptr<TCPConnection> &)> Authentication;
    typedef std::function<void(const std::shared_ptr<TCPConnection> &)> UnauthorizedCallback;
    struct FunctionControl
    {
        int argument_;
        std::string type_;
        Function function_;
    };
    class ConnectionContext
    {
        friend class HTTPServerBoot;
        std::string url_;
        std::map<std::string, FunctionControl>::const_iterator it_;
        StringView account_;
        StringView password_;
    public:
        void *context_;
        void clear()
        {
            url_.clear();
            account_.clear();
            password_.clear();
        }
    };
private:
    HTTPServer server_;
    Authentication authentication_;
    UnauthorizedCallback unauthorizedCallback_;
    static void defaultUnauthorizedCallback(const std::shared_ptr<TCPConnection> &conn);
    void onConnection(const std::shared_ptr<TCPConnection> &conn);
    void onStartLineFinished(const std::shared_ptr<TCPConnection> &conn);
    void onHeadersChanged(const std::shared_ptr<TCPConnection> &conn);
    void onHeadersFinished(const std::shared_ptr<TCPConnection> &conn);
    void onFinished(const std::shared_ptr<TCPConnection> &conn);
    void onErrorOccurred(const std::shared_ptr<TCPConnection> &conn, HTTPParse::EError _errno, int begin, int end);
public:
    std::map<std::string, FunctionControl> functionMap2_;

    HTTPServerBoot(EventLoop *eventLoop, const InetAddress &listenAddr, 
    const std::string &name = "HTTPServerBoot", int numThread = 0, TCPServer::EOption option = TCPServer::EOption::NonReusePort);

    static inline ConnectionContext *getContext(const std::shared_ptr<TCPConnection> &conn) 
    { return static_cast<ConnectionContext *>(HTTPServer::getContext(conn)->context_); }
    static inline const StringView &getAccount(const std::shared_ptr<TCPConnection> &conn) 
    { return getContext(conn)->account_; }
    static inline const StringView &getPassword(const std::shared_ptr<TCPConnection> &conn) 
    { return getContext(conn)->password_; }
    static inline const std::map<std::string, FunctionControl>::const_iterator &getIT(const std::shared_ptr<TCPConnection> &conn) 
    { return getContext(conn)->it_; }
    static inline std::chrono::system_clock::time_point getTime(const std::shared_ptr<TCPConnection> &conn) 
    { return HTTPServer::getTime(conn); }
    const std::string &getName() const { return server_.getName(); }
    
    static void cold(const std::shared_ptr<TCPConnection> &conn, const StringView &status, 
    const StringView &reason = "Omitted", const StringView &version = "HTTP/1.1");
    inline void setAuthentication(Authentication function) { authentication_ = function; }
    inline void setUnauthorizedCallback(UnauthorizedCallback callback) { unauthorizedCallback_ = callback; }
    inline void run() { server_.run(); }
};
}

#endif