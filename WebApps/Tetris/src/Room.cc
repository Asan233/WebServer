#include "../include/Room.h"

namespace Tetris
{

Room::Room(const std::string& roomId, const std::string& creatorId, const std::string& creatorName)
    : roomId_(roomId),status_(RoomStatus::WAITING),
      creatorId_(creatorId), creatorName_(creatorName),
      createTime_(time(nullptr)), lastActiveTime_(time(nullptr))
{
    //players_.emplace(creatorId, PlayerInfo(creatorId, creatorName));
    LOG_INFO << "Room created: " << roomId << "by user : " << creatorId;
}

bool Room::addPlayer(const std::string& userId, const std::string& username)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // 房间已满或者房间已经开始游戏
    if(players_.size() >= 2 && status_ != RoomStatus::WAITING) 
    {
        return false;
    }
    // 添加玩家
    players_.emplace(userId, PlayerInfo(userId, username));
    updateLastActiveTime();
    LOG_INFO << "玩家 " << username << " 加入房间 " << roomId_;
    return true;
}



bool Room::removePlayer(const std::string& userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = players_.find(userId);
    if (it != players_.end() ) {
        players_.erase(it);
        updateLastActiveTime();
        LOG_INFO << "Player " << userId << " left room " << roomId_;
        return true;
    }
    return false;
}

nlohmann::json Room::toJson() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    nlohmann::json playersJson = nlohmann::json::array();
    for (const auto& pair : players_) {
        const auto& player = pair.second;
        playersJson.push_back({
            {"userId", player.userId},
            {"username", player.username},
            {"ready", player.ready}
        });
    }
    
    return {
        {"id", roomId_},
        {"status", status_ == RoomStatus::WAITING ? "waiting" : 
                  (status_ == RoomStatus::PLAYING ? "playing" : "finished")},
        {"players", playersJson},
        {"createTime", createTime_},
        {"lastActiveTime", lastActiveTime_}
    };
}



}

