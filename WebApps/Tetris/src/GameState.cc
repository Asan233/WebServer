#include "../include/GameState.h"

namespace Tetris
{

// 方块形状定义
// 方块形状定义
const std::vector<std::vector<std::vector<uint8_t>>> TETROMINOS = {
    // I
    {
        {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0}
    },
    // J
    {
        {2, 0, 0, 2, 2, 2, 0, 0, 0},
        {0, 2, 2, 0, 2, 0, 0, 2, 0},
        {0, 0, 0, 2, 2, 2, 0, 0, 2},
        {0, 2, 0, 0, 2, 0, 2, 2, 0}
    },
    // L
    {
        {0, 0, 3, 3, 3, 3, 0, 0, 0},
        {0, 3, 0, 0, 3, 0, 0, 3, 3},
        {0, 0, 0, 3, 3, 3, 3, 0, 0},
        {3, 3, 0, 0, 3, 0, 0, 3, 0}
    },
    // O
    {
        {0, 4, 4, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 4, 4, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 4, 4, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 4, 4, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    // S
    {
        {0, 5, 5, 5, 5, 0, 0, 0, 0},
        {0, 5, 0, 0, 5, 5, 0, 0, 5},
        {0, 0, 0, 0, 5, 5, 5, 5, 0},
        {5, 0, 0, 5, 5, 0, 0, 5, 0}
    },
    // T
    {
        {0, 6, 0, 6, 6, 6, 0, 0, 0},
        {0, 6, 0, 0, 6, 6, 0, 6, 0},
        {0, 0, 0, 6, 6, 6, 0, 6, 0},
        {0, 6, 0, 6, 6, 0, 0, 6, 0}
    },
    // Z
    {
        {7, 7, 0, 0, 7, 7, 0, 0, 0},
        {0, 0, 7, 0, 7, 7, 0, 7, 0},
        {0, 0, 0, 7, 7, 0, 0, 7, 7},
        {0, 7, 0, 7, 7, 0, 7, 0, 0}
    }
};

GameState::GameState(int width, int height)
    : width_(width), height_(height), score_(0), gameOver_(false) {
    // 初始化随机数生成器
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rng_.seed(static_cast<unsigned int>(seed));
    
    initialize();
}

void GameState::initialize() {
    // 初始化游戏区域
    board_.resize(height_, std::vector<int>(width_, 0));
    
    // 初始化分数
    score_ = 0;
    
    // 重置游戏结束标志
    gameOver_ = false;
    
    // 生成第一个方块
    nextType_ = static_cast<TetrominoType>(rng_() % 7);
    spawnTetromino();
}

void GameState::spawnTetromino() {
    currentType_ = nextType_;
    nextType_ = static_cast<TetrominoType>(rng_() % 7);
    
    currentRotation_ = 0;
    currentX_ = width_ / 2 - 2;
    currentY_ = 0;
    
    // 检查是否可以放置新方块
    if ( checkCollision(currentX_, currentY_, currentRotation_) ) {
        gameOver_ = true;
    }
}

bool GameState::moveLeft() {
    if (gameOver_) return false;
    
    if (!checkCollision(currentX_ - 1, currentY_, currentRotation_)) {
        currentX_--;
        return true;
    }
    return false;
}

bool GameState::moveRight() {
    if (gameOver_) return false;
    
    if (!checkCollision(currentX_ + 1, currentY_, currentRotation_)) {
        currentX_++;
        return true;
    }
    return false;
}

bool GameState::moveDown() {
    if (gameOver_) return false;
    
    if (!checkCollision(currentX_, currentY_ + 1, currentRotation_)) {
        currentY_++;
        return true;
    } else {
        // 如果不能下移，则锁定方块并生成新方块
        lockTetromino();
        clearLines();
        spawnTetromino();
        return false;
    }
}

bool GameState::rotate() {
    if (gameOver_) return false;
    
    int newRotation = (currentRotation_ + 1) % 4;
    if (!checkCollision(currentX_, currentY_, newRotation)) {
        currentRotation_ = newRotation;
        return true;
    }
    return false;
}

bool GameState::isGameOver() const {
    return gameOver_;
}

void GameState::update() {
    if (!gameOver_) {
        moveDown();
    }
}

int GameState::clearLines() {
    int linesCleared = 0;
    
    for (int y = height_ - 1; y >= 0; y--) {
        bool lineFull = true;
        
        for (int x = 0; x < width_; x++) {
            if (board_[y][x] == 0) {
                lineFull = false;
                break;
            }
        }
        
        if (lineFull) {
            linesCleared++;
            
            // 移动上面的行下来
            for (int yy = y; yy > 0; yy--) {
                for (int x = 0; x < width_; x++) {
                    board_[yy][x] = board_[yy - 1][x];
                }
            }
            
            // 清空最上面的行
            for (int x = 0; x < width_; x++) {
                board_[0][x] = 0;
            }
            
            // 因为当前行已经被上面的行替换，所以需要重新检查当前行
            y++;
        }
    }
    
    // 更新分数
    if (linesCleared > 0) {
        score_ += linesCleared * linesCleared * 100;
    }
    
    return linesCleared;
}

nlohmann::json GameState::toJson() const {
    nlohmann::json boardJson = nlohmann::json::array();
    for (const auto& row : board_) {
        boardJson.push_back(row);
    }
    
    auto currentShape = getTetrominoShape(currentType_, currentRotation_);
    nlohmann::json currentShapeJson = nlohmann::json::array();
    for (const auto& row : currentShape) {
        currentShapeJson.push_back(row);
    }
    
    auto nextShape = getTetrominoShape(nextType_, 0);
    nlohmann::json nextShapeJson = nlohmann::json::array();
    for (const auto& row : nextShape) {
        nextShapeJson.push_back(row);
    }
    
    return {
        {"board", boardJson},
        {"currentType", static_cast<int>(currentType_)},
        {"currentRotation", currentRotation_},
        {"currentX", currentX_},
        {"currentY", currentY_},
        {"currentShape", currentShapeJson},
        {"nextType", static_cast<int>(nextType_)},
        {"nextShape", nextShapeJson},
        {"score", score_},
        {"gameOver", gameOver_}
    };
}

bool GameState::checkCollision(int x, int y, int rotation) const {
    auto shape = getTetrominoShape(currentType_, rotation);
    int size = static_cast<int>(std::sqrt(shape.size()));
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if ( shape[i][j] != 0 ) {
                int boardX = x + j;
                int boardY = y + i;
                
                // 检查边界
                if (boardX < 0 || boardX >= width_ || boardY < 0 || boardY >= height_) {
                    return true;
                }
                
                // 检查碰撞
                if (boardY >= 0 && board_[boardY][boardX] != 0) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void GameState::lockTetromino() {
    auto shape = getTetrominoShape(currentType_, currentRotation_);
    int size = static_cast<int>(std::sqrt(shape.size()));
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (shape[i][j] != 0) {
                int boardX = currentX_ + j;
                int boardY = currentY_ + i;
                
                if (boardY >= 0 && boardY < height_ && boardX >= 0 && boardX < width_) 
                {
                    board_[boardY][boardX] = static_cast<int>(currentType_) + 1;
                }
            }
        }
    }
}

std::vector<std::vector<uint8_t>> GameState::getTetrominoShape(TetrominoType type, int rotation) const {
    int typeIndex = static_cast<int>(type);
    rotation = rotation % 4;
    
    const auto& tetromino = TETROMINOS[typeIndex][rotation];
    int size = static_cast<int>(std::sqrt(tetromino.size()));
    
    std::vector<std::vector<uint8_t>> shape(size, std::vector<uint8_t>(size, 0));
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            shape[i][j] = tetromino[i * size + j];
        }
    }
    
    return shape;
}

}


