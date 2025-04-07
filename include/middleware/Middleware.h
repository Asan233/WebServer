#pragma once

#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"

#include <memory>

namespace http
{
namespace middleware
{

/*  中间件父基类， 用于定义中间件接口，所有中间件必须继承该基类，并且实现自己的处理方法  */
class Middleware
{
public:
    virtual ~Middleware() = default;

    // 请求前处理
    virtual void before(http::HttpRequest& request) = 0;

    // 请求后处理
    virtual void after(http::HttpResponse& response) = 0;

    // 设置下一个中间件
    void setNext(std::shared_ptr<Middleware> next)
    {
        nextMiddleware_ = next;
    }

protected:
    std::shared_ptr<Middleware> nextMiddleware_;

};

}
}



