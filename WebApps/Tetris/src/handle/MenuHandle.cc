#include "../../include/handle/MenuHandler.h"

#include "../../include/TetrisServer.h"

namespace Tetris
{


void MenuHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    // 获取用户Cookie，判断用户是否现在
    auto session = tetrisServer_->getSessionManager()->getSession(req, resp);
    std::string userId = session->getValue("userId");

    if(tetrisServer_->getUserOnline(userId))
    {
        // 用户在线返回菜单页面
        std::string reqFile;
        reqFile.append("../WebApps/Tetris/resource/lobby.html");
        FileUtil fileOperater(reqFile);
        if(!fileOperater.isValid())
        {
            LOG_WARN << reqFile << " not exist";
            // 404 not found
            fileOperater.resetDefaultFile();
        }

        std::vector<char> buffer(fileOperater.size());
        fileOperater.readFile(buffer);
        std::string bufStr = std::string(buffer.data(), buffer.size());

        tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::k200Ok, "ok",
                                   false, "text/html", bufStr.size(), bufStr, resp);
    }else 
    {
        
    }
}



}



