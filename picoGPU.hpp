#pragma once
#include "hmain.hpp"

inline Entity entity_buffer[N_ENTITIES];
inline char text_buffer[TEXT_BUFFER_SIZE];
inline Background background;

struct ScreenContext
{
    uint8_t* data;
    vec2<int> screen_size;
};


enum class SPI_STATE : uint8_t
{
    NONE=0,
    SOURCE,
    ADDR0,
    ADDR1,
    LEN,
    DATA,
};

struct i2c_state_machine
{
    SPI_STATE state;
    SOURCE source;
    uint8_t address0;
    uint8_t address1;
    uint8_t len;
    uint8_t data[255];
    uint8_t data_pos;
    bool read;
};

template<typename T>
T SQ(T v)
{
    return v * v; 
}
template<typename T>
T Distance(T x0, T y0, T x1, T y1)
{
    return sqrt(SQ(x1-x0) + SQ(y1-y0));
}

inline void SetPixel(const ScreenContext& context, uint8_t val, vec2<int> pos)
{
    context.data[pos.y*context.screen_size.x + pos.x] = val;
}
inline void SetPixelSafe(const ScreenContext& context, uint8_t val, vec2<int> pos)
{
    if(pos.x >= context.screen_size.x || pos.x < 0)
        return;
    if(pos.y >= context.screen_size.y || pos.y < 0)
        return;
    SetPixel(context, val, pos);
}

void DrawCircle(const ScreenContext& context, uint8_t border, uint8_t fill, vec2<int> pos, vec2<int> radius, uint8_t rotation);
void DrawRectangle(const ScreenContext& context, uint8_t border, uint8_t fill, vec2<int> pos, vec2<int> size, uint8_t rotation);
void DrawTriangle(const ScreenContext& context, uint8_t fill, vec2<int> pos, vec2<int>size, vec2<int> p0, vec2<int> p1, vec2<int> p2, bool centertriangle);
void DrawLine(const ScreenContext& context, uint8_t fill, vec2<int> p0, vec2<int> p1);
void DrawText(const ScreenContext& context, vec2<uint16_t> pos, FONT font, uint16_t bufstart, uint16_t len, TEXT_ALIGNMENT alignment);
void DrawSprite(const ScreenContext& context, vec2<uint16_t> pos16, vec2<uint16_t> size16, uint32_t start, uint32_t len, bool center);
void DrawEntity(const Entity& entity, const ScreenContext& context);

inline i2c_state_machine i2c_state;
inline uint64_t i2c_last_message = 0;
inline Info information;
inline uint32_t texture_buffer_size = 0;
inline uint8_t* texture_buffer = nullptr;