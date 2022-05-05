#include "nrf_drv_twi.h"
#include "nrf_error.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "lt8619c.h"

/* TWI instance ID */
#define TWI_INSTANCE_ID     0

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

uint8_t refer_resistance;

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
    148500, 88, 44, 148, 1920, 2200,  4,  5,  36, 1080, 1125 //1080P60
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

void lt8619c_set_hpd(Pin_Status level)
{
    lt8619c_write_reg(0xFF, 0x80);

    if(level)
    {
        lt8619c_write_reg(0x06, lt8619c_read_reg(0x06) | 0x08);
    }
    else
    {
        lt8619c_write_reg(0x06, lt8619c_read_reg(0x06) & 0xF7);
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
        lt8619c_write_reg_n(0x90, edid_buf, 256);
    
    lt8619c_write_reg(0x8E, 0x02);
}

bool lt8619c_load_hdcp_key(void)
{
    uint8_t flag_load_key_done = 0, loop_cnt = 0;

    lt8619c_write_reg(0xFF, 0x80);
    lt8619c_write_reg(0xB2, 0x50);
    lt8619c_write_reg(0xA3, 0x77);

    while(loop_cnt < 5)
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
    lt8619c_write_reg(0x2C, lt8619c_read_reg(0x2c) | 0x30);  //RGD_CLK_STABLE_OPT[1:0]
    lt8619c_write_reg(0xFF,0x60);
    lt8619c_write_reg(0x04,0xF2);
    lt8619c_write_reg(0x83,0x3F);
    lt8619c_write_reg(0x80,0x08);  //use xtal_clk as sys_clk.
#ifdef DDR_CLK
    lt8619c_write_reg(0xA4,0x14);//0x10:SDR clk,0x14: DDR clk
#else
    lt8619c_write_reg(0xA4,0x10);//0x10:SDR clk,0x14: DDR clk
#endif

#if 0
    LT8619C_Rx_Eq_Mode_Config();// RX EQ adjust
    LT8619C_RX_EQ_Setting(); 
#endif

    NRF_LOG_INFO("LT8619C OUTPUT_MODE: \n");

    if(LT8619C_OUTPUTMODE == OUTPUT_LVDS_2_PORT)
    {
        NRF_LOG_INFO("LT8619C set to OUTPUT_LVDS_2_PORT\n");
        lt8619c_write_reg(0xFF,0x60);
        lt8619c_write_reg(0x06,0xE7);
        lt8619c_write_reg(0x59,0x40);//bit7 for VESA/JEIDA mode; bit5 for DE/SYNC mode; bit4 for 6/8bit; bit1 for port swap
        lt8619c_write_reg(0xA0,0x58);
        lt8619c_write_reg(0xA4,0x01);
        lt8619c_write_reg(0xA8,0x00);
        lt8619c_write_reg(0xBA,0x18);
        lt8619c_write_reg(0xC0,0x18);
        lt8619c_write_reg(0xB0,0x66);
        lt8619c_write_reg(0xB1,0x66);
        lt8619c_write_reg(0xB2,0x66);
        lt8619c_write_reg(0xB3,0x66);
        lt8619c_write_reg(0xB4,0x66);
        lt8619c_write_reg(0xB5,0x41);
        lt8619c_write_reg(0xB6,0x41);
        lt8619c_write_reg(0xB7,0x41);
        lt8619c_write_reg(0xB8,0x4C);
        lt8619c_write_reg(0xB9,0x41);
        lt8619c_write_reg(0xBB,0x41);
        lt8619c_write_reg(0xBC,0x41);
        lt8619c_write_reg(0xBD,0x41);
        lt8619c_write_reg(0xBE,0x4C);
        lt8619c_write_reg(0xBF,0x41);
        lt8619c_write_reg(0xA0,0x18);
        lt8619c_write_reg(0xA1,0xB0);
        lt8619c_write_reg(0xA2,0x10);
        lt8619c_write_reg(0x5C,0x01);//bit0=0:single port  bit0=1:dual port
    }

    
    lt8619c_write_reg(0xFF,0x60);
    lt8619c_write_reg(0x0E,0xFD);	
    lt8619c_write_reg(0x0E,0xFF);
    lt8619c_write_reg(0x0D,0xFC);	
    lt8619c_write_reg(0x0D,0xFF);
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
    
    lt8619c_write_reg(0xFF,0x80);
    lt8619c_write_reg(0x5D,0xC9);
    lt8619c_write_reg(0x08,0x80);
}


void lt8619c_i2c_init(void)
{
    ret_code_t err_code;
    const nrf_drv_twi_config_t config = 
    {
        .scl                = LT8619C_I2C_SCL_PIN_NUM,
        .sda                = LT8619C_I2C_SDA_PIN_NUM,
        .frequency          = NRF_DRV_TWI_FREQ_100K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .clear_bus_init     = false
    };
    
    err_code = nrf_drv_twi_init(&m_twi, &config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_twi_enable(&m_twi);
}

void lt8619c_write_reg(const uint8_t reg_addr, const uint8_t data)
{
    ret_code_t err_code;

    err_code = nrf_drv_twi_tx(&m_twi, LT8619C_I2C_ADDR, &reg_addr, 1, false);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_twi_tx(&m_twi, LT8619C_I2C_ADDR, &data, 1, false);
    APP_ERROR_CHECK(err_code);
}

/* NEED DEBUG */
void lt8619c_write_reg_n(const uint8_t reg_addr, const uint8_t *pdata, uint16_t length)
{
    ret_code_t err_code;

    err_code = nrf_drv_twi_tx(&m_twi, LT8619C_I2C_ADDR, &reg_addr, 1, false);
    APP_ERROR_CHECK(err_code);

    uint8_t group = length / 255;
    uint8_t extra = length % 255;
    
    for(int i = 0; i < group; i++)
    {
        err_code = nrf_drv_twi_tx(&m_twi, LT8619C_I2C_ADDR, pdata + i * 255, 255, false);
        APP_ERROR_CHECK(err_code);
    }

    err_code = nrf_drv_twi_tx(&m_twi, LT8619C_I2C_ADDR, pdata + group * 255, extra, false);
    APP_ERROR_CHECK(err_code);
}

uint8_t lt8619c_read_reg(const uint8_t reg_addr)
{
    ret_code_t err_code;
    uint8_t data;
    err_code = nrf_drv_twi_tx(&m_twi, LT8619C_I2C_ADDR, &reg_addr, 1, false);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_twi_rx(&m_twi, LT8619C_I2C_ADDR, &data, 1);
    APP_ERROR_CHECK(err_code);

    return data;
}

void lt8619c_init(void)
{
    lt8619c_i2c_init();
    
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




