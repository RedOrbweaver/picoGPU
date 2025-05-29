#pragma once
#include "hmain.hpp"

constexpr float LINE_NS = 64000.0f;

constexpr float BASE_PERIOD_NS = 3.3333f;
constexpr float LINE_BLANKING_NS = 12500.0f;
constexpr float LINE_SYNC_NS = 4700.0f;
constexpr float FRONT_PORCH_NS = 1650.0f;
constexpr float BACK_PORCH_NS = 5700.0f;
constexpr float VISUAL_NS = 52000.0f;



constexpr float SHORT_SYNC_LOW_NS = 2350.0f;
constexpr float SHORT_SYNC_HIGH_NS = LINE_NS - SHORT_SYNC_LOW_NS;
constexpr float LONG_SYNC_HIGH_NS = 4700.0f;
constexpr float LONG_SYNC_LOW_NS = LINE_NS - LONG_SYNC_HIGH_NS;

constexpr float CLOCK_LEN_NS = 3.333333f;

constexpr int black = 80;
constexpr int zero = 0;

#define dac_send_array(a, div) dac_write(a, ArraySize(a), div);

class PAL_DRIVER
{
    public:
    private:
    protected:

    const float div = 15;
    float line_div;//38.2353324?;

    RENDER_MODE mode;
    SAMPLING_MODE smode;
    bool interlaced;

    uint8_t* front_buffer;
    uint8_t* back_buffer;
    uint8_t* send_buffer;
    int lines_x;
    int lines_y;
    int samples_per_pixel;
    int send_buffer_size;

    uint8_t long_sync[640] = {0};
    uint8_t short_sync[640] = {0};

    struct
    {
        uint8_t front_porch[32];
        uint8_t line_sync[96] = {0};
        uint8_t back_porch[112];
    }fb = {0};

    uint8_t zeroes[4] = {0};

    pio_hw_t* pio;
    int sm;
    uint dmachan;


    uint64_t cmptm = 0;
    uint64_t cmptm_tot = 0;
    uint64_t cmptm_max = 0;

    mutex_t non_blanking_mx;


    #define PIN_COUNT 8
    static inline void dac_program_init(PIO pio, uint sm, uint offset, uint pin_base, float divider) 
    {
        for(uint i=pin_base; i<pin_base+PIN_COUNT; i++) 
        {
            pio_gpio_init(pio, i);
        }
        pio_sm_set_consecutive_pindirs(pio, sm, pin_base, PIN_COUNT, true);

        pio_sm_config c = dac_out_program_get_default_config(offset); 

        sm_config_set_out_shift(&c, true, true, 32); // true - shift right, auto pull, # of bits

        sm_config_set_out_pins(&c, pin_base, PIN_COUNT);

        sm_config_set_clkdiv(&c, divider);

        // join the FIFO buffers to get more DMA throughput?
        // we use the transmit only so join RX to the TX?
        sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

        pio_sm_init(pio, sm, offset, &c);
        pio_sm_set_enabled(pio, sm, true);
        pio_sm_clear_fifos(pio, sm);
    }

    void dac_push_output(uint32_t values)
    {
        pio_sm_put(pio0, 0, values);
    }


    void dma_init(pio_hw_t* pio, int sm)
    {

        dmachan = dma_claim_unused_channel(true);    

        auto c = dma_channel_get_default_config(dmachan);
        channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
        channel_config_set_read_increment(&c, true);
        channel_config_set_write_increment(&c, false);
        channel_config_set_dreq(&c, pio_get_dreq(pio, sm, true));
        channel_config_set_irq_quiet(&c, true);
        channel_config_set_high_priority(&c, true);
        channel_config_set_enable(&c, true);
        dma_channel_configure(
            dmachan,
            &c,
            // Write to the FIFO
            &pio->txf[sm],
            NULL,
            0,
            false);
        
        dma_channel_start(dmachan);
    }

    void pio_init()
    {
        pio = pio0;
        sm = pio_claim_unused_sm(pio0, true);

        uint8_t offset = pio_add_program(pio, &dac_out_program);
        dac_program_init(pio, sm, offset, PIN::DAC_OUT[0], 1.0f);
    }
    void dac_init()
    {
        pio_init();
        dma_init(pio, sm);
    }

    void dac_write(uint8_t* data, uint len, float div)
    {
        assert(len % 4 == 0);

        dma_channel_wait_for_finish_blocking(dmachan);
        pio_sm_set_clkdiv(pio, sm, div);
        dma_channel_set_trans_count(dmachan, len/4, false);
        dma_channel_set_read_addr(dmachan, data, true);
    }
    void ComputeSendBuffer(int line)
    {
        cmptm = get_time_us();
        if(smode == SAMPLING_MODE::SPP_1)
        {
            send_buffer = front_buffer + line*lines_x;
        }
        else
        {
            for(int i = 0; i < send_buffer_size; i++)
            {
                uint8_t v = front_buffer[line*lines_x + i/samples_per_pixel];
                send_buffer[i] = v;
            }
        }

        cmptm = get_time_us() - cmptm;
        cmptm_tot += cmptm;
        if(cmptm > cmptm_max)
            cmptm_max = cmptm;
    }


    public:


    int GetLinesX()
    {
        return lines_x;
    }
    int GetLinesY()
    {
        return lines_y;
    }

    uint8_t* GetFrontBuffer()
    {
        return front_buffer;
    }
    uint8_t* GetBackBuffer()
    {
        return back_buffer;
    }
    void SwapBuffersBlocking()
    {
        mutex_enter_blocking(&non_blanking_mx);
        uint8_t* fb = front_buffer;
        front_buffer = back_buffer;
        back_buffer = fb;
        mutex_exit(&non_blanking_mx);
    }

    void ShortSync(int n)
    {
        dac_send_array(zeroes, div);
        for(int i = 0; i < n; i++)
        {
            //dac_send_array(short_sync, div);
            gpio_put(PIN::NOT_SYNC, 0);
            //busy_wait_at_least_cycles(SHORT_SYNC_LOW_NS/CLOCK_LEN_NS);
            sleep_us(2);
            gpio_put(PIN::NOT_SYNC, 1);
            //busy_wait_at_least_cycles(SHORT_SYNC_HIGH_NS/CLOCK_LEN_NS);
            sleep_us(60);
        }
    }
    void LongSync(int n)
    {
        dac_send_array(zeroes, div);
        for(int i = 0; i < n; i++)
        {
            //dac_send_array(long_sync, div);
            gpio_put(PIN::NOT_SYNC, 0);
            //busy_wait_at_least_cycles(LONG_SYNC_LOW_NS/CLOCK_LEN_NS);
            sleep_us(59);
            gpio_put(PIN::NOT_SYNC, 1);
            sleep_us(5);
            //busy_wait_at_least_cycles(LONG_SYNC_HIGH_NS/CLOCK_LEN_NS);
        }
    }

    void LoopInterlaced()
    {
        while(true)
        {
            uint64_t tm;
            uint64_t tmdif;

            tm = get_time_us();
            
            LongSync(5);
            ShortSync(5);
            mutex_enter_blocking(&non_blanking_mx);
            ComputeSendBuffer(0);
            for(int i = 0; i < lines_y; i++)
            {
                dac_write((uint8_t*)&fb, sizeof(fb), div);
                
                dac_write(send_buffer, send_buffer_size, line_div);

                if(i != lines_y-1)
                    ComputeSendBuffer(i+1);
            }
            mutex_exit(&non_blanking_mx);
            ShortSync(5);
            LongSync(5);
            ShortSync(4);

            mutex_enter_blocking(&non_blanking_mx);
            ComputeSendBuffer(0);
            for(int i = 0; i < lines_y; i++)
            {
                dac_write((uint8_t*)&fb, sizeof(fb), div);
                
                dac_write(send_buffer, send_buffer_size, line_div);


                if(i != lines_y-1)
                    ComputeSendBuffer(i+1);
            }
            mutex_exit(&non_blanking_mx);

            ShortSync(6);
            tmdif = (get_time_us()-tm);
            double rtm = double(tmdif) / 1000.0;
            float average = float(cmptm_tot) / float(lines_y);
            printf("%.4f %lli %lli %lli %.3f\n", rtm, cmptm, cmptm_max, cmptm_tot, average);
            cmptm_tot = 0;
            cmptm_max = 0;
        }
    }

    void LoopNonInterlaced()
    {
        while(true)
        {
            uint64_t tm;
            uint64_t tmdif;

            
            tm = get_time_us();

            LongSync(5);
            ShortSync(5);
            mutex_enter_blocking(&non_blanking_mx);
            ComputeSendBuffer(0);
            //dma_channel_wait_for_finish_blocking(dmachan);
            for(int i = 0; i < lines_y; i++)
            {
                //dac_write((uint8_t*)&fb, sizeof(fb), div);
                

                dac_send_array(zeroes, div);

                gpio_put(PIN::NOT_SYNC, 1);

                //busy_wait_at_least_cycles((FRONT_PORCH_NS)/CLOCK_LEN_NS);

                sleep_us(1);

                gpio_put(PIN::NOT_SYNC, 0);

                //busy_wait_at_least_cycles(LINE_SYNC_NS/CLOCK_LEN_NS);

                sleep_us(6);
                
                gpio_put(PIN::NOT_SYNC, 1);

                //busy_wait_at_least_cycles(BACK_PORCH_NS/CLOCK_LEN_NS);

                sleep_us(4);

                
                dac_write(send_buffer, send_buffer_size, line_div);

                //dma_channel_wait_for_finish_blocking(dmachan);

                sleep_us(52);
                

                if(i != lines_y-1)
                    ComputeSendBuffer(i+1);
            }
            mutex_exit(&non_blanking_mx);
            ShortSync(6);
            
            tmdif = (get_time_us()-tm);

            // double rtm = double(tmdif) / 1000.0;
            // float average = float(cmptm_tot) / float(lines_y);
            // printf("%.4f %lli %lli %lli %.3f\n", rtm, cmptm, cmptm_max, cmptm_tot, average);
            cmptm_tot = 0;
            cmptm_max = 0;
        }
    }

    void Start()
    {

        dac_init();

        init_out({PIN::NOT_SYNC}, true);


        for(int i = 0; i < ArraySize(fb.front_porch); i++)
        {
            fb.front_porch[i] = 0;
        }
        for(int i = 0; i < ArraySize(fb.back_porch); i++)
        {
            fb.back_porch[i] = 0;
        }

        memset(long_sync, 0, ArraySize(long_sync));
        memset(short_sync, 0, ArraySize(short_sync));

        // memset(long_sync, 0, ArraySize(long_sync));
        // memset(long_sync + 640 - 94 - 1, black, 94);
        // memset(short_sync, black, ArraySize(short_sync));
        // memset(short_sync, zero, 47);
        
        if(interlaced)
            LoopInterlaced();
        else
            LoopNonInterlaced();
    }

    PAL_DRIVER(RENDER_MODE mode, SAMPLING_MODE smode)
    {
        this->mode = mode;
        this->smode = smode;
        init();
    }
    private:
    void init()
    {
        mutex_init(&non_blanking_mx);
        switch(mode)
        {
            case BW_408_304_DOUBLE_BUF:
            {
                lines_x = 408;
                lines_y = 304;
                front_buffer = new uint8_t[lines_x*lines_y];
                back_buffer = new uint8_t[lines_x*lines_y];
                interlaced = false;
                break;
            }
            case BW_408_305_DOUBLE_BUF_INTERLACED:
            {
                lines_x = 408;
                lines_y = 305;
                front_buffer = new uint8_t[lines_x*lines_y];
                back_buffer = new uint8_t[lines_x*lines_y];
                interlaced = true;
                break;
            }
            default:
            {
                assert(false, "invalid render mode: " + std::to_string((int)mode));
            }
        }
        switch(smode)
        {
            case SPP_1:
            {
                samples_per_pixel = 1;
                break;
            }
            case SPP_2:
            {
                samples_per_pixel = 2;
                break;
            }
            case SPP_4:
            {
                samples_per_pixel = 4;
                break;
            }
            default:
            {
                assert(false, "invalid sampling mode: " + std::to_string((int)smode));
            }
        }
        send_buffer_size = lines_x*samples_per_pixel;
        send_buffer = new uint8_t[send_buffer_size];
        line_div = ((VISUAL_NS/float(lines_x))/CLOCK_LEN_NS)/float(samples_per_pixel);
    }
};