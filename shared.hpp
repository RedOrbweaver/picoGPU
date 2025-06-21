#pragma once
//#include "hmain.hpp"


constexpr uint sysclockkhz = 300000;
constexpr uint i2cspeed = 1000 * 1000;

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
    TEXTURE0,
    TEXTURE1, // adds 65536 to the address to double the useable texture buffer size
    INFO,
    TEXTURE_BUFFER_SIZE,
    FRAME_NUMBER,
    GEOMETRY,
};

constexpr int N_ENTITIES = 256;
constexpr int TEXT_BUFFER_SIZE = 2048;
constexpr int GEOMETRY_BUFFER_SIZE = 512;

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
    SPRITE, // data: buffer start(32 bit), buffer len(32 bit), center; for now size and rotation are ignored
    LINE, // pos is p0, size is p1; data: color
    TEXT, // data: font, text buffer pos (16 bit), text len (16 bit), alignment
    POINT, // data: color
    MULTI_POINT, // data: color, geometry buffer start (16 bit), geometry buffer end (16 bit)
    MULTI_LINE, // data: color, geometry buffer start (16 bit), geometry buffer end (16 bit)
    BEZIER, // data: color, geometry buffer start (16 bit), geometry buffer end (16 bit)
};

enum class SHAPE : uint8_t
{
    CIRCLE, // data: border colour; fill colour, center
    RECTANGLE, // data: border colour; fill colour, center
    TRIANGLE, // data: fill colour; x y z positions (16 bit signed, will be multiplied by size), center around zero;
    MULTI_TRIANGLE, // data: fill, geometry buffer start (16 bit), geometry buffer end (16 bit), center around zero
    EMPTY_CIRCLE, // data: colour, center
    EMPTY_RECTANGLE, // data: colour, center
};

struct PACK Entity // 28 bytes
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
    uint8_t data[16];
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
};

struct PACK Settings
{

};

struct PACK Info
{
    uint64_t last_render_time_us;
    uint32_t entities_drawn;
    uint32_t total_memory;
    uint32_t free_memory;
    float temperature;
    uint32_t frame_number;
};