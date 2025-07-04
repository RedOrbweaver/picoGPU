#include "hmain.hpp"


PAL_DRIVER* driver;

Entity entities[N_ENTITIES];

mutex_t core1mx;

bool update_settings = false;
bool update_background = false;

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
    for(volatile int i = 0; i < N_ENTITIES; i++)
        entity_buffer[i].visible = false;
}

void UpdateBackground()
{
    update_background = true;
}

void UpdateSettings()
{
    update_settings = true;
}

uint64_t RenderFrame(uint32_t& entities_drawn)
{
    uint64_t start = get_time_us();

    entities_drawn = 0;

    uint32_t interrupt_state = save_and_disable_interrupts();
    memcpy(entities, entity_buffer, N_ENTITIES*sizeof(Entity));
    restore_interrupts(interrupt_state);

    uint8_t* video_data = driver->GetBackBuffer();

    ScreenContext context;
    context.data = video_data;
    context.screen_size = {driver->GetLinesX(), driver->GetLinesY()};

    switch(background.mode)
    {
        case BACKGROUND_MODE::SOLID_SHADE:
        {
            memset(context.data, background.value, context.screen_size.x*context.screen_size.y);
            break;
        }
        case BACKGROUND_MODE::TEXTURE:
        {
            for(int y = 0; y < context.screen_size.y; y++)
            {
                int posy = float(y)/float(context.screen_size.y) * float(background.source_size.y);
                for(int x = 0; x < context.screen_size.x; x++)
                {
                    int posx = float(x)/float(context.screen_size.x) * float(background.source_size.x);
                    SetPixel(context, background_texture[posy*background.source_size.x + posx], {x, y});
                }   
            }
            break;
        }
        default:
        {
            printf("invalid background mode: %i\n", (int)background.mode);
        }
    }

    // sort entities by layer
    std::sort(entities, entities + N_ENTITIES, [](const Entity& a, const Entity& b){return a.layer > b.layer;});


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
    while(true)
    {
        mutex_enter_blocking(&core1mx);
        if(driver != nullptr)
            driver->Start();
        mutex_exit(&core1mx);
        sleep_ms(1); // A dirty way to make sure there's enough time for the other core to grab the mutex
    }
}

void InitAll()
{
    printf("Setting sysclock to %i khz\n", sysclockkhz);
    set_sys_clock_khz(sysclockkhz, true);
    uart_set_baudrate(uart0, 115200);
    printf("clock set successfully\n");

    adc_init();
    adc_set_temp_sensor_enabled(true);

    mutex_init(&core1mx);

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

    printf("Remaining memory: %i\n", GetFreeHeap());

    background.mode = BACKGROUND_MODE::SOLID_SHADE;
    background.value = 0;

    while(true)
    {
        if(update_settings)
        {
            driver->Stop();
            mutex_enter_blocking(&core1mx);
            delete driver;
            driver = new PAL_DRIVER(settings.render_mode, settings.sampling_mode);
            mutex_exit(&core1mx);
            update_settings = false;
        }
        if(update_background)
        {
            if(background_texture != nullptr)
                delete[] background_texture;
            if(background.mode == BACKGROUND_MODE::TEXTURE)
            {
                background_texture = new uint8_t[background.source_size.area()];
            }
            update_background = false;
        }
        uint32_t entities_drawn;
        uint64_t render_time = RenderFrame(entities_drawn);
        information.last_render_time_us = render_time;
        information.entities_drawn = entities_drawn;
        information.total_memory = GetTotalHeap();
        information.free_memory = GetFreeHeap();
        information.temperature = ReadTemperature();
        information.lines_x = driver->GetLinesX();
        information.lines_y = driver->GetLinesY();
        information.frame_number++;
    }
}
