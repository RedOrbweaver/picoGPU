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
