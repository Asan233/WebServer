#include "../include/Room.h"

namespace Tetris
{

Room::Room(const std::string& roomId, const std::string& creatorId, const std::string& creatorName)
    : roomId_(roomId),status_(RoomStatus::WAITING),
      creatorId_(creatorId), creatorName_(creatorName),
      createTime_(time(nullptr)), lastActiveTime_(time(nullptr)),
      playerReadyCount_(0), gameStarted_(false)
{
    //players_.emplace(creatorId, PlayerInfo(creatorId, creatorName));
    LOG_INFO << "Room created: " << roomId << "by user : " << creatorId;
}

bool Room::addPlayer(const std::string& userId, const std::string& username)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // 房间已满或者房间已经开始游戏
    if(players_.size() >= 2 || status_ != RoomStatus::WAITING) 
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

nlohmann::json Room::getAllPlayerInfo() const
{
    nlohmann::json playersJson = nlohmann::json::array();

    for (const auto& pair : players_) 
    {
        const auto& player = pair.second;
        playersJson.push_back({
            {"userId", player.userId},
            {"username", player.username},
            {"ready", player.ready}
        });
    }

    return playersJson;
}

nlohmann::json Room::toJson() const
{
    //std::lock_guard<std::mutex> lock(mutex_);
    
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

bool Room::setPlayerReady(const std::string& userId, bool ready)
{
    //std::lock_guard<std::mutex> lock(mutex_);
    auto it = players_.find(userId);
    if (it != players_.end()) 
    {
        it->second.ready = ready;

        if(ready)playerReadyCount_.fetch_add(1, std::memory_order_relaxed);
        else playerReadyCount_.fetch_sub(1, std::memory_order_relaxed);
        
        updateLastActiveTime();
        return true;
    }
    return false;
}

// 处理玩家游戏操作
bool Room::handlePlayerInput(const std::string& userId, const std::string& action)
{
    if (status_ != RoomStatus::PLAYING || !gameState_) 
    {
        return false;
    }

    // 获取玩家在房间中的索引
    auto it = players_.find(userId);
    if (it == players_.end()) 
    {
        return false;
    }
    
    // 根据玩家索引和动作类型处理输入
    // 玩家1控制水平移动，玩家2控制旋转
    bool isFirstPlayer = (std::distance(players_.begin(), it) == 0);
    
    if (isFirstPlayer) 
    {
        // 第一个玩家控制水平移动
        if (action == "left") {
            return gameState_->moveLeft();
        } else if (action == "right") {
            return gameState_->moveRight();
        }
    } else 
    {
        // 第二个玩家控制旋转
        if (action == "rotate") 
        {
            return gameState_->rotate();
        }
    }
    
    // // 两个玩家都可以加速下落
    // if (action == "down") 
    // {
    //     return gameState_->moveDown();
    // }
    
    return false;
}

nlohmann::json Room::getGameState() const
{
    if(gameState_) {
        return gameState_->toJson();
    } else {
        return nlohmann::json::object();
    }
}

void Room::updateGame()
{
    if (gameState_) 
    {
        gameState_->update();
        if (gameState_->isGameOver()) 
        {
            endGame();
        }
    }
}

bool Room::startGame()
{
    // 初始化游戏状态
    gameState_ = std::make_unique<GameState>();
    gameState_->initialize();

    // 设置房间状态在游戏中
    status_ = RoomStatus::PLAYING;

    // 启动游戏后台线程
    gameRunning_ = true;
    gameThread_ = std::thread([this]() {
        while (gameRunning_) 
        {
            updateGame();
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 每秒更新一次
        }
    });
}

void Room::endGame()
{
    if(status_ == RoomStatus::PLAYING)
    {
        
        gameStarted_ = false;
        if(gameThread_.joinable())
        {
            gameThread_.join();
        }
        status_ = RoomStatus::FINISHED;

        // 重置所有玩家准备状态
        for(auto& pair : players_) {
            pair.second.ready = false;
        }

        LOG_INFO << "Game ended in room " << roomId_;
    }
}


}

