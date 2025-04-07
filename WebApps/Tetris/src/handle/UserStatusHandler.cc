#include "../../include/handle/UserStatusHandler.h"

#include "../../include/TetrisServer.h"

namespace Tetris
{

void UserStatusHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    // 获取用户Cookie，判断用户是否现在
    auto session = tetrisServer_->getSessionManager()->getSession(req, resp);
    LOG_INFO << "UserStatusHandler::handle";
    std::string userId = session->getValue("userId");
    
    std::string username = session->getValue("username");
    //LOG_INFO << "userId: " << userId << username << "  is online";
    json userInfo;
    if(tetrisServer_->getUserOnline(userId))
    {
        userInfo["isLoggedIn"] = "true";
        userInfo["username"] = session->getValue("username");
        userInfo["userId"] = session->getValue("userId");
    }
    std::string userBody = userInfo.dump(4);
    tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::k200Ok, "ok",
                               false, "application/json", userBody.size(), userBody, resp);
}

}



