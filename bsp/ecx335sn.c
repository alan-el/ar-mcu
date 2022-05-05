#include "nrf_drv_spi.h"
#include "nrf_error.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "ecx335sn.h"

#define SPI_INSTANCE  2 /**< SPI instance index. */

static const nrf_drv_spi_t m_spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

void ecx335sn_spi_init(uint8_t slave_cs_num)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    
    spi_config.ss_pin   = slave_cs_num;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
    spi_config.frequency = NRF_DRV_SPI_FREQ_8M;

    NRF_LOG_INFO("SPI init.");

    APP_ERROR_CHECK(nrf_drv_spi_init(&m_spi, &spi_config, NULL, NULL));

}

