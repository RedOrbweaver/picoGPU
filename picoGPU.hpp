#pragma once
#include "hmain.hpp"

enum ENTITY_TYPE : uint8_t
{
    SHAPE_SOLID,
    SPRITE,
    LINE,
    TEXT,
};

struct PACK Entity // 32 bytes
{
    ENTITY_TYPE type;
    bool visible;
    uint8_t rotation;
    uint8_t layer;
    uint16_t x;
    uint16_t y;
    uint16_t sx; 
    uint16_t sy;
    uint8_t data[20];
};

struct PACK Background
{
    bool visible = false;
    uint16_t x;
    uint16_t y;
    uint16_t sx;
    uint16_t sy;
    uint16_t px;
    uint16_t py;
    void* data;
};


inline Entity entity_buffer[32];
inline Background background;
