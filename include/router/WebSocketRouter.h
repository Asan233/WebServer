#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include "WebSocketHandler.h"

namespace http
{
namespace router
{

class WebSocketRouter
{
public:
    using HandlerPtr = std::shared_ptr<WebSocketHandler>;

    WebSocketRouter() = default;

    // 注册WebSocket路由处理器
    void registerHandler(const std::string& path, HandlerPtr handler)
    {
        handlers_[path] = handler;
    }

    bool route(const json& jsonMsg, websocket::WebSocketFrame* frame)
    {
        // 查找处理器
        std::string type = jsonMsg["type"];
        LOG_INFO << "WebSocket路由处理器: " << type;
        auto it = handlers_.find(type);
        if (it != handlers_.end())
        {
            it->second->handle(jsonMsg, frame);
            return true;
        }
        return false;
    }

private:
    std::unordered_map<std::string, HandlerPtr> handlers_; // 存储WebSocket路由处理器

};

}
}




