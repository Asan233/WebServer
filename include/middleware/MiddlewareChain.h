#pragma once

#include <vector>
#include <memory>

#include "Middleware.h"

namespace http
{
namespace middleware
{

/*  中间件链类， 定义了中间件的执行顺序，以及请求返执行中间件的顺序  */
class MiddlewareChain
{
public:
    void addMiddleware(std::shared_ptr<Middleware> middleware);
    void processBefore(HttpRequest& request);
    void processAfter(HttpResponse& response);

private:
    std::vector<std::shared_ptr<Middleware>> middlewares_;
};

}
}




