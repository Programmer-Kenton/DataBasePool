/**
 * @Description TODO
 * @Version 1.0.0
 * @Date 2024/8/19 23:05
 * @Github https://github.com/Programmer-Kenton
 * @Author Kenton
 */
#include "../head/Connection.h"

Connection::Connection() {
    // 初始化数据库连接
    _conn = mysql_init(nullptr);
}

Connection::~Connection() {
    if (_conn != nullptr){
        mysql_close(_conn);
    }
}

bool Connection::connect(string ip, unsigned short port, string user, string password, string dbname) {
    MYSQL *p = mysql_real_connect(_conn,ip.c_str(),user.c_str(),password.c_str(),dbname.c_str(),port, nullptr,0);
    return p != nullptr;
}

bool Connection::update(string sql) {
    if (mysql_query(_conn,sql.c_str())){
        LOG("更新失败:" + sql);
        return false;
    }
    return true;
}

MYSQL_RES *Connection::query(string sql) {
    if (mysql_query(_conn,sql.c_str())){
        LOG("查询失败:" + sql);
        return nullptr;
    }

    return mysql_use_result(_conn);
}

void Connection::refreshAliveTime() {
    _alivetime = clock();
}

clock_t Connection::getAliceTime() const{
    return clock() - _alivetime;
}
