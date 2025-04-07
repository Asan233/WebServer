#pragma once
#include <string>
#include <memory>

#include "../websocket/WebSocketFrame.h"
#include "../utils/JsonUtil.h"
namespace http
{
namespace router
{

/**
 *      路由处理抽象类
 *     子类需要继承该类，并实现自己的业务逻辑路由处理方法。
 * 
*/
class WebSocketHandler
{
    using websocketFrame = http::websocket::WebSocketFrame;
public:
    virtual ~WebSocketHandler() = default;
    virtual void handle(const json& , websocketFrame* ) = 0;
};

}
}


