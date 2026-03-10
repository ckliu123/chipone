#include "cts_hal.h"

#include <string.h>

#include "thp/thp_dev_itf.h"
#include "cts_utils.h"
#include "cts_spi.h"
#include "cts_drw.h"

#pragma pack(push, 1)
typedef struct cts_drw_head {
	uint8_t		rwcmd;
	uint8_t		addr[3];
	uint8_t		len[3];
	uint8_t		crc[2];
	uint8_t		wait[4];
} CTS_DRW_HEAD;

typedef struct cts_drw_tail {
	uint8_t		crc[2];
	uint8_t		wait[1];
	uint8_t		ack[2];
} CTS_DRW_TAIL;
#pragma pack(pop)

int cts_enter_drw_mode(void)
{
	uint8_t magic[] = { 0xCC, 0x33, 0x55, 0x5A };
	int ret = -1;
	THP_LOGI("Enter");

	ret = cts_spi_sync_send(magic, sizeof(magic));
	if (ret < 0) {
		THP_LOGE("Enter Drw Mode failed");
		return -1;
	}

	THP_LOGI("Exit");
	return 0;
}

int cts_drw_read_raw(uint32_t addr, uint8_t *rbuf, size_t rlen)
{
	int ret = -1;
	CTS_DRW_HEAD head;
	uint16_t crc16_calc;
	uint16_t crc16_recv;

	uint8_t rx_buf[SPI_BUF_SIZ];
	
	MEMSET(&head, 0, sizeof(CTS_DRW_HEAD));
	head.rwcmd = DRW_SPI_READ;
	cts_put_unaligned_be24(head.addr, addr);
	cts_put_unaligned_be24(head.len, rlen);
	crc16_calc = cts_crc16(&head.rwcmd, offsetof(CTS_DRW_HEAD, crc));
	cts_put_unaligned_be16(head.crc, (uint16_t)~crc16_calc);

	ret = cts_spi_sync_raw((uint8_t *)&head, sizeof(head), rx_buf, rlen + 5);//sizeof(tail));
	if (ret < 0) {
		THP_LOGE("Drw read buf failed");
		return -1;
	}
	crc16_calc = cts_crc16(rx_buf, rlen);
	crc16_recv = ~cts_get_unaligned_be16(rx_buf + rlen);
	if (crc16_calc != crc16_recv) {
		THP_LOGE("crc error: calc %#06x != %#06x recv", crc16_calc, crc16_recv);
		cts_dump_spi_tx((uint8_t *)&head, sizeof(head) > CTS_ERR_DUMP_MAX_SIZ ? CTS_ERR_DUMP_MAX_SIZ : sizeof(head));
		cts_dump_spi_rx(rx_buf, (rlen + 5) > CTS_ERR_DUMP_MAX_SIZ ? CTS_ERR_DUMP_MAX_SIZ : (rlen + 5));
		return -1;
	}
	MEMCPY(rbuf, rx_buf, rlen);

	THP_LOGD("Exit");
	return 0;
}

int cts_drw_write_raw(uint32_t addr, uint8_t *wbuf, size_t wlen)
{
	int ret = -1;
	CTS_DRW_HEAD head;
	uint16_t crc16_calc;

	uint8_t tx_buf[SPI_BUF_SIZ];
	uint8_t rx_buf[SPI_BUF_SIZ];

	head.rwcmd = DRW_SPI_WRITE;
	cts_put_unaligned_be24(head.addr, addr);
	cts_put_unaligned_be24(head.len, wlen);
	crc16_calc = cts_crc16((uint8_t *)&head, offsetof(CTS_DRW_HEAD, crc));
	cts_put_unaligned_be16(head.crc, (uint16_t)~crc16_calc);

	THP_LOGD("adrr=%06x, wlen=%d", addr, wlen);
	MEMSET(tx_buf, 0, sizeof(head) + wlen + sizeof(uint16_t) + 3);
	MEMCPY(tx_buf, &head, sizeof(head));
	MEMCPY(tx_buf + sizeof(head), wbuf, wlen);
	crc16_calc = cts_crc16(wbuf, wlen);
	cts_put_unaligned_be16(tx_buf + sizeof(head) + wlen, (uint16_t)~crc16_calc);

	ret = cts_spi_sync_raw(tx_buf, sizeof(head) + wlen + sizeof(uint16_t) + 3, rx_buf, 3);//sizeof(tail));
	if (ret < 0) {
		THP_LOGE("Drw read buf failed");
		return -1;
	}

	THP_LOGD("Exit");
	return ret;
}

int cts_drw_read_u8(uint32_t addr, uint8_t *rval)
{
	return cts_drw_read_raw(addr, rval, sizeof(uint8_t));
}

int cts_drw_read_u16(uint32_t addr, uint16_t *rval)
{
	return cts_drw_read_raw(addr, (uint8_t *)rval, sizeof(uint16_t));
}

int cts_drw_read_u32(uint32_t addr, uint32_t *rval)
{
	return cts_drw_read_raw(addr, (uint8_t *)rval, sizeof(uint32_t));
}

int cts_drw_write_u8(uint32_t addr, uint8_t wval)
{
	return cts_drw_write_raw(addr, &wval, sizeof(uint8_t));
}

int cts_drw_write_u16(uint32_t addr, uint16_t wval)
{
	return cts_drw_write_raw(addr, (uint8_t *)&wval, sizeof(uint16_t));
}

int cts_drw_write_u32(uint32_t addr, uint32_t wval)
{
	return cts_drw_write_raw(addr, (uint8_t *)&wval, sizeof(uint32_t));
}
