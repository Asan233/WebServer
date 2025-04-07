#include "../include/RoomManager.h"

namespace Tetris
{

std::shared_ptr<Room> RoomManager::createRoom(const std::string& creatorId, const std::string& creatorName)
{
    std::string roomId = generateRoomId();
    auto room = std::make_shared<Room>(roomId, creatorId, creatorName);
    std::lock_guard<std::mutex> lock(mutex_);
    rooms_[roomId] = room;
    //userRoomMap_[creatorId] = roomId;
    return room;
}

std::shared_ptr<Room> RoomManager::getRoom(const std::string& roomId)
{
    //std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(roomId);
    if (it != rooms_.end()) 
    {
        return it->second;
    }
    return nullptr;
}

bool RoomManager::removeRoom(const std::string& roomId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(roomId);
    if (it != rooms_.end()) {
        rooms_.erase(it);
        return true;
    }
    return false;
}

bool RoomManager::joinRoom(const std::string& roomId, const std::string& userId, const std::string& username)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(roomId);
    if ( it != rooms_.end() ) {
        return it->second->addPlayer(userId, username);
    }
    return false;
}

bool RoomManager::leaveRoom(const std::string& roomId, const std::string& userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(roomId);
    if ( it != rooms_.end() ) {
        it->second->removePlayer(userId);
        if(it->second->getPlayerCount() == 0)
        {
            removeRoom(roomId);
        }
        return true;
    }
    return false;
}

std::string RoomManager::generateRoomId() const 
{
    // 生成6位随机数字作为房间ID
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    std::string roomId = std::to_string(dis(gen));
    
    // 确保房间ID唯一
    while (rooms_.find(roomId) != rooms_.end()) {
        roomId = std::to_string(dis(gen));
    }
    return roomId;
}

nlohmann::json RoomManager::getAllRoomsInfo()
{
    nlohmann::json roomsJson = nlohmann::json::array();
    //std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& pair : rooms_) {
        roomsJson.push_back(pair.second->toJson());
    }
    return roomsJson;
}

}


