#pragma once

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>

//#include "glm/glm/vec2.hpp"

#include "pico/stdlib.h"
#include "pico/mutex.h"
#include "pico/multicore.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/interp.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "hardware/adc.h"

#include "redpicolib/RedPicoLib.hpp"
#include "dac_out.pio.h"


namespace PIN
{
    uint8_t DAC_OUT[8] = {2, 3, 4, 5, 6, 7, 8, 9};
    uint8_t NOT_SYNC = 10;
    uint8_t SPI_RX = 16;
    uint8_t SPI_CS = 17;
    uint8_t SPI_SCK = 18;
    uint8_t SPI_TX = 19;
}

#include "shared.hpp"
#include "picoGPU.hpp"
#include "PAL.hpp"


enum class SPI_STATE : uint8_t
{
    NONE=0,
    SOURCE,
    ADDR0,
    ADDR1,
    LEN,
    DATA,
};

struct spi_state_machine
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

inline spi_state_machine spi_state;
inline uint64_t spi_last_message = 0;