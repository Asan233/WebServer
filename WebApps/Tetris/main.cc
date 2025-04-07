#include "include/TetrisServer.h"

#include <muduo/base/AsyncLogging.h>

// 日志文件滚动大小 32MB (1024 * 1024字节)
static const off_t kRollSize = 32 * 1024 * 1024;

// 异步日志实例，用于异步写入
muduo::AsyncLogging* g_asyncLog = nullptr;
muduo::AsyncLogging* getAsyncLog() {
    return g_asyncLog;
}

// 异步日志写入函数
void asyncLog(const char* msg, int len) {
    muduo::AsyncLogging* log = getAsyncLog();
    if(log) log->append(msg, len); // 追加日志
}



int main(int argc, char* argv[])
{
    /*-----------------------------启动日志系统---------------------------------*/
    // 1. 创建日志目录
    const std::string LogDir = "logs";
    mkdir(LogDir.c_str(), 0755);
    // 2. 构建日志文件夹
    std::ostringstream LogfilePath;
    LogfilePath << LogDir << "/" << ::basename(argv[0]);  // 完成日志路径
    muduo::AsyncLogging log(LogfilePath.str(), kRollSize);          // 创建异步日志实例
    g_asyncLog = &log;                                              // 设置全局异步日志实例
    muduo::Logger::setOutput(asyncLog);                             // 设置日志输出函数
    log.start();                                                    // 启动异步日志线程

    /*------------------------------启动服务器-----------------------------------------*/
    Tetris::TetrisServer server(8080, "TetrisServer");
    server.setHttpServerThreadNum(4);
    server.Httpstart();
    return 0;
}



