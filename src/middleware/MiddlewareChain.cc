#include "../../include/middleware/MiddlewareChain.h"

#include <muduo/base/Logging.h>

namespace http
{
namespace middleware
{

void MiddlewareChain::addMiddleware(std::shared_ptr<Middleware> middleware)
{
    middlewares_.push_back(middleware);
}

/* 按中间件链顺序处理 */
void MiddlewareChain::processBefore(HttpRequest& request)
{
    for (auto& middleware : middlewares_)
    {
        middleware->before(request);
    }
}

/* 返回请求，按中间件请求到来处理顺序倒序处理 */
void MiddlewareChain::processAfter(HttpResponse& response)
{
    try
    {
        // 反向处理，以保持中间件的正确执行顺序
        for(auto it = middlewares_.rbegin(); it != middlewares_.rend(); ++it)
        {
            if(*it)
            {
                (*it)->after(response);
            }
        }
    }
    catch( const std::exception& e )
    {
        LOG_ERROR << "Error in middleware after processing: " << e.what();
    }
}

}
}

