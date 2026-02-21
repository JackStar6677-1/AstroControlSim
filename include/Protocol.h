#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>

// 1 byte alignment to ensure consistent structure across network
#pragma pack(push, 1)

enum PacketType : uint8_t {
    CMD_MOVE = 1,
    TELEMETRY = 2,
    CMD_GET_TELEMETRY = 3,
    CMD_RESET = 4
};

struct Packet {
    uint8_t type;
    uint32_t antennaId; // 0 for broadcast/all, or specific ID
    double azimuth;
    double elevation;
    uint8_t state; // AntennaState enum
};

#pragma pack(pop)

#endif
