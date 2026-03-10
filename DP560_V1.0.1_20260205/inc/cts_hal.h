#ifndef CTS_HAL_H
#define CTS_HAL_H

#define PROJECT_ID_1		"B560D81100"		// BOE
#define PROJECT_ID_2		"P503D91302"


#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// hal headers
#include "cts_log.h"
#include "thp/thp_afe_hal.h"
#include "cts_pattern.h"
#include "cts_chip.h"

// linux headers
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

//////////////////////////////////////////////////////////////////////////////////////////////
#define SPI_SPEED							20000000
#define SPI_SPEED_PROG						8000000

//////////////////////////////////////////////////////////////////////////////////////////////
#define ARRAY_SIZE(arr)					(sizeof(arr) / sizeof((arr)[0]))
#define SEND_SIZ_8K						(8 * 1024)
#define SEND_SIZ_4K						(4 * 1000)
#define SEND_SIZ_3K						(3 * 1024)
#define SEND_SIZ_2K						(2 * 1024)
#define SEND_SIZ_1K						(1 * 1024)
static int send_fw_step[] =
	{ SEND_SIZ_8K, SEND_SIZ_4K, SEND_SIZ_3K, SEND_SIZ_2K, SEND_SIZ_1K };

//////////////////////////////////////////////////////////////////////////////////////////////


/********************************************************************************************
 * Macro: CTS_BYTES_ALIGN
 *        @ Bytes align.
 ********************************************************************************************/
#define CTS_BYTES_ALIGN


/********************************************************************************************
 * Macro: CTS_FOR_RELEASE_VER
 *        @ off->force update firmeare, on->update firmware when version not equel.
 *
 * Macro: CTS_READ_WAFERID_ENABLE
 *        @ get wafer id at cts_prework() when restart device.
 * 
 * Macro: MAX_TCS_CMD_NUM
 *        @ cmd nums->set some cmds at cts_get_frame() flow.
 *
 * Macro: CTS_SWITCH_SCAN_STATE_TRACK
 *        @ send scan_state cmd again, when current state unexpected.
 *
 * Macro: CTS_DIFF_SCREEN_STATE_DATA_SIZE
 *        @ transfer different data length according to afe state(whole & 1/3 & 2/3 screen).
 ********************************************************************************************/
#define CTS_FOR_RELEASE_VER
//#define CTS_READ_WAFERID_ENABLE
#define MAX_TCS_CMD_NUM						3
//#define CTS_SWITCH_SCAN_STATE_TRACK
//#define CTS_DIFF_SCREEN_STATE_DATA_SIZE


/********************************************************************************************
 * Macro: CTS_DEBUG_MODE
 *        @ dump tr_order for short test.
 * 
 * Macro: CTS_FOR_TEST_DEBUG
 *        @ dump data array when captest data error.
 * Macro: DEBUG_SOCKET_TOOL
 *        @ Toolbox.
 *
 * Macro: CTS_ERR_DUMP_MAX_SIZ
 *        @ Dump max data length for debugging when communication error.
 ********************************************************************************************/
//#define CTS_DEBUG_MODE
#define CTS_FOR_TEST_DEBUG
//#define DEBUG_SOCKET_TOOL
#define CTS_ERR_DUMP_MAX_SIZ				100



/************************************ FFT Macro *********************************************/
//#define CTS_FOR_FFT_MODE
#ifdef CTS_FOR_FFT_MODE
#define CTS_FFT_WORK_MODE					(0x04)
#define CTS_FFT_MODE_ENABLE					(0x01)
#define CTS_FFT_DATA_GET_TIMES   			50
#endif
/********************************************************************************************/


/********************************** Captest Macro *******************************************/
//#define TEST_SPI_IIC_COMM
#define TEST_RESET
#define TEST_RAWDATA
#define TEST_OPEN
#define TEST_SHORT
//#define TEST_NOISE
#define TEST_HSYNC
#define TEST_HSYNC_ONLY		/* ON: only hsync test, OFF: flash2reg check & osc trim & hsync test */
//#define TEST_APCLK
/********************************************************************************************/


/********************************************************************************************
 * Macro: CTS_READ_FLASH_SECTION_ENABLE
 *        @ get flash2reg section value from flash&reg at cts_inspect_hsync() when do captest.
 *          depends on THEST_HSYNC macro.
 ********************************************************************************************/
//#define TEST_HSYNC_TYPICAL_305

#ifdef TEST_HSYNC_TYPICAL_305
#define TEST_HSYNC_OSC_TRIM_TYPICAL			305			//292
#else
#define TEST_HSYNC_OSC_TRIM_TYPICAL			292			//292
#endif
#define TEST_HSYNC_OSC_TRIM_MAX_RATIO		300			/* permillage, upper thresh */
#ifdef TEST_HSYNC
//#define CTS_READ_FLASH_SECTION_ENABLE
#define TEST_HSYNC_OSC_TRIM_RETRY			5
#define TEST_HSYNC_OSC_TRIM_MIN_RATIO		5//10			/* permillage, lower thresh */
#define TEST_HSYNC_OSC_TRIM_MIN_RATIO_1		5//15			/* permillage, lower thresh */
#define CTS_CP_FT_TRIM_INVERT_ADDR			0x30C00
#define CTS_CP_FT_TRIM_ADDR					0x30FD8
#define CTS_FLASH_2_REG_INFO_START_ADDR		0x30FE8
#define CTS_FLASH_2_REG_START_ADDR			0x31000
#define CTS_FLASH_2_REG_INFO_LAST_ADDR		0x31040
#define CTS_PROJECT_ID_ADDR					0x31400
//#define CTS_OSC_TRIM_CALI_INFO				0x31700			/* 已废弃 0x31700~0x317DF 存放整机校准信息：是否校准(OK/NG)，校准次数及校准次数上限等信息 */
#define CTS_FLASH_2_REG_ADDR				0x317EC
#define CTS_HSYNC_TRIM_ADDR					0x31800			/* 当前已废弃 */
#define CTS_OSC_TRIM_CALI_INFO				0x31800			/* 存放整机校准信息：是否校准(OK/NG)，校准次数等信息 */
#endif


enum cts_apclk_osctrim_cmd {
	AP_CLK_OFF			= 0,
	AP_CLK_ON			= 1,
	OSC_TRIM_INFO		= 2,
};

#define TIME_T								TIMEVAL_STRUCT
#define GET_CURR_TIME()						cts_gettimeofday()
#define TM2MS(tv)							cts_tm2ms(tv)
#define TMDIFF2MS(start, end)				cts_tmdiff2ms(start, end)
#define ELAPSED_MS(start)					cts_elapsedms(start)
#define MSLEEP(us)							usleep(us * 1000)

static inline TIME_T cts_gettimeofday(void) {
	TIME_T tm;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	tm.tv_sec = tv.tv_sec;
	tm.tv_usec = tv.tv_usec;

	return tm;
}

static inline long cts_tm2ms(TIME_T tm) {
	struct timeval *tv = (struct timeval *)&tm;
	return tv->tv_sec * 1000 + tv->tv_usec / 1000;
}

#define SNPRINTF(dst, cnt, fmt, ...)		snprintf(dst, cnt, fmt, ## __VA_ARGS__)
#define MEMSET(dst, val, size)				memset(dst, val, size)
#define MEMCPY(dst, src, size)				memcpy(dst, src, size)
#define MALLOC(size)						malloc(size)	
//#define LFREE(ptr)							free(ptr)
#define LFREE(ptr) \
do { \
	if (ptr) { \
		free(ptr); \
		ptr = NULL; \
	} \
} while (0)

#ifndef __MUSL__
// android headers
#include <android/log.h>
#else
#include <hilog/log.h>
#endif

// thp headers
# include "thp/thp_dev_itf.h"

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
