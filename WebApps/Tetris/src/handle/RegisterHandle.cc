#include "../../include/handle/RegisterHandler.h"

#include "../../include/TetrisServer.h"


namespace Tetris
{
    void RegisterHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
    {
        // 解析body Json 格式
        json parsed = json::parse(req.getBody());
        std::string username = parsed["username"];
        std::string password = parsed["password"];

        // 判断用户是否以及存在
        int userId = insertUser(username, password);
        if(userId != -1)
        {
            // 插入成功
            // 封装成功响应
            json successResp;   
            successResp["status"] = "ok";
            successResp["message"] = "Registration successful";
            //successResp["userId"] = userId;
            std::string successBody = successResp.dump(4);

            terisServer_->packageResp(req.getVersion(), http::HttpResponse::k200Ok, "ok",
                                      false, "application/json", successBody.size(), successBody, resp);
        }
        else
        {
            // 插入失败
            // 插入失败
            json failureResp;
            failureResp["status"] = "error";
            failureResp["message"] = "username already exists";
            std::string failureBody = failureResp.dump(4);

            terisServer_->packageResp(req.getVersion(), http::HttpResponse::k409Conflict, "Conflict",
                                      false, "application/json", failureBody.size(), failureBody, resp);
        }
    }

    int RegisterHandler::insertUser(const std::string &username, const std::string &password)
    {
        // 判断用户是否存在，如果存在则返回-1，否则返回用户id
        if (!isUserExist(username))
        {
            // 用户不存在，插入用户
            std::string sql = "INSERT INTO users (username, password) VALUES ('" + username + "', '" + password + "')";
            mysqlUtil_.executeUpdate(sql);
            std::string sql2 = "SELECT id FROM users WHERE username = '" + username + "'";
            sql::ResultSet* res = mysqlUtil_.executeQuery(sql2);
            if (res->next())
            {
                return res->getInt("id");
            }
        }
        return -1;
    }

    bool RegisterHandler::isUserExist(const std::string &username)
    {
        std::string sql = "SELECT id FROM users WHERE username = '" + username + "'";
        sql::ResultSet* res = mysqlUtil_.executeQuery(sql);
        if (res->next())
        {
            return true;
        }
        return false;
    }
}

