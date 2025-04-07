#pragma once

#include "../../../../include/router/RouterHandler.h"
#include "../../../../include/utils/JsonUtil.h"


namespace Tetris
{

class TetrisServer;

class MenuHandler : public http::router::RouterHandler
{
public:
    explicit MenuHandler(TetrisServer* TetrisServer) : tetrisServer_(TetrisServer) {}

    void handle(const http::HttpRequest& req, http::HttpResponse* resp) override;

private:
    TetrisServer*          tetrisServer_;     // 游戏服务器
};

}






