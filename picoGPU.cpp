#include "hmain.hpp"

#include "fonts.h"
#include "mcufont.h"

PAL_DRIVER* driver;
int dmamemcpychan1;
dma_channel_config dmamemcpychan1c;
int dmamemcpychan4;
dma_channel_config dmamemcpychan4c;
int dmamemsetchan4;
dma_channel_config dmamemsetchan4c;


void dmawaitcpy1()
{
    dma_channel_wait_for_finish_blocking(dmamemcpychan1);
}
void dmamemcpy1(void* to, void* from, size_t size, bool wait)
{
    dma_channel_configure
    (
        dmamemcpychan1,
        &dmamemcpychan1c,
        to,
        from,
        size,
        true
    );
    if(wait)
        dmawaitcpy1();
}
void dmawaitcpy4()
{
    dma_channel_wait_for_finish_blocking(dmamemcpychan4);
}
void dmamemcpy4(void* to, void* from, size_t size, bool wait)
{
    assert(size % 4 == 0);
    dma_channel_configure
    (
        dmamemcpychan4,
        &dmamemcpychan4c,
        to,
        from,
        size/4,
        true
    );
    if(wait)
        dmawaitcpy4();
}
void dmawaitmemset4()
{
    dma_channel_wait_for_finish_blocking(dmamemsetchan4);
}
void dmamemset4(void* to, uint8_t value, size_t size, bool wait)
{
    assert(size % 4 == 0);
    uint8_t val[4] = {value, value, value, value};
    dma_channel_configure
    (
        dmamemsetchan4,
        &dmamemsetchan4c,
        to,
        val,
        size/4,
        true
    );
    if(wait)
        dmawaitmemset4();
}

void dmawaitformemcopies()
{
    dmawaitcpy1();
    dmawaitcpy4();
}

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

float ReadTemperature()
{
    adc_select_input(4);
    uint16_t v = adc_read();
    float temperature = 27 - ((float(v)*(3.3f / (1 << 12))) - 0.706)/0.001721;
    return temperature;
}

void WriteTemperature()
{
    float temp = ReadTemperature();
    printf("Temperature: %.3f\n", temp);
}

void InitRendering()
{
    dmamemcpychan1 = dma_claim_unused_channel(true);
    dmamemcpychan1c = dma_channel_get_default_config(dmamemcpychan1);
    channel_config_set_transfer_data_size(&dmamemcpychan1c, DMA_SIZE_8);
    channel_config_set_read_increment(&dmamemcpychan1c, true);
    channel_config_set_write_increment(&dmamemcpychan1c, true);       
    channel_config_set_enable(&dmamemcpychan1c, true);

    dmamemcpychan4 = dma_claim_unused_channel(true);
    dmamemcpychan4c = dma_channel_get_default_config(dmamemcpychan4);
    channel_config_set_transfer_data_size(&dmamemcpychan4c, DMA_SIZE_32);
    channel_config_set_read_increment(&dmamemcpychan4c, true);
    channel_config_set_write_increment(&dmamemcpychan4c, true);
    channel_config_set_enable(&dmamemcpychan4c, true);


    dmamemsetchan4 = dma_claim_unused_channel(true);
    dmamemsetchan4c = dma_channel_get_default_config(dmamemsetchan4);
    channel_config_set_transfer_data_size(&dmamemsetchan4c, DMA_SIZE_32);
    channel_config_set_read_increment(&dmamemsetchan4c, false);
    channel_config_set_write_increment(&dmamemsetchan4c, true);
    channel_config_set_enable(&dmamemsetchan4c, true);

    for(int i = 0; i < N_ENTITIES; i++)
        entity_buffer[i].visible = false;
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

void DrawCircle(const ScreenContext& context, uint8_t border, uint8_t fill, vec2<int> pos, vec2<int> radius, uint8_t rotation)
{
    int mx = std::max(radius.x, radius.y);

    if(radius.x == radius.y || rotation == 0)
    {
        int lmxx = std::min(pos.x+radius.x, context.screen_size.x);
        int lmxy = std::min(pos.y+radius.y, context.screen_size.y);
        int lmnx = std::max(pos.x-radius.x, 0);
        int lmny = std::max(pos.y-radius.y, 0);

        if(radius.x == radius.y)
        {
            for(int i = lmnx; i < lmxx; i++)
            {
                for(int ii = lmny; ii < lmxy; ii++)
                {
                    int dist = Distance(float(i), float(ii), float(pos.x), float(pos.y));
                    if(dist <= mx)
                    {
                        if(dist == mx || dist == mx-1)
                            SetPixel(context, border, {i, ii});
                        else 
                            SetPixel(context, fill, {i, ii});
                    }    
                }
            }
        }
        else
        {
            for(int i = lmnx; i < lmxx; i++)
            {
                for(int ii = lmny; ii < lmxy; ii++)
                {
                    float d = SQ(float(i)-float(pos.x)) / SQ(float(radius.x)) + SQ(float(ii)-float(pos.y)) / SQ(float(radius.y));
                    if(d <= 1)
                    {
                        if(d < 0.99)
                            SetPixel(context, fill, {i, ii});   
                        else
                            SetPixel(context, border, {i, ii});
                    }
                }
            }
        }
    }
    else
    {
        float rot = M_PI*float(rotation)/255.0f;

        int lmxx = std::min(pos.x+mx, context.screen_size.x);
        int lmxy = std::min(pos.y+mx, context.screen_size.y);
        int lmnx = std::max(pos.x-mx, 0);
        int lmny = std::max(pos.y-mx, 0);

        for(int i = lmnx; i < lmxx; i++)
            {
                for(int ii = lmny; ii < lmxy; ii++)
                {
                    float d = SQ((float(i)-float(pos.x))*cos(rot) + (float(ii)-float(pos.y))*sin(rot))/SQ(float(radius.x)) +
                        SQ((float(i)-float(pos.x))*sin(rot) - (float(ii)-float(pos.y))*cos(rot))/SQ(float(radius.y));
                    if(d <= 1)
                    {
                        if(d < 0.99)
                            SetPixel(context, fill, {i, ii});   
                        else
                            SetPixel(context, border, {i, ii});
                    }
                }
            }
    }
}
void DrawRectangle(const ScreenContext& context, uint8_t border, uint8_t fill, vec2<int> pos, vec2<int> size, uint8_t rotation)
{
    if(rotation == 0)
    {
        size.x -= 1;
        size.y -= 1;
        pos.x += 1;
        pos.y += 1;
        for(int i = std::max(0, pos.x-size.x/2); i < std::min(context.screen_size.x, pos.x+size.x/2); i++)
        {
            for(int ii = std::max(0, pos.y-size.y/2); ii < std::min(context.screen_size.y, pos.y+size.y/2); ii++)
            {
                SetPixel(context, fill, {i, ii});
            }
        }
        size.x += 1;
        size.y += 1;
        pos.x -= 1;
        pos.y -= 1;
        for(int i = 0; i < size.x; i++)
            SetPixel(context, border, {i+pos.x-size.x/2, pos.y-size.y/2});
        for(int i = 0; i < size.x; i++)
            SetPixel(context, border, {i+pos.x-size.x/2, pos.y+size.y/2});
        for(int i = 0; i < size.y; i++)
            SetPixel(context, border, {pos.x-size.x/2, i+pos.y-size.y/2});
        for(int i = 0; i < size.y; i++)
            SetPixel(context, border, {pos.x+size.x/2, i+pos.y-size.y/2});
    }
    else
    {

    }
}
void DrawTriangle(const ScreenContext& context, uint8_t border, uint8_t fill, vec2<int> pos, vec2<int> size, vec2<int> p0, vec2<int> p1, vec2<int> p2)
{
    
}

const mf_font_s* current_font;
void PixelCallback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void* state)
{
    ScreenContext* context = (ScreenContext*)state;
    for(int i = 0; i < count; i++)
    {
        context->data[y * context->screen_size.x + x + i] = 255-alpha;
    }
}
uint8_t CharacterCallback(int16_t x, int16_t y, mf_char character, void* state)
{
    return mf_render_character(current_font, x, y, character, &PixelCallback, state);
}

void DrawText(const ScreenContext& context, vec2<uint16_t> pos, FONT font, uint16_t bufstart, uint16_t len)
{
    static const mf_font_s* fonts[] = {&mf_rlefont_DejaVuSans12.font, &mf_rlefont_DejaVuSans12bw.font, 
        &mf_rlefont_DejaVuSerif16.font, &mf_rlefont_DejaVuSerif32.font, &mf_bwfont_fixed_5x8.font,
        &mf_rlefont_fixed_7x14.font, &mf_rlefont_fixed_10x20.font};
    if((uint8_t)font >= ArraySize(fonts))
    {
        printf("Invalid font %i\n", (int)font);
        return;
    }

    if(bufstart + len > TEXT_BUFFER_SIZE)
    {
        printf("Out of text buffer range at %i for len %i\n", (int)bufstart, (int)len);
    }

    current_font = fonts[(int)font];

    mf_render_aligned(current_font, pos.x, pos.y, MF_ALIGN_LEFT, text_buffer + bufstart, len, &CharacterCallback, (void*)&context);
}



void DrawEntity(const Entity& entity, const ScreenContext& context)
{
    switch(entity.type)
    {
        case ENTITY_TYPE::SHAPE:
        {
            switch((SHAPE)entity.data[0])
            {
                case SHAPE::CIRCLE:
                {
                    DrawCircle(context, entity.data[1], entity.data[2], {entity.pos.x, entity.pos.y}, 
                        {entity.size.x, entity.size.y}, entity.rotation);
                    break;
                }
                case SHAPE::RECTANGLE:
                {
                    DrawRectangle(context, entity.data[1], entity.data[2], {entity.pos.x, entity.pos.y}, {entity.size.x, entity.size.y}, entity.rotation);
                    break;
                }
                case SHAPE::TRIANGLE:
                {
                    break;
                }
                default:
                { 
                    printf("Invalid shape!: %i\n", (int)entity.data[0]);
                }
            }
            break;
        };
        case ENTITY_TYPE::TEXT:
        {
            DrawText(context, entity.pos, (FONT)entity.data[0], *(uint16_t*)(entity.data + 1), *(uint16_t*)(entity.data + 3));
            break;
        }
        default:
        {
            printf("Invalid entity type!: %i\n", (int)entity.type);
        }
    }
}

uint64_t RenderFrame()
{
    uint64_t start = get_time_us();
    Entity entities[N_ENTITIES];

    // Copy entities. Interrupts that may modify entities should wait for the copy to finish 

    dmamemcpy4(entities, entity_buffer, N_ENTITIES*sizeof(Entity), true);
    
    

    uint8_t* video_data = driver->GetBackBuffer();

    ScreenContext context;
    context.data = video_data;
    context.screen_size = {driver->GetLinesX(), driver->GetLinesY()};

    switch(background.mode)
    {
        case BACKGROUND_MODE::SOLID_SHADE:
        {
            dmamemset4(context.data, background.value, context.screen_size.x*context.screen_size.y, true);
            //memset(context.data, background.value, context.screen_size.x*context.screen_size.y);
            break;
        }
        case BACKGROUND_MODE::TEXTURE:
        {
            break;
        }
        default:
        {
            printf("invalid background mode: %i\n", (int)background.mode);
        }
    }

    //background dma still running
    // sort entities by layer
    std::sort(entities, entities + N_ENTITIES, [](const Entity& a, const Entity& b){return a.layer > b.layer;});

    // finish background dma copies
    dmawaitmemset4();
    dmawaitformemcopies();


    for(int i = 0; i < N_ENTITIES; i++)
    {
        if(entities[i].visible)
            DrawEntity(entities[i], context);
    }

    uint64_t tmdf = get_time_us()-start;
    driver->SwapBuffersBlocking();
    return tmdf;
}

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
            // This isn't actually a guarantee, as a new copy operation could start at any moment.
            // Still the entity rendering system is supposed to be safe from buffer overruns,
            // so at worst this will cause an occassional artifact
            dmawaitformemcopies();

            if(address0 >= N_ENTITIES)
            {
                printf("Out of bounds read of the entity buffer at %i!\n", (int)address0);
                break;
            }
            uint8_t* entdt = (uint8_t*)(entity_buffer + address0);
            if(address1 + len > sizeof(Entity))
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

            uint16_t address = uint16_t(address0) << 8 | uint16_t(address1);
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
        
    }
}

void _I2CHandler()
{
    
    if(i2c_last_message != 0)
    {
        // uint64_t time_since_last_message = get_time_us() - i2c_last_message;
        // if(time_since_last_message > 10000) // after 10ms reset the state
        //     spi_state.state = SPI_STATE::NONE;
    }
    i2c_last_message = get_time_us();

    uint8_t value = i2c0->hw->data_cmd & I2C_IC_DATA_CMD_BITS;
    
    //uint8_t value = 0;//spi0_hw->dr;
    //spi_read_blocking(spi0, 0, &value, 1);

    switch(i2c_state.state)
    {
        case SPI_STATE::NONE:
        {
            if(value == 0xEF)
            {
                i2c_state.read = true;
            }
            else if(value != 0xFF)
            {
                printf("ERROR: INVALID FIRST SPI VALUE: %i\n", (int)value);
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
            assert(false, "Invalid SPI state!");
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
        }
    }
    return value;
}


void I2CHandler()
{
    uint8_t status = i2c0->hw->intr_stat;
    if(status & I2C_IC_INTR_STAT_R_RX_FULL_BITS)
    {
        _I2CHandler();
    }
    else if(status & I2C_IC_INTR_STAT_R_RD_REQ_BITS)
    {
        i2c0->hw->data_cmd = I2CGetNextByte();
        i2c0->hw->clr_rd_req;
    }
}

void core1_entry()
{
    driver->Start();
}

int main()
{
    stdio_init_all();

    printf("Starting picoGPU...\n");


    printf("Setting sysclock to %i khz\n", sysclockkhz);
    set_sys_clock_khz(sysclockkhz, true);
    uart_set_baudrate(uart0, 115200);
    printf("clock set successfully\n");


    adc_init();
    adc_set_temp_sensor_enabled(true);



    driver = new PAL_DRIVER(RENDER_MODE::BW_408_304_DOUBLE_BUF, SAMPLING_MODE::SPP_1);

    multicore_launch_core1(core1_entry);

    InitRendering();


    i2c_init(i2c0, 2 * 1000 * 1000);

    gpio_set_function(PIN::I2C_SCL, GPIO_FUNC_I2C);
    gpio_set_pulls(PIN::I2C_SCL, true, false);
    gpio_set_function(PIN::I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_pulls(PIN::I2C_SDA, true, false);

    i2c_set_slave_mode(i2c0, true, I2C_ADDR);
    
    i2c0->hw->enable = 0;
    hw_set_bits(&i2c0->hw->con, I2C_IC_CON_RX_FIFO_FULL_HLD_CTRL_BITS);
    i2c0->hw->enable = 1;


    i2c0->hw->intr_mask =  I2C_IC_INTR_MASK_M_RD_REQ_BITS | I2C_IC_INTR_MASK_M_RX_FULL_BITS;

    irq_set_exclusive_handler(I2C0_IRQ, I2CHandler);
    irq_set_enabled(I2C0_IRQ, true);

    i2c_state = {};


    uint8_t* video_data = driver->GetBackBuffer();

    int lines_x = driver->GetLinesX();
    int lines_y = driver->GetLinesY();
    int video_data_size = lines_x*lines_y;

    // float sx = 10.0f;
    // float sy = 10.0f;
    // float px = 50.00f;
    // float py = 50.0f;
    // float r = 15;


    printf("Remaining memory: %i\n", GetFreeHeap());

    background.mode = BACKGROUND_MODE::SOLID_SHADE;
    background.value = 255; 
    
    // Entity& ball = entity_buffer[0];
    // ball.visible = true;
    // ball.type = ENTITY_TYPE::SHAPE;
    // ball.layer = 0;
    // ball.size = vec2<uint16_t>{uint16_t(r), uint16_t(r)};
    // ball.data[0] = (uint8_t)SHAPE::CIRCLE;
    // ball.data[1] = 128;
    // ball.data[2] = 0;

    Entity& crect = entity_buffer[1];
    crect.visible = true;
    crect.type = ENTITY_TYPE::SHAPE;
    crect.layer = 1;
    crect.pos = {uint16_t(lines_x/2), uint16_t(lines_y/2)};
    crect.size = {50, 25};
    crect.data[0] = (uint8_t)SHAPE::RECTANGLE;
    crect.data[1] = 0;
    crect.data[2] = 200;

    while(true)
    {
        // uint64_t ttm = get_time_us();
        // float nx = px + sx;
        // float ny = py + sy;
        // if(nx < r || nx > float(lines_x)-2*r)
        // {
        //     sx = -sx;
        // }
        // else
        //     px = nx;
        // if(ny < 2*r || ny > float(lines_y)-r)
        // {
        //     sy = -sy;
        // }
        // else
        //     py = ny;

        // ball.pos = vec2<uint16_t>{(uint16_t)px, (uint16_t)py};
        // ball.rotation += 1;
        uint64_t rtm = get_time_us();
        uint64_t rftm = RenderFrame();
        uint64_t rtmd = get_time_us()-rtm;
        double rtmdf = double(rtmd) / 1000.0f;
        double rftmf = double(rftm) / 1000.0f;

        //printf("ttm: %.4f | rftm %.4f | rtm: %.4f\n", ttmdf, rftmf, rtmdf);
        //WriteTemperature();
    }
}
