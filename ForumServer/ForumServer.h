#ifndef ROOTIVE_FORUMSERVER_H
#define ROOTIVE_FORUMSERVER_H

#include "HTTP/HTTPServerBoot.h"
#include <vector>
#include <atomic>
#include "Database/SQLConnectionPool.h"
#include "Database/SQLQuery.h"

namespace Rootive
{
class ForumServer
{
    HTTPServerBoot server_;
    SQLConnectionPool sqlConnectionPool_;
    std::string contextPath_;
    std::unique_ptr<SQLQuery> postPushQuery;
    std::atomic<int> fileCount_;

    bool authentication(const std::shared_ptr<TCPConnection> &connection);
    void updatePostPushQuery();

    /* argument(2): account, password return: { } */
    void login(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(1): filename return: file */
    void file(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(1): file return: path */
    void uploadPostImageAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(1): account return: { } */
    void accountClothing(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(0) return: { } */
    void accountClothingAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(0) return: [ ] */
    void accountFollowAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(2): limit, offset return: [ { } ] */
    void accountFollowedAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(2): another, follow return: affected */
    void setAccountFollowAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(3): post, limit, offset return: [ { } ] */
    void commentPost(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(2); post body return: affected*/
    void setCommentAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(2): post, column return: { } */
    void post(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(2): limit, offset return: [ post ] */
    void postPush(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(2): limit, offset return: [ { } ] */
    void postFollowAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(1): post return: { } */
    void postLikeAndFavouritesAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(4): post like dislike favourites return: affected */
    void setPostLikeAndFavouritesAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(2): limit, offset return: [ ] */
    void postFavouritesAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(3): account, limit, offset return: [ { } ] */
    void postAccount(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(2): limit, offset return: [ { } ] */
    void postInvisibleAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(2): limit, offset return: [ { } ]*/
    void postCheckingAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(2): post, visible return: affected */
    void setPostVisibleAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* administrator argument(2): post, visible return: affected */
    void setPostVisibleAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(5): title, body, type, image, tag return: { } */
    void setPostAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(3): tag, limit, offset return: [ ] */
    void postTag(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(1): tag return: { } */
    void tag(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(1): name return: [ { } ] */
    void tagName(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* argument(1): post return: [ ] */
    void tagPost(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* authentication argument(1): name return: { } */
    void setTagAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);

    /* administrator argument(0) return: { } */
    void postCheckingAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* administrator argument(2): a, b return: affected */
    void joinTagAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    /* administrator argument(2): tag, visible return: affected */
    void setTagVisibleAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    void tagPostAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
    void tagAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);

    /* administrator argument(2): post, size return: affected */
    void setPostSizeAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector);
public:
    ForumServer(EventLoop *eventLoop, const InetAddress &listenAddr, 
    const InetAddress &sqlAddr, const char *user, const char *pw, const char *db, 
    const std::string &name = "Forum", int numThread = 0, int numSQLConnection = 1, 
    TCPServer::EOption option = TCPServer::EOption::NonReusePort);
    void run() { server_.run(); }
    inline void setContextPath(const std::string &argument) { contextPath_ = argument; }
};
}

#endif
