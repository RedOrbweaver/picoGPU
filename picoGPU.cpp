#include "hmain.hpp"

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

}
void DrawTriangle(const ScreenContext& context, uint8_t border, uint8_t fill, vec2<int> pos, vec2<int> size, vec2<int> p0, vec2<int> p1, vec2<int> p2)
{
    
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
                    DrawCircle(context, entity.data[0], entity.data[1], {entity.pos.x, entity.pos.y}, 
                        {entity.size.x, entity.size.y}, entity.rotation);
                    break;
                }
                case SHAPE::RECTANGLE:
                {
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

    uint8_t* video_data = driver->GetBackBuffer();

    int lines_x = driver->GetLinesX();
    int lines_y = driver->GetLinesY();
    int video_data_size = lines_x*lines_y;

    float sx = 10.0f;
    float sy = 10.0f;
    float px = 50.00f;
    float py = 50.0f;
    float r = 15;


    printf("Remaining memory: %i\n", GetFreeHeap());

    background.mode = BACKGROUND_MODE::SOLID_SHADE;
    background.value = 255; 
    
    Entity& ball = entity_buffer[0];
    ball.visible = true;
    ball.type = ENTITY_TYPE::SHAPE;
    ball.layer = 0;
    ball.size = vec2<uint16_t>{uint16_t(r), uint16_t(r)};
    ball.data[0] = (uint8_t)SHAPE::CIRCLE;
    ball.data[1] = 128;
    ball.data[2] = 0;

    while(true)
    {
        uint64_t ttm = get_time_us();
        float nx = px + sx;
        float ny = py + sy;
        if(nx < r || nx > float(lines_x)-2*r)
        {
            sx = -sx;
        }
        else
            px = nx;
        if(ny < 2*r || ny > float(lines_y)-r)
        {
            sy = -sy;
        }
        else
            py = ny;

        ball.pos = vec2<uint16_t>{(uint16_t)px, (uint16_t)py};
        ball.rotation += 1;
        uint64_t ttmd = get_time_us() - ttm;
        uint64_t rtm = get_time_us();
        uint64_t rftm = RenderFrame();
        uint64_t rtmd = get_time_us()-rtm;
        double rtmdf = double(rtmd) / 1000.0f;
        double rftmf = double(rftm) / 1000.0f;
        double ttmdf = double(ttmd) / 1000.0f;

        printf("ttm: %.4f | rftm %.4f | rtm: %.4f\n", ttmdf, rftmf, rtmdf);
        WriteTemperature();
    }
}
