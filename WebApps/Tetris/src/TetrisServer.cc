#include "../include/TetrisServer.h"



namespace Tetris
{

TetrisServer::TetrisServer( int port, const std::string& name, muduo::net::TcpServer::Option option) 
            :  server_(port, name +  "_Http", false, option)
              ,name_(name)                
{
    //std::thread( std::bind(&Tetris::TetrisServer::WebSocketstart, this, port + 1, name , option) ).detach();
    initialize();
}

/* 设置线程数量 */
void TetrisServer::setHttpServerThreadNum(int numThreads)
{
    server_.setThreeadNum(numThreads);
}

/* 服务器运行 */
void TetrisServer::Httpstart()
{
    server_.start();
}


void TetrisServer::initialize()
{
    // 初始化数据库连接池
    http::MysqlUtil::init("tcp://127.0.0.1:3306", "asan", "200909", "tetris", 10);
    // 初始化会话管理
    initializeSession();
    // 初始化中间件
    initializeMiddlewares();
    // 初始化路由
    initializeRouter();
}

void TetrisServer::initializeSession()
{
    // 创建会话存储
    auto sessionStorage = std::make_unique<http::session::MemorySessionStorage>();
    // 创建会话管理器
    auto sessionManager = std::make_unique<http::session::SessionManager>(std::move(sessionStorage));
    // 设置会话管理器
    setSessionManager(std::move(sessionManager));
}

void TetrisServer::initializeMiddlewares()
{
    // 创建中间件
    auto corsMiddleware = std::make_shared<http::middleware::CorsMiddleware>();
    // 添加中间件
    server_.addMiddleware(corsMiddleware);
}

void TetrisServer::initializeRouter()
{
    // 注册url回调处理器
    // 登录注册入口页面
    server_.Get("/", std::make_shared<EntryHandler>(this));                     // 登录注册入口页面
    server_.Get("/entry", std::make_shared<EntryHandler>(this));                // 登录注册入口页面
    server_.Post("/register", std::make_shared<RegisterHandler>(this));         // 注册
    server_.Post("/login", std::make_shared<LoginHandler>(this));               // 登录
    server_.Get("/menu", std::make_shared<MenuHandler>(this));                  // 游戏菜单
    /*---------游戏大厅-------------*/
    server_.Get("/api/user/status", std::make_shared<UserStatusHandler>(this));         // 用户状态
    server_.Get("/api/rooms", std::make_shared<RoomsInfoHandler>(this));                // 房间信息
    server_.Post("/api/rooms/create", std::make_shared<RoomsCreateHandler>(this));      // 创建房间
    server_.addRoute(http::HttpRequest::Method::kGet, "/room/:roomId/op/:op/userId/:userId/username/:username",
                     std::make_shared<RoomsOpHandler>(this));                           // 加入房间
    server_.Get("/ws/upgrade", std::make_shared<UpgradHandler>(this));                   // 加入房间
}

/* WebSocket消息处理类，根据消息类型进行玩家加入或者退出房间，准备或者开始游戏 */
void TetrisServer::handleGame(const json& jsonMsg, http::websocket::WebSocketFrame* frame)
{
    std::string type = jsonMsg["type"];
    if (type == "join")
    {
        // 处理加入房间的逻辑
    }
    else if (type == "leave")
    {
        // 处理离开房间的逻辑
    }
    else if (type == "start")
    {
        // 处理开始游戏的逻辑
    }
}

void TetrisServer::packageResp(const std::string& version,
                     http::HttpResponse::HttpStatusCode statusCode,
                     const std::string& statusMsg,
                     bool close, const std::string& contentType,
                     int contentLen, const std::string& body,
                     http::HttpResponse* resp)
    {
        if(resp == nullptr)
        {
            LOG_ERROR << "Response pointer is nullptr";
            return;
        }

        try
        {
            resp->setStatusLine(version, statusCode, statusMsg);
            resp->setCloseConnection(close);
            resp->setContentType(contentType);
            resp->setContentLength(contentLen);
            resp->setBody(body);
        }
        catch(const std::exception& e)
        {
            LOG_ERROR << "Error in packageResp: " << e.what();
            // 设置一个基本的错误响应
            resp->setStatusCode(http::HttpResponse::k500InternalServerError);
            resp->setStatusMessage("Internal Server Error");
            resp->setCloseConnection(true);
        }
    }



} // namespace Tetris



