#include "../../include/handle/RoomManagerHandler.h"

#include "../../include/TetrisServer.h"

namespace Tetris
{

/* 游戏大厅房间信息获取路由 */
void RoomsInfoHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    nlohmann::json roomsInfo;
    roomsInfo["status"] = "ok";
    roomsInfo["rooms"] =  RoomManager::getInstance()->getAllRoomsInfo();
    std::string buf = roomsInfo.dump(4);
    tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::k200Ok, "ok",
                               false, "application/json", buf.size(), buf, resp);
}

/* 游戏房间创建路由 */
void RoomsCreateHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    nlohmann::json roomsInfo;
    auto session = tetrisServer_->getSessionManager()->getSession(req, resp);
    std::string userId = session->getValue("userId");
    std::string userName = session->getValue("userName");

    if( tetrisServer_->getUserOnline(userId) )
    {
        auto Roomptr = RoomManager::getInstance()->createRoom(userId, userName);
        roomsInfo["status"] = "ok";
        roomsInfo["roomId"] = Roomptr->getId();
    }

    std::string buf = roomsInfo.dump(4);
    tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::k200Ok, "ok",
                               false, "application/json", buf.size(), buf, resp);
}

/* 游戏玩家加入房间处理 */
void RoomsOpHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    std::string roomId = req.getPathParameters("param1");
    std::string op = req.getPathParameters("param2");
    std::string userId = req.getPathParameters("param3");
    std::string userName = req.getPathParameters("param4");

    auto room = RoomManager::getInstance()->getRoom(roomId);

    if(op == "join")
    {
        if ( room->getPlayerCount() < 2)
        {
            //room->addPlayer(userId, userName);
            // 加载房间页面
            std::string reqFile;
            reqFile.append("../WebApps/Tetris/resource/room.html");
            FileUtil fileOperater(reqFile);
            if(!fileOperater.isValid())
            {
                LOG_WARN << reqFile << " not exist";
                // 404 not found
                fileOperater.resetDefaultFile();
            }
            std::vector<char> buffer(fileOperater.size());
            fileOperater.readFile(buffer);
            std::string bufStr = std::string(buffer.data(), buffer.size());
            tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::k200Ok, "ok",
                                       false, "text/html", bufStr.size(), bufStr, resp);
        }
        else
        {
            json message;
            message["status"] = "error";
            message["message"] = "Room is full";
            std::string Body = message.dump(4);
            tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::k200Ok, "error",
                                       false, "text/html", message.size(), Body, resp);
        }
    }else if(op == "quit") 
    {

    }

    // nlohmann::json roomsInfo;
    // auto session = tetrisServer_->getSessionManager()->getSession(req, resp);
    // std::string userId = session->getValue("userId");
    // std::string userName = session->getValue("userName");

    // if( tetrisServer_->getUserOnline(userId) )
    // {
    //     auto roomId = req.getParam("roomId");
    //     auto Roomptr = RoomManager::getInstance()->joinRoom(roomId, userId, userName);
    //     if(Roomptr)
    //     {
    //         roomsInfo["status"] = "ok";
    //         roomsInfo["roomId"] = Roomptr->getId();
    //     }
    //     else
    //     {
    //         roomsInfo["status"] = "error";
    //         roomsInfo["message"] = "Room not found or full";
    //     }
    // }

    // std::string buf = roomsInfo.dump(4);
    // tetrisServer_->packageResp(req.getVersion(), http::HttpResponse::k200Ok, "ok",
    //                            false, "application/json", buf.size(), buf, resp);
}


/*-------------------------------WebSocket相关的路由处理方法-------------------------------------*/
void PlayerJoinHandler::handle(const json& jsonMsg, http::websocket::WebSocketFrame* frame)
{
    std::string roomId = jsonMsg["roomId"];
    std::string userId = jsonMsg["userId"];
    std::string userName = jsonMsg["username"];
    auto room = RoomManager::getInstance()->getRoom(roomId);
    if ( room )
    {
        if ( room->addPlayer(userId, userName) )
        {
            json response;
            response["type"] = "playerJoined";
            response["player"] = userName;
            frame->putPayload(response.dump());
            auto data = frame->dump();
            std::vector<std::string> players = room->getAllUserId();
            for(const auto& playerId : players)
            {
                auto conn = http::websocket::WebSocketManager::instance()->getConnection(playerId);
                if (conn)
                {
                    conn->getLoop()->runInLoop([conn, data]() {
                        conn->send(data.data(), data.size());
                    });
                }
            }
        }
        else
        {
            json response;
            response["status"] = "error";
            response["message"] = "Room is full or already started";
            frame->putPayload(response.dump());
        }
    }
}




}
