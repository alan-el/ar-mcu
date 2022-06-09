#ifndef ECX335SN_H
#define ECX335SN_H

/* OLED A Chip select pin */
#define SPI_OLED_A_SS_PIN     (8)
/* OLED B Chip select pin */
#define SPI_OLED_B_SS_PIN     (4)

#define SPI_MISO_PIN    (5)
#define SPI_MOSI_PIN    (6)
#define SPI_SCK_PIN     (7)

#define OLED_RESET_PIN   (11)

#define ECX335SN_BIT_ORDER NRF_DRV_SPI_BIT_ORDER_LSB_FIRST

#define FRAME_RATE          (50)
#define FRAME_PERIOD_MS     (1000 / FRAME_RATE)
#define FRAME_PERIOD_US     (1000000 / FRAME_RATE)

void ecx335sn_init(void);


#endif 

