#include "thp_ioctl.h"
#include "cts_log.h"
#include "cts_spi.h"

static uint32_t s_spi_speed = DEFAULT_SPI_SPEED;
static uint8_t tx_buf[INTERNAL_SPI_BUF_SIZ];
static uint8_t rx_buf[INTERNAL_SPI_BUF_SIZ];

int cts_set_spi_speed(uint32_t speed_in_hz)
{
    int ret = -1;
    CTS_THP_LOGI("Enter, set spi speed %ld", speed_in_hz);

    ret = thp_ioctl_set_spi_speed(speed_in_hz);
    if (ret < 0)
    {
        CTS_THP_LOGE("Set spi speed failed: %s", strerror(errno));
        return -1;
    }
    s_spi_speed = speed_in_hz;

    CTS_THP_LOGI("Exit, speed=%ld", speed_in_hz);
    return 0;
}

uint32_t cts_get_spi_speed(void)
{
    return s_spi_speed;
}

int cts_spi_sync_raw(uint8_t *tbuf, size_t tlen, uint8_t *rbuf, size_t rlen)
{
    int ret = -1;
    int total_len;

    total_len = tlen + rlen;

    if (!total_len)
    {
        CTS_THP_LOGE("Invalid tlen or rlen");
        return -1;
    }
    else if (total_len > sizeof(tx_buf))
    {
        CTS_THP_LOGE("Huge buffer");
        return -1;
    }

    memset(tx_buf, 0, total_len);
    memset(rx_buf, 0, total_len);

    if (tbuf && tlen)
    {
        memcpy(tx_buf, tbuf, tlen);
    }

    ret = thp_ioctl_spi_sync(tx_buf, rx_buf, total_len);
    if (ret < 0)
    {
        CTS_THP_LOGE("sync buf failed");
        return -1;
    }

    if (rbuf && rlen)
    {
        memcpy(rbuf, rx_buf + tlen, rlen);
    }

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
    int total_len;

    total_len = cmd_len + data_len + rlen;

    if (!data_len)
    {
        CTS_THP_LOGE("ERROR! use cts_spi_sync_buf instead!");
        return -1;
    }

    if (!total_len)
    {
        CTS_THP_LOGE("Invalid tlen or rlen");
        return -1;
    }
    else if (total_len > sizeof(tx_buf))
    {
        CTS_THP_LOGE("Huge buffer");
        return -1;
    }

    memset(tx_buf, 0, total_len);
    memset(rx_buf, 0, total_len);

    if (cmd && cmd_len)
    {
        memcpy(tx_buf, cmd, cmd_len);
        memcpy(tx_buf + cmd_len, data, data_len);
    }

    ret = thp_ioctl_spi_sync(tx_buf, rx_buf, total_len);
    if (ret < 0)
    {
        CTS_THP_LOGE("sync buf failed");
        return -1;
    }

    if (rbuf && rlen)
    {
        memcpy(rbuf, rx_buf + cmd_len + data_len, rlen);
    }

    return 0;
}


