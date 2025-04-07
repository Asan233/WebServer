#pragma once

#include "SessionStorage.h"
#include "../http/HttpResponse.h"
#include "../http/HttpRequest.h"
#include <memory>
#include <random>

namespace http
{
namespace session
{

class SessionManager
{
public:
    explicit SessionManager(std::unique_ptr<SessionStorage> storage);

    // 从请求中获取或者创建会话
    std::shared_ptr<Session> getSession(const HttpRequest& req, HttpResponse* resp);

    // 销毁会话
    void destroySession(const std::string& sessionId);

    // 清理过期会话
    void cleanExpiredSessions();

    // 更新会话
    void updateSession(std::shared_ptr<Session> session)
    {
        storage_->save(session);
    }
    // 从Cookie中获取会话ID
    std::string getSessionIdFromCookie(const HttpRequest& req);

    std::string getValueForSession(const std::string& sessionId, const std::string& key)
    {
        auto session = storage_->load(sessionId);
        if (!session->isExpired() && session)
        {
            return session->getValue(key);
        }
        return "";
    }

private: 
    std::string generateSessionId();                                            // 生成会话ID
                     
    void setSessionCookie(const std::string& sessionId, HttpResponse* resp);    // 设置会话ID到Cookie中

private:
    std::unique_ptr<SessionStorage> storage_;   // 会话存储
    std::mt19937 rng_;                          // 随机数生成器
};

}           // namespace session
}           // namespace http




