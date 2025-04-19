#pragma once


#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <nlohmann/json.hpp>

#include <muduo/base/Logging.h>

#include "GameState.h"

namespace Tetris
{


/* 房间状态 */
enum class RoomStatus
{
    WAITING,
    PLAYING,
    FINISHED
};

// 玩家信息结构
struct PlayerInfo {
    std::string userId;
    std::string username;
    bool ready;

    PlayerInfo(const std::string& userId, const std::string& username)
        : userId(userId), username(username), ready(false) {}
};

class Room {
public:
    Room(const std::string& roomId, const std::string& creatorId, const std::string& creatorName);
    ~Room() = default;

    // 获取房间ID
    std::string getId() const { return roomId_; }
    
    // 获取房间状态
    RoomStatus getStatus() const { return status_; }
    
    // 设置房间状态
    void setStatus(RoomStatus status) { status_ = status; }
    
    // 添加玩家
    bool addPlayer(const std::string& userId, const std::string& username);
    
    // 移除玩家
    bool removePlayer(const std::string& userId);
    
    // 获取玩家列表
    std::vector<PlayerInfo> getPlayers() const;
    
    // 获取玩家数量
    size_t getPlayerCount() const { return players_.size(); }
    
    // 检查玩家是否在房间中
    bool hasPlayer(const std::string& userId) const;
    
    // 设置玩家准备状态
    bool setPlayerReady(const std::string& userId, bool ready);
    
    // 检查是否所有玩家都准备好了
    bool allPlayersReady() const 
    { return playerReadyCount_ == 2; }
    
    // 开始游戏
    bool startGame();
    
    // 处理玩家输入
    bool handlePlayerInput(const std::string& userId, const std::string& action);

    // 更新游戏状态
    void updateGame();

    // 获得游戏状态
    nlohmann::json getGameState() const;

    // 结束游戏
    void endGame();
    
    // 转换为JSON对象（用于API响应）
    nlohmann::json toJson() const;
    
    nlohmann::json getAllPlayerInfo() const;

    // 获取创建时间
    time_t getCreateTime() const { return createTime_; }
    
    // 获取最后活动时间
    time_t getLastActiveTime() const { return lastActiveTime_; }
    
    // 更新最后活动时间
    void updateLastActiveTime() { lastActiveTime_ = time(nullptr); }

    //得到所有用户ID
    std::vector<std::string> getAllUserId() const
    {
        std::vector<std::string> userIds;
        for (const auto& pair : players_) 
        {
            userIds.push_back(pair.first);
        }
        return userIds;
    }

private:
    std::string roomId_;                                  // 房间ID
    RoomStatus status_;                                   // 房间状态
    std::unordered_map<std::string, PlayerInfo> players_; // 玩家信息映射表 (userId -> PlayerInfo)
    mutable std::mutex mutex_;                            // 互斥锁，保护并发访问
    time_t createTime_;                                   // 创建时间
    time_t lastActiveTime_;                               // 最后活动时间
    std::atomic<bool> gameStarted_;                       // 游戏是否已开始
    std::string creatorId_;                               // 创建者ID
    std::string creatorName_;                             // 创建者名称
    std::atomic<int> playerReadyCount_;                   // 准备好的玩家数量

    std::unique_ptr<GameState> gameState_;                // 游戏状态
    std::thread gameThread_;                              // 游戏线程
    std::atomic<bool> gameRunning_;                       // 游戏更新定时器

};

}


