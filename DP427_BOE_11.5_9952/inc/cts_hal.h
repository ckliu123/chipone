#ifndef CTS_HAL_H
#define CTS_HAL_H

// check porject ids */
// #if defined(PROJECT_ID_1) || defined(PROJECT_ID_2)
// #else
// #error "PROJECT_ID UNDEFINED!"
// #endif

// both android and sensorhub
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
// hal headers
#include "cts_log.h"
#include "thp_afe_hal.h"

// linux headers
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#ifndef __MUSL__
// android headers
#include <android/log.h>
#else
#include <hilog/log.h>
#endif




/********************************************************************************************
 * Macro: CTS_FOR_RELEASE_VER
 *        @ off->force update firmeare, on->update firmware when version not equel.
 *
 * Macro: CTS_DIFF_SCREEN_STATE_DATA_SIZE
 *        @ transfer different data length according to afe state(whole & 1/3 & 2/3 screen).
 *
 * Macro: CTS_READ_WAFERID_ENABLE
 *        @ get wafer id at cts_prework() when restart device.
 *
 * Macro: MAX_TCS_CMD_NUM
 *        @ cmd nums->set some cmds at cts_get_frame() flow.
 *
 * Macro: CTS_SWITCH_SCAN_STATE_TRACK
 *        @ send scan_state cmd again, when current state unexpected.
 ********************************************************************************************/
#define CTS_FOR_RELEASE_VER

//yjl


#define MAX_TCS_CMD_NUM                     3


/********************************************************************************************
 * Macro: CTS_DEBUG_MODE
 *        @ dump tr_order for short test.
 *
 * Macro: CTS_FOR_TEST_DEBUG
 *        @ dump data array when captest data error.
 ********************************************************************************************/
//#define CTS_DEBUG_MODE
#define CTS_FOR_TEST_DEBUG




/********************************** Captest Macro *******************************************/
//#define TEST_SPI_IIC_COMM
//#define TEST_VERSION
//#define TEST_RESET
//#define TEST_RAWDATA
//#define TEST_NOISE

#define TEST_RAW_ROWS_COLS_DIFF

//#define TEST_OPEN
//#define TEST_SHORT



//#define TEST_HSYNC
//#define TEST_HSYNC_ONLY       /* ON: only hsync test, OFF: flash2reg check & osc trim & hsync test */
//add  yjl -->  huawei test
#define  HUAWEI_TEST_DISBALE_CHIPONE


/********************************************************************************************/
/*********************************** Eflash Macro *******************************************/
#define EFCTL_BASE                          (0x74000)
#define SYS_RESET_FLASH_IP                  (EFCTL_BASE + 0x0003)
#define EFCTL_FLASH_ADDR                    (EFCTL_BASE + 0x0004)
#define EFCTL_SRAM_ADDR                     (EFCTL_BASE + 0x0008)
#define EFCTL_FLASH_RDY                     (EFCTL_BASE + 0x0010)
#define EFCTL_DATA_LENGTH                   (EFCTL_BASE + 0x000C)
#define EFCTL_CRC_RESULT                    (EFCTL_BASE + 0x001C)
#define EFCTL_SW_CRC_START                  (EFCTL_BASE + 0x0020)
#define EFCTL_SF_BUSY                       (EFCTL_BASE + 0x0024)
enum cts_upfw_cmd
{
    MASS_READ           = 1,
    TEST_PAGE_PROGRAM   = 3,
    SECTOR_ERASE        = 4,
    CHIP_ERASE          = 5,
    PAGE_PROGRAM        = 6,
    AUTO_PAGE_PROGRAM   = 7,
};
/********************************************************************************************/

#define TIME_T                              TIMEVAL_STRUCT
#define GET_CURR_TIME()                     cts_gettimeofday()
#define TM2MS(tv)                           cts_tm2ms(tv)
#define TMDIFF2MS(start, end)               cts_tmdiff2ms(start, end)
#define ELAPSED_MS(start)                   cts_elapsedms(start)
#define MSLEEP(us)                          usleep(us * 1000)

static inline TIME_T cts_gettimeofday(void)
{
    TIME_T tm;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    tm.tv_sec = tv.tv_sec;
    tm.tv_usec = tv.tv_usec;

    return tm;
}

static inline long cts_tm2ms(TIME_T tm)
{
    struct timeval *tv = (struct timeval *)&tm;
    return tv->tv_sec * 1000 + tv->tv_usec / 1000;
}

bool cts_dev_state_check(void);
#define SNPRINTF(dst, cnt, fmt, ...)        snprintf(dst, cnt, fmt, ## __VA_ARGS__)
#define MEMSET(dst, val, size)              memset(dst, val, size)
#define MEMCPY(dst, src, size)              memcpy(dst, src, size)
#define MALLOC(size)                        malloc(size)
#define LFREE(ptr)                          free(ptr)


// mutex lock
extern unsigned char g_lock;
#define mutext_lock() \
do { \
    while (g_lock) { \
        MSLEEP(1); \
    }\
    g_lock = 1; \
} while (0)

#define mutext_unlock() \
g_lock = 0;


#endif /* CTS_HAL_H */
