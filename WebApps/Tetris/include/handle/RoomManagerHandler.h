#pragma once

#include "../RoomManager.h"
#include "../../../../include/router/RouterHandler.h"
#include "../../../../include/router/WebSocketRouter.h"
#include "../../../../include/utils/JsonUtil.h"


namespace Tetris
{

class TetrisServer;

class RoomsInfoHandler : public http::router::RouterHandler
{
public:
    explicit RoomsInfoHandler(TetrisServer* tetrisServer) : tetrisServer_(tetrisServer) {}
    ~RoomsInfoHandler() = default;
    void handle(const http::HttpRequest& req, http::HttpResponse* resp) override;
private:
    TetrisServer*         tetrisServer_;     // 游戏服务器
};

class RoomsCreateHandler : public http::router::RouterHandler
{
public:
    explicit RoomsCreateHandler(TetrisServer* tetrisServer) : tetrisServer_(tetrisServer) {}
    ~RoomsCreateHandler() = default;
    void handle(const http::HttpRequest& req, http::HttpResponse* resp) override;
private:
    TetrisServer*         tetrisServer_;     // 游戏服务器
};


class RoomsOpHandler : public http::router::RouterHandler
{
public:
    explicit RoomsOpHandler(TetrisServer* tetrisServer) : tetrisServer_(tetrisServer) {}
    ~RoomsOpHandler() = default;
    void handle(const http::HttpRequest& req, http::HttpResponse* resp) override;
    
private:
    TetrisServer*         tetrisServer_;     // 游戏服务器

};

/*-------------------------------WebSocket相关的路由处理方法-------------------------------------*/
class PlayerJoinHandler : public http::router::WebSocketHandler
{
public:
    explicit PlayerJoinHandler(TetrisServer* tetrisServer) : tetrisServer_(tetrisServer) {}
    ~PlayerJoinHandler() = default;
    void handle(const json&, http::websocket::WebSocketFrame* ) override;

private:
    TetrisServer* tetrisServer_;
};




}




