#include "cts_hal.h"
#include <string.h>
#include <malloc.h>
#include "thp/thp_dev_itf.h"
#include "cts_utils.h"
#include "cts_spi.h"
#include "cts_drw.h"
#include "cts_core.h"

#include "cts_tcs.h"

extern uint8_t txbuf[SPI_MAX_SIZ];
extern uint8_t rxbuf[SPI_MAX_SIZ];
extern uint16_t s_scan_state;

int cts_tcs_read_pack(uint8_t *tx, enum TcsCmdIndex index, uint16_t rdatalen)
{
	tcs_tx_head *txhdr = (tcs_tx_head *)tx;
	int packlen = 0;
	uint16_t crc16;

	txhdr->addr = TCS_SPI_READ;
	txhdr->cmd = 
		(TcsCmdValue[index][0] << 15) |
		(TcsCmdValue[index][3] << 14) |
		(TcsCmdValue[index][1] <<  8) |
		(TcsCmdValue[index][2] <<  0);
	txhdr->datlen = rdatalen;
	crc16 = cts_crc16((const uint8_t *)txhdr, offsetof(tcs_tx_head, crc16));
	txhdr->crc16 = crc16;
	packlen += sizeof(tcs_tx_head);

	return packlen;
}

int cts_tcs_write_pack(uint8_t *tx, enum TcsCmdIndex index,
		uint8_t *wdata, uint16_t wdatalen)
{
	tcs_tx_head *txhdr = (tcs_tx_head *)tx;
	int packlen = 0;
	uint16_t crc16;

	txhdr->addr = TCS_SPI_WRITE;
	txhdr->cmd = 
		(TcsCmdValue[index][0] << 15) |
		(TcsCmdValue[index][4] << 13) |
		(TcsCmdValue[index][1] <<  8) |
		(TcsCmdValue[index][2] <<  0);
	txhdr->datlen = wdatalen;
	crc16 = cts_crc16((const uint8_t *)txhdr, offsetof(tcs_tx_head, crc16));
	txhdr->crc16 = crc16;
	packlen += sizeof(tcs_tx_head);

	if (wdatalen > 0) {
		MEMCPY(tx + sizeof(tcs_tx_head), wdata, wdatalen);
		crc16 = cts_crc16(wdata, wdatalen);
		*(tx + sizeof(tcs_tx_head) + wdatalen) = ((crc16 >> 0) & 0xFF);
		*(tx + sizeof(tcs_tx_head) + wdatalen + 1) = ((crc16 >> 8) & 0xFF);
		packlen += wdatalen + sizeof(crc16);
	}

	return packlen;
}

static int cts_tcs_trans_check(uint8_t *tx, size_t txlen, uint8_t *rx, size_t rxlen)
{
	uint8_t err_code;
	uint16_t cmd_recv, cmd_send, crc16_recv, crc16_calc;

	err_code = rx[rxlen - 5];
	if (err_code) {
		THP_LOGE("err_code error: %d", err_code);
		return -1;
	}
	
	cmd_send = cts_get_unaligned_le16(tx + 1);
	cmd_recv = cts_get_unaligned_le16(rx + rxlen - 4);
	if (cmd_send != cmd_recv) {
		THP_LOGE("cmd_check error, send %04x != %04x recv", cmd_send, cmd_recv);
		return -1;
	}

	crc16_calc = cts_crc16(rx, rxlen - sizeof(uint16_t));
	crc16_recv = cts_get_unaligned_le16(rx + rxlen - 2);
	if (crc16_calc != crc16_recv) {
		THP_LOGE("crc_check error, calc %04x != %04x recv", crc16_calc, crc16_recv);
		return -1;
	}

	return 0;
}

static int cts_tcs_spi_chichu(uint8_t *tx, size_t txlen, uint8_t *rx, size_t rxlen)
{
	struct thp_ioctl_spi_xfer_data xfer[2];
	struct thp_ioctl_spi_msg_package msg;
	int ret = -1;
	int i;
	uint8_t tx_tmp[rxlen];
	size_t new_txlen, new_rxlen;

#ifdef CTS_BYTES_ALIGN
	size_t align = 4;
	new_txlen = ((txlen + (align - 1))/align)*align;
	new_rxlen = ((rxlen + (align - 1))/align)*align;
#else
	new_txlen = txlen;
	new_rxlen = rxlen;
#endif
	THP_LOGD("txlen: %d, new_txlen: %d", txlen, new_txlen);
	THP_LOGD("rxlen: %d, new_rxlen: %d", rxlen, new_rxlen);

	for (i = 0; i < 3; i++) {
		MEMSET(xfer, 0, sizeof(xfer));
		MEMSET(&msg, 0, sizeof(msg));

		xfer[0].tx = (char *)tx;
		xfer[0].rx = (char *)rx;
		xfer[0].len = new_txlen;
		xfer[0].delay_usecs = 300;
		xfer[0].cs_change = 1;

		MEMSET(tx_tmp, 0x1F, rxlen);

		xfer[1].tx = (char *)tx_tmp;
		xfer[1].rx = (char *)rx;
		xfer[1].len = new_rxlen;
		xfer[1].delay_usecs = 300;
		xfer[1].cs_change = 1;

		msg.speed_hz = cts_get_spi_speed();
		msg.xfer_num = 2;
		msg.xfer_data= xfer;

		ret = thp_dev_multiple_spi_xfer_sync(&msg);
		if (ret < 0) {
			THP_LOGE("Spi xtrans failed: %s", strerror(errno));
			goto err_retry;
		}

		ret = cts_tcs_trans_check(tx, txlen, rx, rxlen);
err_retry:
		if (!ret)
			break;
		else {
			THP_LOGE("Xtrans need retry: %d, delay: %dms", i + 1, 10);
			cts_mdelay(10);
		}
	}

	if (ret) {
		cts_dump_spi_tx(tx, txlen);
		cts_dump_spi_rx(rx, rxlen);
	}
	return ret;
}

int cts_tcs_spi_xing(uint8_t *tx, uint8_t *rx, size_t total_len)
{
	int ret = -1;

	mutext_lock();

	ret = thp_dev_spi_sync(tx, rx, total_len);
	if (ret < 0) {
		THP_LOGE("Spi sync failed: %s", strerror(errno));
		mutext_unlock();
		return -1;
	}

	mutext_unlock();
	return 0;
}

static int cts_find_tcs_index(uint8_t classid, uint8_t cmdid)
{
	uint32_t i;

	for (i = 0; i < sizeof(TcsCmdValue)/sizeof(TcsCmdValue[0]); i++) {
		if ((TcsCmdValue[i][1] == classid) && (TcsCmdValue[i][2] == cmdid)) {
			return i;
		}
	}

	return -1;
}

int cts_tcs_read_spi_for_tool(uint8_t classID, uint8_t cmdID, uint8_t *buf, size_t len)
{
	int txlen;
	int rxlen = len + sizeof(tcs_rx_tail);
	int index;
	int ret;

	index = cts_find_tcs_index(classID, cmdID);
	if (index < 0) {
		THP_LOGE("Not found tcs classid or cmdid!!");
		return -1;
	}

	txlen = cts_tcs_read_pack(txbuf, index, len);
	ret = cts_tcs_spi_chichu(txbuf, txlen, rxbuf, rxlen);
	if (ret == 0) {
		memcpy(buf, rxbuf, len);
	}

	return ret;
}
int cts_tcs_write_spi_for_tool(uint8_t classID, uint8_t cmdID, uint8_t *buf, size_t len)
{
	int txlen;
	int rxlen = sizeof(tcs_rx_tail);
	int index;

	index = cts_find_tcs_index(classID, cmdID);
	if (index < 0) {
		THP_LOGE("Not found tcs classid or cmdid!!");
		return -1;
	}

	txlen = cts_tcs_write_pack(txbuf, index, buf, len);
	return cts_tcs_spi_chichu(txbuf, txlen, rxbuf, rxlen);
}


int cts_tcs_read_buff(enum TcsCmdIndex cmdIdx, uint8_t *rdata, size_t rdatalen)
{
	int txlen;
	int rxlen = rdatalen + sizeof(tcs_rx_tail);
	int ret;

	mutext_lock();

	MEMSET(txbuf, 0, sizeof(txbuf));
	MEMSET(rxbuf, 0, sizeof(rxbuf));
	txlen = cts_tcs_read_pack(txbuf, cmdIdx, rdatalen);
	ret = cts_tcs_spi_chichu(txbuf, txlen, rxbuf, rxlen);
	if (!ret)
	MEMCPY(rdata, rxbuf, rdatalen);
	//cts_dump_spi_tx_rx(txbuf, rxbuf, txlen, rxlen);

	mutext_unlock();
	return ret;
}

int cts_tcs_write_buff(enum TcsCmdIndex cmdIdx, uint8_t *wdata, size_t wdatalen)
{
	int txlen;
	int rxlen = sizeof(tcs_rx_tail);
	int ret;

	mutext_lock();

	MEMSET(txbuf, 0, sizeof(txbuf));
	MEMSET(rxbuf, 0, sizeof(rxbuf));
	txlen = cts_tcs_write_pack(txbuf, cmdIdx, wdata, wdatalen);
	ret = cts_tcs_spi_chichu(txbuf, txlen, rxbuf, rxlen);
	//cts_dump_spi_tx_rx(txbuf, rxbuf, txlen, rxlen);

	mutext_unlock();
	return ret;
}

int cts_tcs_read_attr(enum TcsCmdIndex cmdIdx, uint8_t *rdata, size_t rdatalen)
{
	return cts_tcs_read_buff(cmdIdx, rdata, rdatalen);
}

int cts_tcs_write_attr(enum TcsCmdIndex cmdIdx, uint8_t *wdata, size_t wdatalen)
{
	return cts_tcs_write_buff(cmdIdx, wdata, wdatalen);
}

int cts_tcs_read_u8attr(enum TcsCmdIndex cmdIdx, uint8_t *u8attr)
{
	return cts_tcs_read_attr(cmdIdx, (uint8_t *)u8attr, sizeof(uint8_t));
}

int cts_tcs_read_u16attr(enum TcsCmdIndex cmdIdx, uint16_t *u16attr)
{
	return cts_tcs_read_attr(cmdIdx, (uint8_t *)u16attr, sizeof(uint16_t));
}

int cts_tcs_read_u32attr(enum TcsCmdIndex cmdIdx, uint32_t *u32attr)
{
	return cts_tcs_read_attr(cmdIdx, (uint8_t *)u32attr, sizeof(uint32_t));
}

int cts_tcs_write_u8attr(enum TcsCmdIndex cmdIdx, uint8_t u8attr)
{
	return cts_tcs_write_attr(cmdIdx, (uint8_t *)&u8attr, sizeof(uint8_t));
}

int cts_tcs_write_u16attr(enum TcsCmdIndex cmdIdx, uint16_t u16attr)
{
	return cts_tcs_write_attr(cmdIdx, (uint8_t *)&u16attr, sizeof(uint16_t));
}

int cts_tcs_write_u32attr(enum TcsCmdIndex cmdIdx, uint32_t u32attr)
{
	return cts_tcs_write_attr(cmdIdx, (uint8_t *)&u32attr, sizeof(uint32_t));
}

int cts_tcs_get_fw_ver(uint32_t *fw_ver)
{
	uint8_t buf[4];
	int rc;

	rc = cts_tcs_read_attr(TP_STD_CMD_INFO_FW_VER_RO, buf, sizeof(buf));
	if (!rc) {
		*fw_ver = (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
		return 0;
	}

	return rc;
}

int cts_tcs_get_hw_id(uint32_t *hwid)
{
	uint8_t buf[5];
	int rc;

	rc = cts_tcs_read_attr(TP_STD_CMD_INFO_CHIP_HW_ID_RO, buf, sizeof(buf));
	if (!rc) {
		All_LOG("buf[0]: 0x%x, buf[1]: 0x%x, buf[2]: 0x%x, buf[3]: 0x%x", buf[0], buf[1], buf[2], buf[3]);
		*hwid = cts_get_unaligned_le32(buf);
		return 0;
	}

	return rc;
}

int cts_tcs_get_mst_tx_num(uint8_t * tx_num)
{
	int rc;

	rc = cts_tcs_read_u8attr(TP_STD_CMD_TP_PARA_MASTER_TX_NUM_RO, tx_num);
	if (!rc) {
		THP_LOGI("master tx num: %d", *tx_num);
		return 0;
	}
	return rc;
}

int cts_tcs_get_mst_rx_num(uint8_t * rx_num)
{
	int rc;

	rc = cts_tcs_read_u8attr(TP_STD_CMD_TP_PARA_MASTER_RX_NUM_RO, rx_num);
	if (!rc) {
		THP_LOGI("master rx num: %d", *rx_num);
		return 0;
	}
	return rc;
}

int cts_tcs_get_slv_tx_num(uint8_t * tx_num)
{
	int rc;

	rc = cts_tcs_read_u8attr(TP_STD_CMD_TP_PARA_SLAVE_TX_NUM_RO, tx_num);
	if (!rc) {
		THP_LOGI("slave tx num: %d", *tx_num);
		return 0;
	}
	return rc;
}

int cts_tcs_get_slv_rx_num(uint8_t * rx_num)
{
	int rc;

	rc = cts_tcs_read_u8attr(TP_STD_CMD_TP_PARA_SLAVE_RX_NUM_RO, rx_num);
	if (!rc) {
		THP_LOGI("slave rx num: %d", *rx_num);
		return 0;
	}
	return rc;
}

int cts_tcs_get_tx_tr_order(uint8_t *order)
{
	uint8_t buf[TR_ORDER_MAX];
	int rc, i;

	rc = cts_tcs_read_attr(TP_STD_CMD_TP_PARA_ALL_TX_TR_ORDER_RO, buf, sizeof(buf));
	if (!rc) {
		for (i = 0; i < TR_ORDER_MAX; i++)
			order[i] = buf[i];

		return 0;
	}

	return rc;
}

int cts_tcs_get_rx_tr_order(uint8_t *order)
{
	uint8_t buf[TR_ORDER_MAX];
	int rc, i;

	rc = cts_tcs_read_attr(TP_STD_CMD_TP_PARA_ALL_RX_TR_ORDER_RO, buf, sizeof(buf));
	if (!rc) {
		for (i = 0; i < TR_ORDER_MAX; i++)
			order[i] = buf[i];

		return 0;
	}

	return rc;
}

int cts_tcs_get_rows_cols(uint8_t *rx_num, uint8_t *tx_num)
{
	uint8_t buf[6];
	int rc;

	rc = cts_tcs_read_attr(TP_STD_CMD_TP_PARA_TOUCH_INFO_RO, buf, sizeof(buf));
	if (!rc) {
		*rx_num = buf[4] & 0xFF;
		*tx_num = buf[5] & 0xFF;
		THP_LOGI("rx_num: %d, tx_num: %d", *rx_num, *tx_num);
		return 0;
	}

	return rc;
}

int cts_tcs_get_scan_freqs(uint8_t *scan_freq_num, uint16_t *scan_freqs)
{
	uint8_t num[1], buf[20];
	int i, j;
	int rc;

	rc = cts_tcs_read_attr(TP_STD_CMD_NUM_HOP_FREQ_RO, num, sizeof(num));
	if (!rc)
		*scan_freq_num = num[0];

	rc = cts_tcs_read_attr(TP_STD_CMD_HOP_FREQ_ClC_NUM_RW, buf, sizeof(buf));
	if (!rc) {
		for (i = 0, j = 0; i < 20; i++) {
			*(scan_freqs + j) = buf[i] + (buf[i + 1] << 8);
			if (*(scan_freqs + j)) {
				i++;
				j++;
			} else {
				break;
			}
		}
		return 0;
	}

	return rc;
}

int cts_tcs_get_stylus_scan_freqs(uint8_t *scan_freq_num, uint16_t *scan_freqs)
{
	uint8_t num[1], buf[20];
	int i, j;
	int rc;

	rc = cts_tcs_read_attr(TP_STD_CMD_NUM_STYLUS_HOP_FREQ_RO, num, sizeof(num));
	if (!rc)
		*scan_freq_num = num[0];

	rc = cts_tcs_read_attr(TP_STD_CMD_STYLUS_HOP_FREQ_ClC_NUM_RO, buf, sizeof(buf));
	if (!rc) {
		for (i = 0, j = 0; i < 20; i++) {
			*(scan_freqs + j) = buf[i] + (buf[i + 1] << 8);
			if (*(scan_freqs + j)) {
				i++;
				j++;
			} else {
				break;
			}
		}
		return 0;
	}

	return rc;
}

int cts_tcs_get_scan_rate(uint8_t *num_scan_rate, uint16_t *scan_rate)
{
	uint8_t num[1], buf[10];
	int i, j;
	int rc;

	rc = cts_tcs_read_attr(TP_STD_CMD_SYS_STS_REPORTRATE_NUM_RO, num, sizeof(num));
	if (!rc)
		*num_scan_rate = num[0];

	rc = cts_tcs_read_attr(TP_STD_CMD_SYS_STS_ALL_REPORTRATE_RO, buf, sizeof(buf));
	if (!rc) {
		for (i = 0, j = 0; i < 10; i++) {
			*(scan_rate + j) = buf[i] + (buf[i + 1] << 8);
			if (*(scan_rate + j)) {
				i++;
				j++;
			} else {
				break;
			}
		}
		return 0;
	}

	return rc;
}

int cts_tcs_set_hw_cap_ready(uint8_t value)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_POWER_ON_READ_FLAG_RW, value);

}

int cts_tcs_set_mnt_touch_threshold(uint16_t threshold)
{
	return cts_tcs_write_u16attr(TP_STD_CMD_LOW_POWER_ENTER_MONITOR_TM_RW, threshold);
}

int cts_tcs_set_mnt_baseline_update_interval(uint16_t interval)
{
	return cts_tcs_write_u16attr(TP_STD_CMD_LOW_POWER_IDLE_REPORT_INTERVAL_RW, interval);
}

int cts_tcs_reset_idle_baseline(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_LOW_POWER_RESET_IDLE_BASE_EN_RW, 1);
}

int cts_tcs_force_enter_mnt(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_LOW_POWER_ENTER_IDLE_MODE_RW, 1);
}

int cts_tcs_force_exit_mnt(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_LOW_POWER_EXIT_IDLE_MODE_RW, 1);
}

int cts_tcs_set_scan_state(THP_AFE_SCAN_STATE_ENUM state)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_SCAN_STATE_RW, state);
}

int cts_tcs_get_int_data_types(uint16_t *int_data_types)
{
	int ret = -1;
	ret = cts_tcs_read_u16attr(TP_STD_CMD_SYS_STS_DATA_CAPTURE_FUNC_MAP_RW, int_data_types);
	return ret;
}

int cts_tcs_set_int_data_types(uint16_t int_data_types)
{
	int ret = -1;
	ret = cts_tcs_write_u16attr(TP_STD_CMD_SYS_STS_DATA_CAPTURE_FUNC_MAP_RW, int_data_types);
	return ret;
}

int cts_tcs_get_int_data_method(uint8_t *int_data_method)
{
	int ret = -1;
	ret = cts_tcs_read_u8attr(TP_STD_CMD_SYS_STS_DATA_CAPTURE_EN_RW, int_data_method);
	return ret;
}

int cts_tcs_set_int_data_method(uint8_t int_data_method)
{
	int ret = -1;
	ret = cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_DATA_CAPTURE_EN_RW, int_data_method);
	return ret;
}

int cts_tcs_set_line_data_type(uint8_t line_data_type)
{
	int ret = -1;
	ret = cts_tcs_write_u8attr(TP_STD_CMD_ICTEST_SELF_COMP_RW, line_data_type);
	return ret;
}

int cts_tcs_get_self_dcap_adj_enable(uint8_t *enable)
{
	int ret = -1;
	ret = cts_tcs_read_u8attr(TP_STD_CMD_ICTEST_SELF_DCAP_ADJUSTEN, enable);
	return ret;
}

int cts_tcs_set_self_dcap_adj_enable(uint8_t enable)
{
	int ret = -1;
	ret = cts_tcs_write_u8attr(TP_STD_CMD_ICTEST_SELF_DCAP_ADJUSTEN, enable);
	return ret;
}

int cts_tcs_get_data_ready_flag(uint8_t *ready)
{
	int ret = -1;
	ret = cts_tcs_read_u8attr(TP_STD_CMD_SYS_STS_DAT_RDY_FLAG_RW, ready);
	return ret;
}

int cts_tcs_clr_data_ready_flag(void)
{
	int ret;
	ret = cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_DAT_RDY_FLAG_RW, 0);
	return ret;
}

int cts_tcs_get_data_capture_support(uint8_t * support)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_SYS_STS_DATA_CAPTURE_SUPPORT_RO, support);
}

int cts_tcs_get_work_mode(uint8_t *work_mode)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_SYS_STS_CURRENT_WORKMODE_RO, work_mode);
}

int cts_tcs_set_work_mode(uint8_t work_mode)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_WORK_MODE_RW, work_mode);
}

int cts_tcs_set_short_thresh(uint16_t thresh)
{
	return cts_tcs_write_u16attr(TP_STD_CMD_ICTEST_SHORT_THRESHOLD, thresh);
}

#ifdef CTS_FOR_FFT_MODE
int cts_tcs_set_fft_mode(uint8_t fft_mode)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_FFT_EN_RW, fft_mode);
}

int cts_tcs_get_fft_data_ready_flag(uint8_t *ready)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_SYS_STS_FFT_DAT_RDY_FLAG_RW, ready);
}

int cts_tcs_clr_fft_data_ready_flag(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_FFT_DAT_RDY_FLAG_RW, 0);
}
#endif

int cts_tcs_get_enable_short_test(uint8_t *enable)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_ICTEST_SHORTTEST_EN_RW, enable);
}

int cts_tcs_set_enable_short_test(uint8_t enable)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_ICTEST_SHORTTEST_EN_RW, enable);
}

int cts_tcs_get_enable_open_test(uint8_t *enable)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_ICTEST_OPENTEST_EN_RW, enable);
}

int cts_tcs_set_enable_open_test(uint8_t enable)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_ICTEST_OPENTEST_EN_RW, enable);
}

int cts_tcs_set_enable_hsync_test(uint8_t enable)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_ICTEST_HSYNC_TEST_EN, enable);
}

int cts_tcs_get_real_hsync(uint16_t *hsync)
{
	int ret;
	uint8_t buf[2];
	
	ret = cts_tcs_read_attr(TP_STD_CMD_ICTEST_REAL_HSYNC_FREQ, buf, sizeof(buf));
	if (!ret) {
		*hsync = buf[0] | (buf[1] << 8);
	}
	return ret;
}

int cts_tcs_set_real_hsync(uint16_t hsync)
{
	return cts_tcs_write_u16attr(TP_STD_CMD_ICTEST_REAL_HSYNC_FREQ, hsync);
}

int cts_tcs_get_osc_trim_trigger_ratio(uint8_t *value)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_ICTEST_OSC_SHIFT_TRIM_TRIGGER_RATIO, value);
}

int cts_tcs_set_osc_trim_trigger_ratio(uint8_t value)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_ICTEST_OSC_SHIFT_TRIM_TRIGGER_RATIO, value);
}

int cts_tcs_get_osc_trim_max_ratio(uint16_t *value)
{
	return cts_tcs_read_u16attr(TP_STD_CMD_ICTEST_OSC_SHIFT_TRIM_MAX_RATIO, value);
}

int cts_tcs_set_osc_trim_max_ratio(uint16_t value)
{
	return cts_tcs_write_u16attr(TP_STD_CMD_ICTEST_OSC_SHIFT_TRIM_MAX_RATIO, value);
}

int cts_tcs_get_osc_trim(uint16_t *value)
{
	return cts_tcs_read_u16attr(TP_STD_CMD_ICTEST_OSC_TRIM_VALUE, value);
}

int cts_tcs_get_osc_trim_fine(uint16_t *value)
{
	return cts_tcs_read_u16attr(TP_STD_CMD_ICTEST_OSC_TRIM_VALUE_FINE, value);
}

void cts_dump_spi_tx_rx(uint8_t *tx, uint8_t *rx, size_t tx_len, size_t rx_len)
{
#ifndef RELEASE
	if (tx_len)
		cts_dump_spi_tx(txbuf, tx_len);
	if (rx_len)
		cts_dump_spi_rx(rxbuf, rx_len);
#endif
}

static void cts_tcs_dump_data(const char *desc, const uint8_t *data, size_t size)
{
#define SPLIT_LINE_STR \
	"--------------------------------------------------------"\
	"--------------------------------------------------------"
#define ROW_NUM_FORMAT_STR  "%2d | "
#define COL_NUM_FORMAT_STR  "%-5u "
#define DATA_FORMAT_STR     "%-5u "

	int r, c, nodes = 0;
	char line_buf[550];
	int count = 0;
	int rows, cols;

	if (strstr(desc,"shortdata") != NULL) {
		rows = 13;
		cols = 20;
	} else if ((strstr(desc,"to_gnd") != NULL) ||
		(strstr(desc,"between_channel") != NULL)) {
		rows = 9;
		cols = 15;
	} else {
		rows = ROWS;
		cols = COLS;
	}

	THP_LOGE(SPLIT_LINE_STR);
	count += SNPRINTF(line_buf + count, sizeof(line_buf) - count, "   |  ");
	for (c = 0; c < cols; c++) {
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
			COL_NUM_FORMAT_STR, c);
	}
	THP_LOGE("%s", line_buf);
	THP_LOGE(SPLIT_LINE_STR);

	for (r = 0; r < rows; r++) {
		count = 0;
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
			ROW_NUM_FORMAT_STR, r);
		for (c = 0; (c < cols) && (nodes < size); c++) {
			count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
				DATA_FORMAT_STR, data[r * cols + c]);
			nodes++;
		}
		THP_LOGE("%s", line_buf);
	}
	THP_LOGE(SPLIT_LINE_STR);
	
#undef SPLIT_LINE_STR
#undef ROW_NUM_FORMAT_STR
#undef COL_NUM_FORMAT_STR
#undef DATA_FORMAT_STR
}

int cts_tcs_polling_rawdata(uint8_t *buf, size_t size)
{
	int ret = -1, rc = -1;
	int retries = 100;
	uint8_t ready = 0;
	size_t txlen;
	size_t rxlen = TOUCH_INFO_SIZ + INT_DATA_INFO_SIZ + INT_DATA_TYPE_LEN_SIZ + size + TCS_REPLY_TAIL_SIZ;
	uint16_t crc16_calc, crc16_recv;
	uint32_t temp_buf;

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf1:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf1 invalid");
	} else {
#ifdef CTS_FOR_IC_STATE_DEBUG
		if (temp_buf & BIT(29)) {
			THP_LOGE("temp_buf & BIT(29): 0x%x", temp_buf & BIT(29));
			return 2;
		}
#endif
	}
	
	while (retries--) {
		cts_mdelay(10);	
		ret = cts_tcs_get_data_ready_flag(&ready);
		rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
		THP_LOGE("ictest_buf2:0x%x",temp_buf);
		if (rc) {
			THP_LOGE("read ictest_buf2 invalid");
		} else {
#ifdef CTS_FOR_IC_STATE_DEBUG
			if (temp_buf & BIT(29)) {
				THP_LOGE("temp_buf & BIT(29): 0x%x", temp_buf & BIT(29));
				return 2;
			}
#endif
		}
		if (!ret && (ready == 1)) {
			break;
		}
	}
	THP_LOGI("get data rdy, retries left %d", retries);

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf3:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf3 invalid");
	} else {
#ifdef CTS_FOR_IC_STATE_DEBUG
		if (temp_buf & BIT(29)) {
			THP_LOGE("temp_buf & BIT(29): 0x%x", temp_buf & BIT(29));
			return 2;
		}
#endif
	}
	if (ret) {
		THP_LOGE("Get data rdy failed");
		return 1;
	}
	if (ready != 1) {
		THP_LOGE("time out wait for data rdy");
		return 1;
	}

	txlen = cts_tcs_read_pack(txbuf, TP_STD_CMD_SYS_STS_GET_COORDINATES_RO,
			rxlen - TCS_REPLY_TAIL_SIZ);

	ret = cts_tcs_spi_xing(txbuf, rxbuf, txlen > rxlen ? txlen : rxlen);
	cts_dump_spi_tx_rx(txbuf, NULL, txlen, 0);
	if (ret == 0) {
		crc16_calc = cts_crc16(rxbuf, rxlen - sizeof(uint16_t));
		crc16_recv = rxbuf[rxlen - 2] | (rxbuf[rxlen - 1] << 8);
		if (crc16_recv != crc16_calc) {
			ret = 2;
			THP_LOGW("crc16_recv 0x%x != crc16_calc 0x%x, data_size: %u, crc_calc_len: %d", crc16_recv, crc16_calc, size, (rxlen - sizeof(uint16_t)));
#ifdef CTS_FOR_TEST_DEBUG
			cts_tcs_dump_data("rawdata", rxbuf, rxlen);
#endif
		} else {
			MEMCPY(buf, rxbuf + TOUCH_INFO_SIZ + INT_DATA_INFO_SIZ + INT_DATA_TYPE_LEN_SIZ, size);
		}
	}

	if (cts_tcs_clr_data_ready_flag()) {
		THP_LOGE("Clear data ready flag failed");
	}

	return ret;
}

int cts_tcs_polling_shortdata(uint8_t *buf, size_t size)
{
	int ret = -1, rc = -1;
	int retries = 250;
	uint8_t ready = 0;
	size_t txlen;
	size_t rxlen = FRAME_SHORT_DATA_SIZE + TCS_REPLY_TAIL_SIZ;
	size_t rxlen1 = TEST_SHORT_DATA_SIZE + TCS_REPLY_TAIL_SIZ;
	uint16_t crc16_calc, crc16_recv;
	uint32_t temp_buf;
	uint8_t *buf1 = buf + FRAME_SHORT_DATA_SIZE;
	uint8_t *buf2 = buf1 + TEST_SHORT_DATA_SIZE;
	int i, j;

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf1:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf1 invalid");
	} else {
#ifdef CTS_FOR_IC_STATE_DEBUG
		if (temp_buf & BIT(29)) {
			THP_LOGE("temp_buf & BIT(29): 0x%x", temp_buf & BIT(29));
			return 2;
		}
#endif
	}
	while (retries--) {
		cts_mdelay(20);
		ret = cts_tcs_get_data_ready_flag(&ready);
		rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
		THP_LOGE("ictest_buf2:0x%x",temp_buf);
		if (rc) {
			THP_LOGE("read ictest_buf2 invalid");
		} else {
#ifdef CTS_FOR_IC_STATE_DEBUG
			if (temp_buf & BIT(29)) {
				THP_LOGE("temp_buf & BIT(29): 0x%x", temp_buf & BIT(29));
				return 2;
			}
#endif
		}
		if (!ret && ready == 1) {
			break;
		}
	}
	THP_LOGE("get data rdy, retries left %d, ready flag: %d", retries, ready);

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf3:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf3 invalid");
	} else {
#ifdef CTS_FOR_IC_STATE_DEBUG
		if (temp_buf & BIT(29)) {
			THP_LOGE("temp_buf & BIT(29): 0x%x", temp_buf & BIT(29));
			return 2;
		}
#endif
	}
	if (ret) {
		THP_LOGE("Get data rdy failed");
		return 1;
	}
	if (ready != 1) {
		THP_LOGE("time out wait for data rdy");
		return 1;
	}

	/* short test data */
	THP_LOGI("Short test data");
	txlen = cts_tcs_read_pack(txbuf, TP_STD_CMD_ICTEST_SHORTTEST_RES_VAL_RO,
			rxlen - TCS_REPLY_TAIL_SIZ);
	ret = cts_tcs_spi_chichu(txbuf, txlen, rxbuf, rxlen);
	cts_dump_spi_tx_rx(txbuf, NULL, txlen, 0);
	if (ret == 0) {
		crc16_calc = cts_crc16(rxbuf, rxlen - sizeof(uint16_t));
		crc16_recv = rxbuf[rxlen - 2] | (rxbuf[rxlen - 1] << 8);
		if (crc16_recv != crc16_calc) {
			THP_LOGW("crc16_recv 0x%x != crc16_calc 0x%x, data_size: %u, crc_calc_len: %d",
				crc16_recv, crc16_calc, size, (rxlen - sizeof(uint16_t)));
#ifdef CTS_FOR_TEST_DEBUG
			cts_tcs_dump_data("shortdata", rxbuf, rxlen);
#endif
			ret = 2;
		} else {
			MEMCPY(buf, rxbuf, FRAME_SHORT_DATA_SIZE);
#if 0
			for (i = 0; i < FRAME_SHORT_DATA_SIZE; i++) {
				j = i + 1;
				if ((buf[i] == 0) && (buf[j] == 0)) {
					THP_LOGE("Get shortdata failed, maybe 0");
#ifdef CTS_FOR_TEST_DEBUG
					cts_tcs_dump_data("shortdata", rxbuf, rxlen);
#endif
					ret = 2;
					break;
				}
				i++;
			}
#endif
		}
	}
	if (ret)
		goto err_return;

	/* shortdata to GND */
	THP_LOGI("Shortdata to GND");
	txlen = cts_tcs_read_pack(txbuf, TP_STD_CMD_ICTEST_SHORT_CHANNEL_TO_GND,
			rxlen1 - TCS_REPLY_TAIL_SIZ);
	ret = cts_tcs_spi_chichu(txbuf, txlen, rxbuf, rxlen1);
	cts_dump_spi_tx_rx(txbuf, NULL, txlen, 0);
	if (ret == 0) {
		crc16_calc = cts_crc16(rxbuf, rxlen1 - sizeof(uint16_t));
		crc16_recv = rxbuf[rxlen1 - 2] | (rxbuf[rxlen1 - 1] << 8);
		if (crc16_recv != crc16_calc) {
			THP_LOGW("crc16_recv 0x%x != crc16_calc 0x%x, data_size: %u, crc_calc_len: %d",
				crc16_recv, crc16_calc, TEST_SHORT_DATA_SIZE, (rxlen1 - sizeof(uint16_t)));
#ifdef CTS_FOR_TEST_DEBUG
			cts_tcs_dump_data("short_to_gnd", rxbuf, rxlen1);
#endif
			ret = 2;
		} else {
			MEMCPY(buf1, rxbuf, TEST_SHORT_DATA_SIZE);
		}
	}
	if (ret)
		goto err_return;

	/* shortdata between channel */
	THP_LOGI("Shortdata between channel");
	txlen = cts_tcs_read_pack(txbuf, TP_STD_CMD_ICTEST_MUTUAL_SHORT_CHANNEL,
			rxlen1 - TCS_REPLY_TAIL_SIZ);
	ret = cts_tcs_spi_chichu(txbuf, txlen, rxbuf, rxlen1);
	cts_dump_spi_tx_rx(txbuf, NULL, txlen, 0);
	if (ret == 0) {
		crc16_calc = cts_crc16(rxbuf, rxlen1 - sizeof(uint16_t));
		crc16_recv = rxbuf[rxlen1 - 2] | (rxbuf[rxlen1 - 1] << 8);
		if (crc16_recv != crc16_calc) {
			THP_LOGW("crc16_recv 0x%x != crc16_calc 0x%x, data_size: %u, crc_calc_len: %d",
				crc16_recv, crc16_calc, TEST_SHORT_DATA_SIZE, (rxlen1 - sizeof(uint16_t)));
#ifdef CTS_FOR_TEST_DEBUG
			cts_tcs_dump_data("short_between_channel", rxbuf, rxlen1);
#endif
			ret = 2;
		} else {
			MEMCPY(buf2, rxbuf, TEST_SHORT_DATA_SIZE);
		}
	}

err_return:
	THP_LOGE("Get short data return: %d", ret);
	return ret;
}

int cts_get_hsyncdata(void)
{
	int ret;
	//uint16_t value;
	uint16_t hsyncdata = 0;
	ret = cts_tcs_read_u16attr(TP_STD_CMD_ICTEST_HSYNC_RESULT, &hsyncdata);
	if (ret) {
		THP_LOGE("Get hsyncdata failed");
		return ret;
	} else {
		//value = hsyncdata[0] | hsyncdata[1] << 8;
		All_LOG("hsyncdata: %d", hsyncdata);
	}
	
	return ret;
}

int cts_tcs_polling_hsyncdata(uint8_t *buf, size_t size)
{
	int ret = -1, rc = -1;
	int retries = 100;
	uint8_t ready = 0;
	size_t txlen;
	size_t rxlen = size + TCS_REPLY_TAIL_SIZ;
	uint16_t crc16_calc, crc16_recv;
	uint32_t temp_buf;

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf1:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf1 invalid");
	} else {
#ifdef CTS_FOR_IC_STATE_DEBUG
		if (temp_buf & BIT(29)) {
			THP_LOGE("temp_buf & BIT(29): 0x%x", temp_buf & BIT(29));
			return 2;
		}
#endif
	}

	while (retries--) {
		cts_mdelay(10);
		ret = cts_tcs_get_data_ready_flag(&ready);
		rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
		THP_LOGE("ictest_buf2:0x%x",temp_buf);
		if (rc) {
			THP_LOGE("read ictest_buf2 invalid");
		} else {
#ifdef CTS_FOR_IC_STATE_DEBUG
			if (temp_buf & BIT(29)) {
				THP_LOGE("temp_buf & BIT(29): 0x%x", temp_buf & BIT(29));
				return 2;
			}
#endif
		}
		if (!ret && ready) {
			break;
		}
	}
	THP_LOGI("get data rdy, retries left %d", retries);

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf3:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf3 invalid");
	} else {
#ifdef CTS_FOR_IC_STATE_DEBUG
		if (temp_buf & BIT(29)) {
			THP_LOGE("temp_buf & BIT(29): 0x%x", temp_buf & BIT(29));
			return 2;
		}
#endif
	}
	if (ret) {
		THP_LOGE("Get data rdy failed");
		return 1;
	}
	if (ready != 1) {
		THP_LOGE("time out wait for data rdy, ready: %d", ready);
		return 1;
	}

	THP_LOGE("Get hsync value");
	txlen = cts_tcs_read_pack(txbuf, TP_STD_CMD_ICTEST_HSYNC_RESULT,
			rxlen - TCS_REPLY_TAIL_SIZ);
	ret = cts_tcs_spi_chichu(txbuf, txlen, rxbuf, rxlen);
	if (ret == 0) {
		crc16_calc = cts_crc16(rxbuf, rxlen - sizeof(uint16_t));
		crc16_recv = rxbuf[rxlen - 2] | (rxbuf[rxlen - 1] << 8);
		if (crc16_recv != crc16_calc) {
			THP_LOGW("crc16_recv 0x%x != crc16_calc 0x%x, data_size: %u, crc_calc_len: %d", crc16_recv, crc16_calc, size, (rxlen - sizeof(uint16_t)));
			THP_LOGW("pollingdatar++");
			return 2;
		} else {
			MEMCPY(buf, rxbuf, size);
		}
	}

#ifndef TEST_HSYNC_ONLY
		cts_mdelay(1);
	
		THP_LOGE("Get osc trim value");
		txlen = cts_tcs_read_pack(txbuf, TP_STD_CMD_ICTEST_OSC_TRIM_VALUE,
				rxlen - TCS_REPLY_TAIL_SIZ);
		ret = cts_tcs_spi_chichu(txbuf, txlen, rxbuf, rxlen);
		cts_dump_spi_tx_rx(txbuf, rxbuf, txlen, rxlen);
		if (ret == 0) {
			crc16_calc = cts_crc16(rxbuf, rxlen - sizeof(uint16_t));
			crc16_recv = rxbuf[rxlen - 2] | (rxbuf[rxlen - 1] << 8);
			if (crc16_recv != crc16_calc) {
				THP_LOGW("crc16_recv 0x%x != crc16_calc 0x%x, data_size: %u, crc_calc_len: %d", crc16_recv, crc16_calc, size, (rxlen - sizeof(uint16_t)));
				return -1;
			} else {
				MEMCPY(buf + size, rxbuf, size);
			}
		}
		cts_mdelay(1);
	
		THP_LOGE("Get osc trim value fine");
		txlen = cts_tcs_read_pack(txbuf, TP_STD_CMD_ICTEST_OSC_TRIM_VALUE_FINE,
				rxlen - TCS_REPLY_TAIL_SIZ);
		ret = cts_tcs_spi_chichu(txbuf, txlen, rxbuf, rxlen);
		cts_dump_spi_tx_rx(txbuf, rxbuf, txlen, rxlen);
		if (ret == 0) {
			crc16_calc = cts_crc16(rxbuf, rxlen - sizeof(uint16_t));
			crc16_recv = rxbuf[rxlen - 2] | (rxbuf[rxlen - 1] << 8);
			if (crc16_recv != crc16_calc) {
				THP_LOGW("crc16_recv 0x%x != crc16_calc 0x%x, data_size: %u, crc_calc_len: %d", crc16_recv, crc16_calc, size, (rxlen - sizeof(uint16_t)));
				return -1;
			} else {
				MEMCPY(buf + size * 2, rxbuf, size);
			}
		}
#endif

	return ret;
}

#ifdef CTS_FOR_FFT_MODE
int cts_tcs_polling_fftdata(uint8_t *buf, size_t size)
{
	int ret = -1;
	int retries = 100;
	uint8_t ready = 0;
	size_t txlen;
	size_t rxlen = size + TCS_REPLY_TAIL_SIZ;
	uint16_t crc16_calc, crc16_recv;

	while (retries--) {
		cts_mdelay(15);
		ret = cts_tcs_get_fft_data_ready_flag(&ready);
		if (!ret && (ready == 1))
			break;
	}
	THP_LOGI("get data rdy, retries left %d", retries);
	if (ret) {
		THP_LOGE("Get fft data rdy failed");
		return -1;
	}
	if (!ready) {
		THP_LOGE("time out wait for fft data rdy");
		return -1;
	}

	txlen = cts_tcs_read_pack(txbuf, TP_STD_CMD_FFT_NOISE_VAL_RO,
			rxlen - TCS_REPLY_TAIL_SIZ);
		
	ret = cts_tcs_spi_chichu(txbuf, txlen, rxbuf, rxlen);
	if (ret == 0) {
		crc16_calc = cts_crc16(rxbuf, rxlen - sizeof(uint16_t));
		crc16_recv = rxbuf[rxlen - 2] | (rxbuf[rxlen - 1] << 8);
		if (crc16_recv != crc16_calc) {
			THP_LOGW("crc16_recv 0x%x != crc16_calc 0x%x, data_size: %u, crc_calc_len: %d", crc16_recv, crc16_calc, size, (rxlen - sizeof(uint16_t)));
			THP_LOGW("pollingdatar++");
			return -1;
		} else {
			memcpy(buf, rxbuf, size);
		}
	}

	if (cts_tcs_clr_fft_data_ready_flag())
		THP_LOGE("Clear fft data ready flag failed");

	return ret;
}

int cts_test_polling_fftdata(uint16_t *buf, size_t size)
{
	int retries = 3;
	int ret;

	while (retries--) {
		ret = cts_tcs_polling_fftdata((uint8_t *)buf, size);
		if (!ret) {
			break;
		}
	}

	return ret;
}
#endif

int cts_test_polling_rawdata(uint16_t *buf, size_t size)
{
	return cts_tcs_polling_rawdata((uint8_t *)buf, size);
}

int cts_test_polling_shortdata(uint16_t *buf, size_t size)
{
	return cts_tcs_polling_shortdata((uint8_t *)buf, size);
}

int cts_test_polling_hsyncdata(uint8_t *buf, size_t size)
{
	return cts_tcs_polling_hsyncdata(buf, size);
}

int cts_tcs_enable_freq(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_NOISE_FREQ_HOP_MODE_RW, 1);
}

int cts_tcs_disable_freq(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_NOISE_FREQ_HOP_MODE_RW, 0);
}

int cts_tcs_start_calibration(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_START_RECALIB_RW, 1);
}
int cts_tcs_set_freq_points(uint8_t index)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_FORCE_LOCK_HOP_FREQ_WO, index);
}

int cts_tcs_set_scan_rate(uint16_t rate)
{
	return cts_tcs_write_u16attr(TP_STD_CMD_TP_PARA_REPORT_RATE_RW, rate);
}

int cts_tcs_stylus_freq_immediately(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_FORCE_STYLUS_HOP_FREQ_RW, 1);
}

int cts_tcs_stylus_freq_next_uplink(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_FORCE_STYLUS_HOP_FREQ_RW, 1);
}

int cts_tcs_enable_stylus(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_EXT_CLK_INPUT_RW, 1);
}

int cts_tcs_disable_stylus(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_EXT_CLK_INPUT_RW, 0);
}

int cts_tcs_set_afe_suspend(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_SUSPEND_SCAN_EN_RW, 1);
}

int cts_tcs_set_afe_resume(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_SUSPEND_SCAN_EN_RW, 0);
}

int cts_tcs_enable_wakeup_gesture(uint16_t gesture)
{
	return cts_tcs_write_u16attr(TP_STD_CMD_GSTR_ENABLE_TYPE_RW, gesture);
}

int cts_tcs_disable_wakeup_gesture(uint16_t gesture)
{
	return cts_tcs_write_u16attr(TP_STD_CMD_GSTR_DISABLE_TYPE_RW, gesture);
}

int cts_tcs_clr_afe_status(uint32_t status)
{
	return cts_tcs_write_u32attr(TP_STD_CMD_SYS_STS_CLEAR_FW_STATUS_RW, status);
}
int cts_tcs_clear_gesture_status(uint16_t gesture)
{
	return cts_tcs_write_u16attr(TP_STD_CMD_GSTR_ClEAR_STATUS_RW, gesture);
}
int cts_tcs_clr_stylus_status(uint16_t status)
{
	return cts_tcs_write_u16attr(TP_STD_CMD_SYS_STS_CLEAR_STYLIS_STATUS_RW, status);
}

int cts_tcs_test_enable_freq(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_ICTEST_HOP_FREQ_EN_RW, 1);
}

int cts_tcs_get_scan_mode(uint8_t *scan_mode)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_SCAN_MUTUAL_SCAN_MODE_RW, scan_mode);
}

int cts_tcs_set_scan_mode(uint8_t scan_mode)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SCAN_MUTUAL_SCAN_MODE_RW, scan_mode);
}

int cts_tcs_update_base_to_flash(void)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_WRITE_HOP_FREQ_BASE_TO_FLASH_EN_RW, 1);
}

int cts_tcs_get_scan_enable_select(uint8_t *value)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_SCAN_SCAN_ENABLE_SELECT, value);
}

int cts_tcs_set_scan_enable_select(uint8_t value)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SCAN_SCAN_ENABLE_SELECT, value);
}

int cts_tcs_get_scan_freq_test_en(uint8_t *enable)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_SCAN_SCAN_FREQ_TEST_EN, enable);
}

int cts_tcs_set_scan_freq_test_en(uint8_t enable)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SCAN_SCAN_FREQ_TEST_EN, enable);
}

int cts_tcs_get_scan_freq(uint16_t *value)
{
	return cts_tcs_read_u16attr(TP_STD_CMD_SCAN_SET_SCAN_FREQ, value);
}

int cts_tcs_set_scan_freq(uint16_t value)
{
	return cts_tcs_write_u16attr(TP_STD_CMD_SCAN_SET_SCAN_FREQ, value);
}

int cts_tcs_set_charger(uint8_t enable)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_CHARGER_PLUGIN_RW, enable);
}

int cts_tcs_get_shb_enable(uint8_t *enable)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_SYS_STS_SHB_EN_RW, enable);
}

int cts_tcs_set_shb_enable(uint8_t enable)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_SHB_EN_RW, enable);
}

int cts_tcs_get_compen_enable(uint8_t *enable)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_SYS_STS_FOLD_COMPEN_RW, enable);
}

int cts_tcs_set_compen_enable(uint8_t enable)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_FOLD_COMPEN_RW, enable);
}

int cts_tcs_get_cur_clock(uint8_t *clock)
{
	return cts_tcs_read_u8attr(TP_STD_CMD_ICTEST_AP_CLK_TRIM_OSC, clock);
}

int cts_tcs_set_cur_clock(uint8_t clock)
{
	return cts_tcs_write_u8attr(TP_STD_CMD_ICTEST_AP_CLK_TRIM_OSC, clock);
}

