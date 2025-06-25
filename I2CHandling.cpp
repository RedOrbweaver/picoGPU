#include "hmain.hpp"

void HandleI2CWrite(SOURCE source, uint8_t address0, uint8_t address1, uint8_t len, uint8_t* data)
{
    if(len == 0)
    {
        printf("Zero length i2c write attempted!\n");
        return;
    }
    switch(source)
    {
        case SOURCE::DEBUG_PRITNF:
        {
            uint8_t buf[256];
            memcpy(buf, data, len);
            buf[len] = '\0';
            printf("%s\n", buf);

            break;
        }
        case SOURCE::ENTITY_BUFFER:
        {
            // Somewhat ensures that the entity buffer is not being copied.
            dmawaitformemcopies();

            if(address0 >= N_ENTITIES)
            {
                printf("Out of bounds read of the entity buffer at %i!\n", (int)address0);
                break;
            }
            uint8_t* entdt = (uint8_t*)(entity_buffer + address0);
            if((int)address1 + (int)len > sizeof(Entity))
            {
                printf("Out of bounds write to an entity at %i for len %i!\n",(int)address1, (int)len);
                break;
            }
            for(int i = 0; i < len; i++)
            {
                entdt[address1+i] = data[i];
            }

            break;
        }
        case SOURCE::TEXT_BUFFER:
        {
            // see above
            dmawaitformemcopies();

            uint16_t address = uint16_t(address1) << 8 | uint16_t(address0);
            if(address + len > TEXT_BUFFER_SIZE)
            {
                printf("Attempting to write to the text buffer beyond its bounds at %i for len %i\n", (int)address, (int)len);
                break;
            }
            
            for(int i = 0; i < len; i++)
            {
                text_buffer[address+i] = data[i];
            }
            break;
        }
        case SOURCE::TEXTURE_BUFFER_SIZE:
        {
            uint32_t nsize = *(uint32_t*)data;
            if(nsize != texture_buffer_size)
            {
                uint32_t freemem = GetFreeHeap() + texture_buffer_size;
                if((freemem) < nsize)
                {
                    printf("Attempting to allocate a texture buffer which would not fit into memory! Free memory: %u Buffer size: %u\n", freemem, nsize);
                    break;
                }
                if(texture_buffer != nullptr)
                {
                    delete[] texture_buffer;
                }
                texture_buffer_size = nsize;
                texture_buffer = new uint8_t[nsize];
            }
            break;
        }
        case SOURCE::TEXTURE0:
        case SOURCE::TEXTURE1:
        {
            uint32_t address = uint16_t(address1) << 8 | uint16_t(address0);
            if(source == SOURCE::TEXTURE1)
            {
                address += 65536;
            }
            if(texture_buffer == nullptr)
            {
                printf("Attempting to write to texture buffer before initializing it\n");
                break;
            }
            if(address + len > texture_buffer_size)
            {
                printf("Attempting to write outside the texture buffer bounds at %u for %i\n", address, (int)len);
                break;
            }
            memcpy(texture_buffer + address, data, len);
            break;
        }
        case SOURCE::INFO:
        {
            printf("Writing to INFO is not supported!\n");
            break;
        }
        case SOURCE::BACKGROUND_SETTINGS:
        {
            if(len != sizeof(Background))
            {
                printf("Invalid size for background %u\n", (uint32_t)len);
                break;
            }
            memcpy(&background, data, sizeof(Background));
            UpdateBackground();
            break;
        }
        case SOURCE::BACKGROUND_TEXTURE:
        {
            uint32_t address = uint16_t(address1) << 8 | uint16_t(address0);
            if(background.mode != BACKGROUND_MODE::TEXTURE)
            {
                printf("Background not set to texture mode\n");
                break;
            }
            if(background_texture_buffer == nullptr)
            {
                printf("Background texture is nullptr\n");
                break;
            }
            if(address + len > background.size.x*background.size.y)
            {
                printf("Attempting to write to background texture outside the bonds %u\n", address);
                break;
            }
            memcpy(background_texture_buffer + address, data, len);
            break;
        }
        case SOURCE::GEOMETRY:
        {
            uint32_t address = uint16_t(address1) << 8 | uint16_t(address0);
            if(address + len/4 > GEOMETRY_BUFFER_SIZE)
            {
                printf("Attempting to write to geometry buffer outside the bonds %u\n", address);
                break;
            }
            if(len % 4 != 0)
            {
                printf("Write length geometry buffer must be divisible by 4\n");
                break;
            }
            if(len < 4)
            {
                printf("Write length to the geometry buffer must be at least 4\n");
                break;
            }

            memcpy(geometry_buffer + address, data, len);

            break;
        }
        default:
        {
            printf("Invalid source: %i\n", (int)source);
            break;
        }
    }
}

void I2CHandlerInternal()
{
    
    if(i2c_last_message != 0)
    {
        // uint64_t time_since_last_message = get_time_us() - i2c_last_message;
        // if(time_since_last_message > 10000) // after 10ms reset the state
        //     spi_state.state = SPI_STATE::NONE;
    }
    i2c_last_message = get_time_us();

    uint8_t value = i2c0->hw->data_cmd & I2C_IC_DATA_CMD_BITS;
    

    switch(i2c_state.state)
    {
        case SPI_STATE::NONE:
        {
            if(value == 0xEF)
            {
                i2c_state.read = true;
            }
            else if(value == 0xFF)
            {
                i2c_state.read = false;
            }
            else
            {
                printf("ERROR: INVALID FIRST I2C VALUE: %i\n", (int)value);
                break;
            }
            i2c_state.state = SPI_STATE::SOURCE;
            break;
        }
        case SPI_STATE::SOURCE:
        {
            i2c_state.source = (SOURCE)value;
            i2c_state.state = SPI_STATE::ADDR0;
            break;
        }
        case SPI_STATE::ADDR0:
        {
            i2c_state.address0 = value;
            i2c_state.state = SPI_STATE::ADDR1;
            break;
        }
        case SPI_STATE::ADDR1:
        {
            i2c_state.address1 = value;
            i2c_state.state = SPI_STATE::LEN;
            break;
        }
        case SPI_STATE::LEN:
        {

            i2c_state.len = value;
            i2c_state.data_pos = 0;
            if(i2c_state.read == true)
            {
                i2c_state.state = SPI_STATE::NONE;
                i2c_state.data_pos = 0;
            }
            else
            {
                i2c_state.state = SPI_STATE::DATA;
                //spi_state.state = SPI_STATE::NONE;
                //spi_read_blocking(spi0, 0, spi_state.data, spi_state.len);
                //HandleSPIWrite(spi_state.source, spi_state.address0, spi_state.address1, spi_state.len, spi_state.data);
            }
            break;
        }
        case SPI_STATE::DATA:
        {
            i2c_state.data[i2c_state.data_pos] = value;
            i2c_state.data_pos++;
            if(i2c_state.data_pos == i2c_state.len)
            {
                i2c_state.state = SPI_STATE::NONE;
                HandleI2CWrite(i2c_state.source, i2c_state.address0, i2c_state.address1, i2c_state.len, i2c_state.data);
            }
            break;
        }
        default:
        {
            assert(false, "Invalid I2C state!");
        }
    }
}
uint8_t I2CGetNextByte()
{
    uint8_t value;
    switch(i2c_state.source)
    {
        case SOURCE::DEBUG_PRITNF:
        {
            value = i2c_state.data[i2c_state.data_pos++];
            i2c_state.data_pos %= i2c_state.len;
            break;
        }
        case SOURCE::INFO:
        {
            if(i2c_state.data_pos >= sizeof(Info))
            {
                printf("Out of bounds read of info at %i!\n", (int)i2c_state.data_pos);
                return 0xFF;
            }
            value = ((uint8_t*)&information)[i2c_state.data_pos++];
            break;
        }
        case SOURCE::TEST:
        {
            value = 0x13;
            break;
        }
        case SOURCE::FRAME_NUMBER:
        {
            if(i2c_state.data_pos >= sizeof(Info::frame_number))
            {
                printf("Out of bounds read of frame number at %i!\n", (int)i2c_state.data_pos);
                return 0xFF;
            }
            value = ((uint8_t*)&information.frame_number)[i2c_state.data_pos++];
            break;
        }
        case SOURCE::TEXTURE_BUFFER_SIZE:
        {
            if(i2c_state.data_pos >= sizeof(Info::frame_number))
            {
                printf("Out of bounds read of texture buffer size at %i!\n", (int)i2c_state.data_pos);
                return 0xFF;
            }
            value = ((uint8_t*)&texture_buffer_size)[i2c_state.data_pos++];
            break;
        }
    }
    return value;
}


void I2CHandler()
{
    uint8_t status = i2c0->hw->intr_stat;
    if(status & I2C_IC_INTR_STAT_R_RX_FULL_BITS)
    {
        I2CHandlerInternal();
    }
    else if(status & I2C_IC_INTR_STAT_R_RD_REQ_BITS)
    {
        uint8_t v = I2CGetNextByte();
        //printf("writing %i\n", (int)v);
        i2c0->hw->data_cmd = v;
        i2c0->hw->clr_rd_req;
    }
}