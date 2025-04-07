#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>

namespace http
{
namespace session
{

class SessionManager;

class Session : public std::enable_shared_from_this<Session>
{
public:
    /* 默认构造函数，默认存活时间1小时 */
    Session(const std::string& sessionId, SessionManager* manager, int maxAge = 3600);

    const std::string& getId() const { return sessionId_;}

    bool isExpired() const;         // 是否过期
    void refresh();                 // 刷新过期时间

    void setManager(SessionManager* sessionManager) 
    { sessionManager_ = sessionManager; }
    SessionManager* getManager() const { return sessionManager_;}

    // 数据存取
    void setValue(const std::string& key, const std::string& value);
    std::string getValue(const std::string& key) const;
    void remove(const std::string& key);
    void clear();

private:
    std::string                                     sessionId_; // 会话ID
    std::unordered_map<std::string, std::string>    data_;      // 会话数据
    std::chrono::system_clock::time_point           expiryTime_; // 超时时间
    int                                             maxAge_;    // 最大存活时间
    SessionManager*                                 sessionManager_;   // 会话管理器
};

}       // namespace session
}       // namespace http






