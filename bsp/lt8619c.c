#include <stdlib.h>
#include "nrfx_twi.h"
#include "nrf_error.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "lt8619c.h"

/* TWI instance ID */
#define TWI_INSTANCE_ID     0

/* TWI instance. */
static const nrfx_twi_t m_twi = NRFX_TWI_INSTANCE(TWI_INSTANCE_ID);

lt8619c_rx_status_t lt8619c_rx_status, *plt8619c_rx_status;

uint16_t h_active, v_active;
uint16_t h_syncwidth, v_syncwidth;
uint16_t h_bkporch, v_bkporch;
uint16_t h_total, v_total;
uint8_t  h_syncpol, v_syncpol,color;
uint32_t frame_counter;


uint8_t ONCHIP_EDID[256]={
    //1920*1080
    0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0e, 0xd4, 0x32, 0x31, 0x00, 0x88, 0x88, 0x88,
    0x20, 0x1c, 0x01, 0x03, 0x80, 0x0c, 0x07, 0x78, 0x0a, 0x0d, 0xc9, 0xa0, 0x57, 0x47, 0x98, 0x27,
    0x12, 0x48, 0x4c, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
    0x45, 0x00, 0x80, 0x38, 0x74, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x0a,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC,
    0x00, 0x4C, 0x6F, 0x6E, 0x74, 0x69, 0x75, 0x6D, 0x20, 0x73, 0x65, 0x6D, 0x69, 0x20, 0x01, 0xf5,

    0x02, 0x03, 0x12, 0xf1, 0x23, 0x09, 0x04, 0x01, 0x83, 0x01, 0x00, 0x00, 0x65, 0x03, 0x0c, 0x00,
    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf
};

const video_timing_t lcd_timing =
{
    //148500, 88, 44, 148, 1920, 2200,  4,  5,  36, 1080, 1125 //1080P60
    148500, 528, 44, 148, 1920, 2640, 4, 5, 36, 1080, 1125   //1080p50
};

void lt8619c_write_reg(const uint8_t reg_addr, const uint8_t data);
void lt8619c_write_reg_n(const uint8_t reg_addr, const uint8_t *pdata, uint16_t length);
uint8_t lt8619c_read_reg(const uint8_t reg_addr);

bool lt8619c_check_chip_id(void)
{
    lt8619c_write_reg(0xFF, 0x60);

    if((lt8619c_read_reg(0x00) == 0x16) && (lt8619c_read_reg(0x01) == 0x04))
    {
        NRF_LOG_INFO("Chip id is OK: 0x1604.\n");
        return true;
    }
    else
    {
        NRF_LOG_INFO("Chip id error.\n");
        return false;
    }
}

uint8_t reg_0x60xx_dump[0xFF] = {0};
void lt8619c_reg_0x60xx_dump(void)
{
    lt8619c_write_reg(0xFF, 0x60);
    
    for(int i = 0; i < 0xFF; i++)
        reg_0x60xx_dump[i] = lt8619c_read_reg(i);
}

void lt8619c_set_hpd(Pin_Status level)
{
    lt8619c_write_reg(0xFF, 0x80);
    uint8_t temp = lt8619c_read_reg(0x06);
    NRF_LOG_INFO("[before set] reg[0x8006] = 0x%02x", temp);

    if(level)
    {
        lt8619c_write_reg(0x06,  temp | 0x08);

        temp = lt8619c_read_reg(0x06);
        NRF_LOG_INFO("[after set] reg[0x8006] = 0x%02x", temp);
    }
    else
    {
        lt8619c_write_reg(0x06, temp & 0xF7);

        temp = lt8619c_read_reg(0x06);
        NRF_LOG_INFO("[after set] reg[0x8006] = 0x%02x", temp);
    }
}

void lt8619c_edid_dtb_block_calc(uint8_t *pbuf, const video_timing_t *timing)
{
    uint16_t h_blanking = 0, v_blanking;
    uint16_t pix_clk;
    pix_clk = timing->pix_clk / 10;
    if(pbuf == NULL || timing == NULL)
        return;

    h_blanking = timing->hfp + timing->hs + timing->hbp;
    v_blanking = timing->vfp + timing->vs + timing->vbp;

    pbuf[0] = pix_clk % 256;
    pbuf[1] = pix_clk / 256;
    pbuf[2] = timing->hact % 256;
    pbuf[3] = h_blanking % 256;
    pbuf[4] = ((timing->hact / 256) << 4) + h_blanking / 256;
    pbuf[5] = timing->vact % 256;
    pbuf[6] = v_blanking % 256;
    pbuf[7] = ((timing->vact / 256) << 4) + v_blanking / 256;
    pbuf[8] = timing->hfp % 256;
    pbuf[9] = timing->hs % 256;
    pbuf[10] = ((timing->vfp % 256) << 4) + timing->vs % 256;
    pbuf[11] = ((timing->hfp / 256) << 6) + ((timing->hs / 256) << 4) +
               ((timing->vfp / 256) << 2)+(timing->vs / 256);
    pbuf[17] = 0x1E;           
    
}

uint8_t lt8619c_edid_check_sum(uint8_t block, uint8_t *buf)
{
    uint8_t i = 0, check_sum = 0;
    uint8_t *pbuf = buf + 128 * block;

    if(pbuf == NULL || (pbuf + 127) == NULL)
        return 0;

    for(; i < 127; i++)
    {
        check_sum += pbuf[i];
        check_sum %= 256;
    }

    check_sum = 256 - check_sum;

    return check_sum;
}

void lt8619c_edid_set(const uint8_t *edid_buf)
{
    lt8619c_write_reg(0xFF, 0x80);
    lt8619c_write_reg(0x8E, 0x07);
    lt8619c_write_reg(0x8F, 0x00);

    if(edid_buf == NULL)
        lt8619c_write_reg_n(0x90, &ONCHIP_EDID[0], 256);
    else
    {
        lt8619c_write_reg_n(0x90, edid_buf, 256);
    }
    
    lt8619c_write_reg(0x8E, 0x02);
}

bool lt8619c_load_hdcp_key(void)
{
    uint8_t flag_load_key_done = 0, loop_cnt = 0;

    lt8619c_write_reg(0xFF, 0x80);
    lt8619c_write_reg(0xB2, 0x50);
    lt8619c_write_reg(0xA3, 0x77);

    while(loop_cnt <= 5)
    {
        loop_cnt++;
        nrf_delay_ms(50);
        flag_load_key_done = lt8619c_read_reg(0xC0) & 0x08;

        if(flag_load_key_done)
        {
            break;
        }
    }

    lt8619c_write_reg(0xB2, 0xD0);
    lt8619c_write_reg(0xA3, 0x57);
    if(flag_load_key_done)
    {
        NRF_LOG_INFO("Load KEY done^_^\n");
        return true;
    }
    else
    {
        NRF_LOG_INFO("Load KEY error#####\n");
        return false;
    }
}

void lt8619c_rx_init(void)
{
    lt8619c_write_reg(0xFF, 0x80);

    uint8_t temp = lt8619c_read_reg(0x2C);
    lt8619c_write_reg(0x2C, temp | 0x30);  //RGD_CLK_STABLE_OPT[1:0]
    
    lt8619c_write_reg(0xFF, 0x60);
    lt8619c_write_reg(0x04, 0xF2);
    lt8619c_write_reg(0x83, 0x3F);
    lt8619c_write_reg(0x80, 0x08);  //use xtal_clk as sys_clk.
#ifdef DDR_CLK
    lt8619c_write_reg(0xA4, 0x14);//0x10:SDR clk,0x14: DDR clk
#else
    lt8619c_write_reg(0xA4, 0x10);//0x10:SDR clk,0x14: DDR clk
#endif

#if 0
    LT8619C_Rx_Eq_Mode_Config();// RX EQ adjust
    LT8619C_RX_EQ_Setting(); 
#endif

    NRF_LOG_INFO("LT8619C OUTPUT_MODE: \n");

    if(LT8619C_OUTPUTMODE == OUTPUT_LVDS_2_PORT)
    {
        NRF_LOG_INFO("LT8619C set to OUTPUT_LVDS_2_PORT\n");
        lt8619c_write_reg(0xFF, 0x60);
        lt8619c_write_reg(0x06, 0xE7);
        /* VESA + DE 
        lt8619c_write_reg(0x59, 0x40);//bit7 for VESA/JEIDA mode; bit5 for DE/SYNC mode; bit4 for 6/8bit; bit1 for port swap*/
        /* VESA + SYNC 
        lt8619c_write_reg(0x59, 0x60);*/
        /* JEIDA + DE */
        lt8619c_write_reg(0x59, 0xC0);
        /* JEIDA + SYNC 
        lt8619c_write_reg(0x59, 0xE0);*/
        lt8619c_write_reg(0xA0, 0x58);
        lt8619c_write_reg(0xA4, 0x01);
        lt8619c_write_reg(0xA8, 0x00);
        lt8619c_write_reg(0xBA, 0x18);
        lt8619c_write_reg(0xC0, 0x18);
        lt8619c_write_reg(0xB0, 0x66);
        lt8619c_write_reg(0xB1, 0x66);
        lt8619c_write_reg(0xB2, 0x66);
        lt8619c_write_reg(0xB3, 0x66);
        lt8619c_write_reg(0xB4, 0x66);
        lt8619c_write_reg(0xB5, 0x41);
        lt8619c_write_reg(0xB6, 0x41);
        lt8619c_write_reg(0xB7, 0x41);
        lt8619c_write_reg(0xB8, 0x4C);
        lt8619c_write_reg(0xB9, 0x41);
        lt8619c_write_reg(0xBB, 0x41);
        lt8619c_write_reg(0xBC, 0x41);
        lt8619c_write_reg(0xBD, 0x41);
        lt8619c_write_reg(0xBE, 0x4C);
        lt8619c_write_reg(0xBF, 0x41);
        lt8619c_write_reg(0xA0, 0x18);
        lt8619c_write_reg(0xA1, 0xB0);
        lt8619c_write_reg(0xA2, 0x10);
        lt8619c_write_reg(0x5C, 0x01);//bit0=0:single port  bit0=1:dual port
    }
    else if(  LT8619C_OUTPUTMODE ==  OUTPUT_LVDS_1_PORT )
    {
        NRF_LOG_INFO("\r\nLT8619C set to OUTPUT_LVDS_1_PORT");
        lt8619c_write_reg(0xFF,0x60);
        lt8619c_write_reg(0x06,0xe7);
        lt8619c_write_reg(0x59,0x40);//0xd0 bit7 for VESA/JEIDA mode; bit5 for DE/SYNC mode; bit4 for 6/8bit; bit1 for port swap
        lt8619c_write_reg(0xa0,0x58);
        lt8619c_write_reg(0xa4,0x00);
        //lt8619c_write_reg(0xa8,0x00);
        lt8619c_write_reg(0xba,0x18);
        lt8619c_write_reg(0xc0,0x18);
        lt8619c_write_reg(0xb0,0x66);
        lt8619c_write_reg(0xb1,0x66);
        lt8619c_write_reg(0xb2,0x66);
        lt8619c_write_reg(0xb3,0x66);
        lt8619c_write_reg(0xb4,0x66);
        lt8619c_write_reg(0xb5,0x41);
        lt8619c_write_reg(0xb6,0x41);
        lt8619c_write_reg(0xb7,0x41);
        lt8619c_write_reg(0xb8,0x4c);
        lt8619c_write_reg(0xb9,0x41);
        lt8619c_write_reg(0xbb,0x41);
        lt8619c_write_reg(0xbc,0x41);
        lt8619c_write_reg(0xbd,0x41);
        lt8619c_write_reg(0xbe,0x4c);
        lt8619c_write_reg(0xbf,0x41);
        lt8619c_write_reg(0xa0,0x18);
        lt8619c_write_reg(0xa1,0xb0);
        lt8619c_write_reg(0xa2,0x10);
        lt8619c_write_reg(0x5c,0x00);//bit0=0:single port  bit0=1:dual port
    }

    lt8619c_write_reg(0xFF, 0x60);
    lt8619c_write_reg(0x0E, 0xFD);	
    lt8619c_write_reg(0x0E, 0xFF);
    lt8619c_write_reg(0x0D, 0xFC);	
    lt8619c_write_reg(0x0D, 0xFF);
}

void lt8619c_audio_init(void)
{
    if(AUDIO_INPUT_MODE == I2S_2CH)
    {
        NRF_LOG_INFO("Audio set to I2S_2CH\n");
		lt8619c_write_reg(0xFF,0x60);
		lt8619c_write_reg(0x4C,0x00);
    }
    else if(AUDIO_INPUT_MODE == SPDIF)
    {
        NRF_LOG_INFO("Audio set to SPDIF\n");
		lt8619c_write_reg(0xFF,0x60);
		lt8619c_write_reg(0x4C,0x80);
    }
    
    lt8619c_write_reg(0xFF, 0x80);
    lt8619c_write_reg(0x5D, 0xC9);
    lt8619c_write_reg(0x08, 0x80);
}

void lt8619c_rx_reset(void)
{
    lt8619c_write_reg(0xFF, 0x60);
    lt8619c_write_reg(0x0E, 0xBF);
    lt8619c_write_reg(0x09, 0xFD);
    nrf_delay_ms(5);
    lt8619c_write_reg(0x0E, 0xFF);
    lt8619c_write_reg(0x09, 0xFF);

    lt8619c_write_reg(0xFF, 0x60);
    lt8619c_write_reg(0x0E, 0xC7);
    lt8619c_write_reg(0x09, 0x0F);
    nrf_delay_ms(10);
    lt8619c_write_reg(0x0E, 0xFF);
    nrf_delay_ms(10);
    lt8619c_write_reg(0x09, 0x8F);
    nrf_delay_ms(10);
    lt8619c_write_reg(0x09, 0xFF);
    nrf_delay_ms(50);
    
}
bool lt8619c_clock_detect(void)
{
    uint8_t read_data_1, read_data_2;

    lt8619c_write_reg(0xFF, 0x80);
    read_data_1 = lt8619c_read_reg(0x44) & 0x08;
    
    /*
    uint32_t clk = lt8619c_read_reg(0x44) & 0x07;
    clk <<= 8;
    clk += lt8619c_read_reg(0x45);
    clk <<= 8;
    clk += lt8619c_read_reg(0x46);
    NRF_LOG_INFO("tmds clk freq val: %d kHz", clk);
    */
    
    if(read_data_1)
    {
        if(!plt8619c_rx_status->flag_rx_clk_stable)
        {
            plt8619c_rx_status->flag_rx_clk_stable = !plt8619c_rx_status->flag_rx_clk_stable;
            lt8619c_write_reg(0xFF, 0x60);
            read_data_1 = lt8619c_read_reg(0x97);
            lt8619c_write_reg(0x97, read_data_1 & 0x3F);
            lt8619c_write_reg(0xFF, 0x80);
            lt8619c_write_reg(0x1B, 0x00);
#if 0
            lt8619c_write_reg(0xFF, 0x60);
            read_data_1 = lt8619c_read_reg(0x0E);
            read_data_2 = lt8619c_read_reg(0x09);

            lt8619c_write_reg(0x0E, read_data_1 & 0x87);
            lt8619c_write_reg(0x09, read_data_2 & 0x8F);
            nrf_delay_ms(5);
            lt8619c_write_reg(0x0E, read_data_1 | 0x40);
            nrf_delay_ms(5);
            lt8619c_write_reg(0x09, read_data_2 | 0x70);
            lt8619c_write_reg(0x0E, read_data_1 | 0x38);
            lt8619c_write_reg(0x0E, 0xFD);
            lt8619c_write_reg(0x0E, 0xFF);
#endif
            lt8619c_rx_reset();

            lt8619c_write_reg(0xFF, 0x60);
            lt8619c_write_reg(0x0E, 0xFD);
            lt8619c_write_reg(0x0E, 0xFF);
            nrf_delay_ms(5);

            lt8619c_write_reg(0xFF, 0x80);
            read_data_1 = lt8619c_read_reg(0x87) & 0x10;

            if(read_data_1)
            {
                plt8619c_rx_status->flag_rx_pll_locked = true;
                NRF_LOG_INFO("LT8619C clk detected!!!");
                NRF_LOG_INFO("LT8619C pll lock!!!");

                return true;
            }
            else
            {
                plt8619c_rx_status->flag_rx_pll_locked = false;
                memset(plt8619c_rx_status, 0, sizeof(lt8619c_rx_status_t));
                NRF_LOG_INFO("LT8619C clk detected!!!");
                NRF_LOG_INFO("LT8619C pll unlock!!!");
                return false;
            }
        }
        else
        {
            lt8619c_write_reg(0xFF, 0x80);
            read_data_1 = lt8619c_read_reg(0x87) & 0x10;

            if(read_data_1)
            {
                plt8619c_rx_status->flag_rx_pll_locked = true;
                return true;
            }
            else
            {
                plt8619c_rx_status->flag_rx_pll_locked = false;
                memset(plt8619c_rx_status, 0, sizeof(lt8619c_rx_status_t));
                NRF_LOG_INFO("LT8619C pll unlock!!!");
                return false;
            }
        }
    }
    else
    {
        if(plt8619c_rx_status->flag_rx_clk_stable)
        {
            NRF_LOG_INFO("LT8619C clk disappear!!!");
        }
        memset(plt8619c_rx_status, 0, sizeof(lt8619c_rx_status_t));
        return false;
    }
}

void lt8619c_get_input_info(void)
{
    uint8_t loop_num, read_data;

    lt8619c_write_reg(0xFF, 0x80);

    if(plt8619c_rx_status->flag_rx_clk_stable && plt8619c_rx_status->flag_rx_pll_locked)
    {
        if(lt8619c_read_reg(0x13) & 0x01)
        {
            if(!plt8619c_rx_status->flag_hsync_stable)
            {
                plt8619c_rx_status->flag_hsync_stable = true;
                
                for(loop_num = 0; loop_num < 8; loop_num++)
                {
                    if(!(lt8619c_read_reg(0x13) & 0x01))
                    {
                        NRF_LOG_INFO("LT8619C 8013[0] = 0 !!!#####");
                        plt8619c_rx_status->flag_hsync_stable = false;
                        NRF_LOG_INFO("LT8619C Hsync stable Fail #####");
                        break;
                    }
                }

                if(plt8619c_rx_status->flag_hsync_stable)
                {
                    lt8619c_write_reg(0xFF, 0x60);
                    read_data = lt8619c_read_reg(0x0D);
                    lt8619c_write_reg(0x0D, read_data & 0xF8);  //reset LVDS/BT fifo
                    lt8619c_write_reg(0x0D, read_data | 0x06);
                    lt8619c_write_reg(0x0D, read_data | 0x01);

                    NRF_LOG_INFO("LT8619C Hsync stable!!!");
                }
            }
        }
        else
        {
            if(plt8619c_rx_status->flag_hsync_stable)
            {
                NRF_LOG_INFO("LT8619C Hsync stable to unstable#####");
            }

            plt8619c_rx_status->flag_hsync_stable = false;

            NRF_LOG_INFO("LT8619C Hsync always unstable#####");
        }
    }

    if(plt8619c_rx_status->flag_hsync_stable)
    {
        read_data = lt8619c_read_reg(0x13);
        plt8619c_rx_status->input_hdmi_mode = (read_data & 0x02)? true : false;

        if(plt8619c_rx_status->input_hdmi_mode)
        {
            plt8619c_rx_status->input_vic = lt8619c_read_reg(0x74) & 0x7F;
            plt8619c_rx_status->input_color_space = lt8619c_read_reg(0x71) & 0x60;
            plt8619c_rx_status->input_color_depth = lt8619c_read_reg(0x16) & 0xF0;
            plt8619c_rx_status->input_color_imetry = lt8619c_read_reg(0x72) & 0xC0;
            plt8619c_rx_status->input_ex_color_imetry = lt8619c_read_reg(0x73) & 0x70;
            plt8619c_rx_status->input_quant_range = lt8619c_read_reg(0x73) & 0x0C;
            plt8619c_rx_status->input_pr_factor = lt8619c_read_reg(0x75) & 0x0F;

            if(plt8619c_rx_status->input_pr_factor == 1)
            {
                lt8619c_write_reg(0xFF, 0x60);
                read_data = lt8619c_read_reg(0x97);
                lt8619c_write_reg(0x97, read_data | 0x40);

                lt8619c_write_reg(0xFF, 0x80);
                lt8619c_write_reg(0x1B, 0x20);
            }
            else if(plt8619c_rx_status->input_pr_factor == 3)
            {
                lt8619c_write_reg(0xFF, 0x60);
                read_data = lt8619c_read_reg(0x97);
                lt8619c_write_reg(0x97, read_data | 0x80);

                lt8619c_write_reg(0xFF, 0x80);
                lt8619c_write_reg(0x1B, 0x60);
            }
            else
            {
                lt8619c_write_reg(0xFF, 0x60);
                read_data = lt8619c_read_reg(0x97);
                lt8619c_write_reg(0x97, read_data & 0x3F);

                lt8619c_write_reg(0xFF, 0x80);
                lt8619c_write_reg(0x1B, 0x00);
            }
        }
        else
        {
            plt8619c_rx_status->input_vic = 0;
            plt8619c_rx_status->input_color_space = COLOR_RGB;
            plt8619c_rx_status->input_color_depth = 0;
            plt8619c_rx_status->input_color_imetry = ITU_709;
            plt8619c_rx_status->input_ex_color_imetry = 0;
            plt8619c_rx_status->input_quant_range = FULL_RANGE;
            plt8619c_rx_status->input_pr_factor = 0;

            lt8619c_write_reg(0xFF, 0x60);
            read_data = lt8619c_read_reg(0x97);
            lt8619c_write_reg(0x97, read_data & 0x3F);

            lt8619c_write_reg(0xFF, 0x80);
            lt8619c_write_reg(0x1B, 0x00);
        }
    }
}

void lt8619c_csc_conversion(void)
{
    lt8619c_write_reg(0xFF, 0x60);
    lt8619c_write_reg(0x07, 0xFE);
    if( LT8619C_OUTPUTCOLOR == COLOR_RGB )
    {
        if( plt8619c_rx_status->input_color_space == COLOR_RGB )
        {
            lt8619c_write_reg(0x52, 0x00);
            if( plt8619c_rx_status->input_quant_range == LIMIT_RANGE )
            {
                lt8619c_write_reg(0x53, 0x08);
            }
            else
            {
                lt8619c_write_reg(0x53, 0x00);
            }
        }
        else //input_colorspace = COLOR_YCBCR444 or COLOR_YCBCR422 or <ELSE>.
        {
            if( plt8619c_rx_status->input_color_space == COLOR_YCBCR422 )
            {
                lt8619c_write_reg(0x52, 0x01);
            }
            else
            {
                lt8619c_write_reg(0x52, 0x00);
            }
            if( plt8619c_rx_status->input_quant_range == LIMIT_RANGE )
            {                                                     
                if(plt8619c_rx_status->input_color_imetry == ITU_601 )
                {
                    lt8619c_write_reg(0x53, 0x50);//output to 0~255
                }
                else if(plt8619c_rx_status->input_color_imetry == ITU_709 )
                {
                    lt8619c_write_reg(0x53, 0x70);//output to 0~255
                }
                else  //input_colorimetry = NO_DATA or EXTENDED_COLORIETRY
                {
                    lt8619c_write_reg(0x53, 0x70);
                }
            }
            else if( plt8619c_rx_status->input_quant_range == FULL_RANGE )
            {
                if(plt8619c_rx_status->input_color_imetry == ITU_601 )
                {
                    lt8619c_write_reg(0x53, 0x40);
                }
                else if(plt8619c_rx_status->input_color_imetry == ITU_709 )
                {
                    lt8619c_write_reg(0x53, 0x60);
                }
                else  //input_colorimetry = NO_DATA or EXTENDED_COLORIETRY
                {
                    lt8619c_write_reg(0x53, 0x60);
                }
            }
            else  //DEFAULT_RANGE or RESERVED_VAL
            {
                lt8619c_write_reg(0x53, 0x60);
            }
        }
    }
    else if( LT8619C_OUTPUTCOLOR == COLOR_YCBCR444 )
    {
        if( plt8619c_rx_status->input_color_space == COLOR_RGB )
        {
            lt8619c_write_reg(0x53, 0x00);
            if( plt8619c_rx_status->input_quant_range == LIMIT_RANGE )
            {
                if(plt8619c_rx_status->input_color_imetry == ITU_601 )
                {
                    lt8619c_write_reg(0x52, 0x08);
                }
                else if(plt8619c_rx_status->input_color_imetry == ITU_709 )
                {
                    lt8619c_write_reg(0x52, 0x28);
                }
                else  //input_colorimetry = NO_DATA or EXTENDED_COLORIETRY
                {
                    lt8619c_write_reg(0x52, 0x28);
                }
            }
            else if( plt8619c_rx_status->input_quant_range == FULL_RANGE )
            {
                if(plt8619c_rx_status->input_color_imetry == ITU_601 )
                {
                    lt8619c_write_reg(0x52, 0x18);
                }
                else if(plt8619c_rx_status->input_color_imetry == ITU_709 )
                {
                    lt8619c_write_reg(0x52, 0x38);
                }
                else  //input_colorimetry = NO_DATA or EXTENDED_COLORIETRY
                {
                    lt8619c_write_reg(0x52, 0x38);
                }
            }
            else  //DEFAULT_RANGE or RESERVED_VAL
            {
                lt8619c_write_reg(0x52, 0x38);
            }
        }
        else if( plt8619c_rx_status->input_color_space == COLOR_YCBCR444 )
        {
            lt8619c_write_reg(0x52, 0x00);
            lt8619c_write_reg(0x53, 0x00);
        }
        else if( plt8619c_rx_status->input_color_space == COLOR_YCBCR422 )
        {
            lt8619c_write_reg(0x52, 0x01);
            lt8619c_write_reg(0x53, 0x00);
        }
    }
    else if( LT8619C_OUTPUTCOLOR == COLOR_YCBCR422 )
    {
        if( plt8619c_rx_status->input_color_space == COLOR_RGB )
        {
            lt8619c_write_reg(0x53, 0x00);
            if( plt8619c_rx_status->input_quant_range == LIMIT_RANGE )
            {
                if(plt8619c_rx_status->input_color_imetry == ITU_601 )
                {
                    lt8619c_write_reg(0x52, 0x0A);
                }
                else if(plt8619c_rx_status->input_color_imetry == ITU_709 )
                {
                    lt8619c_write_reg(0x52, 0x2A);
                }
                else  //input_colorimetry = NO_DATA or EXTENDED_COLORIETRY
                {
                    lt8619c_write_reg(0x52, 0x2A);
                }
            }
            else if( plt8619c_rx_status->input_quant_range == FULL_RANGE )
            {
                if(plt8619c_rx_status->input_color_imetry == ITU_601 )
                {
                    lt8619c_write_reg(0x52, 0x1A);
                }
                else if(plt8619c_rx_status->input_color_imetry == ITU_709 )
                {
                    lt8619c_write_reg(0x52, 0x3A);
                }
                else  //input_colorimetry = NO_DATA or EXTENDED_COLORIETRY
                {
                    lt8619c_write_reg(0x52, 0x3A);
                }
            }
            else  //DEFAULT_RANGE or RESERVED_VAL
            {
                lt8619c_write_reg(0x52, 0x3A);
            }
        }
        else if( plt8619c_rx_status->input_color_space == COLOR_YCBCR444 )
        {
            lt8619c_write_reg(0x52, 0x02);
            lt8619c_write_reg(0x53, 0x00);
        }
        else if( plt8619c_rx_status->input_color_space == COLOR_YCBCR422 )
        {
            lt8619c_write_reg(0x52, 0x00);
            lt8619c_write_reg(0x53, 0x00);
        }
    }
}

void lt8619c_video_check(void)
{
    uint8_t   tmp_read;
    if( !plt8619c_rx_status->flag_hsync_stable)
    {
        h_total = 0;
        v_total = 0;
        return;
    }
    
    lt8619c_write_reg(0xFF,0x60);

    h_active       = ((uint16_t)lt8619c_read_reg(0x22)) << 8;
    h_active      += lt8619c_read_reg(0x23);
    v_active       = ((uint16_t)(lt8619c_read_reg(0x20) & 0x0F)) << 8;
    v_active      += lt8619c_read_reg(0x21);
    frame_counter  = ((uint32_t)lt8619c_read_reg(0x10)) << 16;
    frame_counter += ((uint32_t)lt8619c_read_reg(0x11)) << 8;
    frame_counter += lt8619c_read_reg(0x12);

    h_syncwidth  = ((uint16_t)(lt8619c_read_reg(0x14) & 0x0F)) << 8;
    h_syncwidth += lt8619c_read_reg(0x15);
    v_syncwidth  = lt8619c_read_reg(0x13);
    h_bkporch    = ((uint16_t)(lt8619c_read_reg(0x18) & 0x0F)) << 8;
    h_bkporch   += lt8619c_read_reg(0x19);
    v_bkporch    = lt8619c_read_reg(0x16);
    h_total      = ((uint16_t)lt8619c_read_reg(0x1E)) << 8;
    h_total     += lt8619c_read_reg(0x1F);
    v_total      = ((uint16_t)(lt8619c_read_reg(0x1c) & 0x0f)) << 8;
    v_total     += lt8619c_read_reg(0x1D);
    tmp_read     = lt8619c_read_reg(0x24);
    h_syncpol    = tmp_read & 0x01;
    v_syncpol    = (tmp_read & 0x02) >> 1;

#if 0
/***In order to avoid resolution error,Customers can change the judgment conditions according to the resolution used***/
//	if((h_active != 1920)||(v_active !=1080))
		if((h_active != lcd_timing.hact)||(v_active !=lcd_timing.vact)) 
		{	
			memset(plt8619c_rx_status, 0, sizeof(_LT8619C_RXStatus));	
		}
#endif
}

void lt8619c_lvds_pll_lock_det(void)
{
    uint8_t read_data_1;
    uint8_t check_num = 0;
    //NRF_LOG_INFO("LVDSPLL status Det ing");
    lt8619c_write_reg(0xFF, 0x80);
    while((lt8619c_read_reg(0x87) & 0x20)==0x00)//Output LVDSPLL unlock status
    {
        lt8619c_write_reg(0xFF, 0x60);
        read_data_1 = lt8619c_read_reg(0x0E);
        lt8619c_write_reg(0x0E, read_data_1 & 0xFD);//LVDSPLL soft reset
        nrf_delay_ms(5);
        lt8619c_write_reg(0x0E, 0xFF);

        lt8619c_write_reg(0xFF, 0x80);

        check_num++;
        if(check_num > 3)
        {
            break;
        }
    }
}

void lt8619c_bt_setting(void)
{
    uint8_t val_6060;
    uint16_t tmp_data;
    
	
    if( !plt8619c_rx_status->flag_hsync_stable )
    {
        return;
    }

    lt8619c_lvds_pll_lock_det();
		
    lt8619c_write_reg(0xFF,0x60);
    val_6060 = lt8619c_read_reg(0x60) & 0xE7;

    //set BT TX h/vsync polarity
    if( h_syncpol )
    {
        val_6060 |= 0x20;
    }
    if( v_syncpol )
    {
        val_6060 |= 0x10;
    }

    //double the value of v_active&v_total when input is interlace resolution.
    //if user needs to support interlace format not listed here, please add that interlace format info here. 
		
    if( plt8619c_rx_status->input_hdmi_mode )
    {
        switch( plt8619c_rx_status->input_vic )
        {
            case 5:
            case 6: 
            case 7: 
            case 10: 
            case 11: 
            case 20: 
                // NRF_LOG_INFO("VIC20 ");
                val_6060 |= 0x08;
                v_active <<= 1;
                if( v_total % 2 == 1 )
                {
                    v_total = (v_total << 1) - 1;
                }
                else
                {
                    v_total = (v_total << 1) + 1;
                }
                lt8619c_write_reg(0x68, 23);    // TODO 0x23 ? 
                break;

            case 21: 
            case 22:
            case 25:
            case 26:
                NRF_LOG_INFO("VIC26 ");
                val_6060 |= 0x08;
                v_active <<= 1;
                if( v_total % 2 == 1 )
                {
                    v_total = (v_total << 1) - 1;
                }
                else
                {
                    v_total = (v_total << 1) + 1;
                }
                lt8619c_write_reg(0x68, 25); // TODO 0x25 ? 
                break;

            default: 
                lt8619c_write_reg(0x68, 0x00);
                break;
        }

    }
    else  //dvi input
    {
        if( (h_active == 1920) && (v_active == 540) )
        {
            val_6060 |= 0x08;
            v_active <<= 1;
            if( v_total % 2 == 1 )
            {
                v_total = (v_total << 1) - 1;
            }
            else
            {
                v_total = (v_total << 1) + 1;
            }
            lt8619c_write_reg(0x68, 23);    // TODO 0x23 ? 
        }
        else if( (h_active == 1440) && (v_active == 240) )
        {
            val_6060 |= 0x08;
            v_active <<= 1;
            if( v_total % 2 == 1 )
            {
                v_total = (v_total << 1) - 1;
            }
            else
            {
                v_total = (v_total << 1) + 1;
            }
            lt8619c_write_reg(0x68, 23);
        }
        else if( (h_active == 1440) && (v_active == 288) )
        {
            val_6060 |= 0x08;
            v_active <<= 1;
            if( v_total % 2 == 1 )
            {
                v_total = (v_total << 1) - 1;
            }
            else
            {
                v_total = (v_total << 1) + 1;
            }
            lt8619c_write_reg(0x68, 25);
        }
    }

    lt8619c_write_reg(0x60, val_6060);
    tmp_data = h_syncwidth + h_bkporch;
    lt8619c_write_reg(0x61, (uint8_t)(tmp_data >> 8));
    lt8619c_write_reg(0x62, (uint8_t)tmp_data);
    tmp_data = h_active;
    lt8619c_write_reg(0x63, (uint8_t)(tmp_data >> 8));
    lt8619c_write_reg(0x64, (uint8_t)tmp_data);
    tmp_data = h_total;
    lt8619c_write_reg(0x65, (uint8_t)(tmp_data >> 8));
    lt8619c_write_reg(0x66, (uint8_t)tmp_data);
    tmp_data = v_syncwidth + v_bkporch;
    lt8619c_write_reg(0x67, (uint8_t)tmp_data);
    tmp_data = v_active;
    lt8619c_write_reg(0x69, (uint8_t)(tmp_data >> 8));
    lt8619c_write_reg(0x6a, (uint8_t)tmp_data);
    tmp_data = v_total;
    lt8619c_write_reg(0x6b, (uint8_t)(tmp_data >> 8));
    lt8619c_write_reg(0x6c, (uint8_t)tmp_data);
}

void lt8619c_debug_print_rx_info(void)
{
    uint32_t clk=0;
    static uint16_t print_en = 0;
    uint32_t audio_rate = 0;

    if( !plt8619c_rx_status->flag_hsync_stable )
    {
        print_en = 0;
        return;
    }
    if( print_en != 3)
    {
        print_en++;

        if( print_en == 3)
        {	
            print_en = 0;
            /*
            if( plt8619c_rx_status->input_hdmi_mode )
            {
                NRF_LOG_INFO("Input is HDMI signal");

                NRF_LOG_INFO("input_vic = %d ", plt8619c_rx_status->input_vic);

                if( plt8619c_rx_status->input_color_space == COLOR_RGB )
                {
                    NRF_LOG_INFO("input_colorspace is COLOR_RGB");
                }
                else if( plt8619c_rx_status->input_color_space == COLOR_YCBCR444 )
                {
                    NRF_LOG_INFO("input_colorspace is COLOR_YCBCR444");
                }
                else if( plt8619c_rx_status->input_color_space == COLOR_YCBCR422 )
                {
                    NRF_LOG_INFO("input_colorspace is COLOR_YCBCR422");
                }
                else
                {
                    NRF_LOG_INFO("input_colorspace is unkonwn");
                }

                NRF_LOG_INFO("input_colordepth = %d ",plt8619c_rx_status->input_color_depth);

                if( plt8619c_rx_status->input_color_imetry == NO_DATA )
                {
                    NRF_LOG_INFO("input_colorimetry is NO_DATA");
                }
                else if( plt8619c_rx_status->input_color_imetry == ITU_601 )
                {
                    NRF_LOG_INFO("input_colorimetry is ITU_601");
                }
                else if( plt8619c_rx_status->input_color_imetry == ITU_709 )
                {
                    NRF_LOG_INFO("input_colorimetry is ITU_709");
                }
                else
                {
                    NRF_LOG_INFO("input_colorimetry is EXTENDED_COLORIETRY");
                }

                if( plt8619c_rx_status->input_color_imetry == EXTENDED_COLORIETRY )
                {
                    if( plt8619c_rx_status->input_ex_color_imetry == xvYCC601 )
                    {
                        NRF_LOG_INFO("input_ex_colorimetry is xvYCC601");
                    }
                    else if( plt8619c_rx_status->input_ex_color_imetry == xvYCC709 )
                    {
                        NRF_LOG_INFO("input_ex_colorimetry is xvYCC709");
                    }
                    else
                    {
                        NRF_LOG_INFO("input_ex_colorimetry is FUTURE_COLORIETRY");
                    }
                }

                if( plt8619c_rx_status->input_quant_range == DEFAULT_RANGE )
                {
                    NRF_LOG_INFO("input_QuantRange is DEFAULT_RANGE");
                }
                else if( plt8619c_rx_status->input_quant_range == LIMIT_RANGE )
                {
                    NRF_LOG_INFO("input_QuantRange is LIMIT_RANGE");
                }
                else if( plt8619c_rx_status->input_quant_range == FULL_RANGE )
                {
                    NRF_LOG_INFO("input_QuantRange is FULL_RANGE");
                }
                else
                {
                    NRF_LOG_INFO("input_QuantRange is RESERVED_VAL");
                }

                NRF_LOG_INFO("input_PRfactor = %d ",plt8619c_rx_status->input_pr_factor);
            }
            else
            {
                NRF_LOG_INFO("Input is DVI signal");
            }*/


            NRF_LOG_INFO("---------------- Input Timing Info -------------------");
            lt8619c_write_reg(0xFF, 0x80);
            clk = lt8619c_read_reg(0x44) & 0x07;
            clk <<= 8;
            clk += lt8619c_read_reg(0x45);
            clk <<= 8;
            clk += lt8619c_read_reg(0x46);
            NRF_LOG_INFO("tmds clk freq val: %d kHz", clk);
            /*
            lt8619c_write_reg(0xFF,0x60);
            tmp_data = lt8619c_read_reg(0x60);//0x6060 will be set in function [void LT8619C_BTSetting(void)].
            if( tmp_data & 0x08 )
            {
            NRF_LOG_INFO("input is I format!!!");
            }
            else 
            {
            NRF_LOG_INFO("input is P format!!!");
            }
            */
            NRF_LOG_INFO("h_active = %d", h_active);
            NRF_LOG_INFO("v_active = %d", v_active);
            NRF_LOG_INFO("h_syncwidth = %d", h_syncwidth);
            NRF_LOG_INFO("v_syncwidth = %d", v_syncwidth);
            NRF_LOG_INFO("h_bkporch = %d", h_bkporch);
            NRF_LOG_INFO("v_bkporch = %d", v_bkporch);
            NRF_LOG_INFO("h_total = %d", h_total);
            NRF_LOG_INFO("v_total = %d", v_total);
            NRF_LOG_INFO("frame_counter = %d", frame_counter);
            if(h_syncpol)
            {
                NRF_LOG_INFO("h_syncpol is positve!!! ");
            }
            else
            {
                NRF_LOG_INFO("h_syncpol is negative!!! ");
            }
            if(v_syncpol)
            {
                NRF_LOG_INFO("v_syncpol is positve!!! ");
            }
            else
            {
                NRF_LOG_INFO("v_syncpol is negative!!! ");
            }

            //打印音频采样率  zll
            lt8619c_write_reg(0xFF, 0x80);
            audio_rate = ((lt8619c_read_reg(0x11) & 0x03) * 0x100) + lt8619c_read_reg(0x12);
            NRF_LOG_INFO("audio_rate = %d kHz", audio_rate);
            NRF_LOG_INFO("------------------------------------------------------------");
        }
    }		
}

bool lvds_data_check(void)
{
    uint8_t read_data1, read_data2;
	uint16_t i, LVDScheck_bit0;
	uint16_t x = 0;


	for ( i = 0; i < 200; i++ )
	{		
		lt8619c_write_reg(0xFF, 0x60);
		LVDScheck_bit0 = lt8619c_read_reg(0x92)&0x01;
		if(LVDScheck_bit0 == 0)
		{
			x++;
		}
	}
#if 1	
		lt8619c_write_reg(0xFF,0x60);
		read_data1= lt8619c_read_reg(0x92);
		read_data2= lt8619c_read_reg(0x0D);

        NRF_LOG_INFO("LVDS_data_checknum1: %d", x);
        NRF_LOG_INFO("1_0x6092: %d", read_data1);
        NRF_LOG_INFO("1_0x600D: %d", read_data2);	
#endif
			
	if((x == 200) || (x == 0))
	{
#if 1		
		lt8619c_write_reg(0xFF,0x60);
		read_data1= lt8619c_read_reg(0x92);
		read_data2= lt8619c_read_reg(0x0D);	
		
		NRF_LOG_INFO("LVDS_data_checknum2: %d", x);
		NRF_LOG_INFO("1_0x6092: %d", read_data1);
        NRF_LOG_INFO("1_0x600D: %d", read_data2);	
#endif	
		
		return false;
		
	}
	else
	{
		return true;
	}
}

void lt8619c_lvds_data_detect(void)
{
    uint8_t read_data, read_data1, read_data2;
    uint8_t n = 0;
    do
    {
        if(!lvds_data_check())
        {
            lt8619c_write_reg(0xFF, 0x60);
            read_data = lt8619c_read_reg(0x0D);
            lt8619c_write_reg(0x0D, read_data & 0xF8);  //reset LVDS/BT fifo
            nrf_delay_ms(10);
            lt8619c_write_reg(0x0D, read_data | 0x06);
            lt8619c_write_reg(0x0D, read_data | 0x01);

            n++;            
#if 1			
            NRF_LOG_INFO("Reset_LVDS_FIFO_SUCCESS ");
            lt8619c_write_reg(0xFF, 0x60);
            read_data1= lt8619c_read_reg(0x92);
            read_data2= lt8619c_read_reg(0x0D);
            NRF_LOG_INFO("1_0x6092: %d", read_data1);
            NRF_LOG_INFO("1_0x600D: %d", read_data2);  
#endif

        }

        if(n > 4)
        {           
            memset(plt8619c_rx_status, 0, sizeof(lt8619c_rx_status_t));                
            break;
        }       

    }while(!lvds_data_check());

}

void lt8619c_main_loop(void)
{
    /*lt8619c_write_reg(0xFF, 0x80);
    uint8_t temp = lt8619c_read_reg(0x13) & 0x01;
    NRF_LOG_INFO("reg[0x8013] & 0x01 = %d", temp);

    */if(lt8619c_clock_detect())
    {
        nrf_delay_ms(100);
        lt8619c_get_input_info();
        lt8619c_csc_conversion();
        lt8619c_video_check();
        lt8619c_bt_setting();

        lt8619c_debug_print_rx_info();
#if 0
        if(LT8619C_OUTPUTMODE == OUTPUT_LVDS_2_PORT)
        {
            lt8619c_lvds_data_detect();
        }
#endif        
    }
    
}

void lt8619c_i2c_init(void)
{
    ret_code_t err_code;
    const nrfx_twi_config_t config = 
    {
        .scl                = LT8619C_I2C_SCL_PIN_NUM,
        .sda                = LT8619C_I2C_SDA_PIN_NUM,
        .frequency          = NRF_TWI_FREQ_100K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .hold_bus_uninit     = true
    };
    
    err_code = nrfx_twi_init(&m_twi, &config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    
    nrfx_twi_enable(&m_twi);
}

void lt8619c_write_reg(const uint8_t reg_addr, const uint8_t data)
{
    ret_code_t err_code;
    
    uint8_t send_data[2] = {reg_addr, data};
    
    err_code = nrfx_twi_tx(&m_twi, LT8619C_I2C_ADDR, send_data, 2, false);
    APP_ERROR_CHECK(err_code);
    
    /*
    err_code = nrfx_twi_tx(&m_twi, LT8619C_I2C_ADDR, &reg_addr, 1, false);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_twi_tx(&m_twi, LT8619C_I2C_ADDR, &data, 1, false);
    APP_ERROR_CHECK(err_code);
    */
}

/* NEED DEBUG */
void lt8619c_write_reg_n(const uint8_t reg_addr, const uint8_t *pdata, uint16_t length)
{
    ret_code_t err_code;
    
    uint8_t *pbuf = malloc(1 + length);
    *pbuf = reg_addr;
    memcpy(pbuf + 1, pdata, length);
    
    err_code = nrfx_twi_tx(&m_twi, LT8619C_I2C_ADDR, pbuf, 1 + length, false);
    APP_ERROR_CHECK(err_code);

    free(pbuf);
    
    /*
    err_code = nrfx_twi_tx(&m_twi, LT8619C_I2C_ADDR, &reg_addr, 1, true);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_twi_tx(&m_twi, LT8619C_I2C_ADDR, pdata, 255, true);
    APP_ERROR_CHECK(err_code);

    err_code = nrfx_twi_tx(&m_twi, LT8619C_I2C_ADDR, pdata + 255, 1, false);
    APP_ERROR_CHECK(err_code);*/
}

uint8_t lt8619c_read_reg(const uint8_t reg_addr)
{
    ret_code_t err_code;
    uint8_t data;
    err_code = nrfx_twi_tx(&m_twi, LT8619C_I2C_ADDR, &reg_addr, 1, false);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrfx_twi_rx(&m_twi, LT8619C_I2C_ADDR, &data, 1);
    APP_ERROR_CHECK(err_code);

    return data;
}

void lt8619c_init(void)
{
    lt8619c_i2c_init();
    
    plt8619c_rx_status = &lt8619c_rx_status;
    memset(plt8619c_rx_status, 0, sizeof(lt8619c_rx_status_t));

    /*lt8619c_reg_0x60xx_dump();*/

    lt8619c_check_chip_id();

    lt8619c_set_hpd(LOW);

    lt8619c_edid_dtb_block_calc(ONCHIP_EDID + 0x36, &lcd_timing);
    ONCHIP_EDID[127] = lt8619c_edid_check_sum(0, &ONCHIP_EDID[0]);
    lt8619c_edid_set(ONCHIP_EDID);

    nrf_delay_ms(300);
    
    lt8619c_set_hpd(HIGH);
    

#ifdef USE_EXTERNAL_HDCPKEY
    lt8619c_load_hdcp_key();
#endif
    
    lt8619c_rx_init();
    lt8619c_audio_init();
    
    
}




