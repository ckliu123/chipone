#ifndef CTS_SPI_H
#define CTS_SPI_H

#include "cts_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_BUF_SIZ			(8 * 1024)
#define SPI_EXT_SIZ			(1024)
#define SPI_MAX_SIZ			(SPI_BUF_SIZ + SPI_EXT_SIZ)    
#define SPI_DEF_SPEED		12000000

int cts_set_spi_speed(uint32_t speed_in_hz);
uint32_t cts_get_spi_speed(void);

int cts_spi_sync_raw(uint8_t *tbuf, size_t tlen, uint8_t *rbuf, size_t rlen);
int cts_spi_sync_send(uint8_t *tbuf, size_t tlen);
int cts_spi_sync_recv(uint8_t *rbuf, size_t rlen);

int cts_spi_sync_cmd_data(
		uint8_t *cmd, size_t cmd_len,
		uint8_t *data, size_t data_len,
		uint8_t *rbuf, size_t rlen);

#ifdef __cplusplus
}
#endif

#endif /* CTS_SPI_H */

