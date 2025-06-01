#pragma once
#include "hmain.hpp"

constexpr int N_ENTITIES = 64;
constexpr int TEXT_BUFFER_SIZE = 2048;

enum class ENTITY_TYPE : uint8_t
{
    SHAPE, // data: shape; shape data...
    SPRITE,
    LINE,
    TEXT,
};

enum class SHAPE : uint8_t
{
    CIRCLE, // data: fill colour; edge colour
    RECTANGLE, // data: fill colour; edge colour
    TRIANGLE, // data: fill colour; edge colour; x y z positions (multiplied by size)
};

struct PACK Entity // 24 bytes
{
    // 4 bytes
    ENTITY_TYPE type;
    bool visible;
    uint8_t rotation;
    uint8_t layer;

    // 8 bytes
    vec2<uint16_t> pos;
    vec2<uint16_t> size;

    // 12 bytes
    uint8_t data[12];
};

enum class BACKGROUND_MODE
{
    SOLID_SHADE,
    TEXTURE
};

struct PACK Background
{
    bool visible = false;
    BACKGROUND_MODE mode;
    vec2<uint16_t> pos;
    vec2<uint16_t> size;
    vec2<uint16_t> source_size;
    uint8_t value;
    uint8_t* data;
};


inline Entity entity_buffer[N_ENTITIES];
inline char text_buffer[TEXT_BUFFER_SIZE];
inline Background background;

struct ScreenContext
{
    uint8_t* data;
    vec2<int> screen_size;
};
