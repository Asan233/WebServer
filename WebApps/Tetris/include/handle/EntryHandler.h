#pragma once

#include "../../../../include/router/RouterHandler.h"

namespace Tetris
{

class TetrisServer;

class EntryHandler : public http::router::RouterHandler
{
public:
    explicit EntryHandler (TetrisServer* terisServer) : terisServer_(terisServer) {}

    void handle(const http::HttpRequest& req, http::HttpResponse* resp) override;

    ~EntryHandler() = default;

private:
    TetrisServer* terisServer_;
};

}
