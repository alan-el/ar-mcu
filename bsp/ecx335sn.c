#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "ecx335sn.h"

#define SPI_INSTANCE  2 /**< SPI instance index. */

static const nrf_drv_spi_t m_spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */


void ecx335sn_power_on_serial_setting(void);
void ecx335sn_write_reg(uint8_t reg_addr, uint8_t val);

void ecx335sn_reset_io_init(void)
{
    nrf_gpio_cfg_output(LCD_RESET_PIN);
}

void ecx335sn_power_on(void)
{
    nrf_gpio_pin_write(LCD_RESET_PIN, 0);
    nrf_delay_ms(4);
    nrf_gpio_pin_write(LCD_RESET_PIN, 1);
    
    ecx335sn_power_on_serial_setting();
}

/* TODO */
void ecx335sn_power_off(void)
{
    
}

void ecx335sn_power_on_serial_setting(void)
{
    /* serial setting 1 */
    ecx335sn_write_reg(0x02, 0x40);
    ecx335sn_write_reg(0x03, 0xA0);
    ecx335sn_write_reg(0x04, 0x5F);
    ecx335sn_write_reg(0x6D, 0x04);
    ecx335sn_write_reg(0x6F, 0x03);
    ecx335sn_write_reg(0x71, 0x4E);
    ecx335sn_write_reg(0x72, 0x4E);
    
    /* serial setting 2: PS0 off */
    ecx335sn_write_reg(0x00, 0x0F);
    
    /* serial setting 3: PS1 off */
    ecx335sn_write_reg(0x01, 0x01);
    
    /** Datasheet said perform setting 4
     *  at an interval of "> 4XVD" 
     */
    nrf_delay_ms(40);
    
    /* serial setting 4 */
    ecx335sn_write_reg(0x04, 0x3F);
    
    /** Datasheet said perform setting 5
     *  at an interval of "> 1XVD" 
     */
    nrf_delay_ms(10);
    
    /* serial setting 5 */
    ecx335sn_write_reg(0x71, 0x46);
    ecx335sn_write_reg(0x72, 0x46);
    
    /** Datasheet said perform setting 6
     *  at an interval of "> 1XVD" 
     */
    nrf_delay_ms(10);
    
    /* serial setting 6 */
    ecx335sn_write_reg(0x6D, 0x00);
    ecx335sn_write_reg(0x6F, 0x00);
    ecx335sn_write_reg(0x71, 0x00);
    ecx335sn_write_reg(0x72, 0x00);
    
    /** Datasheet said perform setting 7
     *  within V-blanking period after 1V
     *  period from serial setting
     */
     nrf_delay_ms(17);
     
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
    
    nrf_delay_ms(1);
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

void ecx335sn_config(void)
{
    ecx335sn_write_reg(0x08, 0x05);
    nrf_delay_ms(4);
    ecx335sn_write_reg(0x01, 0x00);
    nrf_delay_ms(4);
    ecx335sn_write_reg(0x1D, 0xEE);
    nrf_delay_ms(4);
    ecx335sn_write_reg(0x00, 0x4D);
    nrf_delay_ms(4);
    ecx335sn_write_reg(0x00, 0x4F);
    nrf_delay_ms(4);
}

void ecx335sn_init(void)
{
    ecx335sn_spi_init(SPI_LCD_A_SS_PIN);

    ecx335sn_reset_io_init();
    
    ecx335sn_power_on();

    /* DEBUG 
    ecx335sn_enable_reg_read();
    
    uint8_t ret;
    for(int i = 0; i < 0x80; i++)
    {
        ret = ecx335sn_read_reg(i);
        NRF_LOG_INFO("ecx335sn read register[%02x]: %02x", i, ret);
    }*/
}

