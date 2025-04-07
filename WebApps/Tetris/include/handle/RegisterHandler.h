#pragma once

#include "../../../../include/router/RouterHandler.h"
#include "../../../../include/utils/JsonUtil.h"
#include "../../../../include/utils/MysqlUtil.h"

namespace Tetris
{

class TetrisServer;

class RegisterHandler : public http::router::RouterHandler
{
public:
    explicit RegisterHandler (TetrisServer* terisServer) : terisServer_(terisServer) {}

    void handle(const http::HttpRequest& req, http::HttpResponse* resp) override;
    ~RegisterHandler() = default;

private:
    int insertUser(const std::string& username, const std::string& password);
    bool isUserExist(const std::string& username);

private:
    TetrisServer* terisServer_;
    http::MysqlUtil mysqlUtil_;
};
    

}


