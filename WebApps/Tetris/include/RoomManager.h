#pragma once

#include "Room.h"
#include <algorithm>
#include <random>
namespace Tetris
{

class RoomManager
{
public:
    static RoomManager* getInstance() {
        static RoomManager instance;
        return &instance;
    }

    // 创建房间
    std::shared_ptr<Room> createRoom(const std::string& creatorId, const std::string& creatorName);
    // 获取房间
    std::shared_ptr<Room> getRoom(const std::string& roomId);
    // 删除房间
    bool removeRoom(const std::string& roomId);
    // 玩家加入房间
    bool joinRoom(const std::string& roomId, const std::string& userId, const std::string& username);
    // 玩家离开房间
    bool leaveRoom(const std::string& roomId, const std::string& userId);

    nlohmann::json getAllRoomsInfo();

private:
     RoomManager() = default;
    ~RoomManager() = default;
    
    // 禁止拷贝和赋值
    RoomManager(const RoomManager&) = delete;
    RoomManager& operator=(const RoomManager&) = delete;
    
    // 生成唯一的房间ID
    std::string generateRoomId() const;

private:
    std::unordered_map<std::string, std::shared_ptr<Room>> rooms_;  // 房间映射表 (roomId -> Room)
    std::unordered_map<std::string, std::string> userRoomMap_;      // 用户所在房间映射表 (userId -> roomId)
    std::mutex mutex_;                                              // 互斥锁，保护并发访问
};


}






