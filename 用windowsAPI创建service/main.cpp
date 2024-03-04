#include <iostream>
#include "Connect.h"
#include "mySqlPool.h"
#include "test.h"


int main()
{
	//testSql(100);
	//普通连接数量为 : 10的sql执行时间 : 2838ms
	//普通连接数量为 : 100的sql执行时间: 12299
	//普通连接数量为 : 1000的sql执行时间 : 104528ms
	// 
	//testSqlPool(100);
	//单线程 采用数据库连接池，连接数量为:10的sql执行时间:1745ms
	//单线程 采用数据库连接池，连接数量为:100的sql执行时间:9779ms 
    //单线程 采用数据库连接池，连接数量为:1000的sql执行时间 : 86016ms

	testSqlPool4(1000);
	//4条线程 采用数据库连接池，连接数量为:100的sql执行时间 : 3715ms
    //4条线程 采用数据库连接池，连接数量为:1000的sql执行时间 : 34686ms
	return 0;
}