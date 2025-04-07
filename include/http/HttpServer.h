#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <unordered_map>

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>

#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include "../router/Router.h"
#include "../router/WebSocketRouter.h"
#include "../ssl/SslContext.h"
#include "../ssl/SslConnection.h"
#include "../middleware/MiddlewareChain.h"
#include "../session/SessionManager.h"

#include "../websocket/WebSocketConnection.h"
#include "../websocket/WebSocketManager.h"
#include "../utils/JsonUtil.h"


class HttpRequest;
class HttpResponse;

namespace http
{
/*  支持HTTP升级成WebSocket通信 */
class HttpServer : muduo::noncopyable
{
public:
    // HTTP回调函数
    using HttpCallback = std::function<void (const http::HttpRequest&, http::HttpResponse*)>;
    // WebSocket回调函数
    using WebSocketMessageCallback = std::function<void(const json&, websocket::WebSocketFrame*)>;

    /* 构造函数 */
    HttpServer(int port, const std::string& name, bool useSSL = false, muduo::net::TcpServer::Option option = muduo::net::TcpServer::kNoReusePort);

    void setThreeadNum(int numThreads)
    {
        server_.setThreadNum(numThreads);
    }
    void start();

    muduo::net::EventLoop* getLoop() const
    {
        return server_.getLoop();
    }

    void setHttpCallback(const HttpCallback& cb)
    {
        httpCallback_ = cb;
    }

    // 注册静态路由处理器
    void Get(const std::string& path, const HttpCallback& cb)
    {
        router_.registerCallback(HttpRequest::kGet, path, cb);
    }
    
    // 注册静态路由处理器
    void Get(const std::string& path, router::Router::HandlerPtr handler)
    {
        router_.registerHandler(HttpRequest::kGet, path, handler);
    }

    void Post(const std::string& path, const HttpCallback& cb)
    {
        router_.registerCallback(HttpRequest::kPost, path, cb);
    }

    void Post(const std::string& path, router::Router::HandlerPtr handler)
    {
        router_.registerHandler(HttpRequest::kPost, path, handler);
    }

    // 注册动态路由处理器
    void addRoute(HttpRequest::Method method, const std::string& path, router::Router::HandlerPtr handler)
    {
        router_.addRegexHandler(method, path, handler);
    }

    // 注册动态路由处理函数
    void addRoute(HttpRequest::Method method, const std::string& path, const router::Router::HandlerCallback& callback)
    {
        router_.addRegexCallback(method, path, callback);
    }

    // 注册WebSocket路由处理器
    void addWebSocketRoute(const std::string& op, router::WebSocketRouter::HandlerPtr handler)
    {
        webSocketRouter_.registerHandler(op, handler);
    }

    // 设置会话管理器
    void setSessionManager(std::unique_ptr<session::SessionManager> manager)
    {
        sessionManager_ = std::move(manager);
    }

    // 获取会话管理器
    session::SessionManager* getSessionManager() const
    {
        return sessionManager_.get();
    }

    // 添加中间件的方法
    void addMiddleware(std::shared_ptr<middleware::Middleware> middleware) 
    {
        middlewareChain_.addMiddleware(middleware);
    }

    void enableSSL(bool enable) 
    {
        useSSL_ = enable;
    }

    void setSslConfig(const ssl::SslConfig& config);

    /* HTTP协议升级成 WebSocket */
    bool handleHandshake(const HttpRequest& req, HttpResponse* resp);


private:
    void initialize();

    void onConnection(const muduo::net::TcpConnectionPtr& conn);
    // HTTP消息回调函数
    void onMessageHTTP(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp receiveTime);
    // WebSocket消息回调函数
    void onMessageWebSocket(const muduo::net::TcpConnectionPtr& conn,
                        muduo::net::Buffer* buf,
                        muduo::Timestamp receiveTime);
    // 处理HTTP请求
    bool onRequest(const muduo::net::TcpConnectionPtr&, const HttpRequest&);
    // 处理WebSocket帧
    bool onRequest(const muduo::net::TcpConnectionPtr &conn, const websocket::WebSocketFrame& framne);

    void handleRequest(const HttpRequest& req, HttpResponse* resp);
    void handleWebSocketRequest(const json& jsonMsg, websocket::WebSocketFrame* frame);

private:

    muduo::net::InetAddress         listenAddr_;        // 服务器地址
    muduo::net::TcpServer           server_;            // 服务器对象
    muduo::net::EventLoop           mainLoop_;          // 主事件循环
    HttpCallback                    httpCallback_;      // HTTP请求回调函数
    WebSocketMessageCallback        webSocketCallback_; // WebSocket消息回调函数
    router::Router                  router_;            // HTTP路由
    router::WebSocketRouter         webSocketRouter_;   // WebSocket路由
    std::unique_ptr<session::SessionManager> sessionManager_; // 会话管理器
    middleware::MiddlewareChain         middlewareChain_; // 中间件链
    std::unique_ptr<ssl::SslContext>    sslCtx_; // SSL上下文
    bool                                useSSL_;                                            // 是否使用SSL
    std::map<muduo::net::TcpConnectionPtr, std::unique_ptr<ssl::SslConnection>> sslConns_;  // SSL连接

};


}



