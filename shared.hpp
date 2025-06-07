#pragma once
//#include "hmain.hpp"


constexpr uint sysclockkhz = 300000;

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
    DEBUG_PRITNF,
    ENTITY_BUFFER,
    TEXT_BUFFER,
    BACKGROUND_SETTINGS,
    BACKGROUND_TEXTURE,
    TEXTURE,
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

enum class ENTITY_TYPE : uint8_t
{
    SHAPE, // data: shape; shape data...
    SPRITE,
    LINE,
    TEXT, // data: font, text buffer pos (16 bit), text len (16 bit)
};

enum class SHAPE : uint8_t
{
    CIRCLE, // data: border colour; fill colour
    RECTANGLE, // data: border colour; fill colour
    TRIANGLE, // data: border colour; fill colour; x y z positions (multiplied by size)
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