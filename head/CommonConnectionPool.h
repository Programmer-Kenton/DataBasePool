/**
 * @Description 实现连接池功能模块
 * @Version 1.0.0
 * @Date 2024/8/19 23:05
 * @Github https://github.com/Programmer-Kenton
 * @Author Kenton
 */
#ifndef DATABASEPOOL_COMMONCONNECTIONPOOL_H
#define DATABASEPOOL_COMMONCONNECTIONPOOL_H

#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
#include "public.h"
#include "Connection.h"

using namespace std;

class CommonConnectionPool {

public:
    // 获取连接池对象实例
    static CommonConnectionPool* getConnectionPool();

    // 从外部提供接口 从连接池中获取一个可用的空闲连接
    shared_ptr<Connection> getConnection();


private:
    // 单例 构造函数私有化
    CommonConnectionPool();

    // 从配置文件中加载配置项
    bool loadConfigFile();

    // mysql的ip地址
    string _ip;

    // mysql的端口号
    unsigned short _port;

    // 数据库连接用户
    string _username;

    // mysql连接密码
    string _password;

    // 连接的数据库名
    string _dbname;

    // 连接池的初始连接量
    int _initSize;

    // 连接池的最大连接量
    int _maxSize;

    // 连接池最大空闲时间
    int _maxIdleTime;

    // 连接池获取连接的超时时间
    int _connectionTimeout;

    // 存储mysql连接的队列
    queue<Connection*> _connectionQue;

    // 维护连接队列的线程安全互斥锁
    mutex _queueMutex;

    // 记录连接所创建的connection连接的总数量
    atomic_int _connectionCnt;

    // 运行在独立的线程中 专门负责生产新连接
    void produceConnectionTask();

    // 扫描超过maxIdleTime时间的空闲连接 进行连接回收
    void scannerConnectionTask();

    // 设置条件变量 用于连接生产线程和连接消费线程的通信
    condition_variable cv;
};


#endif //DATABASEPOOL_COMMONCONNECTIONPOOL_H
