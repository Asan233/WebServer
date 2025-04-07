#include "../../include/handle/LoginHandler.h"

#include "../../include/TetrisServer.h"

namespace Tetris
{

void LoginHandler::handle(const http::HttpRequest &req, http::HttpResponse *resp)
{
    // 处理登录逻辑
    // 验证 contentType
    auto contentType = req.getHeader("Content-Type");

    if (contentType.empty() || contentType != "application/json" || req.getBody().empty())
    {
        LOG_INFO << "content" << req.getBody();
        
        tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::k400BadRequest, "Bad Request",
                                   true, "application/json", 0, std::string(), resp);
        return;
    }

    // JSON 解析使用 try catch 捕获异常
    try
    {
        json parsed = json::parse(req.getBody());
        std::string username = parsed["username"];
        std::string password = parsed["password"];
        // 验证用户是否存在
        int userId = queryUser(username, password);
        std::string userIdStr = std::to_string(userId);
        if (userId != -1)
        {
            // 获取会话
            auto session = tetrisServer_->getSessionManager()->getSession(req, resp);
            // 会话都不是同一个会话，因为会话判断是不是同一个会话是通过请求报文中的cookie来判断的
            // 所以不同页面的访问是不可能是相同的会话的，只有该页面前面访问过服务端，才会有会话记录
            // 那么判断用户是否在其他地方登录中不能通过会话来判断
            
            // 在会话中存储用户信息
            session->setValue("userId", userIdStr);
            session->setValue("username", username);
            session->setValue("isLoggedIn", "true");

            // 用户为在线
            if ( !tetrisServer_->getUserOnline( userIdStr ) )
            {
                // 用户存在登录成功
                // 封装json 数据
                json successResp;
                successResp["status"] = "ok";
                successResp["message"] = "Login successful";
                successResp["userId"] = userIdStr;
                successResp["username"] = username;
                std::string successBody = successResp.dump(4);

                // 更新用户在线列表
                tetrisServer_->setUserOnline(userIdStr, true);

                tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::k200Ok, "ok",
                                   false, "application/json", successBody.size(), successBody, resp);

                // resp->setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
                // resp->setCloseConnection(false);
                // resp->setContentType("application/json");
                // resp->setContentLength(successBody.size());
                // resp->setBody(successBody);
                return;
            }
            else
            {
                // FIXME: 当前该用户正在其他地方登录中，将原有登录用户强制下线更好
                // 不允许重复登录，
                json failureResp;
                failureResp["success"] = false;
                failureResp["error"] = "账号已在其他地方登录";
                std::string failureBody = failureResp.dump(4);

                tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::k403Forbidden, "Forbidden",
                                   true, "application/json", failureBody.size(), failureBody, resp);

                // resp->setStatusLine(req.getVersion(), http::HttpResponse::k403Forbidden, "Forbidden");
                // resp->setCloseConnection(true);
                // resp->setContentType("application/json");
                // resp->setContentLength(failureBody.size());
                // resp->setBody(failureBody);
                return;
            }
        }
        else // 账号密码错误，请重新登录
        {
            // 封装json数据
            json failureResp;
            failureResp["status"] = "error";
            failureResp["message"] = "Invalid username or password";
            std::string failureBody = failureResp.dump(4);

            tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::K401Unauthorized, "Unauthorized",
                                   true, "application/json", failureBody.size(), failureBody, resp);

            // resp->setStatusLine(req.getVersion(), http::HttpResponse::K401Unauthorized, "Unauthorized");
            // resp->setCloseConnection(false);
            // resp->setContentType("application/json");
            // resp->setContentLength(failureBody.size());
            // resp->setBody(failureBody);
            return;
        }
    }
    catch (const std::exception &e)
    {
        // 捕获异常，返回错误信息
        json failureResp;
        failureResp["status"] = "error";
        failureResp["message"] = e.what();
        std::string failureBody = failureResp.dump(4);

        
        
        tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::k400BadRequest, "Bad Request",
                                   true, "application/json", failureBody.size(), failureBody, resp);

        // resp->setStatusLine(req.getVersion(), http::HttpResponse::k400BadRequest, "Bad Request");
        // resp->setCloseConnection(true);
        // resp->setContentType("application/json");
        // resp->setContentLength(failureBody.size());
        // resp->setBody(failureBody);
        return;
    }
}


/* 查询用户账号， 密码判断是否有该用户 */
int LoginHandler::queryUser(const std::string& username, const std::string& password)
{
    // 使用预处理语句，防止注入
    std::string sql = "SELECT id FROM users WHERE username = ? AND password = ?";
    sql::ResultSet* res = mysqlUtil_.executeQuery(sql, username, password);
    if(res->next())
    {
        int id = res->getInt("id");
        return id;
    }
    // 结构为空，返回 -1
    return -1;
}



}



