#include "mySqlPool.h"

//获取线程池
mySqlPool* mySqlPool::getMySqlPool()
{
	static mySqlPool pool;
	return &pool;
}
mySqlPool::mySqlPool()
{
	if (!loadConfigFile())
	{
		LOG("load Config File is error!");
		return;
	}
	 
	//创建初始数量的连接
	for (int i = 0; i < _initSize; ++i)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
	}
	//启动一个新线程，作为连接的生产者
	std::thread produce(std::bind(&mySqlPool::produceConnectionTask, this));
	produce.detach();
	//启动一个新线程，作为空闲连接超时的回收者
	std::thread scanner(std::bind(&mySqlPool::scannerConnectionTask, this));
	scanner.detach();

}


//加载配置文件
bool mySqlPool::loadConfigFile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exits!");
		return false;
	}
	while (!feof(pf)) //遍历配置文件
	{
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		std::string str = line;
		int idx = str.find('=', 0); //从0开始找'='符号的位置
		if (idx == -1)continue;
		int endidx = str.find('\n', idx);//从idx寻找'\n'的位置，也就是末尾
		std::string key = str.substr(0, idx); //获取配置文件中=号左边的key
		//从等号后到末尾，刚好是value的string形式
		std::string value = str.substr(idx + 1, endidx - idx - 1);
		if (key == "ip")
		{
			_ip = value;
		}
		else if (key == "port")
		{
			//字符串转换成unsigned short
			_port = static_cast<unsigned short>(std::stoul(value));
		}
		else if (key == "username")
		{
			_username = value;
		}
		else if (key == "password")
		{
			_password = value;
		}
		else if (key == "dbname")
		{
			_dbname = value;
		}
		else if (key == "initSize")
		{
			_initSize = std::stoi(value); 
		}
		else if (key == "maxSize")
		{
			_maxSize = std::stoi(value); 
		}
		else if (key == "maxIdleTime")
		{
			_maxIdleTime = std::stoi(value); 
		}
		else if (key == "connectionTimeOut")
		{
			_connectionTimeOut = std::stoi(value); 
		}
	}
	return true;
}

//从连接池获取一个可用的空闲连接
std::shared_ptr<Connection> mySqlPool::getConnection()
{
	std::unique_lock<std::mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		//如果超时没有获取可用的空闲连接返回空
		if (std::cv_status::timeout == cv.wait_for(lock, std::chrono::milliseconds(100)))

			if (_connectionQue.empty())
			{
				LOG("get Connection error");
				return nullptr;
			}

	}
	std::shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection* pcon) {
		//保证只能同一时刻只能有一个线程归还连接给队列
		std::unique_lock<std::mutex> lock(_queueMutex);
		pcon->refreshAliveTime();//创建的时候刷新存活时间
		_connectionQue.push(pcon);
		});
	_connectionQue.pop();
	cv.notify_all();

	return sp;
}
//运行在独立的线程中，专门负责生产新连接
void mySqlPool::produceConnectionTask()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(_queueMutex);
		while (!_connectionQue.empty())
			cv.wait(lock);     //队列不为空不生产线程

		//没有到上线就可以生产线程
		if (_connectionCnt < _maxSize)
		{
			auto p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();//创建的时候刷新存活时间
			_connectionQue.push(p);
			++_connectionCnt;
		}
		cv.notify_all();
	}
}

//扫描超过maxIdleTime时间的空闲连接，进行队列的连接回收
void mySqlPool::scannerConnectionTask()
{
	while (true)
	{
		//通过sleep模拟定时效果，每_maxIdleTime检查一次
		std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
		
		//扫描整个队列释放多余的超时连接
		std::unique_lock<std::mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			auto p = _connectionQue.front();
			if (p->getAliveeTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				delete p;//这里会调用智能指针，回收到队列中
			}
		}
	}
}