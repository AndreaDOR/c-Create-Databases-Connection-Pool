#include <iostream>

#include "mySqlPool.h"
#include "CommonConnectionPool.h"

 
void f(int n)
{
	ConnectionPool* cp = ConnectionPool::getConnectionPool();
	for (int i = 1; i <= n; ++i)
	{
		std::shared_ptr<Connection> sp = cp->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
		sp->update(sql);
	}
}
//测试连接池连接
void testSqlPool(int n)
{
	clock_t begin = clock();
	std::thread t1(f, n);
	t1.join();
	clock_t end = clock();
	std::cout << "单线程采用数据库连接池，连接数量为:" << n << "的sql执行时间:" << (end - begin) << "ms" << std::endl;
}
//测试连接池连接 4线程
void testSqlPool4(int n)
{
	int n2 = n / 4;
	clock_t begin = clock();
	std::thread t1(f, n2);
	std::thread t2(f, n2);
	std::thread t3(f, n2);
	std::thread t4(f, n2);
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	clock_t end = clock();
	std::cout << "四线程采用数据库连接池，连接数量为:" << n << "的sql执行时间:" << (end - begin) << "ms" << std::endl;
}

//非线程池的连接
void testSql( int n)
{
	clock_t begin = clock();
	std::thread t([&n]() {
		for (int i = 1; i < n; ++i)
		{
			Connection cnn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')", "zhang san", 20, "male");
			cnn.connect("127.0.0.1", 3306, "root", "123456", "chat");
			cnn.update(sql);
		}});
	t.join();
	clock_t end = clock();
	std::cout << "普通连接数量为:" << n << "的sql执行时间:" << (end - begin) << "ms" << std::endl;
}