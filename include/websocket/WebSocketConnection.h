#pragma once

#pragma once

#include <muduo/net/TcpConnection.h>
#include <string>
#include <memory>
#include <functional>

#include "WebSocketFrame.h"

namespace http{
namespace websocket {

class WebSocketConnection : public std::enable_shared_from_this<WebSocketConnection> {
public:
    /* WebSocket帧解析状态 */
    enum WebSocketParseStatus {
        kExpectHeader,          // 期望解析头部(前2字节)
        kExpectExtendedLength,  // 期望解析扩展长度
        kExpectMask,            // 期望解析掩码
        kExpectPayload,         // 期望解析载荷
        kGotAll                 // 解析完成
    };

    WebSocketConnection() : Status_(kExpectHeader) {}
    
    //void send(const std::string& message, OpCode opCode = OpCode::TEXT);
    //void close();
    
    // 解析WebSocket帧
    bool parseWebSocketFrame(muduo::net::Buffer*, size_t length);

    WebSocketFrame& getRequestFrame() { return Requestframe_; }

    bool gotAll() const { return Status_ == kGotAll; }
    
    // 重置上下文状态
    void reset()
    {
        WebSocketFrame dummy;
        Status_ = kExpectHeader;
        Requestframe_ = dummy;
        frameSize_ = 0;
        headerSize_ = 0;
        payloadSize_ = 0;
    }

private:
    // 应用掩码到载荷数据
    void applyMask(uint8_t* data, size_t length, const uint8_t* mask);
    
private:

    WebSocketParseStatus            Status_;  // 解析状态
    WebSocketFrame                  Requestframe_;      // 当前解析帧
    size_t                          frameSize_ = 0;     // 帧总大小
    size_t                          headerSize_ = 0;    // 头部大小(包括扩展长度和掩码)
    size_t                          payloadSize_ = 0;   // 载荷大小

};

} // namespace websocket
} // namespace http
