#include "cts_hal.h"
#include <string.h>
#include "thp/thp_dev_itf.h"
#include "cts_spi.h"
#include "cts_tcs.h"

static uint32_t _spi_speed = SPI_DEF_SPEED;
uint8_t txbuf[SPI_MAX_SIZ];
uint8_t rxbuf[SPI_MAX_SIZ];

int cts_set_spi_speed(uint32_t hz)
{
	int ret = -1;
	All_LOG("Enter, set spi speed %ld", hz);

	ret = thp_dev_set_spi_speed(hz);
	if (ret < 0) {
		THP_LOGE("Set spi speed failed: %s", strerror(errno));
		return -1;
	}
	_spi_speed = hz;

	return 0;
}

uint32_t cts_get_spi_speed(void)
{
	return _spi_speed;
}

int cts_spi_sync_raw(uint8_t *tbuf, size_t tlen, uint8_t *rbuf, size_t rlen)
{
	int ret = -1;
	unsigned int total_len;

	// THP_LOGD("Enter");
	total_len = tlen + rlen;

	if (!total_len) {
		THP_LOGE("Invalid tlen or rlen");
		return -1;
	} else if (total_len > sizeof(txbuf)) {
		THP_LOGE("Huge buffer");
		return -1;
	}

	mutext_lock();

	MEMSET(txbuf, 0, total_len);
	MEMSET(rxbuf, 0, total_len);

	if (tbuf && tlen) {
		MEMCPY(txbuf, tbuf, tlen);
	}
	// cts_dump_spi_tx(txbuf,total_len);
	ret = thp_dev_spi_sync(txbuf, rxbuf, total_len);
	if (ret < 0) {
		THP_LOGE("sync buf failed");
		
		cts_dump_spi_tx(txbuf, tlen > CTS_ERR_DUMP_MAX_SIZ ? CTS_ERR_DUMP_MAX_SIZ : tlen);
		cts_dump_spi_rx(rxbuf, rlen > CTS_ERR_DUMP_MAX_SIZ ? CTS_ERR_DUMP_MAX_SIZ : rlen);
	
		mutext_unlock();
		return -1;
	}

	if (rbuf && rlen) {
		MEMCPY(rbuf, rxbuf + tlen, rlen);
	}
	// cts_dump_spi_tx(rbuf,total_len);
	mutext_unlock();

	return 0;
}

int cts_spi_sync_send(uint8_t *tbuf, size_t tlen)
{
	return cts_spi_sync_raw(tbuf, tlen, NULL, 0);
}

int cts_spi_sync_recv(uint8_t *rbuf, size_t rlen)
{
	return cts_spi_sync_raw(NULL, 0, rbuf, rlen);
}

int cts_spi_sync_cmd_data(
		uint8_t *cmd, size_t cmd_len,
		uint8_t *data, size_t data_len,
		uint8_t *rbuf, size_t rlen)
{
	int ret = -1;
	unsigned int total_len;

	total_len = cmd_len + data_len + rlen;

	if (!data_len) {
		THP_LOGE("ERROR! use cts_spi_sync_buf instead!");
		return -1;
	}

	if (!total_len) {
		THP_LOGE("Invalid tlen or rlen");
		return -1;
	} else if (total_len > sizeof(txbuf)) {
		THP_LOGE("Huge buffer");
		return -1;
	}
	
	mutext_lock();

	MEMSET(txbuf, 0, total_len);
	MEMSET(rxbuf, 0, total_len);

	if (cmd && cmd_len) {
		MEMCPY(txbuf, cmd, cmd_len);
		MEMCPY(txbuf + cmd_len, data, data_len);
	}

	ret = thp_dev_spi_sync(txbuf, rxbuf, total_len);
	if (ret < 0) {
		THP_LOGE("sync buf failed");
		mutext_unlock();
		return -1;
	}

	if (rbuf && rlen) {
		MEMCPY(rbuf, rxbuf + cmd_len + data_len, rlen);
	}

	mutext_unlock();

	return 0;
}


