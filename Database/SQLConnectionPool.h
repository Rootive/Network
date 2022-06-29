#ifndef ROOTIVE_SQLCONNECTIONPOOL_H
#define ROOTIVE_SQLCONNECTIONPOOL_H

#include <vector>
#include <memory>
#include <mutex>
#include "Database/SQLConnection.h"
#include "Basics/Noncopyable.h"

namespace Rootive
{
class SQLConnectionPool : Noncopyable
{
    std::vector<std::unique_ptr<SQLConnection> > connectionVector_;
    int next_;
    std::mutex mutex_;
public:
    SQLConnectionPool(const InetAddress &host, const char *user, const char *pw, const char *db, int numConnection = 1) :
    next_(0)
    {
        for (int _i = 0; _i < numConnection; ++_i)
        {
            connectionVector_.push_back(std::unique_ptr<SQLConnection>(new SQLConnection(host, user, pw, db, nullptr, CLIENT_MULTI_STATEMENTS)));
        }
    }

    SQLConnection *getConnection() 
    { 
        SQLConnection *ret = nullptr;
        {
        std::lock_guard<std::mutex> guard(mutex_);
        if (!connectionVector_.empty())
        {
            ret = connectionVector_[next_++].get();
            next_ %= connectionVector_.size();
        }
        }
        return ret;
    }
};
}

#endif