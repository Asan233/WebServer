#pragma once

#include <iostream>
#include <muduo/net/TcpServer.h>

#include "HttpRequest.h"

namespace http
{

class HttpContext
{
public:
    enum HttpRequestParseState
    {
        kExpectRequestLine,     // 解析请求行
        kExpectHeaders,         // 解析请求头
        kExpectBody,            // 解析请求体
        kGotAll                 // 解析完成
    };

    /* 初始化请求解析状态 */
    HttpContext() : state_(kExpectRequestLine) {}

    bool parseRequest(muduo::net::Buffer* buf, muduo::Timestamp receiveTime);
    /* 解析是否完成 */
    bool gotAll() const { return state_ == kGotAll; }

    /* 重置请求 */
    void reset()
    {
        state_ = kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    /* 获取请求 */
    const HttpRequest& request() const { return request_; }
    HttpRequest& request() { return request_; }

private:
    bool processRequestLine(const char* begin, const char* end);

private:
    HttpRequestParseState state_;       // http请求解析状态
    HttpRequest request_;               // http请求
};


}       //namespace http   




