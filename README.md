# TinyWebServer

Linux下的C++ Web服务器：
* 使用 **线程池** + **非阻塞socket + epoll（支持LT与ET两种模式）**
* 利用**RAII**机制管理数据库连接，减少数据库连接创建于关闭开销
* 事件处理支持 **Reactor 与 Procator（使用Reactor模拟）**的并发模型
* 使用 **状态机** 解析HTTP报文（`http1.1`） 支持`GET` 与 `POST` 请求
* 实现**异步\同步**日志功能
* 实现**时间轮定时器**高效释放超时连接
* 经过`Webbench`压力测试实现上万的并发连接数据交换

## 构建
**环境**
* Ubuntu18.04
* MySQL版本 5.7.42

**数据库创建**

```
// 建立yourdb库
create database yourdb;

// 创建user表
USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    passwd char(50) NULL
)ENGINE=InnoDB;

// 添加数据
INSERT INTO user(username, passwd) VALUES('name', 'passwd');
```

**Build**

` cmake && ./server`
