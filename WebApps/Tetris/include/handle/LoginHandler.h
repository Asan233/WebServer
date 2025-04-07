#pragma once

#include "../../../../include/router/RouterHandler.h"
#include "../../../../include/utils/JsonUtil.h"
#include "../../../../include/utils/MysqlUtil.h"




namespace Tetris
{

class TetrisServer;

class LoginHandler : public http::router::RouterHandler
{
public:
    explicit LoginHandler(TetrisServer* tetrisServer) : tetrisServer_(tetrisServer) {}

    void handle(const http::HttpRequest& req, http::HttpResponse* resp) override;

private:
    int queryUser(const std::string& username, const std::string& password);

private:
    TetrisServer*           tetrisServer_;
    http::MysqlUtil         mysqlUtil_;
};

}
