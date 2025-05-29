#include "hmain.hpp"

PAL_DRIVER* driver;

int SQ(int v)
{
    return v * v; 
}
int distance(int x0, int y0, int x1, int y1)
{
    return sqrt(SQ(x1-x0) + SQ(y1-y0));
}

float read_temperature()
{
    adc_select_input(4);
    uint16_t v = adc_read();
    float temperature = 27 - ((float(v)*(3.3f / (1 << 12))) - 0.706)/0.001721;
    return temperature;
}

void write_temperature()
{
    float temp = read_temperature();
    printf("Temperature: %.3f\n", temp);
}

void core1_entry()
{
    driver->Start();
}

int main()
{
    stdio_init_all();

    printf("Starting PAL_test...\n");


    uint sysclockkhz = 300000;
    printf("Setting sysclock to %i khz\n", sysclockkhz);
    set_sys_clock_khz(300000, true);
    uart_set_baudrate(uart0, 115200);
    printf("clock set successfully\n");




    adc_init();
    adc_set_temp_sensor_enabled(true);



    driver = new PAL_DRIVER(RENDER_MODE::BW_408_304_DOUBLE_BUF, SAMPLING_MODE::SPP_1);

    multicore_launch_core1(core1_entry);

    uint8_t* video_data = driver->GetBackBuffer();

    int lines_x = driver->GetLinesX();
    int lines_y = driver->GetLinesY();
    int video_data_size = lines_x*lines_y;

    float sx = 10.0f;
    float sy = 10.0f;
    float px = 50.0f;
    float py = 50.0f;
    float r = 15.0f;


    printf("Remaining memory: %i\n", GetFreeHeap());
    

    while(true)
    {
        uint8_t* video_data = driver->GetBackBuffer();
        memset(video_data, 0, video_data_size);

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


        for(int i = 0; i < lines_y; i++)
        {
            for(int ii = 0; ii < lines_x; ii++)
            {
                int dist = distance(ii, i, px, py);
                uint8_t val;
                if(dist < r)
                    val = 255;
                else
                {
                    float range = 255.0f;
                    float fv = (float(ii)/float(lines_x))*range;
                    val = (uint8_t)fv;
                }
                video_data[i*lines_x + ii] = val;
            }
        }

        driver->SwapBuffersBlocking();
    }
}
