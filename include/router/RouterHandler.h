#pragma once
#include <string>
#include <memory>
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"


namespace http
{
namespace router
{

/**
 *      路由处理抽象类
 *     子类需要继承该类，并实现自己的业务逻辑路由处理方法。
 * 
*/
class RouterHandler
{
public:
    virtual ~RouterHandler() = default;
    virtual void handle(const HttpRequest& req, HttpResponse* resp) = 0;
};

}       // namespace router
}       // namspace http


