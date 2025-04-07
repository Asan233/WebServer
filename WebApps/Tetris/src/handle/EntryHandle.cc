#include "../../include/handle/EntryHandler.h"

#include "../../include/TetrisServer.h"

namespace Tetris
{

/*  处理网站默认登录页面的Get请求 */
void EntryHandler::handle(const http::HttpRequest& req, http::HttpResponse* resp)
{
    // Get请求Url得到文件则可以直接返回响应
    std::string reqFile;
    reqFile.append("../WebApps/Tetris/resource/index.html");
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


    terisServer_->packageResp(req.getVersion(), http::HttpResponse::k200Ok, "ok",
                              false, "text/html", bufStr.size(), bufStr, resp);

    // 返回请求
    // resp->setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
    // resp->setCloseConnection(false);
    // resp->setContentType("text/html");
    // resp->setContentLength(bufStr.size());
    // resp->setBody(bufStr);
}

    
}



