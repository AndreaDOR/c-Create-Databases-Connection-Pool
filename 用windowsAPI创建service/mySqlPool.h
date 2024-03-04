#pragma once
#include "public.h"
#include "Connect.h"
#include <queue>
#include <mutex>
#include <string>
#include <atomic>
#include <memory>
#include <functional>
#include <condition_variable>

//因为数据库连接池子只有一个，所以我们采用单例模式

class mySqlPool {
public:
	//获取连接池对象实例
	static mySqlPool* getMySqlPool();
	std::shared_ptr<Connection> getConnection();//从连接池获取一个可用的空闲连接
private:
	mySqlPool();//构造函数私有化
	bool loadConfigFile();//从配置文件中加载配置项
	void produceConnectionTask(); //运行在独立的线程中，专门负责生产新连接
	//扫描超过maxIdleTime时间的空闲连接，进行队列的连接回收
	void scannerConnectionTask();

	std::string _ip;//mysql的ip地址
	std::string _dbname;//数据库的名称
	unsigned short _port; //mysql端口号3306
	std::string _username;//mysql用户名
	std::string _password;//mysql登陆密码
	int _initSize;//连接池的初始连接量
	int _maxSize;//连接池的最大连接量
	int _maxIdleTime;//连接池最大空闲时间
	int _connectionTimeOut;//连接池获取连接的超时时间

	std::queue<Connection*> _connectionQue;//存储mysql连接队列
	std::mutex _queueMutex; //维护连接队列的线程安全互斥锁
	std::atomic_int _connectionCnt; //记录连接所创建的connect的数量
	std::condition_variable cv;//设置条件变量，用于生产者线程和消费者线程的通信
};


 

