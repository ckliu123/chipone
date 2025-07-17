#include <string.h>
#include "thp_ioctl.h"
#include "cts_log.h"
#include "cts_utils.h"
#include "cts_spi.h"
#include "cts_prog.h"
#include <string.h>
#include <stddef.h>


#define REG_CHIP_CFG            0x73060

#define PROG_SPI_WR             0x60
#define PROG_SPI_RD             0x61
#define PROG_SPI_DUMMY_BYTE     0x00

#pragma pack(push, 1)
typedef struct
{
    uint8_t                     rdwr;
    uint8_t                     addr_be[3];
} CTS_PROG_CMD_STRUCT;
#pragma pack(pop)

/*
typedef enum
{
    CTS_SPI_TARGET_MASTER    = 0x01,
    CTS_SPI_TARGET_SLAVE     = 0x00,
    CTS_SPI_TARGET_INVALID_ID   = 0xFF,
} CTS_SPI_TARGET_ID_ENUM;

static const char CTS_SWITCH_CODE[] =
{
    0x82, // TGT_SLAVE
    0x81, // TGT_MASTER
    0x83, // CTS_SPI_TARGET_ALL
};

int cts_enter_prog_mode(void)
{
    uint8_t magic[] = { 0xCC, 0x33, 0x55, 0x5A };
    int ret = -1;
    THP_LOGD("Enter");

    ret = cts_spi_sync_send(magic, sizeof(magic));
    if (ret < 0)
    {
        CTS_THP_LOGE("Enter Prog Mode failed");
        return -1;
    }

    THP_LOGD("Exit");
    return 0;
}

int cts_prog_read_raw(uint32_t addr, uint8_t *rbuf, size_t rlen)
{
    int ret = -1;
    CTS_PROG_CMD_STRUCT cmd;
    uint8_t dummy[] = { PROG_SPI_DUMMY_BYTE };

    cmd.rdwr = PROG_SPI_RD;
    cts_put_unaligned_be24(addr, cmd.addr_be);

    ret = cts_spi_sync_cmd_data((uint8_t *)&cmd, sizeof(cmd),
                                dummy, sizeof(dummy), rbuf, rlen);
    if (ret < 0)
    {
        CTS_THP_LOGE("Prog read buf failed");
        return -1;
    }

    THP_LOGD("Exit");
    return 0;
}

int cts_prog_write_raw(uint32_t addr, uint8_t *wbuf, size_t wlen)
{
    int ret = -1;
    CTS_PROG_CMD_STRUCT cmd;
    THP_LOGD("Enter");

    cmd.rdwr = PROG_SPI_WR;
    cts_put_unaligned_be24(addr, cmd.addr_be);

    ret = cts_spi_sync_cmd_data((uint8_t *)&cmd, sizeof(cmd),
                                wbuf, wlen, NULL, 0);
    if (ret < 0)
    {
        CTS_THP_LOGE("Prog write buf failed");
        return -1;
    }

    THP_LOGD("Exit");
    return ret;
}

int cts_prog_readsb(uint32_t addr, uint8_t *rval, size_t len)
{
    return cts_prog_read_raw(addr, rval, len);
}

int cts_prog_read_u8(uint32_t addr, uint8_t *rval)
{
    return cts_prog_read_raw(addr, rval, sizeof(uint8_t));
}

int cts_prog_read_u16(uint32_t addr, uint16_t *rval)
{
    return cts_prog_read_raw(addr, (uint8_t *)rval, sizeof(uint16_t));
}

int cts_prog_read_u32(uint32_t addr, uint32_t *rval)
{
    return cts_prog_read_raw(addr, (uint8_t *)rval, sizeof(uint32_t));
}

int cts_prog_write_u8(uint32_t addr, uint8_t wval)
{
    return cts_prog_write_raw(addr, &wval, sizeof(uint8_t));
}

int cts_prog_write_u16(uint32_t addr, uint16_t wval)
{
    return cts_prog_write_raw(addr, (uint8_t *)&wval, sizeof(uint16_t));
}

int cts_prog_write_u32(uint32_t addr, uint32_t wval)
{
    return cts_prog_write_raw(addr, (uint8_t *)&wval, sizeof(uint32_t));
}

CTS_SPI_TARGET_ENUM cts_prog_get_target_id(void)
{
    uint8_t tgt_id;
    int ret = -1;
    THP_LOGD("Enter");

    ret = cts_prog_read_raw(REG_CHIP_CFG, &tgt_id, sizeof(tgt_id));
    if (ret < 0)
    {
        CTS_THP_LOGE("Reg chip_cfg failed");
        return CTS_SPI_TARGET_INVALID_ID;
    }

    if (tgt_id == CTS_SPI_TARGET_MASTER)
    {
        return tgt_id;
    }
    else if (tgt_id == CTS_SPI_TARGET_SLAVE)
    {
        return tgt_id;
    }
    else
    {
        CTS_THP_LOGE("Unknow chip_cfg value: %02x", tgt_id);
        return CTS_SPI_TARGET_INVALID_ID;
    }
}

int cts_prog_spi_switch(CTS_SPI_TARGET_ENUM tgt)
{
    struct thp_ioctl_spi_xfer_data xfer[2];
    struct thp_ioctl_spi_msg_package msg;
    int ret = -1;

    THP_LOGD("Enter, target=%d", tgt);

    if (tgt < CTS_SPI_TARGET_SLAVE || tgt > CTS_SPI_TARGET_ALL)
    {
        CTS_THP_LOGE("ERROR! Invalid spi target");
        return -1;
    }

    memset(xfer, 0, sizeof(xfer));
    memset(&msg, 0, sizeof(msg));

    xfer[0].tx = (char *)&CTS_SWITCH_CODE[tgt];
    xfer[0].rx = NULL;
    xfer[0].len = sizeof(char);
    xfer[0].delay_usecs = 2000;
    xfer[0].cs_change = 1;

    memcpy(&xfer[1], &xfer[0], sizeof(xfer[1]));

    msg.speed_hz = cts_get_spi_speed();
    msg.xfer_num = 1;
    for (int i = 0; i < 2; i++)
    {
    msg.xfer_data= &xfer[i];

    ret = thp_ioctl_multiple_spi_xfer_sync(&msg);
    if (ret < 0)
    {
        CTS_THP_LOGE("Swith target failed: %s", strerror(errno));
        return -1;
    }
 }
#if 0
    uint8_t tgt_id;

    if ((tgt == CTS_SPI_TARGET_MASTER) || (tgt == CTS_SPI_TARGET_SLAVE))
    {
        tgt_id = cts_prog_get_target_id();
        if (tgt_id == CTS_SPI_TARGET_MASTER)
        {
            s_comm_data.spi_target = CTS_SPI_TARGET_MASTER;
        }
        else if (tgt_id == CTS_SPI_TARGET_SLAVE)
        {
            s_comm_data.spi_target = CTS_SPI_TARGET_SLAVE;
        }
        else
        {
            CTS_THP_LOGE("Unknown target id: %02x", tgt_id);
            return -1;
        }
        CTS_THP_LOGE("Unknown target id: %02x", tgt_id);
    }

    THP_LOGD("Exit, target=%d", s_comm_data.spi_target);
#else
    CTS_THP_LOGE("WARN: check target ic to confirm??");
#endif
    return 0;
}
*/

#define REG_CHIP_CFG            0x73060
#define DRW_SPI_RD              0x61
#define DRW_SPI_WR              0x60

#pragma pack(push, 1)
typedef struct cts_drw_head
{
    uint8_t     rwcmd;
    uint8_t     addr[3];
    uint8_t     len[3];
    uint8_t     crc[2];
    uint8_t     wait[4];
} CTS_DRW_HEAD;

typedef struct cts_drw_tail
{
    uint8_t     crc[2];
    uint8_t     wait[1];
    uint8_t     ack[2];
} CTS_DRW_TAIL;
#pragma pack(pop)

static const char CTS_SWITCH_CODE[] =
{
    0x82, // TGT_SLAVE
    0x81, // TGT_MASTER
    0x83, // CTS_SPI_TARGET_ALL
};

int cts_enter_drw_mode(void)
{
    uint8_t magic[] = { 0xCC, 0x33, 0x55, 0x5A };
    int ret = -1;
    ret = cts_spi_sync_send(magic, sizeof(magic));
    if (ret < 0)
    {
        CTS_THP_LOGE("Enter Drw Mode failed");
        return -1;
    }
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
    head.rwcmd = DRW_SPI_RD;
    cts_put_unaligned_be24(head.addr, addr);
    cts_put_unaligned_be24(head.len, rlen);
    crc16_calc = cts_crc16(&head.rwcmd, offsetof(CTS_DRW_HEAD, crc));
    cts_put_unaligned_be16(head.crc, (uint16_t)~crc16_calc);

    ret = cts_spi_sync_raw((uint8_t *)&head, sizeof(head), rx_buf, rlen + 5);//sizeof(tail));
    if (ret < 0)
    {
        CTS_THP_LOGE("Drw read buf failed");
        return -1;
    }
    crc16_calc = cts_crc16(rx_buf, rlen);
    crc16_recv = ~cts_get_unaligned_be16(rx_buf + rlen);
    if (crc16_calc != crc16_recv)
    {
        CTS_THP_LOGE("crc error: calc %#06x != %#06x recv", crc16_calc, crc16_recv);
        return -1;
    }
    MEMCPY(rbuf, rx_buf, rlen);
    return 0;
}

int cts_drw_write_raw(uint32_t addr, uint8_t *wbuf, size_t wlen)
{
    int ret = -1;
    CTS_DRW_HEAD head;
    uint16_t crc16_calc;

    uint8_t tx_buf[SPI_BUF_SIZ];
    uint8_t rx_buf[SPI_BUF_SIZ];

    head.rwcmd = DRW_SPI_WR;
    cts_put_unaligned_be24(head.addr, addr);
    cts_put_unaligned_be24(head.len, wlen);
    crc16_calc = cts_crc16((uint8_t *)&head, offsetof(CTS_DRW_HEAD, crc));
    cts_put_unaligned_be16(head.crc, (uint16_t)~crc16_calc);

    //THP_LOGD("adrr=%06x, wlen=%d", addr, wlen);
    MEMSET(tx_buf, 0, sizeof(head) + wlen + sizeof(uint16_t) + 3);
    MEMCPY(tx_buf, &head, sizeof(head));
    MEMCPY(tx_buf + sizeof(head), wbuf, wlen);
    crc16_calc = cts_crc16(wbuf, wlen);
    cts_put_unaligned_be16(tx_buf + sizeof(head) + wlen, (uint16_t)~crc16_calc);

    ret = cts_spi_sync_raw(tx_buf, sizeof(head) + wlen + sizeof(uint16_t) + 3, rx_buf, 3);//sizeof(tail));
    if (ret < 0)
    {
        CTS_THP_LOGE("Drw read buf failed");
        return -1;
    }
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

CTS_SPI_TARGET_ENUM cts_drw_get_target_id(void)
{
    uint8_t tgt_id;
    int ret = -1;

    ret = cts_drw_read_raw(REG_CHIP_CFG, &tgt_id, sizeof(tgt_id));
    if (ret < 0)
    {
        CTS_THP_LOGE("Reg chip_cfg failed");
        return CTS_SPI_TARGET_INVALID_ID;
    }

    if (tgt_id == CTS_SPI_TARGET_MASTER)
    {
        return tgt_id;
    }
    else if (tgt_id == CTS_SPI_TARGET_SLAVE)
    {
        return tgt_id;
    }
    else
    {
        CTS_THP_LOGE("Unknow chip_cfg value: %02x", tgt_id);
        return CTS_SPI_TARGET_INVALID_ID;
    }
}

int cts_drw_spi_switch(CTS_SPI_TARGET_ENUM tgt)
{
    struct thp_ioctl_spi_xfer_data xfer[2];
    struct thp_ioctl_spi_msg_package msg;
    int ret = -1;


    CTS_THP_LOGD("set target=0x%x[0:slave 1:master 2:all]", tgt);

    if (tgt < CTS_SPI_TARGET_SLAVE || tgt > CTS_SPI_TARGET_ALL)
    {
        CTS_THP_LOGE("ERROR! Invalid spi target");
        return -1;
    }

    memset(xfer, 0, sizeof(xfer));
    memset(&msg, 0, sizeof(msg));

    xfer[0].tx = (char *)&CTS_SWITCH_CODE[tgt];
    xfer[0].rx = NULL;
    xfer[0].len = sizeof(char);
    xfer[0].delay_usecs = 2000;
    xfer[0].cs_change = 1;

    memcpy(&xfer[1], &xfer[0], sizeof(xfer[1]));

    msg.speed_hz = cts_get_spi_speed();
    msg.xfer_num = 1;

    for (int i = 0; i < 2; i++)
    {
        msg.xfer_data= &xfer[i];

        ret = thp_ioctl_multiple_spi_xfer_sync(&msg);

        if (ret < 0)
        {
            CTS_THP_LOGE("Swith target failed: %s", strerror(errno));
            return -1;
        }
    }
    return 0;
}


