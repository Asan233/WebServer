#pragma once

#include "../../../../include/router/RouterHandler.h"
#include "../../../../include/utils/JsonUtil.h"

namespace Tetris
{
class TetrisServer;

class UserStatusHandler : public http::router::RouterHandler
{
public:
    explicit UserStatusHandler(TetrisServer* tetrisServer) : tetrisServer_(tetrisServer) {}

    void handle(const http::HttpRequest& req, http::HttpResponse* resp) override;

private:
    TetrisServer*          tetrisServer_;

};


}



