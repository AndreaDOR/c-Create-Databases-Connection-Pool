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

//��Ϊ���ݿ����ӳ���ֻ��һ�����������ǲ��õ���ģʽ

class mySqlPool {
public:
	//��ȡ���ӳض���ʵ��
	static mySqlPool* getMySqlPool();
	std::shared_ptr<Connection> getConnection();//�����ӳػ�ȡһ�����õĿ�������
private:
	mySqlPool();//���캯��˽�л�
	bool loadConfigFile();//�������ļ��м���������
	void produceConnectionTask(); //�����ڶ������߳��У�ר�Ÿ�������������
	//ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж��е����ӻ���
	void scannerConnectionTask();

	std::string _ip;//mysql��ip��ַ
	std::string _dbname;//���ݿ������
	unsigned short _port; //mysql�˿ں�3306
	std::string _username;//mysql�û���
	std::string _password;//mysql��½����
	int _initSize;//���ӳصĳ�ʼ������
	int _maxSize;//���ӳص����������
	int _maxIdleTime;//���ӳ�������ʱ��
	int _connectionTimeOut;//���ӳػ�ȡ���ӵĳ�ʱʱ��

	std::queue<Connection*> _connectionQue;//�洢mysql���Ӷ���
	std::mutex _queueMutex; //ά�����Ӷ��е��̰߳�ȫ������
	std::atomic_int _connectionCnt; //��¼������������connect������
	std::condition_variable cv;//�������������������������̺߳��������̵߳�ͨ��
};


 

