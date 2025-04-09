#include "../../include/http/HttpServer.h"

#include <any>
#include <functional>
#include <memory>
#include <muduo/base/Logging.h>
namespace http
{

/* HTTP 默认回调函数 */
void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(int port,
                       const std::string &name,
                       bool useSSL,
                       muduo::net::TcpServer::Option option)
    : listenAddr_(port)
    , server_(&mainLoop_, listenAddr_, name, option)
    , useSSL_(useSSL)
    , httpCallback_(std::bind(&HttpServer::handleRequest, this, std::placeholders::_1, std::placeholders::_2))
    , webSocketCallback_(std::bind(&HttpServer::handleWebSocketRequest, this, std::placeholders::_1, std::placeholders::_2))
{
    initialize();
    // 是否开启SSL功能
    // setSslConfig();
}

// 服务器运行函数
void HttpServer::start()
{
    LOG_WARN << "HttpServer[" << server_.name() << "] starts listening on" << server_.ipPort();
    server_.start();
    mainLoop_.loop();
}

void HttpServer::initialize()
{
    // 设置回调函数
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessageHTTP, this,
                  std::placeholders::_1,
                  std::placeholders::_2,
                  std::placeholders::_3));
}

void HttpServer::setSslConfig(const ssl::SslConfig& config)
{
    if (useSSL_)
    {
        sslCtx_ = std::make_unique<ssl::SslContext>(config);
        if ( !sslCtx_->initialize() )
        {
            LOG_ERROR << "Failed to initialize SSL context";
            abort();
        }
    }
}

void HttpServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        if (useSSL_)
        {
            LOG_INFO << "Https Connection \n";
            auto sslConn = std::make_unique<ssl::SslConnection>(conn, sslCtx_.get());
            sslConn->setMessageCallback(
                std::bind(&HttpServer::onMessageHTTP, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            sslConns_[conn] = std::move(sslConn);
            // 开始SSL握手
            sslConns_[conn]->startHandshake();
        }
        conn->setContext(HttpContext());
    }
    else 
    {
        if (useSSL_)
        {
            sslConns_.erase(conn);
        }
    }
}

void HttpServer::onMessageHTTP(const muduo::net::TcpConnectionPtr &conn,
                           muduo::net::Buffer *buf,
                           muduo::Timestamp receiveTime)
{
    std::string requestData(buf->peek(), buf->readableBytes());
    try
    {
        // 这层判断只是代表是否支持ssl
        if (useSSL_)
        {
            LOG_INFO << "onMessage useSSL_ is true";
            // 1.查找对应的SSL连接
            auto it = sslConns_.find(conn);
            if (it != sslConns_.end())
            {
                LOG_INFO << "onMessage sslConns_ is not empty";
                // 2. SSL连接处理数据
                it->second->onRead(conn, buf, receiveTime);

                // 3. 如果 SSL 握手还未完成，直接返回
                if (!it->second->isHandshakeCompleted())
                {
                    LOG_INFO << "onMessage sslConns_ is not empty";
                    return;
                }

                // 4. 从SSL连接的解密缓冲区获取数据
                muduo::net::Buffer* decryptedBuf = it->second->getDecryptedBuffer();
                if (decryptedBuf->readableBytes() == 0)
                    return; // 没有解密后的数据

                // 5. 使用解密后的数据进行HTTP 处理
                buf = decryptedBuf; // 将 buf 指向解密后的数据
                LOG_INFO << "onMessage decryptedBuf is not empty";
            }
        }
        // HttpContext对象用于解析出buf中的请求报文，并把报文的关键信息封装到HttpRequest对象中
        HttpContext *context = boost::any_cast<HttpContext>(conn->getMutableContext());
        if ( !context->parseRequest(buf, receiveTime) ) // 解析一个http请求
        {
            // 捕获异常，返回错误信息
            // LOG_ERROR << "Exception in onMessage: " << e.what();
            LOG_ERROR << "收到的请求内容：" << requestData;
            // 如果解析http报文过程中出错
            conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->shutdown();
        }
        // 如果buf缓冲区中解析出一个完整的数据包才封装响应报文
        if (context->gotAll())
        {
            LOG_INFO << "HTTP Message: " << context->request().getBody();
            if( onRequest(conn, context->request()) )
            {
                context->request().setQueryParameters(
                                context->request().path().data(), 
                                context->request().path().data() + context->request().path().size());
                
                std::string userId = context->request().getQueryParameters("userId");
                //LOG_WARN << userId;
                // 注册WebSocket连接
                websocket::WebSocketManager::instance()->registerConnection(userId, conn);
                // WebSocket协议升级
                conn->setContext(websocket::WebSocketConnection());
                conn->setMessageCallback(
                    std::bind(&HttpServer::onMessageWebSocket, this,
                              std::placeholders::_1,
                              std::placeholders::_2,
                              std::placeholders::_3));

                
            }
            else {
                context->reset();
            }
        }
    }
    catch (const std::exception &e)
    {
        // 捕获异常，返回错误信息
        LOG_ERROR << "Exception in onMessage: " << e.what();
        LOG_ERROR << "收到的请求内容：" << requestData;
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
}

void HttpServer::onMessageWebSocket(const muduo::net::TcpConnectionPtr &conn,
                           muduo::net::Buffer *buf,
                           muduo::Timestamp receiveTime)
{
    std::string requestData(buf->peek(), buf->readableBytes());
    try
    {
        // HttpContext对象用于解析出buf中的请求报文，并把报文的关键信息封装到HttpRequest对象中
        websocket::WebSocketConnection *context = boost::any_cast<websocket::WebSocketConnection>(conn->getMutableContext());

        if ( !context->parseWebSocketFrame(buf, buf->readableBytes()) ) // 解析一个http请求
        {
            // 捕获异常，返回错误信息
            // LOG_ERROR << "Exception in onMessage: " << e.what();
            LOG_ERROR << "无法解析WebSocket Message, 收到的请求内容: " << requestData;
            // 如果解析http报文过程中出错
            //conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->shutdown();
        }
        // 如果buf缓冲区中解析出一个完整的数据包才封装响应报文
        if ( context->gotAll() )
        {
            LOG_INFO << "WebSocket Message: " << context->getRequestFrame().getPayload() << " conn : " << conn->peerAddress().toIpPort();
            onRequest(conn, context->getRequestFrame());
            context->reset();
        }
    }
    catch (const std::exception &e)
    {
        // 捕获异常，返回错误信息
        LOG_ERROR << "Exception in onMessage: " << e.what();
        LOG_ERROR << "收到的请求内容：" << requestData;
        //conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
}


bool HttpServer::onRequest(const muduo::net::TcpConnectionPtr &conn, const HttpRequest &req)
{
    const std::string &connection = req.getHeader("Connection");
    bool close = ((connection == "close") ||
                  (req.getVersion() == "HTTP/1.0" && connection != "Keep-Alive"));
    HttpResponse response(close);
    
    // 根据请求报文信息来封装响应报文对象
    httpCallback_(req, &response); // 执行onHttpCallback函数

    // 可以给response设置一个成员，判断是否请求的是文件，如果是文件设置为true，并且存在文件位置在这里send出去。
    muduo::net::Buffer buf;
    response.appendToBuffer(&buf);
    // 打印完整的响应内容用于调试
    LOG_INFO << "Sending response:\n" << buf.toStringPiece().as_string();
    conn->send(&buf);

    // 如果是短连接的话，返回响应报文后就断开连接
    if (response.closeConnection())
    {
        conn->shutdown();
    }
    return response.getUpgradeWebSocket();
}

bool HttpServer::onRequest(const muduo::net::TcpConnectionPtr &conn, 
                           const websocket::WebSocketFrame& frame)
{
    try
    {
        std::string payload(frame.getPayload());
        json jsonMsg = json::parse(payload);
        std::string roomId = jsonMsg["roomId"];
        // 执行WebSocket回调函数
        websocket::WebSocketFrame response;
        webSocketCallback_(jsonMsg, &response);
        return false;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << "Exception in onMessageWebSocket: " << e.what();
        LOG_ERROR << "收到的请求内容：" << frame.getPayload();
        return false;
    }
    return false;
}


// 执行请求对应的路由处理函数
void HttpServer::handleRequest(const HttpRequest &req, HttpResponse *resp)
{
    try
    {
        // 处理请求前的中间件
        HttpRequest mutableReq = req;
        middlewareChain_.processBefore(mutableReq);

        // 路由处理
        if ( !router_.route(mutableReq, resp) )
        {
            LOG_INFO << "请求的啥, url: " << req.method() << " " << req.path();
            LOG_INFO << "未找到路由, 返回404";
            resp->setStatusCode(HttpResponse::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setCloseConnection(true);
        }

        // 处理响应后的中间件
        middlewareChain_.processAfter(*resp);
    }
    catch (const HttpResponse& res) 
    {
        // 处理中间件抛出的响应（如CORS预检请求）
        *resp = res;
    }
    catch (const std::exception& e) 
    {
        // 错误处理
        resp->setStatusCode(HttpResponse::k500InternalServerError);
        resp->setBody(e.what());
    }
}

// 执行WebSocket操作处理函数
void HttpServer::handleWebSocketRequest(const json& jsonMsg, websocket::WebSocketFrame* frame)
{
    try
    {
        // 操作路由处理
        webSocketRouter_.route(jsonMsg, frame);
    }
    catch (const std::exception& e) 
    {
        // 错误处理
    }
}


// WebSocket魔术字符串 (RFC6455)
static const std::string kWebSocketMagicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// Base64编码
std::string base64Encode(const unsigned char* input, size_t length) {
    static const char* base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    while (length--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++)
            ret += base64_chars[char_array_4[j]];
        
        while((i++ < 3))
            ret += '=';
    }
    
    return ret;
}

/* 处理协议升级 */
bool HttpServer::handleHandshake(const HttpRequest& req, HttpResponse* resp) 
{
    //LOG_INFO << req.
    // 获取Sec-WebSocket-Key
    std::string key = req.getHeader("Sec-WebSocket-Key");

    // 计算Sec-WebSocket-Accept
    std::string acceptKey = key + kWebSocketMagicString;
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(acceptKey.c_str()), 
         acceptKey.length(), hash);
    
    std::string base64Accept = base64Encode(hash, SHA_DIGEST_LENGTH);
    
    // 设置响应头
    resp->setStatusLine(req.getVersion(), http::HttpResponse::k101SwitchingProtocols, "Switching Protocols");
    resp->addHeader("Upgrade", "websocket");
    resp->addHeader("Connection", "Upgrade");
    resp->addHeader("Sec-WebSocket-Accept", base64Accept);

    // 执行升级回调
    resp->setUpgradeWebSocket(true);
    return true;
}

}


