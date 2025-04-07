#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <regex>
#include <vector>

#include <muduo/base/Logging.h>

#include "RouterHandler.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"

namespace http
{
namespace router
{

// 路由处理函数
// 可以注册对象式路由处理器与函数式路由处理器
// 简单的处理函数则可以直接使用函数式路由处理（简单回调函数），否则注册对象式路由处理器（可以封装多个相关函数，从而实现更复杂的处理逻辑）
class Router
{

public:
    using HandlerPtr = std::shared_ptr<RouterHandler>;
    using HandlerCallback = std::function<void(const HttpRequest &, HttpResponse *)>;

    // 路由键（请求方法 + URL)
    struct RouteKey
    {
        HttpRequest::Method method;
        std::string path;

        bool operator==(const RouteKey &other) const 
        {
            return method == other.method && path == other.path;
        }
    };

    // RouteKey的哈希函数
    struct RouteKeyHash
    {
        size_t operator()(const RouteKey &key) const
        {
            size_t methodHash = std::hash<int>{}(static_cast<int>(key.method));
            size_t pathHash = std::hash<std::string>{}(key.path);
            return methodHash * 31 + pathHash;
        }
    };


    // 注册静态对象式路由处理器
    void registerHandler(HttpRequest::Method method, const std::string &path, HandlerPtr handler);
    // 注册静态函数式路由处理器
    void registerCallback(HttpRequest::Method method, const std::string &path, const HandlerCallback &callback);
    // 注册动态对象式路由处理器
    void addRegexHandler(HttpRequest::Method method, const std::string &path, HandlerPtr handler)
    {
        std::regex pathRegex = convertToRegex(path);
        regexHandlers_.emplace_back(method, pathRegex, handler);
    }
    // 注册动态函数式路由处理器
    void addRegexCallback(HttpRequest::Method method, const std::string &path, const HandlerCallback &callback)
    {
        std::regex pathRegex = convertToRegex(path);
        regexCallbacks_.emplace_back(method, pathRegex, callback);
    }

    // 请求route处理
    bool route(const HttpRequest &req, HttpResponse *resp);

private:

    std::regex convertToRegex(const std::string &pathPattern)
    {   // 将路径模式转换为正则表达式，支持匹配任意路径参数
        std::string regexPattern = "^" + std::regex_replace(pathPattern, std::regex(R"(/:([^/]+))"), R"(/([^/]+))") + "$";
        return std::regex(regexPattern);
    }

    // 提取路径参数
    void extractPathParameters(const std::smatch &match, HttpRequest &request)
    {
        // Assuming the first match is the full path, parameters start from index 1
        for (size_t i = 1; i < match.size(); ++i)
        {
            request.setPathParameters("param" + std::to_string(i), match[i].str());
            //LOG_INFO << "Path parameter " << i << ": " << match[i].str();
        }
    }


private:
    struct RouteCallbackObj
    {
        HttpRequest::Method method_;
        std::regex pathRegex_;
        HandlerCallback callback_;
        RouteCallbackObj(HttpRequest::Method method, std::regex pathRegex, const HandlerCallback &callback)
            : method_(method), pathRegex_(pathRegex), callback_(callback) {}
    };

    struct RouteHandlerObj
    {
        HttpRequest::Method method_;
        std::regex pathRegex_;
        HandlerPtr handler_;
        RouteHandlerObj(HttpRequest::Method method, std::regex pathRegex, HandlerPtr handler)
            : method_(method), pathRegex_(pathRegex), handler_(handler) {}
    };

    std::unordered_map<RouteKey, HandlerPtr, RouteKeyHash>              handlers_;          // 静态对象式路由处理器映射表
    std::unordered_map<RouteKey, HandlerCallback, RouteKeyHash>         callbacks_;         // 静态函数式路由处理器映射表
    std::vector<RouteHandlerObj>                                        regexHandlers_;     // 动态对象式路由处理器列表
    std::vector<RouteCallbackObj>                                       regexCallbacks_;    // 动态函数式路由处理器列表
};


}
}
