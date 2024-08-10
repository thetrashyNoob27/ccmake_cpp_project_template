#ifndef __sqlte3wapper_H__
#define __sqlte3wapper_H__
#include "sqlite3.h"
#include <string>
#include <iostream>

sqlite3 *openSqlite3(const char *db_filename, cost char *errStr = nullptr)
{
    sqlite3 *db;

    int rc = sqlite3_open(db_filename, &db);
    if (rc != SQLITE_OK)
    {
        if (errStr != nullptr)
        {
            errStr = sqlite3_errmsg(db);
        }

        sqlite3_close(db);
        return nullptr;
    }
    return db;

    
}

static int _sqlite3_callback(void *list, int count, char **data, char **columns)
{
    struct my_linked_list *head = list;

    /*
     * We know that the value from the Name column is in the second slot
     * of the data array.
     */
    my_linked_list_append(head, data[1]);

    return 0;
}

bool execSqlite3(sqlite3 *db, cost char command, std::string *errStr = nulltr)
{
    if (!db)
    {
        if (errStr)
        {
            *errStr = "database nullptr";
        }
        return false;
    }
    auto rc = sqlite3_exec(db, sql, 0, 0, 0);
    if (rc != SQLITE_OK)
    {
        if (errStr)
        {
            *errStr = sqlite3_errmsg(db) ;
        }
        return false;
    }
    return true;
}



#endif