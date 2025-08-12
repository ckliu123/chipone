#include "thp_ioctl.h"
#include "cts_log.h"
#include "cts_utils.h"
#include "cts_spi.h"
#include "cts_prog.h"
#include "cts_core.h"
#define _CTS_TCS_C_
#include "cts_tcs.h"

#include <string.h>
#include <stddef.h>





uint8_t BoMstrSwFlag;  //b_SW_FLAG
uint8_t BoMstrKrangEn; //SCAN_KRANG_EN
uint8_t u8MstrDdiR0A;  //Master DDI_R_0A
uint8_t u8SlvDdiR0A;   //Slave DDI_R_0A, None
uint8_t  u8MstrGoErr0_1; //SCAN_GO_ERR0_STS
uint8_t  u8MstrGoErr0_2; //SCAN_GO_ERR0_STS
uint8_t  u8MstrGoErr0_3; //SCAN_GO_ERR0_STS
uint8_t  u8MstrGoErr0_4; //SCAN_GO_ERR0_STS

// uint32_t  u32MstrGoErr1; //SCAN_GO_ERR1_STS
uint8_t  u8MstrGoErr1_1; //SCAN_GO_ERR1_STS
uint8_t  u8MstrGoErr1_2; //SCAN_GO_ERR1_STS
uint8_t  u8MstrGoErr1_3; //SCAN_GO_ERR1_STS
uint8_t  u8MstrGoErr1_4; //SCAN_GO_ERR1_STS

//uint16_t  u16FwVer;      //FIRMWARE_VER
uint8_t  u8FwVer;      //FIRMWARE_VER
uint8_t  u8FwVer;      //FIRMWARE_VER

uint8_t   u8MstrDdiState;//GetDdiStatus(), DDI_FSM_STATE
uint8_t   u8DdiFrame;    //60/90/120
uint8_t  u8Dbg0;        //WorkMode1
uint8_t  u8Dbg1;        //WorkMode2
uint8_t   u8Dbg2;        //LibScan
uint8_t   Dbg3;        //SpiSlave

uint8_t debug;
static uint8_t txbuf[INTERNAL_SPI_BUF_SIZ];
static uint8_t rxbuf[INTERNAL_SPI_BUF_SIZ];
#if 1
void cts_print_u8data(uint8_t *rawdata)
{
    char line_buf[COLS * 6 + 128];
    int offset = 0,i,j;
    offset += snprintf(line_buf + offset, sizeof(line_buf) - offset, "   ");
    for (i = 0; i < ROWS; i++)
    {
        offset = 0;
        for (j = 0; j < COLS; j=j+2)
        {
            offset += snprintf(line_buf + offset,sizeof(line_buf) - offset,"%4x", (rawdata[i * COLS + j]<<8) + rawdata[i * COLS + j+1]);
        }
        offset += snprintf(line_buf + offset,
                           sizeof(line_buf) - offset, "\n");
        CTS_THP_LOGI("%s", line_buf);
    }
}
#endif

static void cts_tcs_read_pack(uint8_t *tx, enum TcsCmdIndex cmdIdx, uint16_t rdatalen)
{
    tcs_tx_head *txhdr = (tcs_tx_head *)tx;
    TcsCmdValue_t *tcv = TcsCmdValue + cmdIdx;
    uint16_t crc16;

    txhdr->addr = TCS_RD_ADDR;
    txhdr->cmd =
        (tcv->baseFlag << 15) |
        (tcv->isRead   << 14) |
        (tcv->classID  <<  8) |
        (tcv->cmdID <<  0);
    txhdr->datlen = rdatalen;
    crc16 = cts_crc16((const uint8_t *)txhdr, offsetof(tcs_tx_head, crc16));
    txhdr->crc16 = crc16;
}

static void cts_tcs_write_pack(uint8_t *tx, enum TcsCmdIndex cmdIdx,
                               uint8_t *wdata, uint16_t wdatalen)
{
    tcs_tx_head *txhdr = (tcs_tx_head *)tx;
    TcsCmdValue_t *tcv = TcsCmdValue + cmdIdx;
    uint16_t crc16;

    txhdr->addr = TCS_WR_ADDR;
    txhdr->cmd =
        (tcv->baseFlag << 15) |
        (tcv->isWrite  << 13) |
        (tcv->classID  <<  8) |
        (tcv->cmdID    <<  0);
    txhdr->datlen = wdatalen;
    crc16 = cts_crc16((const uint8_t *)txhdr, offsetof(tcs_tx_head, crc16));
    txhdr->crc16 = crc16;

    if (wdatalen > 0)
    {
        memcpy(tx + sizeof(tcs_tx_head), wdata, wdatalen);
        crc16 = cts_crc16(wdata, wdatalen);
        *(tx + sizeof(tcs_tx_head) + wdatalen) = ((crc16 >> 0) & 0xFF);
        *(tx + sizeof(tcs_tx_head) + wdatalen + 1) = ((crc16 >> 8) & 0xFF);
    }
}

static int cts_tcs_spi_chichu(uint8_t *tx, size_t txlen, uint8_t *rx, size_t rxlen)
{
    struct thp_ioctl_spi_xfer_data xfer[2];
    struct thp_ioctl_spi_msg_package msg;
    int ret = -1;
    uint16_t crc16_recv, crc16_calc;
    uint16_t cmd_recv, cmd_send;
		memset(xfer, 0, sizeof(xfer));
		memset(&msg, 0, sizeof(msg));
	
		xfer[0].tx = (char *)tx;
		xfer[0].rx = NULL;
		xfer[0].len = txlen;
		xfer[0].delay_usecs = 1000;
		xfer[0].cs_change = 1;
	
		xfer[1].tx = NULL;
		xfer[1].rx = (char *)rx;
		xfer[1].len = rxlen;
		xfer[1].delay_usecs = 1000;
		xfer[1].cs_change = 1;
	
		msg.speed_hz = cts_get_spi_speed();
#if 1
		msg.xfer_num = 1;
		msg.xfer_data= &xfer[0];
		ret = thp_ioctl_multiple_spi_xfer_sync(&msg);
		if (ret < 0)
		{
			CTS_THP_LOGE("Spi xtrans failed: %s", strerror(errno));
        return -1;
		}
		msg.xfer_num = 1;
		msg.xfer_data= &xfer[1];
		ret = thp_ioctl_multiple_spi_xfer_sync(&msg);
		if (ret < 0)
		{
			CTS_THP_LOGE("Spi xtrans failed: %s", strerror(errno));
        return -1;
		}
#else
		msg.xfer_num = 2;
		msg.xfer_data= xfer;
	
		ret = thp_ioctl_multiple_spi_xfer_sync(&msg);
		if (ret < 0)
		{
			CTS_THP_LOGE("Spi xtrans failed: %s", strerror(errno));
			return -1;
		}
#endif
    cmd_recv = cts_get_unaligned_le16(rx +rxlen - 4);
    cmd_send = cts_get_unaligned_le16(tx + 1);
    if (cmd_recv != cmd_send)
    {
        CTS_THP_LOGE("cmd check error, send %04x != %04x recv", cmd_send, cmd_recv);
		cts_dump_spi_err(0, tx, txlen);
		cts_dump_spi_err(1, rx, rxlen);
				return -1;
    }

    crc16_recv = cts_get_unaligned_le16(rx + rxlen - 2);
    crc16_calc = cts_crc16(rx, rxlen - 2);
    if (crc16_recv != crc16_calc)
    {
        CTS_THP_LOGE("crc error: recv %04x != %04x calc", crc16_recv, crc16_calc);
		cts_dump_spi_err(0, tx, txlen);
		cts_dump_spi_err(1, rx, rxlen);
        return -1;
			}
    usleep(100);
    return 0;
}

static int cts_tcs_spi_xing(uint8_t *tx, uint8_t *rx, size_t total_len)
{
    int ret = thp_ioctl_spi_sync(tx, rx, total_len);
    if (ret < 0)
    {
        CTS_THP_LOGE("Spi sync failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

static void cts_tcs_tool_read_pack(uint8_t *tx, uint8_t classID, uint8_t cmdID, uint16_t rdatalen)
{
    tcs_tx_head *txhdr = (tcs_tx_head *)tx;
    uint16_t crc16;

    txhdr->addr = TCS_RD_ADDR;
    txhdr->cmd = BIT(14) |
                 (classID  <<  8) |
                 (cmdID <<  0);
    txhdr->datlen = rdatalen;
    crc16 = cts_crc16((const uint8_t *)txhdr, offsetof(tcs_tx_head, crc16));
    txhdr->crc16 = crc16;
}

static void cts_tcs_tool_write_pack(uint8_t *tx, uint8_t classID, uint8_t cmdID,
                                    uint8_t *wdata, uint16_t wdatalen)
{
    tcs_tx_head *txhdr = (tcs_tx_head *)tx;
    uint16_t crc16;

    txhdr->addr = TCS_WR_ADDR;
    txhdr->cmd = BIT(13) |
                 (classID  <<  8) |
                 (cmdID <<  0);
    txhdr->datlen = wdatalen;
    crc16 = cts_crc16((const uint8_t *)txhdr, offsetof(tcs_tx_head, crc16));
    txhdr->crc16 = crc16;

    if (wdatalen > 0)
    {
        memcpy(tx + sizeof(tcs_tx_head), wdata, wdatalen);
        crc16 = cts_crc16(wdata, wdatalen);
        *(tx + sizeof(tcs_tx_head) + wdatalen) = ((crc16 >> 0) & 0xFF);
        *(tx + sizeof(tcs_tx_head) + wdatalen + 1) = ((crc16 >> 8) & 0xFF);
    }
}

int cts_tcs_read_spi_for_tool(uint8_t classID, uint8_t cmdID, uint8_t *buf, size_t len)
{
    int rxlen = len + TCS_TAIL_LEN;
    int ret;

    cts_tcs_tool_read_pack(txbuf, classID, cmdID, len);
    ret = cts_tcs_spi_chichu(txbuf, TCS_HEAD_LEN, rxbuf, rxlen);
    if (ret == 0)
    {
        memcpy(buf, rxbuf, len);
    }

    return ret;
}

int cts_tcs_write_spi_for_tool(uint8_t classID, uint8_t cmdID, uint8_t *buf, size_t len)
{
    int txlen = TCS_HEAD_LEN + len + sizeof(uint16_t);
    cts_tcs_tool_write_pack(txbuf, classID, cmdID, buf, len);
    return cts_tcs_spi_chichu(txbuf, txlen, rxbuf, TCS_TAIL_LEN);
}

int cts_tcs_read_buff(enum TcsCmdIndex cmdIdx, uint8_t *rdata, size_t rdatalen)
{
    int rxlen = rdatalen + TCS_TAIL_LEN;
    int ret;

    cts_tcs_read_pack(txbuf, cmdIdx, rdatalen);
    ret = cts_tcs_spi_chichu(txbuf, TCS_HEAD_LEN, rxbuf, rxlen);
    memcpy(rdata, rxbuf, rdatalen);

    return ret;
}

int cts_tcs_write_buff(enum TcsCmdIndex cmdIdx, uint8_t *wdata, size_t wdatalen)
{
    int txlen = TCS_HEAD_LEN + wdatalen + sizeof(uint16_t);

    cts_tcs_write_pack(txbuf, cmdIdx, wdata, wdatalen);
    return cts_tcs_spi_chichu(txbuf, txlen, rxbuf, TCS_TAIL_LEN);
}

static int cts_tcs_read_attr(enum TcsCmdIndex cmdIdx, uint8_t *rdata, size_t rdatalen)
{
    return cts_tcs_read_buff(cmdIdx, rdata, rdatalen);
}

static int cts_tcs_write_attr(enum TcsCmdIndex cmdIdx, uint8_t *wdata, size_t wdatalen)
{
    return cts_tcs_write_buff(cmdIdx, wdata, wdatalen);
}


int cts_tcs_write_u8attr(enum TcsCmdIndex cmdIdx, uint8_t u8attr)
{
    return cts_tcs_write_attr(cmdIdx, (uint8_t *)&u8attr, sizeof(uint8_t));
}

int cts_tcs_get_fw_ver(uint16_t *fw_ver)
{
    uint16_t buf;
    int rc=-1;

    rc = cts_tcs_read_attr(TP_STD_CMD_INFO_FW_VER_RO, (uint8_t *)&buf, sizeof(uint16_t));
    if (!rc)
    {
        *fw_ver = buf;
        return 0;
    }

    return rc;
}

int cts_tcs_get_res(uint16_t *res_x, uint16_t *res_y)
{
    uint32_t buf;
    int rc;

    rc = cts_tcs_read_attr(TP_STD_CMD_INFO_TOUCH_XY_INFO_RO, (uint8_t *)&buf, sizeof(uint32_t));
    if (!rc)
    {
        *res_x = buf & 0xFFFF;
        *res_y = (buf >> 16) & 0xFFFF;
        return 0;
    }

    return rc;
}

int cts_tcs_get_module_id(uint16_t *module_id)
{
    uint16_t buf;
    int rc;

    rc = cts_tcs_read_attr(TP_STD_CMD_INFO_MODULE_ID_RO, (uint8_t *)&buf, sizeof(uint16_t));
    if (!rc)
    {
        *module_id = buf;
        return 0;
    }

    return rc;
}

int cts_tcs_get_rows_cols(uint8_t *rows, uint8_t *cols)
{
    uint32_t buf[2];
    int rc;

    rc = cts_tcs_read_attr(TP_STD_CMD_INFO_TOUCH_XY_INFO_RO, (uint8_t *)buf, sizeof(buf));
    if (!rc)
    {
        *cols = buf[1] & 0xFF;
        *rows = (buf[1] >> 8) & 0xFF;
        return 0;
    }

    return rc;
}

int cts_tcs_get_Normal_Fs_Raw_Dest_Value(uint16_t *NormalFsRawDestValue)
{
    uint16_t buf;
    int rc;
    rc = cts_tcs_read_attr(TP_STD_CMD_CNEG_OPTIONS_RW, (uint8_t *)&buf, sizeof(uint16_t));
    if (!rc)
    {
        *NormalFsRawDestValue = buf;
        return 0;
    }
    return rc;
}

int cts_tcs_get_scan_freqs(uint8_t *scan_freq_num, uint16_t *scan_freqs)
{
    uint8_t buf[6];
    int i;
    int rc;

    rc = cts_tcs_read_attr(TP_STD_CMD_NOI_SENSE_FREQ_RW, buf, sizeof(buf));
    if (!rc)
    {
        *scan_freq_num = 0;
        for (i = 0; i < 5; i++)
        {
            *(scan_freqs + i) = buf[i + 1];
            if (buf[i + 1])
            {
                *scan_freq_num += 1;
            }
            else
            {
                break;
            }
        }
        return 0;
    }

    return rc;
}

//add lck
int cts_tcs_get_debug_info()
{

    int rc= -1;
#if 1
    SYS_STS_DBG todebug;
    rc = cts_tcs_read_attr(TP_STD_CMD_SYS_STS_DBG, (uint8_t*)&todebug, sizeof(todebug));    //(uint8_t*)&todebug
    if (!rc)
    {
        CTS_THP_LOGD("%+18s = 0x%02x, %+18s = 0x%02x",
			"11BoMstrSwFlag", todebug.BoMstrSwFlag, "11BoMstrKrangEn", todebug.BoMstrKrangEn);
        CTS_THP_LOGD("%+18s = 0x%02x, %+18s = 0x%02x",
			"11u8MstrDdiR0A", todebug.u8MstrDdiR0A, "11u8SlvDdiR0A", todebug.u8SlvDdiR0A);
        CTS_THP_LOGD("%+18s = 0x%08x", "11u32MstrGoErr0", todebug.u32MstrGoErr0);
        CTS_THP_LOGD("%+18s = 0x%04x", "11u16FwVer", todebug.u16FwVer);
        CTS_THP_LOGD("%+18s = 0x%02x, %+18s = 0x%02x",
			"11u8MstrDdiState", todebug.u8MstrDdiState, "11u8DdiFrame", todebug.u8DdiFrame);
        CTS_THP_LOGD("%+18s = 0x%02x, %+18s = 0x%02x",
			"11u8Dbg0", todebug.u8Dbg0, "11u8Dbg1", todebug.u8Dbg1);
        CTS_THP_LOGD("%+18s = 0x%02x, %+18s = 0x%02x",
			"11u8Dbg2", todebug.u8Dbg2, "11u8Dbg3", todebug.u8Dbg3);

        CTS_THP_LOGD("%+18s = 0x%02x, %+18s = 0x%02x",
			"11u8MstrDdiRAC", todebug.u8MstrDdiRAC, "11u8SlvDdiRAC", todebug.u8SlvDdiRAC);
        CTS_THP_LOGD("%+18s = 0x%02x, %+18s = 0x%02x",
			"11u8MstrDdiRD7", todebug.u8MstrDdiRD7, "11u8SlvDdiRD7", todebug.u8SlvDdiRD7);
        CTS_THP_LOGD("%+18s = 0x%02x, %+18s = 0x%02x",
			"11u8MstrDdiR7A", todebug.u8MstrDdiR7A, "11u8SlvDdiR7A", todebug.u8SlvDdiR7A);
        CTS_THP_LOGD("%+18s = 0x%02x, %+18s = 0x%02x",
			"11u8MstrDdiRD3", todebug.u8MstrDdiRD3, "11u8SlvDdiRD3", todebug.u8SlvDdiRD3);
        return 0;
    }
#endif
    return rc;
}

int cts_tcs_get_scan_rate(uint16_t *scan_rate)
{
    uint8_t buf;
    int rc;

    rc = cts_tcs_read_attr(TP_STD_CMD_DDI_DISP_FRM_RATE_RO, &buf, sizeof(uint8_t));
    if (!rc)
    {
        *scan_rate = buf;
        return 0;
    }

    return rc;
}

int cts_tcs_get_mnt_options(MntOptions *options)
{
    MntOptions opt;
    int rc;

    rc = cts_tcs_read_attr(TP_STD_CMD_MNT_OPTIONS_MNT_RW, (uint8_t *)&opt, sizeof(MntOptions));
    if (!rc)
    {
        *options = opt;
        return 0;
    }

    return rc;
}

int cts_tcs_set_mnt_options(MntOptions *options)
{
    return cts_tcs_write_attr(TP_STD_CMD_MNT_OPTIONS_MNT_RW, (uint8_t *)options, sizeof(MntOptions));
}

int cts_tcs_force_enter_mnt(void)
{
    return cts_tcs_write_u8attr(TP_STD_CMD_MNT_FORCE_ENTER_MNT_WO, 1);
}

int cts_tcs_force_exit_mnt(void)
{
    // update yjl
    //return cts_tcs_write_u8attr(TP_STD_CMD_MNT_FORCE_EXIT_MNT_WO, 0);
    return cts_tcs_write_u8attr(TP_STD_CMD_MNT_FORCE_EXIT_MNT_WO, 0);

    //return cts_tcs_write_u8attr(TP_STD_CMD_MNT_FORCE_ENTER_MNT_WO, 0);
}

int cts_tcs_get_int_data_types(uint16_t *int_data_types)
{
    return cts_tcs_read_attr(TP_STD_CMD_SYS_STS_DATA_CAPTURE_FUNC_MAP_RW,
                             (uint8_t *)int_data_types, sizeof(uint16_t));
}

int cts_tcs_set_int_data_types(uint16_t int_data_types)
{
    return cts_tcs_write_attr(TP_STD_CMD_SYS_STS_DATA_CAPTURE_FUNC_MAP_RW,
                              (uint8_t *)&int_data_types, sizeof(uint16_t));
}

int cts_tcs_get_int_data_method(uint8_t *int_data_method)
{
    return cts_tcs_read_attr(TP_STD_CMD_SYS_STS_DATA_CAPTURE_EN_RW,
                             int_data_method, sizeof(uint8_t));
}

int cts_tcs_set_int_data_method(uint8_t int_data_method)
{
    return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_DATA_CAPTURE_EN_RW,
                                int_data_method);
}

int cts_tcs_get_data_ready_flag(uint8_t *ready)
{
    return cts_tcs_read_attr(TP_STD_CMD_SYS_STS_DAT_RDY_FLAG_RW, ready,
                             sizeof(uint8_t));
}

int cts_tcs_clr_data_ready_flag(void)
{
    return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_DAT_RDY_FLAG_RW, 0);
}
extern uint8_t captest_frame[FRAME_SIZE_HAS_TAIL];
int cts_tcs_polling_rawdata(uint8_t *buf, size_t size)
{
    size_t rxlen = FRAME_SIZE_HAS_TAIL;
    uint16_t crc16_calc, crc16_recv;
    int retries = 100;
    uint8_t ready = 0;
    int ret = -1;

    while (retries--)
    {
        ret = cts_tcs_get_data_ready_flag(&ready);
        if (!ret && ready)
        {
            break;
        }
        mdelay(1);
    }
    if (ret)
    {
        CTS_THP_LOGI("get data rdy, retries left %d", retries);
        CTS_THP_LOGE("%s", "Get data rdy failed");
        return -1;
    }
    if (!ready)
    {
        CTS_THP_LOGE("%s", "time out wait for data rdy");
        return -1;
    }

    cts_tcs_read_pack(txbuf, TP_STD_CMD_GET_DATA_BY_POLLING_RO, FRAME_SIZE);
#ifdef DEBUG_DUMP_IOCTL_SPI_FULL_DATA
    if ((txbuf[3]<<8 | txbuf[4]) < 200 )
        dump_spi_full_data(txbuf, TCS_HEAD_LEN);
    else
        dump_spi_full_data(txbuf, 200);
#endif
    ret = cts_tcs_spi_xing(txbuf, rxbuf, FRAME_SIZE_HAS_TAIL);
#ifdef DEBUG_DUMP_IOCTL_SPI_FULL_DATA
    if ((txbuf[3]<<8 | txbuf[4]) < 200 )
        dump_spi_full_data(rxbuf, rxlen);
    else
        dump_spi_full_data(rxbuf, 200);
#endif

#if  0 // debug  info   yjl
    cts_print_u8data(rxbuf);
#endif

    if (ret == 0)
    {
        crc16_calc = cts_crc16((const uint8_t*)rxbuf, rxlen - 2);
        crc16_recv = rxbuf[rxlen - 2] | (rxbuf[rxlen - 1] << 8);
        if (crc16_recv != crc16_calc)
        {
            CTS_THP_LOGE("recv=0x%04x != calc=0x%04x",crc16_recv, crc16_calc);
            ret = -1;
        }
        else
        {
            memcpy(buf, rxbuf + FRAME_RAWDATA_INDEX, RAWDATA_SIZE);
            memcpy(captest_frame, rxbuf, sizeof(captest_frame));

        }
    }

    // CTS_THP_LOGI("Clear data ready flag");
    if (cts_tcs_clr_data_ready_flag())
    {
        CTS_THP_LOGE("%s", "Clear data ready flag failed");
    }

    return ret;
}

int cts_tcs_get_curr_mode(uint8_t *curr_mode)
{
    return cts_tcs_read_attr(TP_STD_CMD_SYS_STS_KRANG_CURRENT_WORKMODE_RO, curr_mode,
                             sizeof(uint8_t));
}

int cts_tcs_get_work_mode(uint8_t *work_mode)
{
    // return cts_tcs_read_attr(TP_STD_CMD_SYS_STS_WORK_MODE_RW, work_mode,
    return cts_tcs_read_attr(TP_STD_CMD_SYS_STS_KRANG_CURRENT_WORKMODE_RO, work_mode,
                             sizeof(uint8_t));
}

int cts_tcs_set_work_mode(uint8_t work_mode)
{
    if (CTS_FIRMWARE_WORK_MODE_NORM == work_mode)
        CTS_THP_LOGI("set CTS_FIRMWARE_WORK_MODE_NORM mode");

    else if (CTS_FIRMWARE_WORK_MODE_TEST == work_mode)
        CTS_THP_LOGI("set CTS_FIRMWARE_WORK_MODE_TEST mode");


    else if (CTS_FIRMWARE_WORK_MODE_OPEN_SHORT == work_mode)
        CTS_THP_LOGI("set CTS_FIRMWARE_WORK_MODE_OPEN_SHORT mode");


    else  if (CTS_FIRMWARE_WORK_MODE_CFG == work_mode)
        CTS_THP_LOGI("set CTS_FIRMWARE_WORK_MODE_CFG mode");

    int ret = cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_KRANG_WORK_MODE_RW, work_mode);
    mdelay(50);
    CTS_THP_LOGI("Mdelay 50ms");
    return ret;
}

int cts_tcs_disable_esd_ddi_check(void)
{
    CTS_THP_LOGD("cts_tcs_disable_esd_ddi_check");
    return cts_tcs_write_u8attr(TP_STD_CMD_DDI_ESD_EN_RW, 0);
}


int cts_tcs_disable_esd_diff_check(void)
{
    CTS_THP_LOGD("cts_tcs_disable_esd_diff_check");
    return cts_tcs_write_u8attr(TP_STD_CMD_DIFF_ESD_EN_RW, 0);
}

//yjl
int cts_tcs_disable_mnt(void)
{
    CTS_THP_LOGD("cts_tcs_disable_mnt");
    return cts_tcs_write_u8attr(TP_STD_CMD_MNT_EN_RW, 0);
}

int cts_tcs_enable_mnt(void)
{
    CTS_THP_LOGD("cts_tcs_enable_mnt");
    return cts_tcs_write_u8attr(TP_STD_CMD_MNT_EN_RW, 1);
}

int cts_tcs_disable_cneg(void)
{
    return cts_tcs_write_u8attr(TP_STD_CMD_CNEG_EN_RW, 0);
}

int cts_tcs_set_openshort_mode(uint8_t openshort_mode)
{
    if (CTS_TEST_OPEN == openshort_mode)
        CTS_THP_LOGI("set open test --");
    else if (CTS_TEST_SHORT == openshort_mode)
        CTS_THP_LOGI("set short test --");

    return cts_tcs_write_u8attr(TP_STD_CMD_OPENSHORT_MODE_SEL_RW,
                                openshort_mode);
}

int cts_tcs_set_short_test_type(uint8_t short_test_type)
{
    return cts_tcs_write_u8attr(TP_STD_CMD_OPENSHORT_SHORT_SEL_RW,
                                short_test_type);
}

int cts_tcs_enable_get_cneg(void)
{
    return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_CNEG_RD_EN_RW, 1);
}

int cts_tcs_disable_get_cneg(void)
{
    return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_CNEG_RD_EN_RW, 0);
}

int cts_tcs_get_cneg_ready(uint8_t *ready)
{
    return cts_tcs_read_attr(TP_STD_CMD_SYS_STS_CNEG_RDY_FLAG_RW, ready,
                             sizeof(uint8_t));
}

int cts_tcs_polling_cnegdata(uint8_t *buf, size_t size)
{
    int ret = -1;
    int retries = 100;
    uint8_t ready = 0;
    size_t rxlen = FRAME_SIZE_HAS_TAIL;
    uint16_t crc16_calc, crc16_recv;
    bool first = true;
    memset(rxbuf, 0, rxlen);

    while (retries--)
    {
        mdelay(8);
        ret = cts_tcs_get_data_ready_flag(&ready);
        if (!ret && ready)
        {
            if (first)
            {
                cts_tcs_clr_data_ready_flag();
                first = false;
                retries = 100;
                continue;
            }
            break;
        }
    }
    if (ret)
    {
        CTS_THP_LOGI("get data rdy, retries left %d", retries);        
        CTS_THP_LOGE("%s", "Get data rdy failed");
        return -1;
    }
    if (!ready)
    {
        CTS_THP_LOGE("%s", "time out wait for data rdy");
        return -1;
    }

    cts_tcs_read_pack(txbuf, TP_STD_CMD_GET_DATA_BY_POLLING_RO, FRAME_SIZE);

#ifdef DEBUG_DUMP_IOCTL_SPI_FULL_DATA
    dump_spi_full_data(txbuf, TCS_HEAD_LEN);
#endif
    ret = cts_tcs_spi_xing(txbuf, rxbuf, FRAME_SIZE_HAS_TAIL);
#ifdef DEBUG_DUMP_IOCTL_SPI_FULL_DATA
    dump_spi_full_data(rxbuf, rxlen);
#endif

#if  0 // debug  info  yjl 
    cts_print_u8data(rxbuf);
#endif

// yjl add check type
#if  1
    if (1)
    {
        uint16_t  cnegoffset=86*2+0;
        if ( (rxbuf[cnegoffset] & 0x80) != 0x80 )
        {
            CTS_THP_LOGE("cnegtype=0x%04x",rxbuf[cnegoffset]);
            ret= -1;
        }
        else
        {
            ret= 0;
            CTS_THP_LOGD("cnegtype=0x%04x",rxbuf[cnegoffset]);
        }
    }
#endif

    if (ret == 0)
    {
        crc16_recv = cts_crc16(rxbuf, rxlen - sizeof(uint16_t));
        crc16_calc = rxbuf[rxlen - 2] | (rxbuf[rxlen - 1] << 8);
        if (crc16_recv != crc16_calc)
        {
            CTS_THP_LOGE("recv=0x%04x != calc=0x%04x",crc16_recv, crc16_calc);
            ret = -1;
        }
        else
        {
            //CTS_THP_LOGE("index=%d",FRAME_RAWDATA_INDEX);
            memcpy(buf, rxbuf + FRAME_RAWDATA_INDEX, RAWDATA_NODES);
        }
    }

    if (cts_tcs_clr_data_ready_flag())
    {
        CTS_THP_LOGE("%s", "Clear data ready flag failed");
    }
    return ret;
}

int cts_test_polling_rawdata(uint16_t *buf, size_t size)
{
    int retries = 3;
    int ret;

    while (retries--)
    {
        ret = cts_tcs_polling_rawdata((uint8_t *)buf, size);
        if (!ret)
        {
            break;
        }
        mdelay(16);
    }
    return ret;
}

/*
    INT_DATA_TYPE_RAWDATA = BIT(0),
    INT_DATA_TYPE_MANUAL_DIFF = BIT(1),
    INT_DATA_TYPE_REAL_DIFF = BIT(2),
*/
int cts_tcs_polling_data(int type, uint8_t *buf, size_t size)
{
    int ret;
    cts_tcs_set_int_data_types(type);
    cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);

    ret = cts_tcs_polling_rawdata(buf, size);
    if (ret)
    {
        CTS_THP_LOGE("Polling data failed, type: %d", type);
    }

    cts_tcs_set_int_data_types(INT_DATA_TYPE_NONE);
    cts_tcs_set_int_data_method(INT_DATA_METHOD_NONE);
    return ret;
}

int cts_test_polling_cnegdata(uint8_t *buf, size_t size)
{
    int retries = 3;
    int ret;

    while (retries--)
    {
        mdelay(16);
        ret = cts_tcs_polling_cnegdata((uint8_t *)buf, size);
        if (!ret)
        {
            break;
        }
    }

    return ret;
}

int cts_tcs_set_scan_freq(uint8_t freq)
{
    //yjl  update
    return cts_tcs_write_u8attr(TP_STD_CMD_FREQ_SHIFT_FORCE_WO, freq);
}

//yjl add
int cts_tcs_freq_shift_switch(uint8_t enable)
{
    if (enable)
    {
        CTS_THP_LOGD("freq shift enable 1");
        return cts_tcs_write_u8attr(TP_STD_CMD_FREQ_SHIFT_ENABLE_WO, 1);
    }
    else
    {
        CTS_THP_LOGD("freq shift disable 0");
        return cts_tcs_write_u8attr(TP_STD_CMD_FREQ_SHIFT_ENABLE_WO, 0);
    }
}

//yjl add
int cts_tcs_disable_CalibratonCheck(void)
{
    CTS_THP_LOGD("----------cts_tcs_disable_CalibratonCheck");

    return cts_tcs_write_u8attr(TP_STD_CMD_AFE_STATUS_CLEAR_WO, 0);
}
//yjl add
int cts_tcs_enable_CalibratonCheck(void)
{
    CTS_THP_LOGD("----------cts_tcs_enable_CalibratonCheck");

    return cts_tcs_write_u8attr(TP_STD_CMD_AFE_STATUS_CLEAR_WO, 1);
}
//yjl add
int cts_tcs_clear_status(THP_AFE_STATUS_ENUM status)
{
    //CTS_THP_LOGD("----------clear calibration done flag");

    return cts_tcs_write_u8attr(TP_STD_CMD_AFE_STATUS_CLEAR_WO, 2);
}

//yjl add
int cts_tcs_Calib_update(void)
{
    CTS_THP_LOGE("----------Calib_update");
    return cts_tcs_write_u8attr(TP_STD_CMD_AFE_STATUS_CLEAR_WO, 3);
}

int cts_tcs_freq_base_update(void)
{
    //CTS_THP_LOGE("----------FREQ_BASE_update");
    return cts_tcs_write_u8attr(TP_STD_CMD_FREQ_BASE_UPDATE, 1);
}

int cts_tcs_set_krang_stop()
{

    CTS_THP_LOGI("cts_tcs_set_krang_stop");
    return cts_tcs_write_u8attr(TP_STD_CMD_SET_KRANG_STOP, 1);
}

int cts_tcs_set_product_en()
{
    CTS_THP_LOGD("cts_tcs_set_product_en");
    return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_PRODUCTION_TEST_EN_RW, 1);
}

int cts_tcs_read_hw_reg(uint32_t addr, uint8_t *data, size_t len)
{
    uint8_t buf[4] = { 0 };
    int ret;

    buf[0] = 1;
    buf[1] = ((addr >> 0) & 0xFF);
    buf[2] = ((addr >> 8) & 0xFF);
    buf[3] = ((addr >> 16) & 0xFF);

    
    ret = cts_tcs_write_buff(TP_STD_CMD_TP_DATA_OFFSET_AND_TYPE_CFG_RW, buf, sizeof(buf));
    if (ret != 0)
    {
        return ret;
    }

    ret = cts_tcs_read_buff(TP_STD_CMD_TP_DATA_READ_START_RO, data, len);
    return ret;
}

int cts_tcs_write_hw_reg(uint32_t addr, uint8_t *data, size_t len)
{
    uint8_t *buf;
    int ret;

    buf = malloc(len + 6);
    if (buf == NULL)
    {
        return -ENOMEM;
    }

    buf[0] = ((len >> 0) & 0xFF);
    buf[1] = ((len >> 8) & 0xFF);
    buf[2] = ((addr >> 0) & 0xFF);
    buf[3] = ((addr >> 8) & 0xFF);
    buf[4] = ((addr >> 16) & 0xFF);
    buf[5] = 0x00;
    memcpy(buf + 6, data, len);

    ret = cts_tcs_write_buff(TP_STD_CMD_TP_DATA_WR_REG_RAM_SEQUENCE_WO, buf, len + 6);

    free(buf);
    return ret;
}

int cts_tcs_set_pwr_mode(uint8_t pwr_mode)
{
    return cts_tcs_write_u8attr(TP_STD_CMD_SYS_STS_PWR_STATE_RW, pwr_mode);
}


