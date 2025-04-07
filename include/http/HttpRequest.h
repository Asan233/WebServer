#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <muduo/base/Timestamp.h>
#include <muduo/net/TcpConnection.h>


namespace http
{

class HttpRequest
{
public:
    // 请求方法
    enum Method
    {
        kInvalid, kGet, kPost, kHead, kPut, kDelete, kOptions
    };

    /* 默认构造函数 */
    HttpRequest() : method_(kInvalid), version_("unknown") {}

    /* 设置接收时间 */
    void setReceiveTime(const muduo::Timestamp& t);
    /* 返回接收时间 */
    muduo::Timestamp receiveTime() const { return receiveTime_; }

    /* 设置请求方法 */
    bool setMethod(const char* start, const char* end);
    /* 返回请求方法 */
    Method method() const { return method_;}

    /* 设置请求路径 */
    void setPath(const char* start, const char* end);
    /* 返回请求路径 */
    std::string path() const { return path_; }

    /* 设置路径参数 */
    void setPathParameters(const std::string& key, const std::string& value);
    /* 返回路径参数 */
    std::string getPathParameters(const std::string& key) const;

    /* 设置查询参数 */
    void setQueryParameters(const char* start, const char* end);
    /* 返回查询参数 */
    std::string getQueryParameters(const std::string& key) const;

    /* 设置HTTP版本 */
    void setVersion(std::string v){ version_ = v; }
    /* 返回HTTP版本 */
    std::string getVersion() const { return version_; }

    /* 设置请求头 */
    void addHeader(const char* start, const char* colon, const char* end);
    /* 返回请求头 */
    std::string getHeader(const std::string& field) const;
    /* 返回请求头 */
    const std::map<std::string, std::string>& headers() const { return headers_; }

    /*设置请求体*/
    void setBody(std::string& body) { content_ = body; }
    /*设置请求体*/
    void setBody(const char* start, const char* end) 
    { 
        if(end >= start)
        {
            content_.assign(start, end - start);
        } 
    }
    /*返回请求体*/
    std::string getBody() const { return content_; }

    /*设置请求体长度*/
    void setContentLength(uint64_t len) { contentLength_ = len; }
    /*返回请求体长度*/
    uint64_t contentLength() const { return contentLength_; }

    void swap(HttpRequest& that);

    //void UpgradWebsocket(bool is) const { isUpgradeWebSocket_ = is; }
    //bool getUpgradeWebsocket() const { return isUpgradeWebSocket_; }

private:
    Method                                              method_;                // 请求方法
    std::string                                         version_;               // HTTP版本
    std::string                                         path_;                  // 请求路径
    std::unordered_map<std::string, std::string>        pathParameters_;        //  路径参数
    std::unordered_map<std::string, std::string>        queryParameters_;       // 查询参数
    muduo::Timestamp                                    receiveTime_;           // 接收时间
    std::map<std::string, std::string>                  headers_;               // 请求头
    std::string                                         content_;               // 请求体
    uint64_t                                            contentLength_ {0};     // 请求体长度
    //mutable bool                                        isUpgradeWebSocket_;   // 是否执行协议升级   
};

}       // namespace http


