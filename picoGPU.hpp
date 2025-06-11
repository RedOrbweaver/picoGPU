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

inline i2c_state_machine i2c_state;
inline uint64_t i2c_last_message = 0;
inline Info information;