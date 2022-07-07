#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "ecx335sn.h"

#define SPI_INSTANCE_ID  2 /**< SPI instance index. */

static const nrf_drv_spi_t m_spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE_ID);  /**< SPI instance. */

uint8_t regs_val[0x81];

void ecx335sn_power_on_serial_setting(void);
void ecx335sn_enable_reg_read(void);
void ecx335sn_write_reg(uint8_t reg_addr, uint8_t val);
uint8_t ecx335sn_read_reg(uint8_t reg_addr);

void ecx335sn_reset_io_init(void)
{
    nrf_gpio_cfg_output(OLED_RESET_PIN);
}

void ecx335sn_power_on(void)
{
    nrf_gpio_pin_write(OLED_RESET_PIN, 0);
    nrf_delay_ms(100);
    nrf_gpio_pin_write(OLED_RESET_PIN, 1);
    nrf_delay_ms(100);

    /* DEBUG, registers value dump 
    ecx335sn_enable_reg_read();
    for(int i = 0; i < 0x81; i++)
    {
        regs_val[i] = ecx335sn_read_reg(i);
    }*/
}

/* TODO */
void ecx335sn_power_off(void)
{
    
}

void ecx335sn_power_on_serial_setting(void)
{
    /* serial setting 1 - LVDS */
    ecx335sn_write_reg(0x02, 0x40);
    ecx335sn_write_reg(0x03, 0xA0);
    ecx335sn_write_reg(0x04, 0x5F);
    /* Luminance and white chromaticity preset mode selection = 0 */
    ecx335sn_write_reg(0x05, 0x80);
    /* Luminance and white chromaticity preset mode selection = 3 
    ecx335sn_write_reg(0x05, 0x83);*/
    /* Luminance and white chromaticity preset mode selection = 4 
    ecx335sn_write_reg(0x05, 0x84);*/
    ecx335sn_write_reg(0x07, 0x40);
    /* OTPCALDAC_REGDIS = 1 
    ecx335sn_write_reg(0x08, 0x04);*/
    ecx335sn_write_reg(0x1C, 0x0A);
    /* Luminance adjustment = 93 */
    ecx335sn_write_reg(0x1D, 0x5D);
    /* Luminance adjustment = 255 
    ecx335sn_write_reg(0x1D, 0xFF);*/
    ecx335sn_write_reg(0x1E, 0x22);
    ecx335sn_write_reg(0x26, 0x00);
    ecx335sn_write_reg(0x27, 0x00);
    ecx335sn_write_reg(0x2B, 0x00);
    ecx335sn_write_reg(0x2C, 0x00);
    ecx335sn_write_reg(0x2E, 0x19);
    ecx335sn_write_reg(0x2F, 0x1A);
    ecx335sn_write_reg(0x33, 0xFF);
    ecx335sn_write_reg(0x38, 0x05);
    ecx335sn_write_reg(0x3E, 0xE6);
    ecx335sn_write_reg(0x40, 0x5D);
    ecx335sn_write_reg(0x5B, 0x00);
    ecx335sn_write_reg(0x5D, 0x00);
    ecx335sn_write_reg(0x5E, 0x00);
    ecx335sn_write_reg(0x60, 0x00);
    ecx335sn_write_reg(0x61, 0x00);
    ecx335sn_write_reg(0x63, 0x00);
    ecx335sn_write_reg(0x64, 0x00);
    ecx335sn_write_reg(0x6D, 0x04);
    ecx335sn_write_reg(0x6F, 0x03);
    ecx335sn_write_reg(0x71, 0x4E);
    ecx335sn_write_reg(0x72, 0x4E);
    ecx335sn_write_reg(0x74, 0x41);
    ecx335sn_write_reg(0x75, 0x27);
    ecx335sn_write_reg(0x76, 0x85);
    ecx335sn_write_reg(0x77, 0x30);
    ecx335sn_write_reg(0x78, 0x06);
    ecx335sn_write_reg(0x79, 0x36);
    ecx335sn_write_reg(0x7A, 0x80);
    ecx335sn_write_reg(0x7B, 0x25);
    ecx335sn_write_reg(0x7C, 0x47);
    ecx335sn_write_reg(0x7D, 0x61);
    ecx335sn_write_reg(0x80, 0x01);
    ecx335sn_write_reg(0x81, 0x81);

    /* serial setting 1 - mini-LVDS 
    ecx335sn_write_reg(0x02, 0x40);
    ecx335sn_write_reg(0x03, 0xA0);
    ecx335sn_write_reg(0x04, 0x5F);
    ecx335sn_write_reg(0x05, 0x80);
    ecx335sn_write_reg(0x07, 0x40);
    ecx335sn_write_reg(0x1C, 0x0A);
    ecx335sn_write_reg(0x1D, 0x5D);
    ecx335sn_write_reg(0x1E, 0x22);
    ecx335sn_write_reg(0x20, 0x4E);
    ecx335sn_write_reg(0x22, 0x0E);
    ecx335sn_write_reg(0x26, 0x00);
    ecx335sn_write_reg(0x27, 0x00);
    ecx335sn_write_reg(0x29, 0x46);
    ecx335sn_write_reg(0x2A, 0x16);
    ecx335sn_write_reg(0x2B, 0x00);
    ecx335sn_write_reg(0x2C, 0x00);
    ecx335sn_write_reg(0x2E, 0x19);
    ecx335sn_write_reg(0x2F, 0x1A);
    ecx335sn_write_reg(0x33, 0xFF);
    ecx335sn_write_reg(0x38, 0x05);
    ecx335sn_write_reg(0x3E, 0xE6);
    ecx335sn_write_reg(0x40, 0x5D);
    ecx335sn_write_reg(0x5B, 0x00);
    ecx335sn_write_reg(0x5D, 0x00);
    ecx335sn_write_reg(0x5E, 0x00);
    ecx335sn_write_reg(0x60, 0x00);
    ecx335sn_write_reg(0x61, 0x00);
    ecx335sn_write_reg(0x63, 0x00);
    ecx335sn_write_reg(0x64, 0x00);
    ecx335sn_write_reg(0x6D, 0x04);
    ecx335sn_write_reg(0x6F, 0x03);
    ecx335sn_write_reg(0x71, 0x4E);
    ecx335sn_write_reg(0x72, 0x4E);
    ecx335sn_write_reg(0x74, 0x41);
    ecx335sn_write_reg(0x75, 0x27);
    ecx335sn_write_reg(0x76, 0x85);
    ecx335sn_write_reg(0x77, 0x30);
    ecx335sn_write_reg(0x78, 0x06);
    ecx335sn_write_reg(0x79, 0x36);
    ecx335sn_write_reg(0x7A, 0x80);
    ecx335sn_write_reg(0x7B, 0x25);
    ecx335sn_write_reg(0x7C, 0x47);
    ecx335sn_write_reg(0x7D, 0x61);
    ecx335sn_write_reg(0x80, 0x01);
    ecx335sn_write_reg(0x81, 0x81);*/

    
    /* serial setting 2: PS0 off */
    ecx335sn_write_reg(0x00, 0x0F);

    /** Datasheet said perform setting 3
     *  at an interval of "> 200us" 
     */
    nrf_delay_us(250);
    
    /* serial setting 3: PS1 off */
    ecx335sn_write_reg(0x01, 0x01);
    
    /** Datasheet said perform setting 4
     *  at an interval of "> 4XVD" 
     *  XVD means the time of transmitting
     *  a frame.
     *  XVD = frame period
     */
    nrf_delay_ms(FRAME_PERIOD_MS * 4 + 1);
    
    /* serial setting 4 */
    ecx335sn_write_reg(0x04, 0x3F);
    
    /** Datasheet said perform setting 5
     *  at an interval of "> 1XVD" 
     */
    nrf_delay_ms(FRAME_PERIOD_MS + 1);
    
    /* serial setting 5 */
    ecx335sn_write_reg(0x71, 0x46);
    ecx335sn_write_reg(0x72, 0x46);
    
    /** Datasheet said perform setting 6
     *  at an interval of "> 1XVD" 
     */
    nrf_delay_ms(FRAME_PERIOD_MS + 1);
    
    /* serial setting 6 */
    ecx335sn_write_reg(0x6D, 0x00);
    ecx335sn_write_reg(0x6F, 0x00);
    ecx335sn_write_reg(0x71, 0x00);
    ecx335sn_write_reg(0x72, 0x00);
    
    /** Datasheet said perform setting 7
     *  within V-blanking period after 1V
     *  period from serial setting 6
     *  V-blanking = 800us in case of 50 Hz
     */
    nrf_delay_us(FRAME_PERIOD_US);
    nrf_delay_us(100);
    /* serial setting 7 */
    ecx335sn_write_reg(0x03, 0x20);
}

void ecx335sn_spi_init(uint8_t slave_cs_num)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    
    spi_config.ss_pin   = slave_cs_num;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
    
    spi_config.frequency = NRF_DRV_SPI_FREQ_8M;
    spi_config.bit_order = ECX335SN_BIT_ORDER;

    NRF_LOG_INFO("SPI init.");

    APP_ERROR_CHECK(nrf_drv_spi_init(&m_spi, &spi_config, NULL, NULL));

}

void ecx335sn_spi_uninit(void)
{
    nrf_drv_spi_uninit(&m_spi);
}

void ecx335sn_write_reg(uint8_t reg_addr, uint8_t val)
{
    uint8_t buf[2];
    buf[0] = reg_addr;
    buf[1] = val;

    APP_ERROR_CHECK(nrf_drv_spi_transfer(&m_spi, buf, 2, NULL, 0));
    
}

void ecx335sn_enable_reg_read(void)
{
    uint8_t buf[2] = {0x80, 0x01};

    APP_ERROR_CHECK(nrf_drv_spi_transfer(&m_spi, buf, 2, NULL, 0));
}

void ecx335sn_disable_reg_read(void)
{
    uint8_t buf[2] = {0x80, 0x00};

    APP_ERROR_CHECK(nrf_drv_spi_transfer(&m_spi, buf, 2, NULL, 0));
}


uint8_t ecx335sn_read_reg(uint8_t reg_addr)
{
    uint8_t buf[2];
    buf[0] = 0x81;
    buf[1] = reg_addr;

    APP_ERROR_CHECK(nrf_drv_spi_transfer(&m_spi, buf, 2, NULL, 0));

    uint8_t val[2];
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&m_spi, buf, 1, val, 2));

    return val[1];
    
}

void ecx335sn_init(void)
{
    ecx335sn_reset_io_init();
    ecx335sn_power_on();
    
    ecx335sn_spi_init(SPI_OLED_A_SS_PIN);
    ecx335sn_power_on_serial_setting();
    ecx335sn_spi_uninit();

    ecx335sn_spi_init(SPI_OLED_B_SS_PIN);
    ecx335sn_power_on_serial_setting();
}

