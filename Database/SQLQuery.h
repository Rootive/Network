#ifndef ROOTIVE_SQLQUERY
#define ROOTIVE_SQLQUERY

#include <mariadb/mysql.h>

#include "Basics/Noncopyable.h"
#include "Basics/StringView.h"
#include "Database/SQLConnection.h"
#include "Log/LogLine.h"

#include <iostream>

namespace Rootive
{
class SQLQuery : Noncopyable
{
    MYSQL_RES *res_;
    my_ulonglong affected_;
public:

    SQLQuery(SQLConnection *connection, const char *sql, unsigned long length) :
    affected_(0)
    {
        ROOTIVE_LOG_DEBUG << StringView(sql, length);
        std::lock_guard<std::mutex> lock(connection->mutex_);
        mysql_real_query(connection->mysql_, sql, length);
        res_ = mysql_store_result(connection->mysql_);
        auto tmp = mysql_affected_rows(connection->mysql_);
        affected_ += (tmp == (my_ulonglong)-1 ? 0 : tmp);
        while (mysql_more_results(connection->mysql_))
        {
            mysql_free_result(res_);
            mysql_next_result(connection->mysql_);
            res_ = mysql_store_result(connection->mysql_);
            tmp = mysql_affected_rows(connection->mysql_);
            affected_ += (tmp == (my_ulonglong)-1 ? 0 : tmp);
        }
    }
    SQLQuery(SQLConnection *connection, const StringView &sql) :
    SQLQuery(connection, sql.begin(), sql.getLength())
    {}

    ~SQLQuery() { mysql_free_result(res_); }

    inline my_ulonglong numRow() const { return res_ ? mysql_num_rows(res_) : 0; }
    inline my_ulonglong numCol() const { return res_ ? mysql_num_fields(res_) : 0; }
    inline my_ulonglong numAffected() const { return affected_; }
    inline const char *colName(unsigned long long index) const 
    { 
        mysql_field_seek(res_, index);
        return mysql_fetch_field(res_)->name; 
    }

    MYSQL_ROW operator[](unsigned long long index)
    { 
        mysql_data_seek(res_, index);
        return mysql_fetch_row(res_);
    }
    operator bool() const
    {
        return numAffected() + numRow();
    }
};
}

#endif