#pragma once

#include <muduo/net/TcpConnection.h>
#include <muduo/base/Logging.h>
#include <string>
#include <map>
#include <memory>
#include <set>
#include <mutex>
#include "WebSocketFrame.h"
#include "../utils/JsonUtil.h"



namespace http
{
namespace websocket
{

class WebSocketManager
{

public:
    // 实现单例模式
    static WebSocketManager* instance()
    {
        static WebSocketManager instance;
        return &instance;
    }

    // 注册WebSocket连接
    void registerConnection(const std::string& userId, const muduo::net::TcpConnectionPtr& conn)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_[userId] = conn;
        LOG_INFO << "WebSocket连接注册: 用户ID= " << userId;
    }

    // 注销WebSocket连接
    void unregisterConnection(const std::string& userId)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_.erase(userId);
        LOG_INFO << "WebSocket注销注册: 用户ID= " << userId;
    }

    muduo::net::TcpConnectionPtr getConnection(const std::string& userId)
    {
        auto it = connections_.find(userId);
        if (it != connections_.end())
        {
            return it->second;
        }
        return nullptr;
    }

private:
    WebSocketManager() = default;
    ~WebSocketManager() = default;

private:
    std::mutex mutex_;                                                 // 互斥锁
    std::map<std::string, muduo::net::TcpConnectionPtr> connections_;  // 连接列表
};


}       // namespace websocket
}       // namespace http


