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
}
Pin_Status;

void lt8619c_init(void);

#endif


