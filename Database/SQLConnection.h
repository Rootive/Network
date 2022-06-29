#ifndef ROOTIVE_SQLCONNECTION_H
#define ROOTIVE_SQLCONNECTION_H

#include <mariadb/mysql.h>
#include <mutex>
#include "Basics/Noncopyable.h"
#include "Network/InetAddress.h"

namespace Rootive
{
class SQLQuery;
class SQLConnection : Noncopyable
{
    friend class SQLQuery;

    MYSQL *mysql_;
    std::mutex mutex_;
public:
    SQLConnection(const InetAddress &host, const char *user, const char *pw, const char *db, const char *unixSocket, unsigned long clientFlag)
    {
        mysql_ = mysql_init(nullptr);
        if (!mysql_real_connect(mysql_, host.toIPString().c_str(), user, pw, db, host.getPort_HostEndian(), unixSocket, clientFlag)) { throw ; }
    }
    ~SQLConnection() { mysql_close(mysql_); }
    
};
}

#endif