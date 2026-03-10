#include "cts_hal.h"
#if defined(PROJECT_ID_1) || defined(PROJECT_ID_2)
#else
#error "PROJECT_ID UNDEFINED!"
#endif

#include "cts_core.h"
#include "cts_log.h"
#include "cts_tcs.h"
#include "cts_tool.h"

#define PORT						9268
#define BUF_SIZE					1024
#define HEADER_LEN					10
#define HEADER_READ_REG				"READREG   "
#define HEADER_WRITE_REG			"WRITEREG  "
#define HEADER_RAWDATA				"RAWDATA   "
#define HEADER_DIFFDATA				"DIFFDATA  "
#define HEADER_STOP					"STOPDATA  "
#define HEADER_SAVE_LOG				"SAVELOG   "
#define HEADER_CLOSE_LOG			"CLOSELOG  "

#define HEADER_CLASSID_INDEX		(HEADER_LEN)
#define HEADER_CMDID_INDEX			(HEADER_LEN + 1)
#define HEADER_LENH_INDEX			(HEADER_LEN + 2)
#define HEADER_LENL_INDEX			(HEADER_LEN + 3)

static pthread_t pthid;
static int socket_fd;
static int client_fd;
static int stop_get_rawdata_flag = 0;

static FILE *logfile;
static char out_filepath[256] = "";
static time_t calendar_time;
static struct tm *local_time = NULL;

typedef enum  {
	FRAME_TYPE_MASK			= 0x00F0,
	FRAME_TYPE_1			= 0x0010,
	FRAME_TYPE_SUB_11		= 0x0011,
	FRAME_TYPE_2			= 0x0020,
	FRAME_TYPE_SUB_21		= 0x0021,
	FRAME_TYPE_SUB_22		= 0x0022,
	FRAME_TYPE_SUB_23		= 0x0023,
	FRAME_TYPE_SUB_24		= 0x0024,
	FRAME_TYPE_SUB_25		= 0x0025,
	FRAME_TYPE_3			= 0x0030,
	FRAME_TYPE_SUB_31		= 0x0031,
} CTS_FRAME_TYPE_ENUM;

static void cts_tool_read_tcs_reg(uint8_t *msg)
{
	uint8_t send_buf[BUF_SIZE] = {0};
	int classid, cmdid, len;
	int ret;

	classid = msg[HEADER_CLASSID_INDEX];
	cmdid = msg[HEADER_CMDID_INDEX];
	len = msg[HEADER_LENH_INDEX] << 8 | msg[HEADER_LENL_INDEX];
	THP_LOGI("read reg classid:0x%02x, cmdid:0x%02x, len:0x%02x",
			classid, cmdid, len);
	ret = cts_tcs_read_spi_for_tool(classid, cmdid, send_buf, len);
	if (ret < 0) {
		THP_LOGI("read failed!!");
	} else {
		write(client_fd, send_buf, len);
	}
}

static void cts_tool_write_tcs_reg(uint8_t *msg)
{
	uint8_t send_buf[BUF_SIZE] = {0};
	int classid, cmdid, len;
	int i;
	int ret;

	classid = msg[HEADER_CLASSID_INDEX];
	cmdid = msg[HEADER_CMDID_INDEX];
	len = msg[HEADER_LENH_INDEX] << 8 | msg[HEADER_LENL_INDEX];
	THP_LOGI("write reg classid:0x%02x, cmdid:0x%02x, len:0x%02x",
			classid, cmdid, len);
	memcpy(send_buf, msg + HEADER_LEN + 4, len);
	for (i = 0; i < len; i++) {
		THP_LOGI("write data: %02x", send_buf[i]);
	}
	ret = cts_tcs_write_spi_for_tool(classid, cmdid, send_buf, len);
	if (ret < 0) {
		THP_LOGI("write failed!!");
	}
}

void *cts_tool_thread(void *arg)
{
	struct sockaddr_in addr;
	int soclen = sizeof(addr);
	struct sockaddr_in clientAddr;
	int clientLen = sizeof(clientAddr);
	uint8_t client_msg[BUF_SIZE];
	int read_len;
	static bool first_time = true;

	if (!first_time) {
		THP_LOGI("No first time, no need socket again!");
		return NULL;
	}

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1){
		THP_LOGI("created socket error");
		return NULL;
	}

	THP_LOGI("Socket_fd: %d", socket_fd);

	addr.sin_family = AF_INET;
	addr.sin_port  = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;
/*
	int sockoptval = 1;
	if (-1 == setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(sockoptval))) {
		THP_LOGI("setsockopt fail %s", strerror(errno));
		close(socket_fd);
		exit(EXIT_FAILURE);
	}
*/
	if (-1 == bind(socket_fd, (struct sockaddr*)&addr, soclen)){
		THP_LOGI("bind socket fail, port: %d(%s)", PORT, strerror(errno));
		close(socket_fd);
		return NULL;
	}

	if (-1 == listen(socket_fd, 10)) {
		THP_LOGI("listen socket fail %s", strerror(errno));
		return NULL;
	}

next:
	first_time = false;
	THP_LOGD("server waiting connect...");
	client_fd = accept(socket_fd, (struct sockaddr*)&clientAddr,
			(socklen_t *)&clientLen);
	if (client_fd < 0) {
		THP_LOGI("server accept failed!");
		goto next;
		// return NULL;
	}

	// THP_LOGD("Accept client from %s:%d", inet_ntoa(clientAddr.sin_addr),
	// clientAddr.sin_port);
	while ((read_len = read(client_fd, client_msg, BUF_SIZE)) > 0) {
		client_msg[read_len] = '\0';
		if (memcmp(client_msg, HEADER_RAWDATA, HEADER_LEN) == 0) {
			THP_LOGI("get rawdata");
			stop_get_rawdata_flag = 0;
		}
		if (memcmp(client_msg, HEADER_DIFFDATA, HEADER_LEN) == 0) {
			THP_LOGI("get diffdata");
			stop_get_rawdata_flag = 0;
		}
		if (memcmp(client_msg, HEADER_STOP, HEADER_LEN) == 0) {
			THP_LOGI("stop rawdata");
			stop_get_rawdata_flag = 1;
		}
		if (memcmp(client_msg, HEADER_READ_REG, HEADER_LEN) == 0) {
			THP_LOGI("read reg");
			cts_tool_read_tcs_reg(client_msg);
		}
		if (memcmp(client_msg, HEADER_WRITE_REG, HEADER_LEN) == 0) {
			THP_LOGI("write reg");
			cts_tool_write_tcs_reg(client_msg);
		}
		if (memcmp(client_msg, HEADER_SAVE_LOG, HEADER_LEN) == 0) {
			if (logfile)
				break;

			THP_LOGI("save log");
			calendar_time = time(NULL);
			local_time = localtime(&calendar_time);
			snprintf(out_filepath, sizeof(out_filepath),
				"/data/cts_data_%04d%02d%02d_%02d%02d%02d.bin",
				local_time->tm_year + 1900,
				local_time->tm_mon + 1,
				local_time->tm_mday,
				local_time->tm_hour,
				local_time->tm_min,
				local_time->tm_sec);
			logfile = fopen(out_filepath, "wb+");
			if(logfile == NULL) {
				THP_LOGE("Open touch data log file '%s' failed %d(%s)\n",
				out_filepath, errno, strerror(errno));
			}
		}
		if (memcmp(client_msg, HEADER_CLOSE_LOG, HEADER_LEN) == 0) {
			if (logfile) {
				THP_LOGI("close log");
				fclose(logfile);
				logfile = NULL;
			}
		}
		break;
	}

	goto next;

	close(client_fd);
	close(socket_fd);
	return NULL;
} 

void cts_tool_start_thread(void)
{
	pthread_create(&pthid, 0, cts_tool_thread, NULL);
}
/*
void cts_tool_send_grid_to_client(CTS_FRAME_STRUCT0 *cts_frame, int len)
{
	if (!stop_get_rawdata_flag) {
		write(client_fd, cts_frame, len);
	}
}

void cts_tool_send_stylus_to_client(CTS_FRAME_STRUCT1 *cts_frame, int len)
{
	if (!stop_get_rawdata_flag) {
		write(client_fd, cts_frame, len);
	}
}
*/
void cts_tool_send_to_client(CTS_FRAME_STRUCT *cts_frame)
{
	CTS_FRAME_STRUCT0 *cts_frame0;
	CTS_FRAME_STRUCT1 *cts_frame1;
	int sel = -1;
	int len = sizeof(CTS_FRAME_STRUCT);

	if (cts_frame->header.frame_type == FRAME_TYPE_SUB_11) {
		cts_frame0 = (CTS_FRAME_STRUCT0 *)cts_frame;
		len = sizeof(CTS_FRAME_STRUCT0);
		sel = 0;
	} else if ((cts_frame->header.frame_type == FRAME_TYPE_SUB_21)
		|| (cts_frame->header.frame_type == FRAME_TYPE_SUB_22)
		|| (cts_frame->header.frame_type == FRAME_TYPE_SUB_23)
		|| (cts_frame->header.frame_type == FRAME_TYPE_SUB_24)
		|| (cts_frame->header.frame_type == FRAME_TYPE_SUB_25)
		|| (cts_frame->header.frame_type == FRAME_TYPE_SUB_31)) {
		cts_frame1 = (CTS_FRAME_STRUCT1 *)cts_frame;
		len = sizeof(CTS_FRAME_STRUCT1);
		sel = 1;
	}
	if (!stop_get_rawdata_flag) {
		if (sel == 0)
			write(client_fd, cts_frame0, len);
		else if (sel == 1)
			write(client_fd, cts_frame1, len);
		else
			write(client_fd, cts_frame, len);
	}
}
/*
void cts_tool_save_frame_grid_data(CTS_FRAME_STRUCT0 *cts_frame, int len)
{
	if (logfile)
		fwrite(cts_frame, len, 1, logfile);
}

void cts_tool_save_frame_stylus_data(CTS_FRAME_STRUCT1 *cts_frame, int len)
{
	if (logfile)
		fwrite(cts_frame, len, 1, logfile);
}
*/
void cts_tool_save_frame_data(CTS_FRAME_STRUCT *cts_frame)
{
	CTS_FRAME_STRUCT0 *cts_frame0;
	CTS_FRAME_STRUCT1 *cts_frame1;
	int sel = -1;
	int len = sizeof(CTS_FRAME_STRUCT);

	if (cts_frame->header.frame_type == FRAME_TYPE_SUB_11) {
		cts_frame0 = (CTS_FRAME_STRUCT0 *)cts_frame;
		len = sizeof(CTS_FRAME_STRUCT0);
		sel = 0;
	} else if ((cts_frame->header.frame_type == FRAME_TYPE_SUB_21)
		|| (cts_frame->header.frame_type == FRAME_TYPE_SUB_22)
		|| (cts_frame->header.frame_type == FRAME_TYPE_SUB_23)
		|| (cts_frame->header.frame_type == FRAME_TYPE_SUB_24)
		|| (cts_frame->header.frame_type == FRAME_TYPE_SUB_25)
		|| (cts_frame->header.frame_type == FRAME_TYPE_SUB_31)) {
		cts_frame1 = (CTS_FRAME_STRUCT1 *)cts_frame;
		len = sizeof(CTS_FRAME_STRUCT1);
		sel = 1;
	}

	if (logfile) {
		if (sel == 0)
			fwrite(cts_frame0, len, 1, logfile);
		else if (sel == 1)
			fwrite(cts_frame1, len, 1, logfile);
		else
			fwrite(cts_frame, len, 1, logfile);
	}
}


