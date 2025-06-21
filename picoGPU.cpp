#include "hmain.hpp"


PAL_DRIVER* driver;
int dmamemcpychan1;
dma_channel_config dmamemcpychan1c;
int dmamemcpychan4;
dma_channel_config dmamemcpychan4c;
int dmamemsetchan4;
dma_channel_config dmamemsetchan4c;


Entity entities[N_ENTITIES];

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

    for(volatile int i = 0; i < N_ENTITIES; i++)
        entity_buffer[i].visible = false;
}

void UpdateBackground()
{
    
}

uint64_t RenderFrame(uint32_t& entities_drawn)
{
    uint64_t start = get_time_us();

    entities_drawn = 0;

    //dmamemcpy4(entities, entity_buffer, N_ENTITIES*sizeof(Entity), true);
    memcpy(entities, entity_buffer, N_ENTITIES*sizeof(Entity));

    uint8_t* video_data = driver->GetBackBuffer();

    ScreenContext context;
    context.data = video_data;
    context.screen_size = {driver->GetLinesX(), driver->GetLinesY()};

    switch(background.mode)
    {
        case BACKGROUND_MODE::SOLID_SHADE:
        {
            // if(context.screen_size.x*context.screen_size.y % 4 == 0)
            //     dmamemset4(context.data, background.value, context.screen_size.x*context.screen_size.y, true);
            // else 
                memset(context.data, background.value, context.screen_size.x*context.screen_size.y);
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
    dmawaitformemcopies();

    //background dma still running
    // sort entities by layer
    std::sort(entities, entities + N_ENTITIES, [](const Entity& a, const Entity& b){return a.layer > b.layer;});

    // finish background dma copy
    dmawaitmemset4();

    for(volatile int i = 0; i < N_ENTITIES; i++)
    {
        if(entities[i].visible)
        {
            DrawEntity(entities[i], context);
            entities_drawn++;
        }
    }
    uint64_t tmdf = get_time_us()-start;
    driver->SwapBuffersBlocking();
    return tmdf;
}



void InitI2C()
{
    i2c_init(i2c0, i2cspeed);

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
}

void core1_entry()
{
    driver->Start();
}

void InitAll()
{
    printf("Setting sysclock to %i khz\n", sysclockkhz);
    set_sys_clock_khz(sysclockkhz, true);
    uart_set_baudrate(uart0, 115200);
    printf("clock set successfully\n");

    adc_init();
    adc_set_temp_sensor_enabled(true);

    driver = new PAL_DRIVER(RENDER_MODE::BW_408_304_DOUBLE_BUF, SAMPLING_MODE::SPP_1);

    multicore_launch_core1(core1_entry);

    InitRendering();

    InitI2C();

    information.frame_number = 0;
}


int main()
{
    stdio_init_all();

    printf("Starting picoGPU...\n");

    InitAll();

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

    Entity& ctriangle = entity_buffer[1];
    ctriangle.visible = true;
    ctriangle.type = ENTITY_TYPE::SHAPE;
    ctriangle.layer = 1;
    ctriangle.pos = {uint16_t(lines_x/2), uint16_t(lines_y/2)};
    ctriangle.size = {1, 1};
    ctriangle.data[0] = (uint8_t)SHAPE::TRIANGLE;
    ctriangle.data[1] = 0;
    
    ctriangle.data[2] = 0;
    ctriangle.data[3] = 0;
    ctriangle.data[4] = 0;
    ctriangle.data[5] = 0;

    ctriangle.data[6] = 80;
    ctriangle.data[7] = 0;
    ctriangle.data[8] = 0;
    ctriangle.data[9] = 0;

    ctriangle.data[10] = 40;
    ctriangle.data[11] = 0;
    ctriangle.data[12] = 69;
    ctriangle.data[13] = 0;

    ctriangle.data[14] = 1;

    Entity& cline = entity_buffer[2];
    cline.visible = true;
    cline.type = ENTITY_TYPE::LINE;
    cline.layer = 1;
    cline.pos = {50, 50};
    cline.size = {200, 100};
    cline.data[0] = 0;

    Entity& crect = entity_buffer[3];
    crect.visible = true;
    crect.type = ENTITY_TYPE::SHAPE;
    crect.rotation = 50;
    crect.layer = 2;
    crect.pos = {50, 150};
    crect.size = {40, 20};
    crect.data[0] = (uint8_t)SHAPE::RECTANGLE;
    crect.data[1] = 100;
    crect.data[2] = 0;

    Entity& cerec = entity_buffer[4];
    cerec.visible = true;
    cerec.type = ENTITY_TYPE::SHAPE;
    cerec.rotation = 100;
    cerec.layer = 2;
    cerec.pos = {150, 200};
    cerec.size = {40, 20};
    cerec.data[0] = (uint8_t)SHAPE::EMPTY_RECTANGLE;
    cerec.data[1] = 0;
    cerec.data[2] = 1;

    Entity& cecirc = entity_buffer[5];
    cecirc.visible = true;
    cecirc.type = ENTITY_TYPE::SHAPE;
    cecirc.rotation = 0;
    cecirc.layer = 0;
    cecirc.pos = {300, 200};
    cecirc.size = {25, 25};
    cecirc.data[0] = (uint8_t)SHAPE::EMPTY_CIRCLE;
    cecirc.data[1] = 50;

    Entity& cmultil = entity_buffer[6];
    cmultil.visible = true;
    cmultil.type = ENTITY_TYPE::MULTI_LINE;
    cmultil.rotation = 0;
    cmultil.layer = 0;
    cmultil.pos = {50, 250};
    cmultil.size = {25, 25};
    cmultil.data[0] = 0;
    cmultil.data[1] = 0;
    cmultil.data[2] = 0;
    cmultil.data[3] = 4;
    cmultil.data[4] = 0;

    geometry_buffer[0] = {0, 0};
    geometry_buffer[1] = {10, 0};
    geometry_buffer[2] = {15, 5};
    geometry_buffer[3] = {5, 15};
    geometry_buffer[4] = {0, 0};

    Entity& cerec2 = entity_buffer[7];
    cerec2.visible = true;
    cerec2.type = ENTITY_TYPE::SHAPE;
    cerec2.rotation = 0;
    cerec2.layer = 2;
    cerec2.pos = {0, 0};
    cerec2.size = {150, 150};
    cerec2.data[0] = (uint8_t)SHAPE::EMPTY_RECTANGLE;
    cerec2.data[1] = 0;
    cerec2.data[2] = 0;



    Entity& cbezier = entity_buffer[8];
    cbezier.visible = true;
    cbezier.type = ENTITY_TYPE::BEZIER;
    cbezier.rotation = 0;
    cbezier.layer = 1;
    cbezier.pos = {200, 250};
    cbezier.data[0] = 0;
    cbezier.data[1] = 5;
    cbezier.data[2] = 0;
    cbezier.data[3] = 9;
    cbezier.data[4] = 0;

    geometry_buffer[5] = {0, 0};
    geometry_buffer[6] = {50, -100};
    geometry_buffer[7] = {75, 0};
    geometry_buffer[8] = {80, 50};
    geometry_buffer[9] = {100, 0};

    vec2<int> tpoint = vec2<int>{0, 50};
    int dirx = 3;
    int diry = -1;
    while(true)
    {
        tpoint.y += diry;
        if(tpoint.y >= 100 || tpoint.y <= -200)
            diry = -diry;
        tpoint.x += dirx;
        if(tpoint.x >= 100 || tpoint.x <= -200)
            dirx = -dirx;
        geometry_buffer[6] = tpoint;
        // uint64_t start = get_time_us();
        // sleep_ns(52000);
        // uint64_t tdif = get_time_us() - start;
        //printf("%llu\n", tdif);
        crect.rotation++;
        cerec.rotation++;
        cmultil.rotation--;
        ctriangle.rotation--;
        //ctriangle.rotation++;
        uint32_t entities_drawn;
        uint64_t render_time = RenderFrame(entities_drawn);
        information.last_render_time_us = render_time;
        information.entities_drawn = entities_drawn;
        information.total_memory = GetTotalHeap();
        information.free_memory = GetFreeHeap();
        information.temperature = ReadTemperature();
        information.frame_number++;
    }
}
