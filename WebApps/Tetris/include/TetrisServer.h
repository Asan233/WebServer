#pragma once

#include <atomic>
#include <memory>
#include <mutex>

#include "TetrisGame.h"

#include "handle/EntryHandler.h"
#include "handle/RegisterHandler.h"
#include "handle/LoginHandler.h"
#include "handle/MenuHandler.h"
#include "handle/UserStatusHandler.h"
#include "handle/RoomManagerHandler.h"
#include "handle/UpgradHandler.h"

#include "../../../include/http/HttpServer.h"                   // HTTP
#include "../../../include/websocket/WebSocketConnection.h"     // WebSocket


#include "../../../include/utils/MysqlUtil.h"
#include "../../../include/utils/FileUtil.h"

#include "../../../include/session/SessionManager.h"
#include "../../../include/middleware/MiddlewareChain.h"
#include "../../../include/middleware/cors/CorsMiddleware.h"

#include <muduo/base/AsyncLogging.h>



namespace Tetris
{

class TetrisServer
{

public:
    TetrisServer(int port, const std::string& name, muduo::net::TcpServer::Option = muduo::net::TcpServer::kNoReusePort);

    /* 设置HTTP服务器线程数量 */
    void setHttpServerThreadNum(int numThreads);

    /* 启动服务器 */
    void Httpstart();

    void WebSocketstart(int , std::string, muduo::net::TcpServer::Option);

    void setSessionManager(std::unique_ptr<http::session::SessionManager> manager)
    {
        server_.setSessionManager(std::move(manager));
    }

    http::session::SessionManager* getSessionManager() const
    {
        return server_.getSessionManager();
    }

    bool getUserOnline(std::string& userId)
    {
        return onlineUsers_.find(userId) != onlineUsers_.end() && onlineUsers_[userId] == true;
    }

    void setUserOnline(std::string& userId, bool online)
    {
        std::lock_guard<std::mutex> lock(mutexFourOnlineUsers_);
        onlineUsers_[userId] = online;
        // 更新在线人数
        if(online)
        {
            maxOnline_.fetch_add(1, std::memory_order_release);
        }else {
            maxOnline_.fetch_sub(1, std::memory_order_release);
        }
    }

    /* URL Cookie检查，检查是否在线 */
    bool checkUserOnline(const http::HttpRequest& req )
    {
        auto sessionId = getSessionManager()->getSessionIdFromCookie(req);
        if(!sessionId.empty())
        {
            getSessionManager()->getValueForSession(sessionId, "userId");
            return true;
        }
        return false;
    }


    /* 升级HTTP握手成WebSocket协议 */
    bool handleUpgrade(const http::HttpRequest& req, http::HttpResponse* resp)
    {
        if( server_.handleHandshake(req, resp) )
        {
            return true;
        }
        return false;
    }

    // 封装响应报文
    void packageResp(const std::string& version,
                     http::HttpResponse::HttpStatusCode statusCode,
                     const std::string& statusMsg,
                     bool close, const std::string& contentType,
                     int contentLen, const std::string& body,
                     http::HttpResponse* resp);

    void handleGame(const json& jsonMsg, http::websocket::WebSocketFrame* frame);

private:
    void initialize();              // 初始化
    void initializeRouter();        // 初始化路由
    void initializeSession();       // 初始化会话
    void initializeMiddlewares();   // 初始化中间件

private:
    http::HttpServer        server_;        // Http服务器对象

    http::MysqlUtil         mysqlUtil_;     // 数据库工具类
    std::string             name_;          // 服务器名称

    std::mutex mutexForGames_;

    // 在线用户映射
    std::unordered_map<std::string, bool>    onlineUsers_;
    std::mutex                       mutexFourOnlineUsers_;

    // 在线统计数据
    std::atomic<int> maxOnline_;
};

}




