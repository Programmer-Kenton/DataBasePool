/**
 * @Description TODO
 * @Version 1.0.0
 * @Date 2024/8/19 23:05
 * @Github https://github.com/Programmer-Kenton
 * @Author Kenton
 */

#include "../head/CommonConnectionPool.h"

// 线程安全的懒汉单例函数接口
CommonConnectionPool *CommonConnectionPool::getConnectionPool() {
    static CommonConnectionPool pool;
    return &pool;
}

bool CommonConnectionPool::loadConfigFile() {
    FILE *pf = fopen("../mysql.ini","r");
    if (pf == nullptr){
        LOG("mysql.ini file is not exist");
        return false;
    }

    while (!feof(pf)){
        char line[1024] = {0};
        fgets(line,1024,pf);
        string str = line;
        int idx = str.find('=',0);
        // 无效的配置项
        if (idx == -1){
            continue;
        }
        // password=123456\n
        int endidx = str.find('\n',idx);
        string key = str.substr(0,idx);
        string value = str.substr(idx+1, endidx-idx-1);

        // cout << endl;
        if (key == "ip"){
            _ip = value;
        }else if (key == "port"){
            _port = atoi(value.c_str());
        }else if (key == "username"){
            _username = value;
        }else if (key == "initSize"){
            _initSize = atoi(value.c_str());
        }else if (key == "maxSize"){
            _maxSize = atoi(value.c_str());
        }else if (key == "maxIdleTime"){
            _maxIdleTime = atoi(value.c_str());
        }else if (key == "connectionTimeout"){
            _connectionTimeout = atoi(value.c_str());
        }else if (key == "dbname"){
            _dbname = key;
        }
    }
    return true;
}

CommonConnectionPool::CommonConnectionPool() {
    // 加载数据库连接配置项
    if (!loadConfigFile()){
        return;
    }

    // 创建初始数量的连接
    for (int i = 0; i < _initSize; ++i) {
        Connection *p = new Connection();
        p->connect(_ip,_port,_username,_password,_dbname);
        // 刷新开始空闲的起始时间
        p->refreshAliveTime();
        _connectionQue.push(p);
        _connectionCnt++;
    }

    // 启动新的线程作为连接生产者
    thread produce(std::bind(&CommonConnectionPool::produceConnectionTask,this));
    // 分离线程 使其成为一个独立的线程，不再受原线程对象的控制
    // 这意味着脱离后的线程会独立运行，直到完成其任务，而不需要等待主线程或任何其他线程对其进行 join 操作
    // 一旦一个线程被 detach，就没有办法知道它何时结束，也无法获取它的退出状态
    produce.detach();

    // 启动新的线程 扫描多余的空闲连接超过maxIdleTime的空闲连接 进行连接回收
    thread scanner(std::bind(&CommonConnectionPool::scannerConnectionTask,this));
    produce.detach();
}

shared_ptr<Connection> CommonConnectionPool::getConnection() {
    unique_lock<mutex> lock(_queueMutex);
    while (_connectionQue.empty()){
        // 等待连接超时时间
        if (cv_status::timeout == cv.wait_for(lock,chrono::milliseconds(_connectionTimeout))){
            if (_connectionQue.empty()){
                LOG("获取空闲连接超时...获取连接失败\n");
                return nullptr;
            }
        }
    }

    /**
     * 智能指针析构时会把connection资源直接delete掉
     * 这里需要自定义shared_ptr释放资源的方式 把connection直接归还到queue当中
     */
    shared_ptr<Connection> sp(_connectionQue.front(),
                              [&](Connection *pcon){
        // 保障线程安全
        unique_lock<mutex> lock(_queueMutex);
        pcon->refreshAliveTime();
        _connectionQue.push(pcon);
    });
    _connectionQue.pop();
    cv.notify_all();
    return sp;
}

void CommonConnectionPool::produceConnectionTask() {
    for(;;){

        unique_lock<mutex> lock(_queueMutex);

        while(!_connectionQue.empty()){
            // 队列不空 此处生产线程进入等待状态
            cv.wait(lock);
        }

        // 连接数量没有到底上线继续创建
        if (_connectionCnt < _maxSize){
            Connection *p = new Connection();
            p->connect(_ip,_port,_username,_password,_dbname);
            _connectionQue.push(p);
            p->refreshAliveTime();
            _connectionCnt++;
        }

        // 通知消费者线程 可以消费连接
        cv.notify_all();
    }
}

void CommonConnectionPool::scannerConnectionTask() {
    for(;;){
        // 通过sleep模拟定时效果
        this_thread::sleep_for(chrono::seconds(_maxIdleTime));
        // 扫描整个队列 释放多余的连接
        unique_lock<mutex> lock(_queueMutex);
        while (_connectionCnt > _initSize){
            Connection *p = _connectionQue.front();
            if (p->getAliceTime() > (_maxIdleTime * 1000)){
                _connectionQue.pop();
                _connectionCnt--;
                delete p;
            }else{
                // 队头的连接没有超过_maxIdleTime * 1000 其他连接肯定没有
                break;
            }
        }
    }
}
