#ifndef LT8619C_H
#define LT8619C_H

#include <stdint.h>

#define LT8619C_I2C_SDA_PIN_NUM     (19)
#define LT8619C_I2C_SCL_PIN_NUM     (20)

/* LT8619C settings */

#define LT8619C_I2C_ADDR            (0x64 >> 1)

#define USE_EXTERNAL_HDCPKEY
#define LT8619C_OUTPUTMODE  OUTPUT_LVDS_2_PORT
#define AUDIO_INPUT_MODE    I2S_2CH
#define LT8619C_OUTPUTCOLOR COLOR_RGB

typedef struct
{
    uint32_t pix_clk;
    uint16_t hfp;
    uint16_t hs;
    uint16_t hbp;
    uint16_t hact;
    uint16_t htotal;
    uint16_t vfp;
    uint16_t vs;
    uint16_t vbp;
    uint16_t vact;
    uint16_t vtotal;
    
}video_timing_t;

typedef struct
{
    bool flag_rx_clk_stable;
    bool flag_rx_clk_detected;
    bool flag_rx_pll_locked;
    bool flag_hsync_stable;
    bool input_hdmi_mode;
    uint8_t input_vic;
    uint8_t input_color_space;
    uint8_t input_color_depth;
    uint8_t input_color_imetry;
    uint8_t input_ex_color_imetry;
    uint8_t input_quant_range;
    uint8_t input_pr_factor;
    uint8_t input_videoindex;
    uint32_t clk_freq_val_current;
    uint32_t clk_freq_val_previous;
}lt8619c_rx_status_t;

typedef enum
{
    COLOR_RGB = 0x00,
    COLOR_YCBCR444 = 0x40,
    COLOR_YCBCR422 = 0x20
}input_color_space_t;

typedef enum
{
    NO_DATA = 0x00,
    ITU_601 = 0x40,
    ITU_709 = 0x80,
    EXTENDED_COLORIETRY = 0xc0
}input_color_ietry_t;

typedef enum
{
    DEFAULT_RANGE = 0x00,
    LIMIT_RANGE = 0x04,
    FULL_RANGE  = 0x08,
    RESERVED_VAL=0xc0
}input_quant_range_t;

typedef enum
{
    xvYCC601 = 0x00,
    xvYCC709 = 0x10
    //FUTURE_COLORIETRY
}input_ex_color_ietry_t;

enum LT8619C_OUTPUTMODE_ENUM
{
    OUTPUT_RGB888 = 0,
    OUTPUT_RGB666,
    OUTPUT_RGB565,
    OUTPUT_YCBCR444,
    OUTPUT_YCBCR422_16BIT,
    OUTPUT_YCBCR422_20BIT,
    OUTPUT_YCBCR422_24BIT,
    OUTPUT_BT656_8BIT,
    OUTPUT_BT656_10BIT,
    OUTPUT_BT656_12BIT,
    OUTPUT_BT1120_16BIT,
    OUTPUT_BT1120_20BIT,
    OUTPUT_BT1120_24BIT,
    OUTPUT_LVDS_2_PORT,
    OUTPUT_LVDS_1_PORT
};

enum LT8619C_AUDIOINPUT_MODE
{
    I2S_2CH=0,
	SPDIF
};


typedef  enum
{
	LOW = 0,
	HIGH = !LOW
}Pin_Status;

void lt8619c_init(void);
void lt8619c_main_loop(void);

#endif


