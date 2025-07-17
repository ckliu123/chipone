#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wunused-variable"

#include "cts_core.h"
#include "cts_log.h"
#include "cts_utils.h"
#include "thp_ioctl.h"
#include "cts_tcs.h"

struct thp_ioctl_get_frame_data
{
    char            *buf;
    char            *tv; /* struct timeval* */
    unsigned int    size;
};

struct thp_ioctl_spi_sync_data
{
    char            *tx;
    char            *rx;
    unsigned int    size;
};

/* This "_compat"  is used for compatling android 32bit */
struct thp_ioctl_get_frame_data_compat
{
    uint32_t        buf;
    uint32_t        tv; /* struct timeval */
    uint32_t        size;
};

struct thp_ioctl_spi_sync_data_compat
{
    uint32_t        tx;
    uint32_t        rx;
    uint32_t        size;
};

struct thp_ioctl_set_afe_status
{
    int type;
    int status;
    int parameter;
};

#define THP_IO_TYPE                              (0xB8)
#define THP_IOCTL_CMD_GET_FRAME                  _IOWR(THP_IO_TYPE, 0x01, struct thp_ioctl_get_frame_data)
#define THP_IOCTL_CMD_RESET                      _IOW(THP_IO_TYPE, 0x02, uint32_t)
#define THP_IOCTL_CMD_SET_TIMEOUT                _IOW(THP_IO_TYPE, 0x03, uint32_t)
#define THP_IOCTL_CMD_SPI_SYNC                   _IOWR(THP_IO_TYPE, 0x04, struct thp_ioctl_spi_sync_data)
#define THP_IOCTL_CMD_FINISH_NOTIFY              _IO(THP_IO_TYPE, 0x05)
#define THP_IOCTL_CMD_SET_BLOCK                  _IOW(THP_IO_TYPE, 0x06, uint32_t)
#define THP_IOCTL_CMD_SET_IRQ                    _IOW(THP_IO_TYPE, 0x07, uint32_t)
#define THP_IOCTL_CMD_GET_FRAME_COUNT            _IOW(THP_IO_TYPE, 0x08, uint32_t)
#define THP_IOCTL_CMD_CLEAR_FRAME_BUFFER         _IOW(THP_IO_TYPE, 0x09, uint32_t)
#define THP_IOCTL_CMD_GET_IRQ_GPIO_VALUE         _IOW(THP_IO_TYPE, 0x0A, uint32_t)
#define THP_IOCTL_CMD_SET_SPI_SPEED              _IOW(THP_IO_TYPE, 0x0B, uint32_t)
#define THP_IOCTL_CMD_SET_AFE_STATUS             _IOW(THP_IO_TYPE, 0x0D, struct thp_ioctl_set_afe_status)
#define THP_IOCTL_CMD_MUILTIPLE_SPI_XFRE_SYNC    _IOWR(THP_IO_TYPE, 0x0E, struct thp_ioctl_spi_msg_package)
#define THP_IOCTL_CMD_GET_FRAME_COMPAT           _IOWR(THP_IO_TYPE, 0x01, struct thp_ioctl_get_frame_data_compat)
#define THP_IOCTL_CMD_SPI_SYNC_COMPAT            _IOWR(THP_IO_TYPE, 0x04, struct thp_ioctl_spi_sync_data_compat)
#define THP_DEV_FILE        "/dev/thp"

/*************************************************************/
/* ATTENTION: Not all ioctl return value is less than 0.     */
/*            Force return 0(Sucess)/-1(Failed) to caller.   */
/*************************************************************/

static int s_dev_fd = -1;
int thp_dev_close_check(void)
{
    return s_dev_fd;
}

int thp_dev_open(void)
{
    int ret = -1;
    if (s_dev_fd < 0)
    {
        s_dev_fd = open(THP_DEV_FILE, O_RDWR);
        if (s_dev_fd < 0)
        {
            s_dev_fd = -1;
            CTS_THP_LOGE("Thp device open failed: %s", strerror(errno));
            ret = -1;
        }
        else
        {
            ret = 0;
        }
    }
    CTS_THP_LOGI("thp_dev_open, fd=%d", s_dev_fd);
    return ret;
}

int thp_dev_close(void)
{
    int ret = -1;
    if (s_dev_fd > 0)
    {
        close(s_dev_fd);
        s_dev_fd = -1;
    }
    ret = 0;

    CTS_THP_LOGI("thp_dev_close, fd=%d", s_dev_fd);
    return ret;
}

int  thp_ioctl_hal_set_afe_status(int type, int status, int parameter)
{
    struct thp_ioctl_set_afe_status ioctl_afe_status;
    int ret;
    ioctl_afe_status.type = type;
    ioctl_afe_status.status = status;
    ioctl_afe_status.parameter = parameter;
    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_AFE_STATUS, &ioctl_afe_status);
    if (ret)
    {
        CTS_THP_LOGE("Exit: thp ioctl THP_IOCTL_CMD_SET_AFE_STATUS failed: %s", strerror(errno));
        ret = -1;
    }
    return ret;
}

int thp_dev_set_block(uint32_t flag)
{
    int ret = -1;
    CTS_THP_LOGI("flag=%d", flag);

    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_BLOCK, flag);
    if (ret < 0)
    {
        CTS_THP_LOGE("Thp ioctl SET_BLOCK with %d failed: %s", flag, strerror(errno));
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

int thp_ioctl_get_frame(uint8_t *framebuf, size_t framelen, struct timeval *tv)
{
    int ret = -1;
    struct thp_ioctl_get_frame_data ioctl_frame_data;

    ioctl_frame_data.buf = (char *)framebuf;
    ioctl_frame_data.tv = (char *)tv;
    ioctl_frame_data.size = framelen;

    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_GET_FRAME, ioctl_frame_data);

#if 0//def DEBUG_DUMP_IOCTL_SPI_FULL_DATA
    CTS_THP_LOGI("Enter, size=%ld", framelen);
    CTS_THP_LOGI("Exit: tv=%lld", tv->tv_sec * 1000 + tv->tv_usec / 1000);
    if (framelen < 200)
        dump_spi_full_data(framebuf, framelen);
    else
        dump_spi_full_data(framebuf, 200);
#endif

    if (ret)
    {
        CTS_THP_LOGE("Exit: thp ioctl GET_FRAME failed: %s", strerror(errno));
    }

    return ret;
}

int thp_ioctl_reset(uint8_t level)
{
    int ret = -1;
    CTS_THP_LOGI("Enter, level=%d", level);
    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_RESET, level);
    if (ret < 0)
    {
        CTS_THP_LOGE("Thp ioctl RESET with %d failed: %s", level, strerror(errno));
        ret = -1;
    }
    else
    {
        ret = 0;
    }
    return ret;
}

int thp_ioctl_set_timeout(uint32_t timeout)
{
    int ret = -1;
    //CTS_THP_LOGI("Enter, timeout=%d", timeout);

    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_TIMEOUT, timeout);
    if (ret < 0)
    {
        CTS_THP_LOGE("Thp ioctl SET_TIMEOUT with %d failed: %s", timeout, strerror(errno));
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    //CTS_THP_LOGI("Exit");
    return ret;
}

int thp_ioctl_spi_sync(uint8_t *tx_buf, uint8_t *rx_buf, size_t total_len)
{
    int ret = -1;
    struct thp_ioctl_spi_sync_data spi_sync_data;
    // CTS_THP_LOGI("Enter, len=%d", total_len);

    spi_sync_data.rx = (char *)rx_buf;
    spi_sync_data.tx = (char *)tx_buf;
    spi_sync_data.size = total_len;

#ifdef DEBUG_DUMP_IOCTL_SPI_FULL_DATA
    // uint32_t len = (tx_buf[4]<<16 | tx_buf[5]<<8 | tx_buf[6]  ) ;
    //CTS_THP_LOGI("Enter11111, len=%d,%d", total_len,len);

    // if (  len   < 50 )
    dump_spi_full_data(tx_buf, total_len);
    // else
    //   dump_spi_full_data(tx_buf, 50);
#endif
    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SPI_SYNC, spi_sync_data);
    /* Warning: WTF! ret is 1 if tx_buf is NULL */
    if (ret)
    {
        CTS_THP_LOGE("Thp ioctl SPI_SYNC failed: %s", strerror(errno));
        ret = -1;
    }
    else
    {
#ifdef DEBUG_DUMP_IOCTL_SPI_FULL_DATA
        //  CTS_THP_LOGI("Enter22222, len=%d,%d", total_len,len);

        //  if (  len   < 50 )
        dump_spi_full_data(rx_buf, total_len);
        //  else
        //     dump_spi_full_data(rx_buf, 50);

#endif
        ret = 0;
    }

    return ret;
}

int thp_ioctl_finish_notify(void)
{
    int ret = -1;
    CTS_THP_LOGI("Enter");

    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_FINISH_NOTIFY);
    if (ret < 0)
    {
        CTS_THP_LOGE("Thp ioctl FINISH_NOTIFY failed: %s", strerror(errno));
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    CTS_THP_LOGI("Exit");
    return ret;
}

/*
    Set block or noblock when getting frame:
        1 THP_GET_FRAME_BLOCK
        0 THP_GET_FRAME_NONBLOCK
*/
int thp_ioctl_set_block(uint32_t flag)
{
    int ret = -1;
    CTS_THP_LOGI("Enter, flag=%d", flag);

    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_BLOCK, flag);
    if (ret < 0)
    {
        CTS_THP_LOGE("Thp ioctl SET_BLOCK with %d failed: %s", flag, strerror(errno));
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    CTS_THP_LOGI("Exit");
    return ret;
}

int thp_ioctl_set_irq(uint32_t enable)
{
    int ret = -1;
    CTS_THP_LOGI("Enter, enable=%d", enable);

    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_IRQ, enable);
    if (ret < 0)
    {
        CTS_THP_LOGE("Thp ioctl SET_IRQ with %d failed: %s", enable, strerror(errno));
        ret = -1;
    }
    else
    {
        ret = 0;
    }
    return ret;
}

int thp_ioctl_get_frame_count(uint32_t *count)
{
    int ret = -1;
    int cnt = 0;

    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_GET_FRAME_COUNT, &cnt);
    if (ret < 0)
    {
        CTS_THP_LOGE("Thp ioctl GET_FRAME_COUNT failed: %s", strerror(errno));
        ret = -1;
    }
    else
    {
        ret = 0;
        *count = cnt;
    }
    return ret;
}
/*
    The flag is ommited by huawei thp driver
*/
int thp_ioctl_clear_frame_buffer(uint32_t flag)
{
    int ret = -1;
    //LOGI("Enter, flag=%d", flag);

    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_CLEAR_FRAME_BUFFER, flag);
    if (ret < 0)
    {
        CTS_THP_LOGE("Thp ioctl CLEAR_FRAME_BUFFER with %d failed: %s", flag, strerror(errno));
        ret = -1;
    }
    else
    {
        ret = 0;
    }
    return ret;
}

int thp_ioctl_set_spi_speed(uint32_t speed)
{
    int ret = -1;
    CTS_THP_LOGI("Enter, speed=%d", speed);
    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_SPI_SPEED, speed);
    if (ret < 0)
    {
        CTS_THP_LOGE("Thp ioctl SET_SPI_SPEED with %d failed: %s", speed, strerror(errno));
        ret = -1;
    }
    else
    {
        ret = 0;
    }
    return ret;
}

int thp_ioctl_multiple_spi_xfer_sync(struct thp_ioctl_spi_msg_package *msg)
{
    int ret = -1;
    int i;
    struct thp_ioctl_spi_xfer_data *xfer;
    //CTS_THP_LOGD("Enter, %d xfers", msg->xfer_num);
#ifdef DUMP_SPI
    for (i = 0; i < msg->xfer_num; i++)
    {
        xfer = &msg->xfer_data[i];
        if (xfer->tx)
        {
            cts_dump_spi(0, (uint8_t *)xfer->tx, xfer->len);
        }
    }
#endif
    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_MUILTIPLE_SPI_XFRE_SYNC, msg);
    if (ret < 0)
    {
        CTS_THP_LOGE("Thp ioctl MULTIPLE_SPI_XFER_SYNC failed: %s", strerror(errno));
        ret = -1;
    }
    else
    {
        ret = 0;
#ifdef DUMP_SPI
        for (i = 0; i < msg->xfer_num; i++)
        {
            xfer = &msg->xfer_data[i];
            if (xfer->rx)
            {
                cts_dump_spi(1, (uint8_t *)xfer->rx, xfer->len);
            }
        }
#endif        
    }
    return ret;
}

/*
void read_hard_reg()
{
    int ret;
    int i;
    uint8_t number;
    uint8_t go_number[8];
    ret = cts_tcs_read_hw_reg(0x79110, &number, 1);
    CTS_THP_LOGD("SW_FLAG : %d", number);
    if (ret < 0)
    {
        CTS_THP_LOGE("SW_FLAG:0x%x failed!!", number);
    }
    ret = cts_tcs_read_hw_reg(0x79475, &number, 1);
    CTS_THP_LOGD("SCAN_KRANG_EN : %d", number);
    if (ret < 0)
    {
        CTS_THP_LOGE("SCAN_KRANG_EN:0x%x failed!!", number);
    }
    ret = cts_tcs_read_hw_reg(0x7C028, &number, 1);
    CTS_THP_LOGD("Master DDI_R_0A: %x", number);
    if (ret < 0)
    {
        CTS_THP_LOGE("Master DDI_R_0A:0x%x failed!!", number);
    }
    ret = cts_tcs_read_hw_reg(0x87C028, &number, 1);
    CTS_THP_LOGD("Salve DDI_R_0A : %x", number);
    if (ret < 0)
    {
        CTS_THP_LOGE("Salve DDI_R_0A:0x%x failed!!", number);
    }
    ret = cts_tcs_read_hw_reg(0x791F0, go_number, 8);
    for(i=0; i<7; i++)
    {
        CTS_THP_LOGD("SCAN_GO_ERR0_STS/SCAN_GO_ERR1_STS[%d], %x", i,go_number[i]);
    }

    if (ret < 0)
    {
        CTS_THP_LOGE("SCAN_GO_ERR0_ST:0x%x failed!!", number);
    }
    ret = cts_tcs_read_hw_reg(0x7003F, &number, 1);
    CTS_THP_LOGD("DDI_FSM_STATE : %x ", number&0x0F);
    if (ret < 0)
    {
        CTS_THP_LOGE("DDI_FSM_STATEg:0x%x failed!!", number);
    }
}*/

// add lck    2024528
int thp_cts_ioctl_set_afe_status(void)
{
    struct thp_ioctl_set_afe_status status = {2,0, 0};
    int ret = -1;

    CTS_THP_LOGI("Enter");
    ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_AFE_STATUS, &status);
    if (ret < 0)
    {
        CTS_THP_LOGE("Thp ioctl SET_AFE_STATUS failed: %s", strerror(errno));
        ret = -1;
    }
    else
    {
        ret = 0;
    }

    CTS_THP_LOGI("Exit");
    return ret;
}
