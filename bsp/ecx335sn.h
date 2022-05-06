#ifndef ECX335SN_H
#define ECX335SN_H

/* LCD A Chip select pin */
#define SPI_LCD_A_SS_PIN     (8)
/* LCD B Chip select pin */
#define SPI_LCD_B_SS_PIN     (4)

#define SPI_MISO_PIN    (5)
#define SPI_MOSI_PIN    (6)
#define SPI_SCK_PIN     (7)

#define LCD_RESET_PIN   (11)

#define ECX335SN_BIT_ORDER NRF_DRV_SPI_BIT_ORDER_LSB_FIRST

void ecx335sn_init(void);


#endif 

