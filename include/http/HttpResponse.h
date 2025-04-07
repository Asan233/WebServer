#pragma once

#include <muduo/net/TcpServer.h>

namespace http
{

class HttpResponse
{
public:
    enum HttpStatusCode
    {
        kUnknown,
        k101SwitchingProtocols = 101,
        k200Ok = 200,
        k204NoContent = 204,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        K401Unauthorized = 401,
        k403Forbidden = 403,
        k404NotFound = 404,
        k409Conflict = 409,
        k500InternalServerError = 500,
    };

    /* 默认构造函数 */
    HttpResponse(bool close = true) : statusCode_(kUnknown), closeConnection_(close), isUpgradeWebSocket_(false) {}
    /* 设置版本号 */
    void setVersion(std::string version)
    { httpVersion_ = version; }
    /* 设置状态码 */
    void setStatusCode(HttpStatusCode code)
    { statusCode_ = code; }
    /* 得到状态码 */
    HttpStatusCode getStatusCode() const
    { return statusCode_; }
    /* 设置状态消息 */
    void setStatusMessage(std::string message)
    { statusMessage_ = message; }
    /* 设置是否关闭 */
    void setCloseConnection(bool on)
    { closeConnection_ = on; }
    bool closeConnection() const
    { return closeConnection_; }
    /* 设置返回类型 */
    void setContentType(const std::string& contentType)
    { addHeader("Content-Type", contentType); }
    /* 设置消息体长度 */
    void setContentLength(uint64_t length)
    { addHeader("Content-Length", std::to_string(length)); }
    /* 设置消息头 */
    void addHeader(const std::string& key, const std::string& value)
    { headers_[key] = value; }
    void setBody(const std::string& body)
    {
        body_ = body;
    }

    /* 设置状态行 */
    void setStatusLine(const std::string& version, HttpStatusCode code, const std::string& message);
    /* 错误头 */
    void setErrorHeader(){}
    /* 添加消息 */
    void appendToBuffer(muduo::net::Buffer* outputBuf) const;

    /* 设置是否升级 */
    void setUpgradeWebSocket(bool is)
    { isUpgradeWebSocket_ = is; }
    /* 得到是否升级 */
    bool getUpgradeWebSocket() const
    { return isUpgradeWebSocket_; }

private:
    std::string     httpVersion_;   // HTTP版本
    HttpStatusCode  statusCode_;    // 状态码
    std::string     statusMessage_; // 状态消息
    bool            closeConnection_; // 是否关闭连接
    std::map<std::string, std::string> headers_; // 响应头
    std::string     body_;      // 响应体
    bool            isFile_;    // 是否是文件
    bool            isUpgradeWebSocket_; // 是否是WebSocket升级
};

}




