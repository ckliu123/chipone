#include "cts_hal.h"
#include "thp/thp_dev_itf.h"

struct thp_ioctl_get_frame_data {
	char 			*buf;
	char 			*tv; /* struct timeval* */
	unsigned int	size;
};

struct thp_ioctl_spi_sync_data {
	char 			*tx;
	char 			*rx;
	unsigned int	 size;
};

/* This "_compat"  is used for compatling android 32bit */
struct thp_ioctl_get_frame_data_compat {
	uint32_t		buf;
	uint32_t		tv; /* struct timeval */
	uint32_t		size;
};

struct thp_ioctl_spi_sync_data_compat {
	uint32_t		tx;
	uint32_t		rx;
	uint32_t		size;
};

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "cts_utils.h"
#include "thp/thp_dev_itf.h"

#define THP_IO_TYPE										(0xB8)
#define THP_IOCTL_CMD_GET_FRAME							_IOWR(THP_IO_TYPE, 0x01, struct thp_ioctl_get_frame_data)
#define THP_IOCTL_CMD_RESET 							_IOW(THP_IO_TYPE, 0x02, uint32_t)
#define THP_IOCTL_CMD_SET_TIMEOUT						_IOW(THP_IO_TYPE, 0x03, uint32_t)
#define THP_IOCTL_CMD_SPI_SYNC							_IOWR(THP_IO_TYPE, 0x04, struct thp_ioctl_spi_sync_data)
#define THP_IOCTL_CMD_FINISH_NOTIFY						_IO(THP_IO_TYPE, 0x05)
#define THP_IOCTL_CMD_SET_BLOCK							_IOW(THP_IO_TYPE, 0x06, uint32_t)
#define THP_IOCTL_CMD_SET_IRQ							_IOW(THP_IO_TYPE, 0x07, uint32_t)
#define THP_IOCTL_CMD_GET_FRAME_COUNT					_IOW(THP_IO_TYPE, 0x08, uint32_t)
#define THP_IOCTL_CMD_CLEAR_FRAME_BUFFER				_IOW(THP_IO_TYPE, 0x09, uint32_t)
#define THP_IOCTL_CMD_GET_IRQ_GPIO_VALUE				_IOW(THP_IO_TYPE, 0x0A, uint32_t)
#define THP_IOCTL_CMD_SET_SPI_SPEED						_IOW(THP_IO_TYPE, 0x0B, uint32_t)
#define THP_IOCTL_CMD_SPI_SYNC_SSL_BL					_IOWR(THP_IO_TYPE, 0x0c, struct thp_ioctl_spi_sync_data)
#define THP_IOCTL_CMD_SET_AFE_STATUS					_IOW(THP_IO_TYPE, 0x0d, struct thp_ioctl_set_afe_status)
#define THP_IOCTL_CMD_MUILTIPLE_SPI_XFRE_SYNC			_IOWR(THP_IO_TYPE, 0x0e, struct thp_ioctl_spi_msg_package)
#define THP_IOCTL_CMD_HW_LOCK							_IOW(THP_IO_TYPE, 0x0f, uint32_t)
#define THP_IOCTL_CMD_SPI_SYNC_NO_LOCK					_IOWR(THP_IO_TYPE, 0x10, struct thp_ioctl_spi_sync_data)
#define THP_IOCTL_CMD_MUILTIPLE_SPI_XFRE_SYNC_NO_LOCK	_IOWR(THP_IO_TYPE, 0x11, struct thp_ioctl_spi_msg_package)
#define THP_IOCTL_CMD_GET_WORK_STATUS					_IOWR(THP_IO_TYPE, 0x12, uint32_t)

#define THP_IOCTL_CMD_GET_FRAME_COMPAT					_IOWR(THP_IO_TYPE, 0x01, struct thp_ioctl_get_frame_data_compat)
#define THP_IOCTL_CMD_SPI_SYNC_COMPAT					_IOWR(THP_IO_TYPE, 0x04, struct thp_ioctl_spi_sync_data_compat)

#define THP_DEV_FILE		"/dev/thp"


/*************************************************************/
/* ATTENTION: Not all ioctl return value is less than 0.     */
/*            Force return 0(Sucess)/-1(Failed) to caller.   */
/*************************************************************/

static int s_dev_fd = -1;

int thp_dev_open(void)
{
	int ret = -1;

	if (s_dev_fd < 0) {
		s_dev_fd = open(THP_DEV_FILE, O_RDWR);
		if (s_dev_fd < 0) {
			s_dev_fd = -1;
			All_LOG("Thp device open failed: path %s, %s", THP_DEV_FILE, strerror(errno));
			ret = -1;
		} else {
			ret = 0;
		}
	}

	All_LOG("fd=%d", s_dev_fd);
	return ret;
}

int thp_dev_close(void)
{
	int ret = -1;

	if (s_dev_fd >= 0) {
		THP_LOGI("CLOSE");
		close(s_dev_fd);
		s_dev_fd = -1;
	}
	ret = 0;

	THP_LOGI("fd=%d", s_dev_fd);
	return ret;
}

int thp_dev_get_frame(uint8_t *framebuf, size_t framelen, TIMEVAL_STRUCT *tv)
{
	int ret = -1;
	struct thp_ioctl_get_frame_data ioctl_frame_data;

	ioctl_frame_data.buf = (char *)framebuf;
	ioctl_frame_data.tv = (char *)tv;
	ioctl_frame_data.size = framelen;

	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_GET_FRAME, ioctl_frame_data);
	if (ret != 0) {
		return errno;
		//ret = -1;
	} else {
		ret = 0;
	}

	return ret;
}

int thp_dev_reset(uint8_t level)
{
	int ret = -1;
	THP_LOGI("level=%d", level);

	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_RESET, level);
	if (ret < 0) {
		THP_LOGE("Thp ioctl RESET with %d failed: %s", level, strerror(errno));
		ret = -1;
	} else {
		ret = 0;
	}

	return ret;
}

int thp_dev_set_timeout(uint32_t timeout)
{
	int ret = -1;
	THP_LOGI("timeout=%d", timeout);

	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_TIMEOUT, timeout);
	if (ret < 0) {
		THP_LOGE("Thp ioctl SET_TIMEOUT with %d failed: %s", timeout, strerror(errno));
		ret = -1;
	} else {
		ret = 0;
	}

	return ret;
}

int thp_dev_set_afe_status(void)
{
	int ret = -1;

	struct thp_ioctl_set_afe_status afe_status;
	afe_status.type = 2;
	afe_status.status = 0;
	afe_status.parameter = 0;

	THP_LOGI("thp_dev_set_afe_status +");

	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_AFE_STATUS, &afe_status);
	if (ret < 0) {
		THP_LOGE("Thp dev set afe status with failed: %d", ret);
		ret = -1;
	} else {
		ret = 0;
	}

	return ret;
}

int thp_dev_spi_sync(uint8_t *tx_buf, uint8_t *rx_buf, size_t total_len)
{
	int ret = -1;
	struct thp_ioctl_spi_sync_data spi_sync_data;
	// THP_LOGD("len=%d", total_len);

	spi_sync_data.rx = (char *)rx_buf;
	spi_sync_data.tx = (char *)tx_buf;
	spi_sync_data.size = total_len;

	//cts_dump_spi_tx(tx_buf, total_len);
	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SPI_SYNC, spi_sync_data);
	if (ret) {
		THP_LOGE("Thp ioctl SPI_SYNC failed: %s", strerror(errno));
		ret = -1;
	} else {
		//cts_dump_spi_rx(rx_buf, total_len);
		ret = 0;
	}

	return ret;
}

int thp_dev_finish_notify(void)
{
	int ret = -1;
	THP_LOGI("finish notify");

	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_FINISH_NOTIFY);
	if (ret < 0) {
		THP_LOGE("Thp ioctl FINISH_NOTIFY failed: %s", strerror(errno));
		ret = -1;
	} else {
		ret = 0;
	}

	return ret;
}

/*
	Set block or noblock when getting frame:
		1 THP_GET_FRAME_BLOCK
		0 THP_GET_FRAME_NONBLOCK
*/
int thp_dev_set_block(uint32_t flag)
{
	int ret = -1;
	THP_LOGI("flag=%d", flag);

	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_BLOCK, flag);
	if (ret < 0) {
		THP_LOGE("Thp ioctl SET_BLOCK with %d failed: %s", flag, strerror(errno));
		ret = -1;
	} else {
		ret = 0;
	}

	return ret;
}

int thp_dev_set_irq(uint32_t enable)
{
	int ret = -1;
	THP_LOGI("enable=%d", enable);

	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_IRQ, enable);
	if (ret < 0) {
		THP_LOGE("Thp ioctl SET_IRQ with %d failed: %s", enable, strerror(errno));
		ret = -1;
	} else {
		ret = 0;
	}

	return ret;
}

int thp_dev_get_frame_count(uint32_t *count)
{
	int ret = -1;
	int cnt = 0;

	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_GET_FRAME_COUNT, &cnt);
	if (ret < 0) {
		ret = -1;
	} else {
		*count = cnt;
		THP_LOGI("count=%d", *count);
		ret = 0;
	}

	return ret;
}

/*
	The flag is ommited by huawei thp driver
*/
int thp_dev_clear_frame_buffer(uint32_t flag)
{
	int ret = -1;
	THP_LOGI("flag=%d", flag);

	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_CLEAR_FRAME_BUFFER, flag);
	if (ret < 0) {
		THP_LOGE("Thp ioctl CLEAR_FRAME_BUFFER with %d failed: %s", flag, strerror(errno));
		ret = -1;
	} else {
		ret = 0;
	}

	return ret;
}

int thp_dev_set_spi_speed(uint32_t speed)
{
	int ret = -1;
	THP_LOGI("speed=%d", speed);

	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_SET_SPI_SPEED, speed);
	if (ret < 0) {
		THP_LOGE("Thp ioctl SET_SPI_SPEED with %d failed: %s", speed, strerror(errno));
		ret = -1;
	} else {
		ret = 0;
	}

	return ret;
}

int thp_dev_multiple_spi_xfer_sync(struct thp_ioctl_spi_msg_package *msg)
{
	int ret = -1;
	int i;
	struct thp_ioctl_spi_xfer_data *xfer;

	for (i = 0; i < msg->xfer_num; i++) {
		xfer = &msg->xfer_data[i];
		if (xfer->tx) {
			//cts_dump_spi_tx((uint8_t *)xfer->tx, xfer->len);
		}
	}

	ret = ioctl(s_dev_fd, THP_IOCTL_CMD_MUILTIPLE_SPI_XFRE_SYNC, msg);
	if (ret < 0) {
		THP_LOGE("Thp ioctl MULTIPLE_SPI_XFER_SYNC failed: %s", strerror(errno));
		ret = -1;
	} else {
		ret = 0;
		for (i = 0; i < msg->xfer_num; i++) {
			xfer = &msg->xfer_data[i];
			if (xfer->rx) {
				//cts_dump_spi_rx((uint8_t *)xfer->rx, xfer->len);
			}
		}
	}

	return ret;
}

