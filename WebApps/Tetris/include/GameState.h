#pragma once

#include <vector>
#include <random>
#include <chrono>
#include "../../../include/utils/JsonUtil.h"

namespace Tetris
{

// 方块类型
enum class TetrominoType {
    I, J, L, O, S, T, Z
};

// 游戏状态类
class GameState
{
public:
GameState(int width = 10, int height = 20);
~GameState() = default;

// 初始化游戏
void initialize();

// 生成新方块
void spawnTetromino();

// 移动方块
bool moveLeft();
bool moveRight();
bool moveDown();

// 旋转方块
bool rotate();

// 检查游戏是否结束
bool isGameOver() const;

// 更新游戏状态
void update();

// 清除已完成的行
int clearLines();

// 转换为JSON
nlohmann::json toJson() const;

private:
    // 游戏区域大小
    int width_;
    int height_;
    
    // 游戏区域 (0表示空，其他值表示方块类型)
    std::vector<std::vector<int>> board_;
    
    // 当前方块
    TetrominoType currentType_;
    int currentRotation_;
    int currentX_;
    int currentY_;
    
    // 下一个方块
    TetrominoType nextType_;
    
    // 游戏分数
    int score_;
    
    // 游戏是否结束
    bool gameOver_;
    
    // 随机数生成器
    std::mt19937 rng_;
    
    // 检查碰撞
    bool checkCollision(int x, int y, int rotation) const;
    
    // 锁定当前方块到游戏区域
    void lockTetromino();
    
    // 获取方块形状
    std::vector<std::vector<uint8_t>> getTetrominoShape(TetrominoType type, int rotation) const;
};

}

