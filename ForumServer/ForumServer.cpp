#include "ForumServer.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "Json/SQLToJson.h"
#include "IO/OutputFile.h"

namespace Rootive
{
bool ForumServer::authentication(const std::shared_ptr<TCPConnection> &connection)
{
    const auto &account = HTTPServerBoot::getAccount(connection);
    const auto &password = HTTPServerBoot::getPassword(connection);
    const auto &type = HTTPServerBoot::getIT(connection)->second.type_;
    std::string sql = "SELECT DISTINCT * FROM Account WHERE `id` = ";
    sql.append(account.begin(), account.getLength());
    sql += " AND `password` = '";
    sql.append(password.begin(), password.getLength());
    sql += "' AND `type` >= ";
    sql += type;
    sql += " LIMIT 1;";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    return query;
}
void ForumServer::updatePostPushQuery()
{
    std::string sql = "SELECT id FROM Post WHERE `visible` = 'True' "
    "ORDER BY (`favourites` * 2 + `like` - `dislike`) / TIMESTAMPDIFF(DAY, `time`, NOW()) DESC, `time` DESC;";
    postPushQuery.reset(new SQLQuery(sqlConnectionPool_.getConnection(), sql));
    ROOTIVE_LOG_TRACE << SQLToJson::columnToJson(*postPushQuery);
}
void ForumServer::login(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT DISTINCT * FROM Account WHERE id = ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " AND `type` <> 'Black' AND password = '";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += "' LIMIT 1;";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::file(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string path = contextPath_;
    path.append(argumentVector[0].begin(), argumentVector[0].getLength());
    struct stat fileStat;
    if (!stat(path.c_str(), &fileStat))
    {
        HTTPBuilder builder(fileStat.st_size + 128);
        builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
        builder.writeEntityBody(path.c_str(), fileStat.st_size);
        connection->send(builder.getBuffer());
    }
    else { HTTPServerBoot::cold(connection, "404"); }
}
void ForumServer::uploadPostImageAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    fileCount_ = fileCount_ % 100;
    std::string path = "PostImage/" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + std::to_string(fileCount_);
    ssize_t written = 0;
    int fd = ::open((contextPath_ + path).c_str(), O_WRONLY | O_CREAT, 0777);
    while (written < argumentVector[0].getLength())
    {
        written += ::write(fd, argumentVector[0].begin() + written, argumentVector[0].getLength() - written);
    }
    ::close(fd);
    HTTPBuilder builder(path.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(path);
    connection->send(builder.getBuffer());
}

void ForumServer::accountClothing(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT DISTINCT * FROM AccountClothing WHERE id = ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " LIMIT 1;";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::accountClothingAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT DISTINCT * FROM AccountClothing WHERE id = ";
    const auto &account = HTTPServerBoot::getAccount(connection);
    sql.append(account.begin(), account.getLength());
    sql += " LIMIT 1;";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::accountFollowAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT `another` FROM Account_Account WHERE `follow` = '1' AND account = ";
    const auto &account = HTTPServerBoot::getAccount(connection);
    sql.append(account.begin(), account.getLength());
    sql += ";";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::columnToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::accountFollowedAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT `account` FROM Account_Account WHERE `follow` = '1' AND another = ";
    const auto &account = HTTPServerBoot::getAccount(connection);
    sql.append(account.begin(), account.getLength());
    sql += " LIMIT ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " OFFSET ";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += ';';
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordsToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::setAccountFollowAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    const auto &account = HTTPServerBoot::getAccount(connection);
    std::string res;
    if (argumentVector[1] == "0")
    {
        std::string sql = "DELETE FROM Account_Account WHERE account = ";
        sql.append(account.begin(), account.getLength());
        sql += " AND another = ";
        sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
        sql += " LIMIT 1;";
        SQLQuery query(sqlConnectionPool_.getConnection(), sql);
        res = std::to_string(query.numAffected());
    }
    else
    {
        std::string sql = "INSERT INTO Account_Account VALUE(";
        sql.append(account.begin(), account.getLength());
        sql += ", ";
        sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
        sql += ", ";
        sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
        sql += ") ON DUPLICATE KEY UPDATE `follow` = ";
        sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
        sql += ";";
        SQLQuery query(sqlConnectionPool_.getConnection(), sql);
        res = std::to_string(query.numAffected());
    }
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
    
}
void ForumServer::commentPost(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT * FROM Comment WHERE post = ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " LIMIT ";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += " OFFSET ";
    sql.append(argumentVector[2].begin(), argumentVector[2].getLength());
    sql += ';';
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordsToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::setCommentAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "INSERT INTO Comment(post, body, account, time) VALUE('";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += "', '";
    std::string body(argumentVector[1].begin(), argumentVector[1].getLength());
    SQLToJson::escapeCharacter(body);
    sql += body;
    sql += "', '";
    const auto &account = HTTPServerBoot::getAccount(connection);
    sql.append(account.begin(), account.getLength());
    sql += "', NOW());";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = std::to_string(query.numAffected());
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::post(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT DISTINCT ";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += ", `visible` AS b FROM Post WHERE id = ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " LIMIT 1;";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res;
    if (query)
    {
        StringView visible(query[0][query.numCol() - 1]);
        if (visible != "True")
        {
            res = "{ \"visible\": \"";
            res.append(visible.begin(), visible.getLength());
            res += "\" }";
        }
    }
    if (res.empty()) { res = SQLToJson::recordToJson(query); }
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::postPush(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    int limit = argumentVector[0].toInt();
    int offset = 0;
    offset = argumentVector[1].toInt();
    auto res = SQLToJson::columnToJson(*postPushQuery, limit, offset);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::postFollowAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT Post.* FROM Post, Account_Account "
    "WHERE Post.visible = 'True' AND Post.account = Account_Account.another "
    "AND Account_Account.account = ";
    const auto &account = HTTPServerBoot::getAccount(connection);
    sql.append(account.begin(), account.getLength());
    sql += " LIMIT ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " OFFSET ";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += ";";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordsToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::postLikeAndFavouritesAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT DISTINCT * FROM Account_Post WHERE account = ";
    const auto &account = HTTPServerBoot::getAccount(connection);
    sql.append(account.begin(), account.getLength());
    sql += " AND post = ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " LIMIT 1;";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::setPostLikeAndFavouritesAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    const auto &account = HTTPServerBoot::getAccount(connection);
    std::string sql;
    if (argumentVector[1] == "0" && argumentVector[2] == "0" && argumentVector[3] == "0")
    {
        sql = "DELETE FROM Account_Post WHERE account = ";
        sql.append(account.begin(), account.getLength());
        sql += " AND post = ";
        sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
        sql += " LIMIT 1;";
    }
    else
    {
        sql = "INSERT INTO Account_Post VALUE('";
        sql.append(account.begin(), account.getLength());
        sql += "', '";
        sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
        sql += "', '";
        sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
        sql += "', '";
        sql.append(argumentVector[2].begin(), argumentVector[2].getLength());
        sql += "', '";
        sql.append(argumentVector[3].begin(), argumentVector[3].getLength());
        sql += "') ON DUPLICATE KEY UPDATE `like` = '";
        sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
        sql += "', `dislike` = '";
        sql.append(argumentVector[2].begin(), argumentVector[2].getLength());
        sql += "', `favourites` = '";
        sql.append(argumentVector[3].begin(), argumentVector[3].getLength());
        sql += "';";
    }
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = std::to_string(query.numAffected());
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::postFavouritesAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT post FROM Account_Post WHERE account = ";
    const auto &account = HTTPServerBoot::getAccount(connection);
    sql.append(account.begin(), account.getLength());
    sql += " AND favourites = 1 LIMIT ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " OFFSET ";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += ";";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::columnToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::postAccount(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT * FROM Post "
    "WHERE visible = 'True' AND account = ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " ORDER BY time DESC LIMIT ";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += " OFFSET ";
    sql.append(argumentVector[2].begin(), argumentVector[2].getLength());
    sql += ";";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordsToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::postInvisibleAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT * FROM Post "
    "WHERE visible <> 'True' && visible <> 'Checking' AND account = ";
    const auto &account = HTTPServerBoot::getAccount(connection);
    sql.append(account.begin(), account.getLength());
    sql += " LIMIT ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " OFFSET ";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += ";";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordsToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::postCheckingAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT * FROM Post "
    "WHERE visible = 'Checking' AND account = ";
    const auto &account = HTTPServerBoot::getAccount(connection);
    sql.append(account.begin(), account.getLength());
    sql += " LIMIT ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " OFFSET ";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += ";";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordsToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::setPostVisibleAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string res;
    if (argumentVector[1] == "True" || argumentVector[1] == "Account")
    {
        std::string sql = "UPDATE Post SET `visible` = '";
        sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
        sql += "' WHERE id = ";
        sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
        sql += " AND (`visible` = 'Account' || `visible` = 'True') AND account = ";
        const auto &account = HTTPServerBoot::getAccount(connection);
        sql.append(account.begin(), account.getLength());
        sql += " LIMIT 1;";
        SQLQuery query(sqlConnectionPool_.getConnection(), sql);
        res = std::to_string(query.numAffected());
    }
    else
    {
        res += '0';
    }
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::setPostVisibleAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string res;
    if (argumentVector[1] == "True" || argumentVector[1] == "Administrator")
    {
        std::string sql = "UPDATE Post SET `visible` = '";
        sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
        sql += "' WHERE id = ";
        sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
        sql += " AND `visible` <> 'Account'  LIMIT 1;";
        SQLQuery query(sqlConnectionPool_.getConnection(), sql);
        res = std::to_string(query.numAffected());
    }
    else
    {
        res += '0';
    }
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::setPostAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string res;
    if (argumentVector[2] == "Text" || argumentVector[2] == "Image" || argumentVector[2] == "ImageAndTitle")
    {
        std::string sql = "INSERT INTO Post(`title`, `body`, `type`, `image`, `account`, `time`) VALUES('";
        std::string title(argumentVector[0].begin(), argumentVector[0].getLength());
        SQLToJson::escapeCharacter(title);
        sql += title; sql += "', '";
        std::string body(argumentVector[1].begin(), argumentVector[1].getLength());
        SQLToJson::escapeCharacter(body);
        sql += body; sql += "', '";
        sql.append(argumentVector[2].begin(), argumentVector[2].getLength());
        sql += "', '";
        std::string image(argumentVector[3].begin(), argumentVector[3].getLength());
        SQLToJson::escapeCharacter(image);
        sql += image; sql += "', '";
        const auto &account = HTTPServerBoot::getAccount(connection);
        sql.append(account.begin(), account.getLength());
        sql += "', NOW());";
        SQLQuery query(sqlConnectionPool_.getConnection(), sql);
        if (query)
        {
            std::string sql = "SELECT LAST_INSERT_ID();";
            SQLQuery query(sqlConnectionPool_.getConnection(), sql);
            if (query)
            {
                auto split = argumentVector[4].split(',');
                for (const auto &s : split)
                {
                    if (!s.bEmpty())
                    {
                        std::string sql = "INSERT INTO Post_Tag VALUE('";
                        sql += query[0][0]; sql += "', '";
                        sql.append(s.begin(), s.getLength()); sql += "');";
                        SQLQuery query(sqlConnectionPool_.getConnection(), sql);
                    }
                }
            }
        }
        res = std::to_string(query.numAffected());
    }
    else
    {
        res += '0';
    }
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::postTag(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT `post` FROM Post_Tag WHERE `tag` = (SELECT `id` FROM Tag WHERE `id` = '";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += "' AND `visible` = 'True') LIMIT ";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += " OFFSET ";
    sql.append(argumentVector[2].begin(), argumentVector[2].getLength());
    sql += ";";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::columnToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::tag(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT DISTINCT * FROM TagName WHERE `tag` = (SELECT `id` FROM Tag WHERE `id` = '";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += "' AND `visible` = 'True') LIMIT 1;";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::tagName(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT TagName.`tag`, TagName.`name` "
    "FROM TagName, Tag WHERE `name` LIKE '%";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += "%' AND Tag.`visible` = 'True' AND Tag.`id` = TagName.`tag` LIMIT 3;";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordsToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::tagPost(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT tag FROM Post_Tag, Tag WHERE Tag.id = Post_Tag.`tag` "
    "AND Tag.visible = 'True' AND post = ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += ";";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::columnToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::setTagAuthentication(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT DISTINCT * FROM TagName WHERE `name` = '";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += "' LIMIT 1;";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res;
    if (query)
    {
        res = SQLToJson::recordToJson(query);
    }
    else
    {
        std::string sql = "INSERT INTO Tag() VALUE();";
        SQLQuery query(sqlConnectionPool_.getConnection(), sql);
        if (query)
        {
            std::string sql = "SELECT LAST_INSERT_ID();";
            SQLQuery idQuery(sqlConnectionPool_.getConnection(), sql);
            if (idQuery)
            {
                std::string sql = "INSERT INTO TagName(`name`, `tag`) VALUE('";
                sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
                sql += "', ";
                sql += idQuery[0][0];
                sql += ");";
                SQLQuery query(sqlConnectionPool_.getConnection(), sql);
                if (query)
                {
                    res = "{ \"tag\": \"";
                    int rest = 10 - strlen(idQuery[0][0]);
                    for (int _i = 0; _i < rest; ++_i)
                    {
                        res += '0';
                    }
                    res += idQuery[0][0];
                    res += "\", \"name\": \"";
                    res.append(argumentVector[0].begin(), argumentVector[0].getLength());
                    res += "\" }";
                }
            }
        }
    }
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::postCheckingAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT * FROM Post WHERE `visible` = 'Checking' LIMIT 1;";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::joinTagAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "START TRANSACTION; UPDATE TagName SET tag = ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += " WHERE tag = ";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += "; UPDATE Post_Tag a SET tag = '";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += "' WHERE tag = '";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += "' AND NOT EXISTS (SELECT * FROM Post_Tag b WHERE b.post = a.post AND b.tag = '";//"); DELETE FROM Tag WHERE id = ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += "'); DELETE FROM Post_Tag WHERE tag = '";//" COMMIT;";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += "';DELETE FROM Tag WHERE id = '";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += "'; COMMIT;";
    std::cout << sql << std::endl;
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = std::to_string(query.numAffected());
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::setTagVisibleAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string res;
    if (argumentVector[1] == "True" || argumentVector[1] == "Administrator")
    {
        std::string sql = "UPDATE Tag SET `visible` = '";
        sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
        sql += "' WHERE id = ";
        sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
        sql += ";";
        SQLQuery query(sqlConnectionPool_.getConnection(), sql);
        res = std::to_string(query.numAffected());
    }
    else
    {
        res += '0';
    }
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::tagPostAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT tag FROM Post_Tag, Tag WHERE Tag.id = Post_Tag.`tag` AND post = ";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += ";";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::columnToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::tagAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "SELECT DISTINCT TagName.tag, TagName.name, Tag.visible FROM TagName, Tag WHERE TagName.`tag` = '";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += "' AND Tag.`id` = TagName.`tag` LIMIT 1;";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = SQLToJson::recordToJson(query);
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
void ForumServer::setPostSizeAdministrator(const std::shared_ptr<TCPConnection> &connection, const std::vector<StringView> &argumentVector)
{
    std::string sql = "UPDATE Post SET `size` = '";
    sql.append(argumentVector[1].begin(), argumentVector[1].getLength());
    sql += "' WHERE `id` = '";
    sql.append(argumentVector[0].begin(), argumentVector[0].getLength());
    sql += "';";
    SQLQuery query(sqlConnectionPool_.getConnection(), sql);
    std::string res = std::to_string(query.numAffected());
    HTTPBuilder builder(res.size() + 128);
    builder.writeStartLine("HTTP/1.1", "200", "Omitted.");
    builder.writeHeader("Content-Type", "text/plain; charset=UTF-8");
    builder.writeEntityBody(res);
    connection->send(builder.getBuffer());
}
ForumServer::ForumServer(EventLoop *eventLoop, const InetAddress &listenAddr, 
const InetAddress &sqlHost, const char *user, const char *pw, const char *db, 
const std::string &name, int numThread, int numSQLConnection, TCPServer::EOption option) : 
server_(eventLoop, listenAddr, name, numThread, option), 
sqlConnectionPool_(sqlHost, user, pw, db, numSQLConnection)
{
    server_.functionMap2_["login"] = HTTPServerBoot::FunctionControl(2, "", std::bind(&ForumServer::login, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["file"] = HTTPServerBoot::FunctionControl(1, "", std::bind(&ForumServer::file, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["uploadPostImageAuthentication"] = HTTPServerBoot::FunctionControl(1, "2", std::bind(&ForumServer::uploadPostImageAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["accountClothing"] = HTTPServerBoot::FunctionControl(1, "", std::bind(&ForumServer::accountClothing, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["accountClothingAuthentication"] = HTTPServerBoot::FunctionControl(0, "2", std::bind(&ForumServer::accountClothingAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["accountFollowAuthentication"] = HTTPServerBoot::FunctionControl(0, "2", std::bind(&ForumServer::accountFollowAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["accountFollowedAuthentication"] = HTTPServerBoot::FunctionControl(2, "2", std::bind(&ForumServer::accountFollowedAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["setAccountFollowAuthentication"] = HTTPServerBoot::FunctionControl(2, "2", std::bind(&ForumServer::setAccountFollowAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["commentPost"] = HTTPServerBoot::FunctionControl(3, "", std::bind(&ForumServer::commentPost, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["setCommentAuthentication"] = HTTPServerBoot::FunctionControl(2, "2", std::bind(&ForumServer::setCommentAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["post"] = HTTPServerBoot::FunctionControl(2, "", std::bind(&ForumServer::post, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["postPush"] = HTTPServerBoot::FunctionControl(2, "", std::bind(&ForumServer::postPush, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["postFollowAuthentication"] = HTTPServerBoot::FunctionControl(2, "2", std::bind(&ForumServer::postFollowAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["postLikeAndFavouritesAuthentication"] = HTTPServerBoot::FunctionControl(1, "2", std::bind(&ForumServer::postLikeAndFavouritesAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["setPostLikeAndFavouritesAuthentication"] = HTTPServerBoot::FunctionControl(4, "2", std::bind(&ForumServer::setPostLikeAndFavouritesAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["postFavouritesAuthentication"] = HTTPServerBoot::FunctionControl(2, "2", std::bind(&ForumServer::postFavouritesAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["postAccount"] = HTTPServerBoot::FunctionControl(3, "", std::bind(&ForumServer::postAccount, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["postInvisibleAuthentication"] = HTTPServerBoot::FunctionControl(2, "2", std::bind(&ForumServer::postInvisibleAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["postCheckingAuthentication"] = HTTPServerBoot::FunctionControl(2, "2", std::bind(&ForumServer::postCheckingAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["setPostVisibleAuthentication"] = HTTPServerBoot::FunctionControl(2, "2", std::bind(&ForumServer::setPostVisibleAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["setPostVisibleAdministrator"] = HTTPServerBoot::FunctionControl(2, "3", std::bind(&ForumServer::setPostVisibleAdministrator, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["setPostAuthentication"] = HTTPServerBoot::FunctionControl(5, "2", std::bind(&ForumServer::setPostAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["postTag"] = HTTPServerBoot::FunctionControl(3, "", std::bind(&ForumServer::postTag, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["tag"] = HTTPServerBoot::FunctionControl(1, "", std::bind(&ForumServer::tag, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["tagName"] = HTTPServerBoot::FunctionControl(1, "", std::bind(&ForumServer::tagName, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["tagPost"] = HTTPServerBoot::FunctionControl(1, "", std::bind(&ForumServer::tagPost, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["setTagAuthentication"] = HTTPServerBoot::FunctionControl(1, "2", std::bind(&ForumServer::setTagAuthentication, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["postCheckingAdministrator"] = HTTPServerBoot::FunctionControl(0, "3", std::bind(&ForumServer::postCheckingAdministrator, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["tagPostAdministrator"] = HTTPServerBoot::FunctionControl(1, "3", std::bind(&ForumServer::tagPostAdministrator, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["tagAdministrator"] = HTTPServerBoot::FunctionControl(1, "3", std::bind(&ForumServer::tagAdministrator, this, std::placeholders::_1, std::placeholders::_2));

    server_.functionMap2_["joinTagAdministrator"] = HTTPServerBoot::FunctionControl(2, "3", std::bind(&ForumServer::joinTagAdministrator, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["setTagVisibleAdministrator"] = HTTPServerBoot::FunctionControl(2, "3", std::bind(&ForumServer::setTagVisibleAdministrator, this, std::placeholders::_1, std::placeholders::_2));
    server_.functionMap2_["setPostSizeAdministrator"] = HTTPServerBoot::FunctionControl(2, "3", std::bind(&ForumServer::setPostSizeAdministrator, this, std::placeholders::_1, std::placeholders::_2));
    updatePostPushQuery();
    server_.setAuthentication(std::bind(&ForumServer::authentication, this, std::placeholders::_1));//setCommentAuthentication
}
}
