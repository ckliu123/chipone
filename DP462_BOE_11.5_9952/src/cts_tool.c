#include "cts_core.h"

#ifndef PROJECT_ID
#error "PROJECT_ID UNDEFINED!"
#endif

#include "cts_log.h"
#include "cts_tcs.h"
#include "cts_utils.h"
#include "cts_tool.h"
#include "thp_ioctl.h"

#define PORT                       9951
#define BUF_SIZE                   1024
#define HEADER_LEN                 10
#define HEADER_READ_HW_REG         "READHWREG "
#define HEADER_WRITE_HW_REG        "WRITEHWREG"
#define HEADER_READ_REG            "READREG   "
#define HEADER_WRITE_REG           "WRITEREG  "
#define HEADER_RAWDATA             "RAWDATA   "
#define HEADER_DIFFDATA            "DIFFDATA  "
#define HEADER_STOP                "STOPDATA  "
#define HEADER_SAVE_LOG            "SAVELOG   "
#define HEADER_CLOSE_LOG           "CLOSELOG  "
#define HEADER_RESET_SET_LOW       "RESETLOW  "
#define HEADER_RESET_SET_HIGH      "RESETHIGH "
#define HEADER_STYLUS              "STYLUS    "

#define HEADER_START_GESTURE_DATA  "ENGESTURE "
#define HEADER_STOP_GESTURE_DATA   "DISGESTURE"

#define HEADER_CLASSID_INDEX       (HEADER_LEN)
#define HEADER_CMDID_INDEX         (HEADER_LEN + 1)
#define HEADER_LENH_INDEX          (HEADER_LEN + 2)
#define HEADER_LENL_INDEX          (HEADER_LEN + 3)

#define HEADER_HWREG_ADDR_INDEX       (HEADER_LEN)
#define HEADER_HWREG_READ_LEN_INDEX   (HEADER_LEN + 4)

static pthread_t pthid;
static int socket_fd;
static int client_fd;
static int stop_get_rawdata_flag = 0;

static uint8_t header[24] =
{
    'C', 'h', 'i', 'p', 'o', 'n', 'e', 0x10, // header, len
    0x00, 0x00, 0x01, 0x00,                  // version
    0x01, 0x00, ROWS, COLS,                  // type, method, rows, cols
    0x00, 0x00, 0x00, 0x00,                  // frame count
    0x00, 0x00, 0x00, 0x00                   // total time
};
static uint32_t frame_cnt = 0;
static FILE *logfile;
static char out_filepath[256] = "";
static time_t calendar_time;
static struct tm *local_time = NULL;

static pthread_t gesture_pthid;
static uint16_t rawdata[RAWDATA_NODES];
static int gesture_flag = 0;
// static uint8_t frame_data[FRAME_SIZE_HAS_TAIL - 3];

static void cts_tool_read_hw_reg(uint8_t *msg)
{
    //msg: HEADER + addr(4) + len(2)
    uint32_t addr;
    size_t len;
    uint8_t *rbuf;
    int ret;

    addr = msg[HEADER_HWREG_ADDR_INDEX] << 24
           | msg[HEADER_HWREG_ADDR_INDEX + 1] << 16
           | msg[HEADER_HWREG_ADDR_INDEX + 2] << 8
           | msg[HEADER_HWREG_ADDR_INDEX + 3] << 0;
    len = msg[HEADER_HWREG_READ_LEN_INDEX] << 8
          | msg[HEADER_HWREG_READ_LEN_INDEX + 1];

    rbuf = malloc(len);
    if (rbuf == NULL)
    {
        CTS_THP_LOGE("malloc failed!!");
        return;
    }

    ret = cts_tcs_read_hw_reg(addr, rbuf, len);
    if (ret < 0)
    {
        CTS_THP_LOGE("read hw reg:0x%x failed!!", addr);
    }
    else
    {
        write(client_fd, rbuf, len);
    }
}

static void cts_tool_write_hw_reg(uint8_t *msg)
{
    //msg: HEADER + addr(4) + len(1) + x1 + x2..
    uint32_t addr;
    uint16_t len;
    int ret;

    addr = msg[HEADER_HWREG_ADDR_INDEX] << 24
           | msg[HEADER_HWREG_ADDR_INDEX + 1] << 16
           | msg[HEADER_HWREG_ADDR_INDEX + 2] << 8
           | msg[HEADER_HWREG_ADDR_INDEX + 3] << 0;
    len = msg[HEADER_HWREG_READ_LEN_INDEX];
    //ret = cts_tcs_write_hw_reg(addr, msg + 5, len);
    ret = cts_tcs_write_hw_reg(addr, &msg[HEADER_HWREG_READ_LEN_INDEX+1], len);
    if (ret < 0)
    {
        CTS_THP_LOGE("Write hw reg:0x%x failed!!", addr);
    }
}

static void cts_tool_read_tcs_reg(uint8_t *msg)
{
    uint8_t send_buf[BUF_SIZE] = {0};
    int classid, cmdid, len;
    int ret;

    classid = msg[HEADER_CLASSID_INDEX];
    cmdid = msg[HEADER_CMDID_INDEX];
    len = msg[HEADER_LENH_INDEX] << 8 | msg[HEADER_LENL_INDEX];
    CTS_THP_LOGE("read reg classid:0x%02x, cmdid:0x%02x, len:0x%02x",
             classid, cmdid, len);
    ret = cts_tcs_read_spi_for_tool(classid, cmdid, send_buf, len);
    if (ret < 0)
    {
        CTS_THP_LOGE("read failed!!");
    }
    else
    {
        CTS_THP_LOGE("send client");
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
    CTS_THP_LOGD("write reg classid:0x%02x, cmdid:0x%02x, len:0x%02x",
             classid, cmdid, len);
    memcpy(send_buf, msg + HEADER_LEN + 4, len);
    for (i = 0; i < len; i++)
    {
        CTS_THP_LOGD("write data: %02x", send_buf[i]);
    }
    ret = cts_tcs_write_spi_for_tool(classid, cmdid, send_buf, len);
    if (ret < 0)
    {
        CTS_THP_LOGD("write failed!!");
    }
}

static void cts_send_gesture_data(uint8_t *data, size_t len)
{
    write(client_fd, data, len);
}
static void *cts_get_gesture_data(void *arg)
{
    // int type = (int *)arg;
    int ret;
    while (gesture_flag)
    {
        ret = cts_tcs_polling_data(4, (uint8_t *)rawdata, RAWDATA_NODES * 2);
        if (ret)
        {
            CTS_THP_LOGD("Polling gesture data failed!!");
            // break;
        }
        // mdelay(3);
    }
    cts_send_gesture_data((uint8_t *)rawdata, RAWDATA_NODES * 2);

    return NULL;
}

void cts_tool_start_gesture_data_thread()
{
    if (0 == gesture_flag)
    {
        CTS_THP_LOGD("Start gesture data");
        gesture_flag = 1;
        pthread_create(&gesture_pthid, 0, cts_get_gesture_data, NULL);
    }
    else
    {
        CTS_THP_LOGD("gesture thread already exist");
    }
}

void cts_tool_stop_gesture_data_thread()
{
    gesture_flag = 0;
}

static int cts_bind_socket(void)
{
    struct sockaddr_in addr;
    int soclen = sizeof(addr);
    // struct sockaddr_in clientAddr;
    // int clientLen = sizeof(clientAddr);
    // int read_len;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        CTS_THP_LOGD("created socket error");
        return -1;
    }

	int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // ÔÊÐíµØÖ·¸´ÓÃ
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port  = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (-1 == bind(socket_fd, (struct sockaddr*)&addr, soclen))
    {
        CTS_THP_LOGD("bind socket fail");
        close(socket_fd);
        return -1;
    }

    if (-1 == listen(socket_fd, 10))
    {
        CTS_THP_LOGD("listen socket fail");
        return -1;
    }
    return 0;
}

void *cts_tool_thread(void *arg)
{
    // struct sockaddr_in addr;
    // int soclen = sizeof(addr);
    struct sockaddr_in clientAddr;
    int clientLen = sizeof(clientAddr);
    uint8_t client_msg[BUF_SIZE];
    int read_len;

    // socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    // if (socket_fd == -1){
    //  CTS_THP_LOGD("created socket error");
    //  return NULL;
    // }

    // addr.sin_family = AF_INET;
    // addr.sin_port  = htons(PORT);
    // addr.sin_addr.s_addr = INADDR_ANY;

    // if (-1 == bind(socket_fd, (struct sockaddr*)&addr, soclen)){
    //  CTS_THP_LOGD("bind socket fail");
    //  close(socket_fd);
    //  return NULL;
    // }

    // if (-1 == listen(socket_fd, 10)) {
    //  CTS_THP_LOGD("listen socket fail");
    //  return NULL;
    // }
    if (cts_bind_socket())
    {
        CTS_THP_LOGE("bind socket failed");
        return NULL;
    }

next:
    CTS_THP_LOGD("server waiting connect...");
    client_fd = accept(socket_fd, (struct sockaddr*)&clientAddr,
                       (socklen_t *)&clientLen);
    if (client_fd < 0)
    {
        CTS_THP_LOGD("server accept failed!");
        goto next;
    }

    while ((read_len = read(client_fd, client_msg, BUF_SIZE)) > 0)
    {
        client_msg[read_len] = '\0';
        //
        if (memcmp(client_msg, HEADER_STYLUS, HEADER_LEN) == 0)
        {
            stop_get_rawdata_flag = 0;
        }
        if (memcmp(client_msg, HEADER_RAWDATA, HEADER_LEN) == 0)
        {
            CTS_THP_LOGD("get rawdata");
            stop_get_rawdata_flag = 0;
        }
        if (memcmp(client_msg, HEADER_DIFFDATA, HEADER_LEN) == 0)
        {
            CTS_THP_LOGD("get diffdata");
            stop_get_rawdata_flag = 0;
        }
        if (memcmp(client_msg, HEADER_STOP, HEADER_LEN) == 0)
        {
            CTS_THP_LOGD("stop rawdata");
            stop_get_rawdata_flag = 0;
        }
        if (memcmp(client_msg, HEADER_READ_HW_REG, HEADER_LEN) == 0)
        {
            CTS_THP_LOGD("read hw reg");
            cts_tool_read_hw_reg(client_msg);
        }
        if (memcmp(client_msg, HEADER_WRITE_HW_REG, HEADER_LEN) == 0)
        {
            CTS_THP_LOGD("write hw reg");
            cts_tool_write_hw_reg(client_msg);
        }
        if (memcmp(client_msg, HEADER_READ_REG, HEADER_LEN) == 0)
        {
            CTS_THP_LOGE("read reg");
            cts_tool_read_tcs_reg(client_msg);
        }
        if (memcmp(client_msg, HEADER_WRITE_REG, HEADER_LEN) == 0)
        {
            CTS_THP_LOGE("write reg");
            cts_tool_write_tcs_reg(client_msg);
        }
        if (memcmp(client_msg, HEADER_RESET_SET_LOW, HEADER_LEN) == 0)
        {
            CTS_THP_LOGD("reset low");
            thp_ioctl_reset(0);
        }
        if (memcmp(client_msg, HEADER_RESET_SET_HIGH, HEADER_LEN) == 0)
        {
            CTS_THP_LOGD("reset high");
            thp_ioctl_reset(1);
        }
        if (memcmp(client_msg, HEADER_SAVE_LOG, HEADER_LEN) == 0)
        {
            if (logfile)
                break;

            CTS_THP_LOGD("save log");
            calendar_time = time(NULL);
            local_time = localtime(&calendar_time);
            snprintf(out_filepath, sizeof(out_filepath),
                     "/sdcard/chipone_frame_data_%04d%02d%02d_%02d%02d%02d.bin",
                     //"/sdcard/chipone_frame_data_%04d%02d%02d_%02d%02d%02d.csv",
                     local_time->tm_year + 1900,
                     local_time->tm_mon + 1,
                     local_time->tm_mday,
                     local_time->tm_hour,
                     local_time->tm_min,
                     local_time->tm_sec);
            logfile = fopen(out_filepath, "wb+");
            //logfile = fopen(out_filepath, "w+");
            if (logfile == NULL)
            {
                CTS_THP_LOGE("Open touch data log file '%s' failed %d(%s)\n",
                         out_filepath, errno, strerror(errno));
            }
            fwrite(header, 24, 1, logfile);
        }
        if (memcmp(client_msg, HEADER_CLOSE_LOG, HEADER_LEN) == 0)
        {
            if (logfile)
            {
                CTS_THP_LOGD("close log");
                fseek(logfile, 16, SEEK_SET);
                fwrite(&frame_cnt, 4, 1, logfile);
                fclose(logfile);
                logfile = NULL;
            }
        }
        if (memcmp(client_msg, HEADER_START_GESTURE_DATA, HEADER_LEN) == 0)
        {
            cts_tool_start_gesture_data_thread();
        }
        if (memcmp(client_msg, HEADER_STOP_GESTURE_DATA, HEADER_LEN) == 0)
        {
            cts_tool_stop_gesture_data_thread();
        }
        break;
    }

    goto next;
    close(client_fd);
    close(socket_fd);
    return NULL;
}

void cts_tool_send_data_to_client(CTS_FRAME_STRUCT *cts_frame)
{
    uint8_t *data = (uint8_t *)cts_frame;
    uint16_t crc = cts_crc16(data, FRAME_SIZE);
    if (!stop_get_rawdata_flag)
    {
        data[FRAME_SIZE] = crc;
        data[FRAME_SIZE + 1] = crc >> 8;
        write(client_fd, cts_frame, FRAME_SIZE_HAS_TAIL - 3);
    }
}

void cts_tool_save_frame_data(CTS_FRAME_STRUCT *cts_frame)
{
    if (logfile)
    {
        frame_cnt += 1;
        fwrite(cts_frame, FRAME_SIZE, 1, logfile);
    }
}

void cts_tool_start_thread()
{
    pthread_create(&pthid, 0, cts_tool_thread, NULL);
}


