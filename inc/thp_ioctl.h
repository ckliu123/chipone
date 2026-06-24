#ifndef THP_IOCTL_H
#define THP_IOCTL_H

#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SPI_XFER_DATA_NUM 5
/* thp_ioctl_spi_xfer_data
 * delay_usecs: delay time(us) after each xfer
 * cs_change:
 * 1:cs will be pull up after each xfer;
 * 0:cs keep low state throughout the transmission process.
 */
struct thp_ioctl_spi_xfer_data
{
    char            *tx;
    char            *rx;
    unsigned int    len;
    unsigned short  delay_usecs;
    unsigned char   cs_change;
    unsigned char   reserved[3];
};

/* thp_ioctl_spi_msg_package
 * speed_hz:
 * 0:no need to change transfer speed;
 * others: change spi speed to this value,and restore the original
 * speed at the end of the transfer
 */
struct thp_ioctl_spi_msg_package
{
    unsigned int                    speed_hz;
    unsigned int                    xfer_num;
    unsigned int                    reserved[2];
    struct thp_ioctl_spi_xfer_data  *xfer_data;
};

int thp_dev_close_check(void);
int thp_dev_open(void);
int thp_dev_close(void);
int thp_ioctl_hal_set_afe_status(int type, int status, int parameter);
int thp_dev_set_block(uint32_t flag);
int thp_ioctl_get_frame(uint8_t *framebuf, size_t framelen, struct timeval *tv);
int thp_ioctl_reset(uint8_t level);
int thp_ioctl_set_timeout(uint32_t timeout);
int thp_ioctl_spi_sync(uint8_t *tx_buf, uint8_t *rx_buf, size_t total_len);
int thp_ioctl_finish_notify(void);
int thp_ioctl_set_block(uint32_t flag);
int thp_ioctl_set_irq(uint32_t enable);
int thp_ioctl_get_frame_count(uint32_t *count);
int thp_ioctl_clear_frame_buffer(uint32_t flag);
int thp_ioctl_set_spi_speed(uint32_t speed);
int thp_ioctl_multiple_spi_xfer_sync(struct thp_ioctl_spi_msg_package *msg);

int thp_cts_ioctl_set_afe_status(void);
//void read_hard_reg();

#ifdef __cplusplus
}
#endif
#endif /* THP_IOCTL_H */

