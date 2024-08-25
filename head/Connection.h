/**
 * @Description 实现数据库连接操作
 * @Version 1.0.0
 * @Date 2024/8/19 23:05
 * @Github https://github.com/Programmer-Kenton
 * @Author Kenton
 */
#ifndef DATABASEPOOL_CONNECTION_H
#define DATABASEPOOL_CONNECTION_H

#include <iostream>
#include <ctime>
#include "public.h"
#include "mysql.h"

using namespace std;

class Connection {

public:
    // 初始化数据库连接
    Connection();

    // 释放数据库连接
    ~Connection();

    // 连接数据库
    bool connect(string ip,unsigned short port,string user,string password,string dbname);

    // 更新操作
    bool update(string sql);

    // 查询操作
    MYSQL_RES *query(string sql);

    // 刷新链接的起始的空闲时间点
    void refreshAliveTime();

    // 返回存活的时间
    clock_t getAliceTime() const;

private:
    // 表示和MYSQL Server的一条连接
    MYSQL *_conn;

    // 记录进入空闲状态后的起始存活时间
    clock_t _alivetime;
};


#endif //DATABASEPOOL_CONNECTION_H
