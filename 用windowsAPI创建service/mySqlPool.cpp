#include "mySqlPool.h"

//��ȡ�̳߳�
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
	 
	//������ʼ����������
	for (int i = 0; i < _initSize; ++i)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
	}
	//����һ�����̣߳���Ϊ���ӵ�������
	std::thread produce(std::bind(&mySqlPool::produceConnectionTask, this));
	produce.detach();
	//����һ�����̣߳���Ϊ�������ӳ�ʱ�Ļ�����
	std::thread scanner(std::bind(&mySqlPool::scannerConnectionTask, this));
	scanner.detach();

}


//���������ļ�
bool mySqlPool::loadConfigFile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exits!");
		return false;
	}
	while (!feof(pf)) //���������ļ�
	{
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		std::string str = line;
		int idx = str.find('=', 0); //��0��ʼ��'='���ŵ�λ��
		if (idx == -1)continue;
		int endidx = str.find('\n', idx);//��idxѰ��'\n'��λ�ã�Ҳ����ĩβ
		std::string key = str.substr(0, idx); //��ȡ�����ļ���=����ߵ�key
		//�ӵȺź�ĩβ���պ���value��string��ʽ
		std::string value = str.substr(idx + 1, endidx - idx - 1);
		if (key == "ip")
		{
			_ip = value;
		}
		else if (key == "port")
		{
			//�ַ���ת����unsigned short
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

//�����ӳػ�ȡһ�����õĿ�������
std::shared_ptr<Connection> mySqlPool::getConnection()
{
	std::unique_lock<std::mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		//�����ʱû�л�ȡ���õĿ������ӷ��ؿ�
		if (std::cv_status::timeout == cv.wait_for(lock, std::chrono::milliseconds(100)))

			if (_connectionQue.empty())
			{
				LOG("get Connection error");
				return nullptr;
			}

	}
	std::shared_ptr<Connection> sp(_connectionQue.front(), [&](Connection* pcon) {
		//��ֻ֤��ͬһʱ��ֻ����һ���̹߳黹���Ӹ�����
		std::unique_lock<std::mutex> lock(_queueMutex);
		pcon->refreshAliveTime();//������ʱ��ˢ�´��ʱ��
		_connectionQue.push(pcon);
		});
	_connectionQue.pop();
	cv.notify_all();

	return sp;
}
//�����ڶ������߳��У�ר�Ÿ�������������
void mySqlPool::produceConnectionTask()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(_queueMutex);
		while (!_connectionQue.empty())
			cv.wait(lock);     //���в�Ϊ�ղ������߳�

		//û�е����߾Ϳ��������߳�
		if (_connectionCnt < _maxSize)
		{
			auto p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime();//������ʱ��ˢ�´��ʱ��
			_connectionQue.push(p);
			++_connectionCnt;
		}
		cv.notify_all();
	}
}

//ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж��е����ӻ���
void mySqlPool::scannerConnectionTask()
{
	while (true)
	{
		//ͨ��sleepģ�ⶨʱЧ����ÿ_maxIdleTime���һ��
		std::this_thread::sleep_for(std::chrono::seconds(_maxIdleTime));
		
		//ɨ�����������ͷŶ���ĳ�ʱ����
		std::unique_lock<std::mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			auto p = _connectionQue.front();
			if (p->getAliveeTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				delete p;//������������ָ�룬���յ�������
			}
		}
	}
}