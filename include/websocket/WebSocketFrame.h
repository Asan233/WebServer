#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace http
{
namespace websocket
{

// WebSocket操作码
enum class OpCode : uint8_t {
    CONTINUATION = 0x0,
    TEXT = 0x1,
    BINARY = 0x2,
    CLOSE = 0x8,
    PING = 0x9,
    PONG = 0xA
};

// WebSocket帧结构
struct WebSocketFrame {
    bool fin;           // 是否是最后一个分片
    bool rsv1;          // 保留位1
    bool rsv2;          // 保留位2
    bool rsv3;          // 保留位3
    OpCode opcode;      // 操作码
    bool mask;          // 是否使用掩码
    uint64_t payload_length; // 载荷长度
    uint8_t masking_key[4];        // 掩码密钥
    std::vector<uint8_t> payload;   // 载荷数据

    std::string getPayload() const 
    {
        return std::string(payload.begin(), payload.end());
    }

    WebSocketFrame() : fin(true), rsv1(false), rsv2(false), rsv3(false),
                      opcode(OpCode::TEXT), mask(false), payload_length(0) 
    {
        masking_key[0] = masking_key[1] = masking_key[2] = masking_key[3] = 0;
    }

    void putPayload(const std::string& data) 
    {
        payload.resize(data.size());
        std::copy(data.begin(), data.end(), payload.begin());
        payload_length = data.size();
    }

        // 新增：序列化WebSocket帧为二进制数据
    std::vector<uint8_t> dump() const 
    {
        std::vector<uint8_t> frameData;
        
        // 第一个字节: FIN位 + 保留位 + 操作码
        uint8_t byte1 = 0;
        if (fin) byte1 |= 0x80;
        if (rsv1) byte1 |= 0x40;
        if (rsv2) byte1 |= 0x20;
        if (rsv3) byte1 |= 0x10;
        byte1 |= static_cast<uint8_t>(opcode) & 0x0F;
        frameData.push_back(byte1);
        
        // 第二个字节: 掩码位 + 负载长度
        uint8_t byte2 = 0;
        if (mask) byte2 |= 0x80;
        
        // 设置负载长度
        if (payload_length < 126) 
        {
            byte2 |= payload_length & 0x7F;
            frameData.push_back(byte2);
        } else if (payload_length <= 0xFFFF) 
        {
            byte2 |= 126;
            frameData.push_back(byte2);
            
            // 16位长度，网络字节序(大端)
            uint16_t len16 = htons(static_cast<uint16_t>(payload_length));
            frameData.push_back((len16 >> 8) & 0xFF);
            frameData.push_back(len16 & 0xFF);
        } else 
        {
            byte2 |= 127;
            frameData.push_back(byte2);
            
            // 64位长度，网络字节序(大端)
            uint64_t len64 = htobe64(payload_length);
            for (int i = 0; i < 8; ++i) 
            {
                frameData.push_back((len64 >> ((7 - i) * 8)) & 0xFF);
            }
        }
        
        // 添加掩码(如果有)
        if (mask) 
        {
            for (int i = 0; i < 4; ++i) {
                frameData.push_back(masking_key[i]);
            }
            
            // 如果使用掩码，需要对负载数据进行掩码处理
            std::vector<uint8_t> maskedPayload = payload;
            for (size_t i = 0; i < maskedPayload.size(); ++i) {
                maskedPayload[i] ^= masking_key[i % 4];
            }
            
            // 添加掩码后的负载数据
            frameData.insert(frameData.end(), maskedPayload.begin(), maskedPayload.end());
        } else 
        {
            // 添加原始负载数据
            frameData.insert(frameData.end(), payload.begin(), payload.end());
        }
        
        return frameData;
    }
};

}
}



