#pragma once
//#include "hmain.hpp"


constexpr uint sysclockkhz = 300000;
constexpr uint i2cspeed = 100 * 1000;

enum RENDER_MODE
{
    BW_408_304_DOUBLE_BUF,
    BW_408_305_DOUBLE_BUF_INTERLACED,
};
enum SAMPLING_MODE
{
    SPP_1,
    SPP_2,
    SPP_4,
};
enum class SOURCE : uint8_t
{
    TEST,
    DEBUG_PRITNF,
    SETTINGS,
    ENTITY_BUFFER,
    TEXT_BUFFER,
    BACKGROUND_SETTINGS,
    BACKGROUND_TEXTURE,
    TEXTURE,
    INFO,
};

constexpr int N_ENTITIES = 64;
constexpr int TEXT_BUFFER_SIZE = 2048;

enum class FONT : uint8_t
{
    DEJAVUSANS_12 = 0,
    DEJAVUSANS_12_BW = 1,
    DEJAVUSERIF_F_16 = 2,
    DEJAVUSERIF_F_32 = 3,
    FIXED_5_8 = 4,
    FIXED_7_14 = 5,
    FIXED_10_20 = 6,
};

enum class TEXT_ALIGNMENT : uint8_t
{
    LEFT=0,
    CENTER=1,
    RIGHT=2
};

enum class ENTITY_TYPE : uint8_t
{
    SHAPE, // data: shape; shape data...
    SPRITE, // data: buffer start(16 bit), buffer len(16 bit)
    LINE, // data: color, width. pos is p1 size is p2
    TEXT, // data: font, text buffer pos (16 bit), text len (16 bit), alignment
};

enum class SHAPE : uint8_t
{
    CIRCLE, // data: border colour; fill colour
    RECTANGLE, // data: border colour; fill colour
    TRIANGLE, // data: fill colour; x y z positions (8 bit, will be multiplied by size), center
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

struct PACK Settings
{

};

struct PACK Info
{
    uint64_t last_render_time_us;
};