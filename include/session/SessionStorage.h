#pragma once

#include "Session.h"
#include <memory>

namespace http
{
namespace session
{

/*  SessionStorage抽象基类， 用户需要继承该基类并且实现自己的Session存储方案，例如数据库的存储方案等等 */
class SessionStorage
{
public:
    virtual ~SessionStorage() = default;
    virtual void save(std::shared_ptr<Session> session) = 0;                    // 保存会话
    virtual std::shared_ptr<Session> load(const std::string& sessionId) = 0;    // 加载会话
    virtual void remove(const std::string& sessionId) = 0;                      // 移除会话
};

/* 使用内存空间实现SessionStorage, 存在内存中不使用数据库 */
class MemorySessionStorage : public SessionStorage
{
public:
    void save(std::shared_ptr<Session> session) override;
    std::shared_ptr<Session> load(const std::string& sessionId) override;
    virtual void remove(const std::string& sessionId) override;

private:
    ::std::unordered_map<std::string, std::shared_ptr<Session> > sessions_;
};

}
}
