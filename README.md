# 用C++创建一个简单的数据库连接池子
我的这个项目是基于MySql8.0版本的mysql接口实现的，使用的开发环境是VS2022

如果你需要用代码，你需要，在VS上需要进行相应的头文件和库文件的配置，如下：
* 1.右键项目 - C/C++ - 常规 - 附加包含目录，填写mysql.h头文件的路径
* 2.右键项目 - 链接器 - 常规 - 附加库目录，填写libmysql.lib的路径
* 3.右键项目 - 链接器 - 输入 - 附加依赖项，填写libmysql.lib库的名字
* 4.把libmysql.dll动态链接库（Linux下后缀名是.so库）放在工程目录下

  我的项目里面的数据库名称是：chat，使用的数据表user，
  使用sql就可以创建好了
  ```
CREATE DATABASE chat;
use chat;
CREATE TABLE user (
       id INT(11) NOT NULL AUTO_INCREMENT,
       name VARCHAR(50) NOT NULL,
       age INT(11) NOT NULL,
        sex ENUM('male', 'female') NOT NULL,
        PRIMARY KEY (id)
      );
  ```
