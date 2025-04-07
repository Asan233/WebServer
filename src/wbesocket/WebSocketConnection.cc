#include "../../include/websocket/WebSocketConnection.h"
#include <muduo/base/Logging.h>
#include <vector>
#include <arpa/inet.h>

namespace http{
namespace websocket {

bool WebSocketConnection::parseWebSocketFrame(muduo::net::Buffer* buf, size_t length)
{
    size_t readable = buf->readableBytes();
    const char* data = buf->peek();
    //uint8_t payload_len = 0;

    // 状态机处理
    bool hasMore = true;
    bool ok = true;
    while(hasMore)
    {
        if(Status_ == kExpectHeader)
        {
            if(readable < 2)
            {
                // 数据不足，等待更多数据
                hasMore = false;
            }else
            {
                // 解析第一个字节
                Requestframe_.fin = (data[0] & 0x80) != 0;
                Requestframe_.rsv1 = (data[0] & 0x40) != 0;
                Requestframe_.rsv2 = (data[0] & 0x20) != 0;
                Requestframe_.rsv3 = (data[0] & 0x10) != 0;
                Requestframe_.opcode = static_cast<OpCode>(data[0] & 0x0F);
                
                // 解析第二个字节
                Requestframe_.mask = (data[1] & 0x80) != 0;
                Requestframe_.payload_length = data[1] & 0x7F;
                
                // 确定下一步状态和头部长度
                headerSize_ = 2;
                if (Requestframe_.payload_length == 126 
                    || Requestframe_.payload_length == 127) 
                {
                    Status_ = kExpectExtendedLength;
                }else 
                {
                    Status_ = Requestframe_.mask ? kExpectMask : kExpectPayload;
                }
                
                buf->retrieve(2);
                data = buf->peek();
                readable = buf->readableBytes();
            }

        }else if(Status_ == kExpectExtendedLength)
        {       
            if (Requestframe_.payload_length == 126) 
            {
                if (readable < 2) {
                    hasMore = false;
                }
                uint16_t len;
                memcpy(&len, data, 2);
                Requestframe_.payload_length = ntohs(len);
                headerSize_ += 2;
                buf->retrieve(2);
            } else if (Requestframe_.payload_length == 127) 
            {
                if (readable < 8) 
                {
                    hasMore = false;
                }
                uint64_t len;
                memcpy(&len, data, 8);
                Requestframe_.payload_length = be64toh(len);
                headerSize_ += 8;
                buf->retrieve(8);
            }
            
            Status_ = Requestframe_.mask ? kExpectMask : kExpectPayload;
            data = buf->peek();
            readable = buf->readableBytes();
        }else if(Status_ == kExpectMask)
        {
            if (readable < 4) 
            {
                hasMore = false;
            }else
            {
                memcpy(Requestframe_.masking_key, data, 4);
                headerSize_ += 4;
                buf->retrieve(4);
                Status_ = kExpectPayload;
                data = buf->peek();
                readable = buf->readableBytes();
            }
        }else if(Status_ == kExpectPayload)
        {
            if(readable < Requestframe_.payload_length) 
            {
                // 数据不足等待更多数据
                hasMore = false;
                return true;
            }
            // 读取载荷数据
            Requestframe_.payload.resize(Requestframe_.payload_length);
            memcpy(Requestframe_.payload.data(), data, Requestframe_.payload_length);
            buf->retrieve(Requestframe_.payload_length);
            // 应用掩码
            if (Requestframe_.mask) 
            {
                applyMask(Requestframe_.payload.data(), Requestframe_.payload_length, Requestframe_.masking_key);
            }
            Status_ = kGotAll;
            hasMore = false;
        }
    }
    return ok;
}

void WebSocketConnection::applyMask(uint8_t* data, size_t length, const uint8_t* mask) 
{
    for (size_t i = 0; i < length; ++i) 
    {
        data[i] ^= mask[i % 4];
    }
}

} // namespace websocket
} // namespace http



