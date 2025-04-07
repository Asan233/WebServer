#include "../../include/handle/UpgradHandler.h"

#include "../../include/TetrisServer.h"

namespace Tetris
{

/*  协议升级  */
void UpgradHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    std::string upgrade = req.getHeader("Upgrade");
    /* 执行协议升级 */
    if(upgrade == "websocket")
    {
        
        // 协议升级
        if ( tetrisServer_->handleUpgrade(req, resp) )
        {
            // 升级回调函数使用WebSocket回调函数
        }
    }
    else
    {
        LOG_INFO << "Invalid upgrade request " << req.getVersion() << " " << req.path() << " " << upgrade;
        // 非法请求
        resp->setStatusLine(req.getVersion(), http::HttpResponse::k400BadRequest, "Bad Request");
    }
}



}


