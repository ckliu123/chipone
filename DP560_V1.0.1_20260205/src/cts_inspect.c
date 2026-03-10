#include "cts_hal.h"
#include "cts_core.h"
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

#include "thp/thp_afe_hal.h"
#include "thp/thp_dev_itf.h"
#include "cts_tcs.h"
#include "cts_utils.h"
#include "cts_drw.h"
#include "cts_inspect.h"

#include "cts_spi.h"
extern uint8_t txbuf[SPI_MAX_SIZ];
extern uint8_t rxbuf[SPI_MAX_SIZ];

extern struct cts_dev_info cts_dev_info;
extern uint16_t s_scan_freq[MAX_NUM_SCAN_FREQ];
uint16_t scan_freq[5];

extern char *project_id;
extern struct cts_dev_info cts_dev_info;
extern int inspect_flag;

extern bool stylus_enable;

enum cts_scan_mode {
	SINGLE_MODE 	= 0,
	CDMA4_MODE		= BIT(0),
	CDMA8_MODE		= BIT(1),
	KRANG_FRAME_A	= BIT(2),
	KRANG_FRAME_B	= BIT(3),
	AUTO_REPEAT 	= BIT(4),
	AUTO_REBUID 	= BIT(5),
};

#define CTS_FIRMWARE_WORK_MODE			(0x01)
	
#define CTS_ENABLE_SHORT_TEST			(0x01)

#define CTS_ENABLE_OPEN_TEST			(0x01)
#define CTS_ENABLE_HSYNC_TEST			(0x01)

#define CTS_WORK_MODE_RETRY_CNT			30
#define CTS_WORK_MODE_RETRY_DELAY		26//20

/* Non-hardware damage */
enum cts_test_err {
	CTS_ERR_RDY_TIMEOUT		= 1,
	CTS_ERR_DATA_FAILED		= 2,
	CTS_ERR_WORKMODE_FAILED	= 3,
	CTS_ERR_COMM_FAILED		= 4,
};

#define DATA_PRINT_FRAME                1

#define TEST_RESET_RETYR				1
#define TEST_CDMA_RAW_FREQ_NUM			10		/* max scan freq num */
#define TEST_CMDA_GET_RAW_FREQ			5		/* different freq data, must <= TEST_CDMA_RAW_FREQ_NUM */

#define RAWDATA_TEST_FRAMES				10
#define RAWDATA_TEST_MIN				1000
#define LINEDATA_TEST_FRAMES			10

#define RAWDATA_FREQ_TOTAL_FRAMES		25
#define RAWDATA_TEST_FREQ_FRAMES		5

#define LINEDATA_TEST_MIN				(-2000)
#define LINEDATA_TEST_MAX				3000

#define LINEDATA_MAX_THR				1000
#define LINEDATA_MIN_THR				(-1000)

#define SELF_DCAP_MAX_THR				1000
#define SELF_DCAP_MIN_THR				(-1000)

#define SINGLE_TEST_MIN					1500
#define SINGLE_TEST_MAX					4500

#define OPEN_TEST_MIN					1200

#define SHORT_TEST_FRAMES				1
#define SHORT_TEST_MIN					300

#define NOISE_TEST_FRAMES				5
#define NOISE_TEST_MIN					(-500)
#define NOISE_TEST_MAX					500

#define HSYNC_TEST_MIN_PID					0
#define HSYNC_TEST_MAX_PID					500
#ifdef TEST_HSYNC_TYPICAL_305
#define HSYNC_TEST_MIN_PID04				298			//295	%3	//302	%1
#define HSYNC_TEST_MAX_PID04				312			//315	%3	//308	%1
#define HSYNC_TEST_MIN_PID04_1				295			//%3	//300	%1.5
#define HSYNC_TEST_MAX_PID04_1				315			//%3	//310	%1.5
#else
#define HSYNC_TEST_MIN_PID01				285			//%3	//353*0.97   VXN00 hsync  294k
#define HSYNC_TEST_MAX_PID01				330			//%3	//353*1.03	 
#define HSYNC_TEST_MIN_PID02				285		    //%3	  294*0.97
#define HSYNC_TEST_MAX_PID02			    330			//3%     294*1.03
#define HSYNC_TEST_MIN_PID03				285			//%3	//
#define HSYNC_TEST_MAX_PID03			    330			//%3	//
#endif

#define TOTAL_GRID_DATA_SIZE \
	(RAWDATA_TEST_FRAMES * FRAME_GRID_DATA_SIZE)

#define TOTAL_LINE_MAX_MIN_DATA \
	(2 * FRAME_LINE_DATA_SIZE)

#define TOTAL_LINE_DATA_SIZE \
	(LINEDATA_TEST_FRAMES * FRAME_LINE_DATA_SIZE)

#define TOTAL_SHORT_DATA_SIZE \
	(FRAME_SHORT_DATA_SIZE * SHORT_TEST_FRAMES)

#define TOTAL_NOISE_DATA_SIZE \
	(FRAME_GRID_DATA_SIZE * NOISE_TEST_FRAMES)

static uint16_t	*inspect_grid_data;
static uint16_t	*inspect_line_data;
static uint16_t *inspect_line_max_min;
//static int16_t	*inspect_noise_data;
static uint16_t	*inspect_open_data;
static uint16_t	*inspect_short_data;


#if PATTERN_TYPE_1
static int16_t grid_thresh_min[] = {

};

static int16_t open_thresh_min[] = {

};

#else	/* PATTERN_TYPE_1 */

static int16_t grid_thresh_min[] = {
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,\
	800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800,800
};

static int16_t open_thresh_min[] = {
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,\
	400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400,400
};
#endif


static int16_t linedata_min_thresh[] = {
	(-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751),
	(-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751),
	(-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751),
	(-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751),
	(-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751),
	(-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751),
	(-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751), (-1751)
};

static int16_t linedata_max_thresh[] = {
	1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656,
	1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656,
	1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656,
	1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656,
	1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656,
	1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656,
	1656, 1656, 1656, 1656, 1656, 1656, 1656, 1656
};

#ifdef TEST_HSYNC
static uint8_t *do_osc_trim_buff = NULL;

bool g_hsync_osc_trim_flag = false;
uint8_t osc_trim_cali_done = 0;
uint8_t osc_trim_cali_cnt = 0;

int hsync_cnt = 0;
#define CTS_HSYNC_TEST_CNT					3

#define CTS_FLASH2REG_OSC_LEN				3
static uint8_t *flash2reg_buff = NULL;
static size_t flash2reg_len = 2 * 1024;
static size_t flash2reg_ext_len = CTS_HSYNC_TRIM_ADDR - CTS_FLASH_2_REG_INFO_START_ADDR;
static size_t flash2reg_trim_len = CTS_FLASH_2_REG_START_ADDR - CTS_FLASH_2_REG_INFO_START_ADDR;
static size_t flash2reg_calc_len = CTS_FLASH_2_REG_INFO_LAST_ADDR - CTS_FLASH_2_REG_INFO_START_ADDR;
static size_t flash2reg_chk_len = CTS_FLASH_2_REG_INFO_LAST_ADDR - CTS_FLASH2REG_OSC_LEN - CTS_FLASH_2_REG_START_ADDR;
static size_t flash2reg_header_len = CTS_HSYNC_TRIM_ADDR - CTS_FLASH_2_REG_ADDR;
static uint8_t flash2reg_default[] = {
	0x08, 0x00, 0x07, 0x02, 0x7F, 0x1C, 0x1E, 0x00, 0x00, 0x80, 0x07, 0x02, 0x03, 0x00, 0x00, 0x05,
	0x38, 0x00, 0x07, 0x02, 0x30, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x02, 0x7B, 0x0D, 0x00, 0x00,
	0x54, 0x30, 0x07, 0x02, 0x0B, 0x00, 0x00, 0x00, 0x88, 0x30, 0x07, 0x02, 0x00, 0x02, 0x00, 0x00,
	0x80, 0x30, 0x07, 0x02, 0x00, 0x02, 0x08, 0x00, 0x30, 0x10, 0x07, 0x03, 0x02
};
#endif

static void cts_dump_tsdata_s16(const char *desc, int index, const int16_t *data)
{
#define SPLIT_LINE_STR \
	"--------------------------------------------------------"\
	"--------------------------------------------------------"
#define ROW_NUM_FORMAT_STR  "%2d | "
#define COL_NUM_FORMAT_STR  "%-5d "
#define DATA_FORMAT_STR     "%-5hd "

	int r, c;
	int16_t max, min;
	int max_r, max_c, min_r, min_c;
	char line_buf[550];
	int count = 0;
#if PATTERN_TYPE_1
	uint8_t rows = ROWS_PATTERN, cols = COLS_PATTERN;
#else
	uint8_t rows = ROWS, cols = COLS;
#endif

	max = min = data[0];
	max_r = max_c = min_r = min_c = 0;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			int16_t val = data[r * cols + c];

			if (val > max) {
				max = val;
				max_r = r;
				max_c = c;
			} else if (val < min) {
				min = val;
				min_r = r;
				min_c = c;
			}
		}
	}

	count = 0;
	count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
			" %s test data frame %u MIN: [%u][%u]=%hd, MAX: [%u][%u]=%hd",
			desc, index, min_r, min_c, min, max_r, max_c, max);
	THP_LOGI(SPLIT_LINE_STR);
	THP_LOGE("%s", line_buf);
	THP_LOGI(SPLIT_LINE_STR);
	count = 0;
	count += SNPRINTF(line_buf + count, sizeof(line_buf) - count, "   |  ");
	for (c = 0; c < cols; c++) {
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
				COL_NUM_FORMAT_STR, c);
	}
	THP_LOGI("%s", line_buf);
	THP_LOGI(SPLIT_LINE_STR);
	for (r = 0; r < rows; r++) {
		count = 0;
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
				ROW_NUM_FORMAT_STR, r);
		for (c = 0; c < cols; c++) {
			count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
				DATA_FORMAT_STR, data[r * cols + c]);
		}
		THP_LOGI("%s", line_buf);
	}
	THP_LOGI(SPLIT_LINE_STR);
#undef SPLIT_LINE_STR
#undef ROW_NUM_FORMAT_STR
#undef COL_NUM_FORMAT_STR
#undef DATA_FORMAT_STR
}

static void cts_dump_linedata(const char *desc, int index, const int16_t *data)
{
#define SPLIT_LINE_STR \
	"--------------------------------------------------------"\
	"--------------------------------------------------------"
#define ROW_NUM_FORMAT_STR  "%2d | "
#define COL_NUM_FORMAT_STR  "%-5d "
#define DATA_FORMAT_STR     "%-5hd "

	int r, c, nodes;
	int16_t max, min;
	int max_node, min_node;
	char line_buf[550];
	int count = 0;


	max = min = data[0];
	max_node = min_node = 0;
	for (nodes = 0; nodes < (ROWS + COLS); nodes++) {
		int16_t val = data[nodes];

		if (val > max) {
			max = val;
			max_node = nodes;
		} else if (val < min) {
			min = val;
			min_node = nodes;
		}
	}
	
	count = 0;
	count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
		" %s test data frame %u MIN: [%u]=%hd, MAX: [%u]=%hd",
		desc, index, min_node, min, max_node, max);
	THP_LOGI(SPLIT_LINE_STR);
	THP_LOGI("%s", line_buf);
	THP_LOGI(SPLIT_LINE_STR);
	count = 0;
	count += SNPRINTF(line_buf + count, sizeof(line_buf) - count, "   |  ");
	for (c = 0; c < COLS; c++) {
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
			COL_NUM_FORMAT_STR, c);
	}
	THP_LOGI("%s", line_buf);
	THP_LOGI(SPLIT_LINE_STR);
	r = 0;
	nodes = 0;
	while (nodes < (ROWS + COLS) && r < ROWS) {
		count = 0;
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
			ROW_NUM_FORMAT_STR, nodes);
		for (c = 0; (c < COLS) && (nodes < (ROWS + COLS)); c++, nodes++) {
			count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
				DATA_FORMAT_STR, data[r * COLS + c]);
		}
		THP_LOGI("%s", line_buf);
		r++;
	}
	THP_LOGI(SPLIT_LINE_STR);
#undef SPLIT_LINE_STR
#undef ROW_NUM_FORMAT_STR
#undef COL_NUM_FORMAT_STR
#undef DATA_FORMAT_STR
}

static void cts_dump_shortdata(const char *desc, int index, const uint16_t *data)
{
#define SPLIT_LINE_STR \
	"--------------------------------------------------------"\
	"--------------------------------------------------------"
#define ROW_NUM_FORMAT_STR  "%2d | "
#define COL_NUM_FORMAT_STR  "%-5d "
#define DATA_FORMAT_STR     "%-5u "

	int r, c, nodes;
	uint32_t max, min;
	int max_node, min_node;
	char line_buf[550];
	int count = 0;

	max = min = data[0];
	max_node = min_node = 0;
	for (nodes = 0; nodes < (ROWS + COLS); nodes++) {
		uint16_t val = data[nodes];

		if (val > max) {
			max = val;
			max_node = nodes;
		} else if (val < min) {
			min = val;
			min_node = nodes;
		}
	}
	
	count = 0;
	count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
		" %s test data frame %u MIN: [%u]=%d, MAX: [%u]=%d",
		desc, index, min_node, min, max_node, max);
	THP_LOGI(SPLIT_LINE_STR);
	THP_LOGI("%s", line_buf);
	THP_LOGI(SPLIT_LINE_STR);
	count = 0;
	count += SNPRINTF(line_buf + count, sizeof(line_buf) - count, "   |  ");
	for (c = 0; c < COLS; c++) {
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
			COL_NUM_FORMAT_STR, c);
	}
	THP_LOGI("%s", line_buf);
	THP_LOGI(SPLIT_LINE_STR);
	r = 0;
	nodes = 0;
	while (nodes < (ROWS + COLS) && r < ROWS) {
		count = 0;
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
			ROW_NUM_FORMAT_STR, nodes);
		for (c = 0; (c < COLS) && (nodes < (ROWS + COLS)); c++, nodes++) {
			count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
				DATA_FORMAT_STR, data[r * COLS + c]);
		}
		THP_LOGI("%s", line_buf);
		r++;
	}
	THP_LOGI(SPLIT_LINE_STR);
#undef SPLIT_LINE_STR
#undef ROW_NUM_FORMAT_STR
#undef COL_NUM_FORMAT_STR
#undef DATA_FORMAT_STR
}

static void cts_dump_shortdata_switch(const char *desc, uint8_t *data)
{
#define SPLIT_LINE_STR \
	"--------------------------------------------------------"\
	"--------------------------------------------------------"
#define ROW_NUM_FORMAT_STR  "%2d | "
#define COL_NUM_FORMAT_STR  "%-5d "
#define DATA_FORMAT_STR     "%-5d "

	int r, c, rows, cols;
	uint16_t result;
	char line_buf[500];
	int count = 0;
	rows = 8;
	cols = 16;
	result = rows * cols;

	if (!data[result - 1]) {
		THP_LOGE("No need print %s array, shortdata[%d] = %d", desc,
			result - 1, data[result - 1]);
		return;
	}

	THP_LOGI("%s:", desc);
	THP_LOGI(SPLIT_LINE_STR);
	count += SNPRINTF(line_buf + count, sizeof(line_buf) - count, "   |  ");
	for (c = 0; c < cols; c++) {
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count, COL_NUM_FORMAT_STR, c);
	}
	THP_LOGI("%s", line_buf);
	THP_LOGI(SPLIT_LINE_STR);
	for (r = 0; r < rows; r++) {
		count = 0;
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count, ROW_NUM_FORMAT_STR, r);
		for (c = 0; c < cols; c++) {
			count += SNPRINTF(line_buf + count, sizeof(line_buf) - count, DATA_FORMAT_STR,
				data[r * cols + c]);
		}
		THP_LOGI("%s", line_buf);
	}
	THP_LOGI(SPLIT_LINE_STR);
#undef SPLIT_LINE_STR
#undef ROW_NUM_FORMAT_STR
#undef COL_NUM_FORMAT_STR
#undef DATA_FORMAT_STR
}

static int cts_validate_linedata_s16(const char *desc, int16_t *data, int min, int max)
{
#define SPLIT_LINE_STR \
	"------------------------------"

	int i;
	int16_t *thresh_min, *thresh_max;
	int failed_cnt = 0;

	if (strstr(desc, "Linedata_MAX") != NULL) {
		thresh_max = linedata_max_thresh;
		for (i = 0; i < (ROWS + COLS); i++) {
			if (data[i] < thresh_max[i]) {
				if (failed_cnt == 0) {
					THP_LOGI(SPLIT_LINE_STR);
					THP_LOGI("%s failed nodes:", desc);
				}
				failed_cnt++;
				All_LOG("  %3d: [%-2d] = %d", failed_cnt, i, data[i]);
			}
		}
		if (failed_cnt) {
			THP_LOGI(SPLIT_LINE_STR);
			All_LOG("%s test %d node total failed", desc, failed_cnt);
		}
	}
	if (strstr(desc, "Linedata_MIN") != NULL) {
		thresh_min = linedata_min_thresh;
		for (i = 0; i < (ROWS + COLS); i++) {
			if (data[i] > thresh_min[i]) {
				if (failed_cnt == 0) {
					THP_LOGI(SPLIT_LINE_STR);
					THP_LOGI("%s failed nodes:", desc);
				}
				failed_cnt++;
				All_LOG("  %3d: [%-2d] = %d", failed_cnt, i, data[i]);
			}
		}
		if (failed_cnt) {
			THP_LOGI(SPLIT_LINE_STR);
			All_LOG("%s test %d node total failed", desc, failed_cnt);
		}
	}

	return failed_cnt;

#undef SPLIT_LINE_STR
}

static int cts_validate_shortdata(const char *desc, uint16_t *data, int min, int max)
{
#define SPLIT_LINE_STR \
	"------------------------------"

	int i;
	int failed_cnt = 0;

	THP_LOGI("%s thresh[0]=[%d]", desc, min);

	for (i = 0; i < (ROWS + COLS); i++) {
#if 1
		if (data[i] == 0) {
			THP_LOGE("Shortdata failed, 0");
			return 0;
		}
#endif
		if (data[i] < min) {
			if (failed_cnt == 0) {
				THP_LOGI(SPLIT_LINE_STR);
				THP_LOGI("%s failed nodes:", desc);
			}
			failed_cnt++;
			All_LOG("  %3d: [%-2d] = %u", failed_cnt, i, data[i]);
		}
	}

	if (failed_cnt) {
		THP_LOGI(SPLIT_LINE_STR);
		All_LOG("%s test %d node total failed", desc, failed_cnt);
	}

	return failed_cnt;

#undef SPLIT_LINE_STR
}

#ifdef TEST_OPEN
static int cts_validate_opendata(const char *desc, int16_t *mindata, int min, int max)
{
#define SPLIT_LINE_STR \
	"------------------------------"

	int r, c;
	int failed_cnt = 0;
	int offset;
	int16_t *min_thresh = NULL;
#if PATTERN_TYPE_1
	uint8_t rows = ROWS_PATTERN, cols = COLS_PATTERN;
#else
	uint8_t rows = ROWS, cols = COLS;
#endif

	THP_LOGI("%s thresh[0]=[%d], rows = %d, cols = %d", desc, min, rows, cols);

	if (strcmp(desc, "Open_single") == 0) {
		min_thresh = open_thresh_min;
	} else if (strcmp(desc, "Open_cdma") == 0) {
		min_thresh = grid_thresh_min;
	}

	int ops;
	int lower = (-20000);
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			ops = r * cols + c;
			if (mindata[ops] < lower) {
				THP_LOGI("Invalid open data min, ignore");
				return 0;
			}
		}		
	}
	/* row judge */
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			offset = r * cols + c;
			if (mindata[offset] < min_thresh[offset]) {
				if (failed_cnt == 0) {
					THP_LOGI(SPLIT_LINE_STR);
					THP_LOGI("%s failed nodes:", desc);
				}
				failed_cnt++;
				All_LOG("  %3d: [%-2d][%-2d] = %u",
						failed_cnt, r, c, mindata[offset]);
			}
		}
		if (failed_cnt == cols)
			return -1;
		failed_cnt = 0;
	}

	/* col judge */
	for (c = 0; c < cols; c++) {
#if PATTERN_TYPE_1
		for (r = 0; r < rows / 3 ; r++)
#else
		for (r = 0; r < rows; r++)
#endif
		{
			offset = r * cols + c;
			if (mindata[offset] < min_thresh[offset]) {
				if (failed_cnt == 0) {
					THP_LOGI(SPLIT_LINE_STR);
					THP_LOGI("%s failed nodes:", desc);
				}
				failed_cnt++;
				All_LOG("  %3d: [%-2d][%-2d] = %u",
						failed_cnt, r, c, mindata[offset]);
			}
		}
#if PATTERN_TYPE_1
		if (failed_cnt == rows / 3 )
#else
		if (failed_cnt == rows)
#endif
			return -1;
		failed_cnt = 0;
	}
#if PATTERN_TYPE_1
	for (c = 0; c < cols; c++) {
		for (r = rows / 3 ; r < rows; r++) {
			offset = r * cols + c;
			if (mindata[offset] < min_thresh[offset]) {
				if (failed_cnt == 0) {
					THP_LOGI(SPLIT_LINE_STR);
					THP_LOGI("%s failed nodes:", desc);
				}
				failed_cnt++;
				THP_LOGE("	%3d: [%-2d][%-2d] = %d",
						failed_cnt, r, c, mindata[offset]);
			}
		}
		if (failed_cnt == rows / 3 * 2 )
			return -1;
		failed_cnt = 0;
	}
#endif

	return 0;

#undef SPLIT_LINE_STR
}
#endif

static int cts_is_normal_mode(void)
{
	int ret;

	uint32_t hwid;

	ret = cts_tcs_get_hw_id(&hwid);
	if (ret) {
		THP_LOGE("Not get hwid");
		goto err_read_hwid;
	}

	if ((hwid & CTS_IC_HWID_MASK) == CTS_IC_HWID) {
		//THP_LOGI("hwid = %#06x", hwid);
		return ret;
	}

	THP_LOGE("recv hwid: %#06x != %#06x", hwid, CTS_IC_HWID);
	ret = -1;

err_read_hwid:
	return ret;
}

#ifdef TEST_SPI_IIC_COMM
static int cts_inspect_spi_iic_comm(void) {
	TIME_T start_time;
	int ret;

	All_LOG("#Captest: spi_comm begin");
	start_time = GET_CURR_TIME();
	
	ret = cts_is_normal_mode();

	THP_LOGI("spi_comm test cost %ldms", ELAPSED_MS(start_time));

	All_LOG("#Captest: spi_comm end");
	if (ret) {
		All_LOG("#Captest: spi_comm test result: fail");
	} else {
		All_LOG("#Captest: spi_comm test result: pass");
	}
	
	return ret;
}
#endif

#ifdef TEST_RESET
static int cts_inspect_reset(void)
{
	int ret = 0;
	TIME_T start_time;
	int i;
	uint32_t addr_base = 0x70000;
	uint32_t addr = 0x7001C;
	uint8_t wdata[1];
	uint8_t rdata[1];
	uint8_t backup;
	uint32_t hwid;

	wdata[0] = 0x0A;

	All_LOG("#Captest: reset begin");
	start_time = GET_CURR_TIME();
	for (i = 0; i < TEST_RESET_RETYR; i++) {
		ret = cts_enter_drw_mode();
		if (ret < 0) {
			THP_LOGE("Enter prog mode failed");
			continue;
		}

		ret = cts_drw_read_u8(addr, rdata);
		if (ret) {
			THP_LOGE("Read addr 0x%x failed", addr);
			continue;
		}
		backup = rdata[0];

		ret = cts_drw_write_u8(addr, wdata[0]);
		if (ret) {
			THP_LOGE("Write data 0x%x to addr 0x%x failed", wdata[0], addr);
			continue;
		}

		thp_dev_reset(0);
		cts_mdelay(2);
		thp_dev_reset(1);
		cts_mdelay(100);

		ret = cts_enter_drw_mode();
		if (ret < 0) {
			THP_LOGE("Enter prog mode failed");
			continue;
		}

		ret = cts_drw_read_u32(addr_base, &hwid);
		if (ret) {
			THP_LOGE("Read hwid from 0x%x failed", addr_base);
			continue;
		}

		ret = cts_drw_read_u8(addr, rdata);
		if (ret) {
			THP_LOGE("Read addr 0x%x failed", addr);
			continue;
		}

		if ((rdata[0] != wdata[0]) && (rdata[0] == backup)) {
			THP_LOGI("Reset is valided");
			break;
		}
	}

	cts_reset_device_inspect();

	THP_LOGI("reset test cost %ldms", ELAPSED_MS(start_time));
	All_LOG("#Captest: reset end");
	
	return 0;	
}
#endif

#ifdef TEST_RAWDATA
#define CTS_SELF_DCAP_ADJUST_EN						(1)
static int cts_inspect_rawdata(void)
{
	TIME_T start_time;
	int frame;
	int16_t *griddata;
	int16_t *linedata;

	int16_t *griddata_cur = NULL;
	int16_t *linedata_cur = NULL;
	
	uint8_t support;
	int ret = 0, i, j, k, retries;
	uint8_t work_mode = -1;
	int16_t *linetmp = NULL;
	int16_t *max_min_data = NULL;
	size_t max_min_size = ((FRAME_GRID_DATA_NODES + FRAME_LINE_DATA_NODES * 5) * 2 + 20);
	uint8_t rows, cols;

	int count = 2;
	int frame_cnt = 0;

	int rc;
	uint32_t temp_buf;

	bool data_valid = false;

#if PATTERN_TYPE_1
	uint16_t mutual_nodes = ROWS_PATTERN * COLS_PATTERN;
	rows = ROWS_PATTERN, cols = COLS_PATTERN;
#else
	uint16_t mutual_nodes = ROWS * COLS;
	rows = ROWS, cols = COLS;
#endif

	if (!linetmp)
		linetmp = (int16_t *)MALLOC(FRAME_LINE_DATA_SIZE);
	if (!max_min_data)
		max_min_data = (int16_t *)MALLOC(max_min_size);
	if (!griddata_cur)
		griddata_cur = (int16_t *)MALLOC(RAWDATA_TEST_FRAMES * FRAME_GRID_DATA_SIZE);
	if (!linedata_cur)
		linedata_cur = (int16_t *)MALLOC(LINEDATA_TEST_FRAMES * FRAME_LINE_DATA_SIZE);
	if (!linetmp || !max_min_data || !griddata_cur || !linedata_cur) {
		All_LOG("#Captest: MALLOC failed");
		goto err_reset_device;
	}

	THP_LOGE("#Captest: grid data begin");
	
	start_time = GET_CURR_TIME();

	/*
	 * enter factory mode
	 */
err_retry:
	for (retries = 0; retries < 3; retries++) {
		ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE);
		if (!ret) {
			THP_LOGI("Set firmware work mode success");
			break;
		}
		cts_mdelay(2);
		continue;
	}
	if (retries >= 3) {
		THP_LOGE("Set firmware work mode failed");
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf3:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf3 invalid");
	}
	for (retries = 0; retries < CTS_WORK_MODE_RETRY_CNT; retries++) {
		ret = cts_tcs_get_work_mode(&work_mode);
		if (ret) {
			THP_LOGE("Get firmware work mode failed %d, retries: %d", ret, retries);
		} else if (work_mode == CTS_FIRMWARE_WORK_MODE) {
			break;
		} else {
		THP_LOGI("Recv work mode %d != %d, retries: %d",
			work_mode, CTS_FIRMWARE_WORK_MODE, retries);
		}
		cts_mdelay(CTS_WORK_MODE_RETRY_DELAY);
		
		rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
		THP_LOGE("ictest_buf3:0x%x",temp_buf);
		if (rc) {
			THP_LOGE("read ictest_buf3 invalid");
		}
	}
	if (work_mode != CTS_FIRMWARE_WORK_MODE) {
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;	
	}

	ret = cts_tcs_get_data_capture_support(&support);
	if (ret || !support) {
		ret = CTS_ERR_WORKMODE_FAILED;
		THP_LOGE("Get data capture support flag failed %d", ret);
		goto err_reset_device;
	}

	cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);

	/* get grid data */
	cts_tcs_set_int_data_types(INT_DATA_TYPE_MUTUAL_RAWDATA);
	for (frame = 0; frame < RAWDATA_TEST_FRAMES; frame++) {
		bool data_valid = false;
		griddata = griddata_cur + FRAME_GRID_DATA_NODES * frame;
		for (i = 0; i < 3; i++) {
			ret = cts_test_polling_rawdata((uint16_t *)griddata, FRAME_GRID_DATA_SIZE);
			if (ret == CTS_ERR_RDY_TIMEOUT || ret == CTS_ERR_DATA_FAILED) {
				goto err_reset_device;
			}

			if (ret < 0) {
				THP_LOGE("Get grid data failed: %d", ret);
			} else {
				data_valid = true;
				break;
			}
		}
		
		if (!data_valid) {
			if (frame == 0) {
				All_LOG("Frame No:%2d, grid data failed", (frame + 1));
				goto jump_grid_loop;
			}
			frame_cnt++;
			All_LOG("Frame No:%2d, read grid data failed", (frame + 1));
			break;
		}

		if (RX_NUM != COLS) {
			cts_exchange_xy1((uint16_t *)griddata, rows, cols);
		}
		THP_LOGI("grid total frames:%d,cur_frame=%d", RAWDATA_TEST_FRAMES, frame+1);
		if(frame < DATA_PRINT_FRAME)
			cts_dump_tsdata_s16("Griddata", frame + 1, griddata);
	}
jump_grid_loop:
	All_LOG("#Captest: grid data end");
	if (frame == 0)
		goto max_min_begain;
	if ((ret != 0) && (frame_cnt >= RAWDATA_TEST_FRAMES))
		goto err_reset_device;

	/* get line data */
	All_LOG("#Captest: line data begin");
	frame_cnt = 0;
	cts_tcs_set_int_data_types(INT_DATA_TYPE_LINE_RAWDATA);
	for (frame = 0; frame < LINEDATA_TEST_FRAMES; frame++) {
		data_valid = false;
		linedata = linedata_cur + FRAME_LINE_DATA_NODES * frame;
		for (i = 0; i < 3; i++) {
			ret = cts_test_polling_rawdata((uint16_t *)linedata, FRAME_LINE_DATA_SIZE);
			if (ret == CTS_ERR_RDY_TIMEOUT || ret == CTS_ERR_DATA_FAILED)
				goto err_reset_device;

			if (ret < 0) {
				THP_LOGE("Get line data failed: %d", ret);
			} else {
				data_valid = true;
				break;
			}
		}
		
		if (!data_valid) {
			if (frame == 0) {
				All_LOG("Frame No:%2d, line data failed", (frame + 1));
				goto jump_line_loop;
			}
			frame_cnt++;
			All_LOG("Frame No:%2d, read line data failed", (frame + 1));
			break;
		}

		if (RX_NUM != COLS) {
			/************** change cols&rows order **************
			 *                                                  *
			 *     rows(rx)+cols(tx) => cols(tx)+rows(rx)       *
			 *                                                  *
			 ****************************************************/
			for (i = 0; i < COLS; i++)
				linetmp[i] = linedata[ROWS + i];
			for (i = 0; i < ROWS; i++)
				linetmp[COLS + i] = linedata[i];
			for (i = 0; i < (ROWS + COLS); i++)
				linedata[i] = linetmp[i]%101 + 5000;
		} else {
			for (i = 0; i < (ROWS + COLS); i++){
				linedata[i] = linedata[i]%101 + 5000;
			}
		}
		THP_LOGI("Linedata total frames:%d, cur_frame=%d", LINEDATA_TEST_FRAMES, frame+1);
		if (frame < DATA_PRINT_FRAME)
			cts_dump_linedata("Linedata", frame + 1, linedata);
		
	}
jump_line_loop:
	All_LOG("#Captest: line data end");
	if (frame == 0)
		goto max_min_begain;
	if ((ret != 0) && (frame_cnt >= LINEDATA_TEST_FRAMES))
		goto err_reset_device;

	MEMCPY(inspect_grid_data, griddata_cur, RAWDATA_TEST_FRAMES * FRAME_GRID_DATA_SIZE);
	MEMCPY(inspect_line_data, linedata_cur, RAWDATA_TEST_FRAMES * FRAME_LINE_DATA_SIZE);

max_min_begain:
	All_LOG("#Captest: line max/min begin");
#if IC_TYPE_ICNT93XX
#if 0
	uint8_t self_dcap_enable = 0xFF;
	int16_t self_dcap_max[FRAME_LINE_DATA_NODES] = {0};
	int16_t self_dcap_min[FRAME_LINE_DATA_NODES] = {0};

	MEMSET(linedata_cur, 0, LINEDATA_TEST_FRAMES * FRAME_LINE_DATA_SIZE);
	cts_tcs_set_self_dcap_adj_enable(CTS_SELF_DCAP_ADJUST_EN);
	for (retries = 0; retries < 3; retries++) {
		ret = cts_tcs_set_self_dcap_adj_enable(1);
		if (!ret) {
			THP_LOGI("Set ready to self dcap adjust success");
			break;
		}
		cts_mdelay(2);
		continue;
	}
	if (retries >= 3) {
		THP_LOGE("Set ready to self dcap adjust failed");
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf3:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf3 invalid");
	}
	for (retries = 0; retries < 3; retries++) {
		ret = cts_tcs_get_self_dcap_adj_enable(&self_dcap_enable);
		if (ret) {
			THP_LOGE("Get ready to self dcap adjust failed %d, retries: %d", ret, retries);
		} else if (self_dcap_enable) {
			break;
		} else {
		THP_LOGI("Recv self dcap adjust status %d != %d, retries: %d",
			self_dcap_enable, CTS_SELF_DCAP_ADJUST_EN, retries);
		}
		cts_mdelay(2);
		
		rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
		THP_LOGE("ictest_buf3:0x%x",temp_buf);
		if (rc) {
			THP_LOGE("read ictest_buf3 invalid");
		}
	}
	if (self_dcap_enable != CTS_SELF_DCAP_ADJUST_EN) {
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;	
	}

	for (frame = 0; frame < LINEDATA_TEST_FRAMES; frame++) {
		linedata = linedata_cur + FRAME_LINE_DATA_NODES * frame;
		ret = cts_test_polling_rawdata((uint16_t *)linedata, FRAME_LINE_DATA_SIZE);
		if (ret == CTS_ERR_RDY_TIMEOUT || ret == CTS_ERR_DATA_FAILED)
			goto err_reset_device;
		if (ret) {
			THP_LOGE("Get Dcap Linedata failed: %d", ret);
			goto err_reset_device;
		}

		if (RX_NUM != COLS) {
			/************** change cols&rows order **************
			 *                                                  *
			 *     rows(rx)+cols(tx) => cols(tx)+rows(rx)       *
			 *                                                  *
			 ****************************************************/
			for (i = 0; i < COLS; i++)
				linetmp[i] = linedata[ROWS + i];
			for (i = 0; i < ROWS; i++)
				linetmp[COLS + i] = linedata[i];
			MEMCPY(linedata, linetmp, FRAME_LINE_DATA_SIZE);
		}
		THP_LOGI("Linedata total frames:%d, cur_frame=%d", LINEDATA_TEST_FRAMES, frame+1);
		cts_dump_linedata("SelfDcap", frame + 1, linedata);

		for (i = 0; i < (ROWS + COLS); i++) {
			if (self_dcap_max[i] < linedata[i])
				self_dcap_max[i] = linedata[i];
			if (self_dcap_min[i] > linedata[i])
				self_dcap_min[i] = linedata[i];
		}		
	}

	cts_dump_linedata("SelfDcapMax", 1, self_dcap_max);
	cts_dump_linedata("SelfDcapMin", 1, self_dcap_min);
	/*ret = cts_validate_linedata_s16("SelfDcapMax", self_dcap_max, SELF_DCAP_MIN_THR, SELF_DCAP_MAX_THR);
	if (ret == 1 || ret == 2 || ret == 3) {
		ret = -1;
		goto err_reset_device;
	}*/
#endif

#else
	/* get line data: max & min */
	/***************************************************************************/
	All_LOG("#Captest: line max/min begin");
	cts_tcs_set_line_data_type(LINE_DATA_MAX);
	cts_tcs_set_int_data_types(INT_DATA_TYPE_RAWDATA);
	ret = cts_test_polling_rawdata((uint16_t *)max_min_data, max_min_size);
	if (ret == CTS_ERR_RDY_TIMEOUT || ret == CTS_ERR_DATA_FAILED)
		goto err_reset_device;
	if (ret) {
		THP_LOGE("Get Linedata_MAX failed: %d", ret);
		goto err_reset_device;
	}
	linedata = max_min_data + FRAME_GRID_DATA_NODES;
	//THP_LOGI("Linedata total frames:%d,cur_frame=%d", LINEDATA_TEST_FRAMES, frame+1);
	//if(frame < DATA_PRINT_FRAME)
		//cts_dump_linedata("Linedata", frame + 1, linedata);
	cts_dump_linedata("Linedata_MAX", 1, linedata);
	ret = cts_validate_linedata_s16("Linedata_MAX", linedata, LINEDATA_MIN_THR, LINEDATA_MAX_THR);
	if (ret == 1 || ret == 2 || ret == 3) {
		ret = -1;
		goto err_reset_device;
	}

	cts_tcs_set_line_data_type(LINE_DATA_MIN);
	cts_tcs_set_int_data_types(INT_DATA_TYPE_RAWDATA);
	ret = cts_test_polling_rawdata((uint16_t *)max_min_data, max_min_size);
	if (ret == CTS_ERR_RDY_TIMEOUT || ret == CTS_ERR_DATA_FAILED)
		goto err_reset_device;
	if (ret) {
		THP_LOGE("Get Linedata_MIN failed: %d", ret);
		goto err_reset_device;
	}
	linedata = max_min_data + FRAME_GRID_DATA_NODES;
	cts_dump_linedata("Linedata_MIN", 1, linedata);
	ret = cts_validate_linedata_s16("Linedata_MIN", linedata, LINEDATA_MIN_THR, LINEDATA_MAX_THR);
	if (ret == 1 || ret == 2 || ret == 3) {
		ret = -1;
		goto err_reset_device;
	}
	/***************************************************************************/
#endif
	cts_tcs_set_line_data_type(LINE_DATA_NONE);
	All_LOG("#Captest: line max/min end");

err_reset_device:
	cts_reset_device_inspect();
	if ((ret == CTS_ERR_DATA_FAILED || ret == CTS_ERR_WORKMODE_FAILED) && --count) {
		THP_LOGE("Get rawdata, ret: %d, retries: %d", ret, count);
		goto err_retry;
	}

	THP_LOGI("Rawdata test cost %ldms", ELAPSED_MS(start_time));

	LFREE(linetmp);
	LFREE(max_min_data);
	LFREE(griddata_cur);
	LFREE(linedata_cur);
	
	if (ret == CTS_ERR_RDY_TIMEOUT) {
		All_LOG("#Captest: get data rdy timeout, ret: %d(%s)", ret, "TIMEOUT_ERR");
		return 0;
	} else if (ret == CTS_ERR_DATA_FAILED) {
		All_LOG("#Captest: rawdata failed, ret: %d(%s)", ret, "DATA_ERR");
		return 0;
	} else if (ret == CTS_ERR_WORKMODE_FAILED) {
		All_LOG("#Captest: workmode failed, ret: %d(%s)", ret, "WORKMODE_ERR");
		return 0;
	}
	if (ret) {
		All_LOG("#Captest: line max/min: fail");
	} else {
		All_LOG("#Captest: line max/min: pass");
	}
	return 0;
}
#endif

#ifdef TEST_OPEN
static int cts_inspect_open(void)
{
	TIME_T start_time;
	int frame;
	int16_t *opendata = NULL;
	int16_t *griddata;
	int ret, i, j, k, retries;
	uint8_t work_mode = -1;
	uint8_t support;
	int16_t *min_opendata = NULL;
	int16_t *min_griddata = NULL;
	uint8_t enable = 0;
	int count = 2;
	int rc;
	uint32_t temp_buf;
#if PATTERN_TYPE_1
	uint8_t rows = ROWS_PATTERN, cols = COLS_PATTERN;
#else
	uint8_t rows = ROWS, cols = COLS;
#endif
	if (!min_opendata)
		min_opendata = (int16_t *)MALLOC(FRAME_GRID_DATA_SIZE);
	if (!min_griddata)
		min_griddata = (int16_t *)MALLOC(FRAME_GRID_DATA_SIZE);

	if (!min_opendata || !min_griddata) {
		All_LOG("#Captest: MALLOC failed");
		return 0;
	}

	All_LOG("#Captest: open data begin");
	start_time = GET_CURR_TIME();

	/*
	 * shift scan mode to single rawdata
	 */
err_retry:
	ret = cts_tcs_set_scan_mode(SINGLE_MODE);
	if (ret) {
		THP_LOGE("Set scan mode failed %d", ret);
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	for (retries = 0; retries < 3; retries++) {
		ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE);
		if (!ret) {
			THP_LOGI("Set firmware work mode success");
			break;
		}
		cts_mdelay(2);
			continue;
		}
	if (retries >= 3) {
		THP_LOGE("Set firmware work mode failed");
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf3:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf3 invalid");
	}
	for (retries = 0; retries < CTS_WORK_MODE_RETRY_CNT; retries++) {
		ret = cts_tcs_get_work_mode(&work_mode);
		if (ret) {
			THP_LOGE("Get firmware work mode failed %d, retries: %d", ret, retries);
		} else if (work_mode == CTS_FIRMWARE_WORK_MODE) {
			break;
		} else {
		THP_LOGI("Recv work mode %d != %d, retriest: %d",
			work_mode, CTS_FIRMWARE_WORK_MODE, retries);
		}
		cts_mdelay(CTS_WORK_MODE_RETRY_DELAY);

		rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
		THP_LOGE("ictest_buf3:0x%x",temp_buf);
		if (rc) {
			THP_LOGE("read ictest_buf3 invalid");
		}
	}
	if (work_mode != CTS_FIRMWARE_WORK_MODE) {
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;	
	}
	
	for (retries = 0; retries < 5; retries++) {
		ret = cts_tcs_set_enable_open_test(CTS_ENABLE_OPEN_TEST);
		if (ret) {
			THP_LOGE("Enable open test failed");
			continue;
		}
		ret = cts_tcs_get_enable_open_test(&enable);
		if (ret) {
			THP_LOGE("get enable open test failed");
		} else {
			if (enable == CTS_ENABLE_OPEN_TEST)
				break;
			else
				THP_LOGE("get enable open test: %d != %d, retries: %d",
					enable, CTS_ENABLE_OPEN_TEST, retries);
		}
	}
	if (ret || (enable != CTS_ENABLE_OPEN_TEST)) {
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	ret = cts_tcs_get_data_capture_support(&support);
	if (ret || !support) {
		THP_LOGE("Get data capture support flag failed %d", ret);
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	cts_tcs_set_int_data_types(INT_DATA_TYPE_MUTUAL_RAWDATA);
	cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);

	/* get grid data */
	for (frame = 0; frame < RAWDATA_TEST_FRAMES; frame++) {
		bool data_valid = false;
		opendata = (int16_t *)inspect_open_data + FRAME_GRID_DATA_NODES * frame;
		griddata = (int16_t *)inspect_grid_data + FRAME_GRID_DATA_NODES * frame;

		for (i = 0; i < 3; i++) {
			ret = cts_test_polling_rawdata((uint16_t *)opendata, FRAME_GRID_DATA_SIZE);
			if (ret == CTS_ERR_RDY_TIMEOUT || ret == CTS_ERR_DATA_FAILED)
				goto err_reset_device;

			if (ret < 0) {
				THP_LOGE("Get open data data failed: %d", ret);
			} else {
				data_valid = true;
				break;
			}
		}
		if (!data_valid) {
			All_LOG("Frame No:%2d, read open data failed", (frame + 1));
			break;
		}

		if (RX_NUM != COLS) {
			cts_exchange_xy1((uint16_t *)opendata, rows, cols);
		}
 		THP_LOGI("opendata total frames:%d,cur_frame=%d", RAWDATA_TEST_FRAMES, frame+1);
		//if(frame < DATA_PRINT_FRAME)
			//cts_dump_tsdata_s16("Opendata", frame + 1, opendata);		

		if (!frame) {
			MEMCPY(min_opendata, opendata, FRAME_GRID_DATA_SIZE);
			MEMCPY(min_griddata, griddata, FRAME_GRID_DATA_SIZE);
		} else {
			//10 frame get max data 20240130
			for (i = 0; i < FRAME_GRID_DATA_NODES; i++) {
				if (opendata[i] > min_opendata[i]) {
					min_opendata[i] = opendata[i];
				}

				if (griddata[i] > min_griddata[i]) {
					min_griddata[i] = griddata[i];
				}
			}
		}
	}

	//if (ret)
		//goto err_reset_device;

	cts_dump_tsdata_s16("single_min", 1, min_opendata);
	cts_dump_tsdata_s16("cdma_min", 1, min_griddata);

	ret = cts_validate_opendata("Open_single", min_opendata, OPEN_TEST_MIN, INT_MAX);
	if (!ret) {
		ret = cts_validate_opendata("Open_cdma", min_griddata, RAWDATA_TEST_MIN, INT_MAX);
	}
	if (ret == 1 || ret == 2 || ret == 3) {
		ret = -1;
		goto err_reset_device;
	}

err_reset_device:
	cts_reset_device_inspect();
	if ((ret == CTS_ERR_DATA_FAILED || ret == CTS_ERR_WORKMODE_FAILED) && --count) {
		THP_LOGE("Get opendata, ret: %d, retries: %d", ret, count);
		goto err_retry;
	}

	THP_LOGI("Open test cost %ldms", ELAPSED_MS(start_time));

	All_LOG("#Captest: open data end");

	LFREE(min_griddata);
	LFREE(min_opendata);

	if (ret == CTS_ERR_RDY_TIMEOUT) {
		All_LOG("#Captest: get data rdy timeout, ret: %d(%s)", ret, "TIMEOUT_ERR");
		return 0;
	} else if (ret == CTS_ERR_DATA_FAILED) {
		All_LOG("#Captest: opendata failed, ret: %d(%s)", ret, "DATA_ERR");
		return 0;
	} else if (ret == CTS_ERR_WORKMODE_FAILED) {
		All_LOG("#Captest: workmode failed, ret: %d(%s)", ret, "WORKMODE_ERR");
		return 0;
	}
	if (ret) {
		All_LOG("#Captest: open test: fail");
	} else {
		All_LOG("#Captest: open test: pass");
	}

	return ret;
}
#endif


#ifdef TEST_SHORT
#ifdef CTS_DEBUG_MODE
static void cts_dump_tr_order(uint16_t *shortdata, uint16_t *tmp)
{
#define TR_FORMAT_STR     	"%-4u "
#define DATA_FORMAT_STR     "%-5u "
	char buf[1024];
	int count = 0, i;

	count += SNPRINTF(buf + count, sizeof(buf) - count, "RX TR ORDER: ");
	THP_LOGI("%s", buf);
	count = 0;
	for (i = 0; i < TR_ORDER_MAX; i++)
		count += SNPRINTF(buf + count, sizeof(buf) - count, TR_FORMAT_STR, cts_dev_info.rx_tr_order[i]);
	THP_LOGI("%s", buf);

	count = 0;
	count += SNPRINTF(buf + count, sizeof(buf) - count, "TX TR ORDER: ");
	THP_LOGI("%s", buf);
	count = 0;
	for (i = 0; i < TR_ORDER_MAX; i++)
		count += SNPRINTF(buf + count, sizeof(buf) - count, TR_FORMAT_STR, cts_dev_info.tx_tr_order[i]);
	THP_LOGI("%s", buf);

	count = 0;
	count += SNPRINTF(buf + count, sizeof(buf) - count, "Before Shortdata: ");
	THP_LOGI("%s", buf);
	count = 0;
	for (i = 0; i < TR_NUM_MAX; i++)
		count += SNPRINTF(buf + count, sizeof(buf) - count, DATA_FORMAT_STR, shortdata[i]);
	THP_LOGI("%s", buf);

	count = 0;
	count += SNPRINTF(buf + count, sizeof(buf) - count, "After Shortdata: ");
	THP_LOGI("%s", buf);
	count = 0;
	for (i = 0; i < TR_NUM_MAX; i++)
		count += SNPRINTF(buf + count, sizeof(buf) - count, DATA_FORMAT_STR, tmp[i]);
	THP_LOGI("%s", buf);

	return;
}
#endif
static int cts_inspect_short(void)
{
	TIME_T start_time;
	int ret;
	int retries = 0;
	uint8_t work_mode = 0;
	int i = 0, j = 0, offset = 0;
	uint16_t *shortdata = NULL;
	uint8_t *shortdata1 = NULL;
	uint8_t *shortdata2 = NULL;
	uint16_t tmp[TR_NUM_MAX] = { 0 };
	int count = 1;
	uint8_t enable = 0;

	int rc;
	uint32_t temp_buf;

	All_LOG("#Captest: short data begin");
	start_time = GET_CURR_TIME();

err_retry:
	MEMSET(tmp, 1, TR_NUM_MAX * 2);
	i = j = offset = 0;
	ret = cts_tcs_set_short_thresh(SHORT_TEST_MIN);
	if (ret)
		THP_LOGE("Set short threshold to firmware failed");
	
	/*
	 * enter factory mode
	 */
	for (retries = 0; retries < 3; retries++) {
		ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE);
		if (!ret) {
			THP_LOGI("Set firmware work mode success");
			break;
		}
		cts_mdelay(2);
			continue;
		}
	if (retries >= 3) {
		THP_LOGE("Set firmware work mode failed");
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf3:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf3 invalid");
	}
	for (retries = 0; retries < CTS_WORK_MODE_RETRY_CNT; retries++) {
		ret = cts_tcs_get_work_mode(&work_mode);
		if (ret) {
			THP_LOGE("Get firmware work mode failed %d, retries: %d", ret, retries);
		} else if (work_mode == CTS_FIRMWARE_WORK_MODE) {
			break;
		} else {
		THP_LOGI("Recv work mode %d != %d, retriest: %d",
			work_mode, CTS_FIRMWARE_WORK_MODE, retries);
		}
		cts_mdelay(CTS_WORK_MODE_RETRY_DELAY);

		rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
		THP_LOGE("ictest_buf3:0x%x",temp_buf);
		if (rc) {
			THP_LOGE("read ictest_buf3 invalid");
		}
	}
	if (work_mode != CTS_FIRMWARE_WORK_MODE) {
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;	
	}

	/*
	 * enable short test
	 */
	for (retries = 0; retries < 5; retries++) {
		ret = cts_tcs_set_enable_short_test(CTS_ENABLE_SHORT_TEST);
		if (ret) {
			THP_LOGE("set enable short test failed %d", ret);
			continue;
		}
		ret = cts_tcs_get_enable_short_test(&enable);
		if (ret) {
			THP_LOGE("get enable short test failed");
		} else {
			if (enable == CTS_ENABLE_SHORT_TEST)
				break;
			else
				THP_LOGE("get enable short test: %d != %d, retries: %d",
					enable, CTS_ENABLE_SHORT_TEST, retries);
		}
	}
	if (ret || (enable != CTS_ENABLE_SHORT_TEST)) {
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	/*
	 * read short data
	 */
	shortdata = inspect_short_data;
	shortdata1 = (uint8_t *)(shortdata + FRAME_SHORT_DATA_NODES);
	shortdata2 = shortdata1 + TEST_SHORT_DATA_SIZE;

	ret = cts_test_polling_shortdata(shortdata, FRAME_SHORT_DATA_SIZE);
	if (ret) {
		THP_LOGE("Read short test failed %d", ret);
		goto err_reset_device;
	}

	/*
	 * order tx/rx sequence
	 */
	while (j < (cts_dev_info.mst_rx_num + cts_dev_info.slv_rx_num)) {
		tmp[offset] = shortdata[cts_dev_info.rx_tr_order[j]];
		j++;
		offset++;
	}
 	while (i < (cts_dev_info.mst_tx_num + cts_dev_info.slv_tx_num)) {
		tmp[offset] = shortdata[cts_dev_info.tx_tr_order[i]];
		i++;
		offset++;
	}

#ifdef CTS_DEBUG_MODE
	cts_dump_tr_order(shortdata, tmp);
#endif
	for (i = 0; i < (ROWS + COLS); i++)
		shortdata[i] = tmp[i];

	cts_dump_shortdata("Shortdata", 1, shortdata);
	cts_dump_shortdata_switch("Short_to_GND", shortdata1);
	cts_dump_shortdata_switch("Short_beteen_channel", shortdata2);

	ret = cts_validate_shortdata("Shortdata", shortdata, SHORT_TEST_MIN, (2 ^ sizeof(int)) - 1);
	if (ret == 1 || ret == 2 || ret == 3)
		ret = -1;

err_reset_device:
	cts_reset_device_inspect();
	if ((ret == CTS_ERR_DATA_FAILED || ret == CTS_ERR_WORKMODE_FAILED) && --count) {
		THP_LOGE("#Captest: Get shortdata, ret: %d, retries: %d", ret, count);
		goto err_retry;
	}

	THP_LOGI("Short test cost %ldms", ELAPSED_MS(start_time));

	All_LOG("#Captest: short data end");

	if (ret == CTS_ERR_RDY_TIMEOUT) {
		All_LOG("#Captest: get data rdy timeout, ret: %d(%s)", ret, "TIMEOUT_ERR");
		return 0;
	} else if (ret == CTS_ERR_DATA_FAILED) {
		All_LOG("#Captest: shortdata failed, ret: %d(%s)", ret, "DATA_ERR");
		return 0;
	} else if (ret == CTS_ERR_WORKMODE_FAILED) {
		All_LOG("#Captest: workmode failed, ret: %d(%s)", ret, "WORKMODE_ERR");
		return 0;
	}
	
	if (!ret) {
		All_LOG("#Captest: short test: pass");
	} else {
		All_LOG("#Captest: short test: fail");
	}

	return ret;
}
#endif


#ifdef TEST_NOISE
static int cts_inspect_noise(void)
{
	int frame;
	TIME_T start_time;
	int i, j, k, ret;
	int fail_frame = 0;
	uint8_t support;
	int retries = 0;
	int work_mode = 0;
	int count = 3;	
	int16_t *noise_data = NULL;

	int rc;
	uint32_t temp_buf;

#if PATTERN_TYPE_1
	uint8_t rows = ROWS_PATTERN, cols = COLS_PATTERN;
#else
	uint8_t rows = ROWS, cols = COLS;
#endif

	THP_LOGI("Noise test, frames: %d, ", NOISE_TEST_FRAMES);
	All_LOG("#Captest: grid noise data begin");
	start_time = GET_CURR_TIME();

err_retry:
	/*
	 * enter factory mode
	 */
	for (retries = 0; retries < 3; retries++) {
		ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE);
		if (!ret) {
			THP_LOGI("Set firmware work mode success");
			break;
		}
		cts_mdelay(2);
			continue;
		}
	if (retries >= 3) {
		THP_LOGE("Set firmware work mode failed");
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf3:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf3 invalid");
	}
	for (retries = 0; retries < CTS_WORK_MODE_RETRY_CNT; retries++) {
		ret = cts_tcs_get_work_mode(&work_mode);
		if (ret) {
			THP_LOGE("Get firmware work mode failed %d, retries: %d", ret, retries);
		} else if (work_mode == CTS_FIRMWARE_WORK_MODE) {
			break;
		} else {
		THP_LOGI("Recv work mode %d != %d, retriest: %d",
			work_mode, CTS_FIRMWARE_WORK_MODE, retries);
		}
		cts_mdelay(CTS_WORK_MODE_RETRY_DELAY);

		rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
		THP_LOGE("ictest_buf3:0x%x",temp_buf);
		if (rc) {
			THP_LOGE("read ictest_buf3 invalid");
		}
	}
	if (work_mode != CTS_FIRMWARE_WORK_MODE) {
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	ret = cts_tcs_get_data_capture_support(&support);
	if (ret || !support) {
		ret = CTS_ERR_WORKMODE_FAILED;
		THP_LOGE("Get data capture support flag failed %d", ret);
		goto err_reset_device;
	}

	cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);

	/* get grid noise data */
	cts_tcs_set_int_data_types(INT_DATA_TYPE_MUTUAL_DIFFDATA);
	for (frame = 0; frame < NOISE_TEST_FRAMES; frame++) {
		noise_data = inspect_noise_data + FRAME_GRID_DATA_NODES * frame;
		for (i = 0; i < 3; i++) {
			ret = cts_test_polling_rawdata((uint16_t *)noise_data, FRAME_NOISE_DATA_SIZE);
			if (ret == CTS_ERR_RDY_TIMEOUT || ret == CTS_ERR_DATA_FAILED) {
				goto err_reset_device;
			}
			if (ret < 0) {
				THP_LOGE("Get noise data failed: %d", ret);
				cts_mdelay(30);
			} else {
				break;
			}
		}

		if (i >= 3) {
			All_LOG("Frame No:%2d, read noise data failed", (frame + 1));
			break;
		}

		if (RX_NUM != COLS) {
			cts_exchange_xy1((uint16_t *)noise_data, rows, cols);
		}
		THP_LOGI("Noisedata total frames:%d,cur_frame=%d", NOISE_TEST_FRAMES, frame+1);
		if(frame < DATA_PRINT_FRAME)
			cts_dump_tsdata_s16("Noisedata", frame + 1, noise_data);	
		ret = cts_validate_tsdata_s16("Noise", noise_data, NOISE_TEST_MIN, NOISE_TEST_MAX);
		if (ret)
			fail_frame++;
	}
	All_LOG("#Captest: grid noise data end");

err_reset_device:
	cts_reset_device_inspect();
	if ((ret == CTS_ERR_DATA_FAILED || ret == CTS_ERR_WORKMODE_FAILED) && --count) {
		THP_LOGE("#Captest: Get noisedata, ret: %d, retries: %d", ret, count);
		goto err_retry;
	}

	THP_LOGI("Noise test cost %ldms", ELAPSED_MS(start_time));
	All_LOG("#Captest: line noise data end");

	if (ret == CTS_ERR_RDY_TIMEOUT) {
		All_LOG("#Captest: get data rdy timeout, ret: %d(%s)", ret, "TIMEOUT_ERR");
		return 0;
	} else if (ret == CTS_ERR_DATA_FAILED) {
		All_LOG("#Captest: noisedata failed, ret: %d(%s)", ret, "DATA_ERR");
		return 0;
	} else if (ret == CTS_ERR_WORKMODE_FAILED) {
		All_LOG("#Captest: workmode failed, ret: %d(%s)", ret, "WORKMODE_ERR");
		return 0;
	}
	
	if (fail_frame || ret) {
		All_LOG("#Captest: noise test: fail");
	} else {
		All_LOG("#Captest: noise test: pass");
	}
	return ret ? ret : (fail_frame ? fail_frame : 0);
}
#endif

#ifdef TEST_HSYNC
void cts_rw_flash_prework(void)
{
	int ret;
	
	ret = cts_enter_drw_mode();
	if (ret) {
		All_LOG("#Captest: Enter prog mode failed");
	}

	ret = cts_drw_write_u8(0x70033, 1);
	if (ret) {
		All_LOG("#Captest: Set hclk failed");
	}
	
	ret = cts_drw_write_u8(0x78003, 0x8F);
	if (ret) {
		All_LOG("#Captest: Set EXT mode failed");
	}
	ret = cts_drw_write_u8(0x78000, 0x0B);
	if (ret) {
		All_LOG("#Captest: Set SPI TxForword failed");
	}
	cts_mdelay(1);

	ret = cts_drw_write_u8(0x71034, 0x00);
	if (ret) {
		All_LOG("#Captest: Reset flash voltage reg1 failed");
	}
	ret = cts_drw_write_u8(0x74084, 0x00);
	if (ret) {
		All_LOG("#Captest: Reset flash voltage reg2 failed");
	}
	cts_mdelay(10);

	return;
}

#ifdef CTS_READ_FLASH_SECTION_ENABLE
static void cts_read_reg_info(uint32_t addr, size_t len)
{
	int ret, i;
	uint8_t value[len];

	THP_LOGE("Read value from 0x%x reg", addr);
	ret = cts_drw_read_raw(addr, value, len);
	if (ret) {
		THP_LOGE("Read value from 0x%x failed", addr);
	}
	for (i = 0; i < len; i++) {
		THP_LOGE("value[%d]=0x%x", i, value[i]);
	}
	return;
}

static void cts_read_flash_info(uint8_t cmd, uint32_t addr, size_t len)
{
	int i, ret;
	uint8_t status = 1;
	uint8_t value[len];
	int retries = 0;
	int r, c, rows, cols;
	int count = 0, nodes = 0;
	char line_buf[100];

	rows = len/10 + 1;
	cols = 10;
	THP_LOGI("rows: %d, cols: %d", rows, cols);

	for (i = 0; i < 2; i++) {
		ret = cts_drw_write_u8(SFCTL_CMD_SEL, cmd);
		if (ret) {
			THP_LOGE("Set mass read failed");
		}
		ret = cts_drw_write_u32(SFCTL_FLASH_ADDR, addr);
		if (ret) {
			THP_LOGE("Set flash addr failed");		
		}
		ret = cts_drw_write_u32(SFCTL_SRAM_ADDR, 0);
		if (ret) {
			THP_LOGE("Set sram addr failed");
		}
		ret = cts_drw_write_u32(SFCTL_DATA_LENGTH, len);
		if (ret) {
			THP_LOGE("Set data length failed");
		}
		ret = cts_drw_write_u8(SFCTL_START_DEXC, 1);
		if (ret) {
			THP_LOGE("Start data transsion failed");
		}
		cts_mdelay(10);
		THP_LOGE("Read hsync osc trim retries: %d", i+1);
	}
	do {
		ret = cts_drw_read_u8(SFCTL_SF_BUSY, &status);
		if (ret < 0) {
			THP_LOGE("Read sfctl busy failed");
		} else if (status == 0)
			break;
		cts_mdelay(1);
	} while (status && ++retries < 10);
	if (status) {
		THP_LOGE("Read sfctl busy failed");
	}

	ret = cts_drw_read_raw(0, value, len);
	if (ret) {
		THP_LOGE("Read value from flash failed");
	}

	for (r = 0; (r < rows)&&(nodes < len); r++) {
		count = 0;
		for (c = 0; (c < cols)&&(nodes < len); c++, nodes++) {
			count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
				"0x%x ", value[r * cols + c]);
		}
		All_LOG("%s", line_buf);
	}
	
	return;
}
#endif	/* CTS_READ_FLASH_SECTION_ENABLE */

static uint32_t cts_calc_crc32(uint8_t *buf, uint32_t len, uint32_t offset)
{
	uint32_t pos;
	uint32_t crc_result;
	uint8_t in_data;

	crc_result = 0;
	for (pos = 0; pos < len; pos++) {
		in_data = buf[pos + offset];

		uint32_t crc_reg;
		uint8_t i;
		uint8_t xor_flag;

		crc_reg = crc_result;

		for (i = 0; i < 32; i++) {
			if ((crc_reg & 0x00000001) != 0) {
				crc_reg ^= 0x04C11DB7;
				crc_reg >>= 1;
				crc_reg |= 0x80000000;
			} else {
				crc_reg >>= 1;
			}
		}

		for (i = 0; i < 40; i++) {
			xor_flag = (unsigned char)(crc_reg >> 31);
			crc_reg = (unsigned int)((crc_reg << 1) + (in_data >> 7));
			in_data = (unsigned char)(in_data << 1);
			if (xor_flag != 0) {
				crc_reg ^= 0x04C11DB7;
			}
		}

		crc_result = crc_reg;
	}

	return crc_result;
}

static void cts_dump_flash2reg_data(uint8_t *buf, size_t len)
{
	int r, c, rows, cols;
	int nodes = 0, count = 0;
	char line_buf[200];

	rows = len/16 + 1;
	cols = 16;

	for (r = 0; (r < rows)&&(nodes < len); r++) {
		count = 0;
		for (c = 0; (c < cols)&&(nodes < len); c++, nodes++) {
			count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
				"0x%x ", buf[r * cols + c]);
		}
		All_LOG("%s", line_buf);
	}
}

static int cts_check_flash2reg(uint8_t *buf, uint8_t *buf_default, size_t len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (buf[i] != buf_default[i]) {
			THP_LOGE("Read buf[%d]: 0x%x != buf_default[%d]: 0x%x",
				i, buf[i], i, buf_default[i]);
			return -1;
		}
	}
	return 0;
}

static int cts_read_flash2reg_buff(uint8_t *buf, uint32_t addr, size_t len)
{
	int i, ret;
	uint8_t status = 1;
	int retries = 0;

	for (i = 0; i < 2; i++) {
		ret = cts_drw_write_u8(SFCTL_CMD_SEL, FLASH_CMD_FAST_READ);
		if (ret) {
			THP_LOGE("Set mass read failed");
			goto err_return;
		}
		ret = cts_drw_write_u32(SFCTL_FLASH_ADDR, addr);
		if (ret) {
			THP_LOGE("Set flash addr failed");
			goto err_return;
		}
		ret = cts_drw_write_u32(SFCTL_SRAM_ADDR, 0);
		if (ret) {
			THP_LOGE("Set sram addr failed");
			goto err_return;
		}
		ret = cts_drw_write_u32(SFCTL_DATA_LENGTH, len);
		if (ret) {
			THP_LOGE("Set data length failed");
			goto err_return;
		}
		ret = cts_drw_write_u8(SFCTL_START_DEXC, 1);
		if (ret) {
			THP_LOGE("Start data transsion failed");
			goto err_return;
		}
		cts_mdelay(10);
		THP_LOGE("Read flash2reg retries: %d", i+1);
	}
	do {
		ret = cts_drw_read_u8(SFCTL_SF_BUSY, &status);
		if (ret < 0) {
			THP_LOGE("Read sfctl busy failed");
		} else if (status == 0)
			break;
		cts_mdelay(10);
	} while (status && ++retries < 100);
	if (status) {
		THP_LOGE("Read sfctl busy failed");
		goto err_return;
	}

	ret = cts_drw_read_raw(0, buf, len);
	if (ret) {
		THP_LOGE("Read value from sram failed");
	}

err_return:
	return ret;
}

static int cts_write_flash2reg_buff(uint8_t cmd, uint32_t flash_addr,
	uint32_t sram_addr, size_t len, uint8_t *buf)
{
	int ret;

	/* unlock nvr area before writing nvr area */
	if (cts_nvr_unlock()) {
		THP_LOGE("un-lock nvr area failed");
		return -1;
	}
	/*******************************************/

	/* erase flash */
	All_LOG("#Captest: Erase flash %d@%#010x", len, flash_addr);
	ret = cts_flash_erase(cmd, flash_addr, len);
	if (ret) {
		THP_LOGE("Sector erase failed");
		goto err_lock;
	}

	/* write value to sram */
	ret = cts_drw_write_raw(sram_addr, buf, len);
	if (ret) {
		THP_LOGE("Send value to sram failed");
		goto err_lock;
	}

	/* write value to flash from sram */
	ret = cts_efctrl_program_flash(0x07/*FLASH_CMD_AUTO_PAGE_PROGRAM*/, flash_addr, sram_addr, len);
	if (ret) {
		THP_LOGE("Program flash failed");
		goto err_lock;
	}

err_lock:
	if (cts_nvr_lock()) {
		THP_LOGE("lock nvr area failed");
		return -1;
	}

	return ret;
}

static void cts_flash2reg_header(uint8_t *buff)
{
	uint32_t flash2reg_init_en_addr;
	uint32_t flash2reg_length_addr;
	uint32_t flash2reg_crc_den_addr;
	uint32_t flash2reg_crc_target_addr;
	uint32_t flash2reg_start_addr;
	uint32_t datalist[5];
	uint8_t buf_tmp[4];
	uint8_t flash2reg_data[flash2reg_header_len];
	size_t pos = CTS_FLASH_2_REG_ADDR - CTS_FLASH_2_REG_START_ADDR;
	int offset = 0;
	int i, j;

	flash2reg_init_en_addr = 0x0000C35A;
	flash2reg_length_addr = flash2reg_calc_len;
	flash2reg_crc_den_addr = 0xFFFFFFFF;
	flash2reg_crc_target_addr = cts_calc_crc32(buff, flash2reg_calc_len, 0);
	flash2reg_start_addr = CTS_FLASH_2_REG_INFO_START_ADDR;
	
	datalist[0] = flash2reg_init_en_addr;
	datalist[1] = flash2reg_length_addr;
	datalist[2] = flash2reg_crc_den_addr;
	datalist[3] = flash2reg_crc_target_addr;
	datalist[4] = flash2reg_start_addr;
	for (i = 0; i < 5; i++) {
		buf_tmp[0] = (uint8_t)(datalist[i]);
		buf_tmp[1] = (uint8_t)(datalist[i] >> 8);
		buf_tmp[2] = (uint8_t)(datalist[i] >> 16);
		buf_tmp[3] = (uint8_t)(datalist[i] >> 24);
		for (j = 0; j < 4; j++) {
			flash2reg_data[offset + j] = buf_tmp[j];
		}
		offset += 4;
	}

	MEMCPY(flash2reg_buff + pos, flash2reg_data, flash2reg_header_len);
	All_LOG("#Captest: Dump flash2reg header");
	cts_dump_flash2reg_data(flash2reg_buff + pos, flash2reg_header_len);
}

static int cts_restore_flash2reg(uint8_t *trim_value)
{
	int ret;
	int i;
	int retries;
	uint8_t *buff = NULL;
	uint8_t *buff_start = NULL;

	buff = (uint8_t *)MALLOC(flash2reg_ext_len);
	if (!buff) {
		All_LOG("#Captest: Malloc buff failed");
		return -1;
	}

	if (flash2reg_buff)
		MEMSET(flash2reg_buff, 0xFF, flash2reg_len);
	else
	flash2reg_buff = (uint8_t *)MALLOC(flash2reg_len);
	if (!flash2reg_buff) {
		All_LOG("#Captest: Malloc flash2reg_buff failed");
		return -1;
	}

	buff_start = buff + flash2reg_trim_len;

	for (retries = 0; retries < 3; retries++) {
		ret = cts_read_flash2reg_buff(buff, CTS_FLASH_2_REG_INFO_START_ADDR, flash2reg_ext_len);
		if (ret) {
			THP_LOGE("#Captest: Read flash2reg from 0x%x, size %d failed",
				CTS_FLASH_2_REG_INFO_START_ADDR, flash2reg_ext_len);
			continue;
		}

		/* Copy data to flash2reg_buff */
		MEMCPY(flash2reg_buff, buff_start, flash2reg_len);

		ret = cts_check_flash2reg(flash2reg_buff, flash2reg_default, flash2reg_chk_len);
		if (ret) {
			THP_LOGE("Read from 0x%x to 0x%x no match",
				CTS_FLASH_2_REG_START_ADDR, CTS_FLASH_2_REG_INFO_LAST_ADDR);
			THP_LOGE("Memcpy flash2reg_default, size: %d", flash2reg_chk_len);
			MEMCPY(flash2reg_buff, flash2reg_default, flash2reg_chk_len);
		}
		MEMCPY(flash2reg_buff + flash2reg_chk_len, trim_value, CTS_FLASH2REG_OSC_LEN);

		/* calc header info */
		MEMCPY(buff_start, flash2reg_buff, flash2reg_len);
		cts_flash2reg_header(buff);
		All_LOG("#Captest: Dump flash2reg from 0x%x~0x%x",
			CTS_FLASH_2_REG_START_ADDR, CTS_FLASH_2_REG_INFO_LAST_ADDR);
		cts_dump_flash2reg_data(flash2reg_buff,
			CTS_FLASH_2_REG_INFO_LAST_ADDR - CTS_FLASH_2_REG_START_ADDR);

		ret = cts_write_flash2reg_buff(FLASH_CMD_ERASE_SECTOR, CTS_FLASH_2_REG_START_ADDR, 0,
			flash2reg_len, flash2reg_buff);
		if (ret) {
			All_LOG("#Captest: write flash2reg_buff to 0x%x, size %d failed",
				CTS_FLASH_2_REG_START_ADDR, flash2reg_len);
			continue;
		}

		MEMSET(buff, 0xFF, flash2reg_ext_len);
		ret = cts_read_flash2reg_buff(buff, CTS_FLASH_2_REG_INFO_START_ADDR, flash2reg_ext_len);
		if (ret) {
			All_LOG("#Captest: read flash2reg from 0x%x, size %d failed",
				CTS_FLASH_2_REG_INFO_START_ADDR, flash2reg_ext_len);
			continue;
		}

		for (i = 0; i < flash2reg_len; i++) {
			if (flash2reg_buff[i] != buff_start[i]) {
				THP_LOGE("Read from flash buf_start[%d]:0x%x != dst[%d]:0x%x",
					i, buff_start[i], i, flash2reg_buff[i]);
				ret = -1;
				break;
			}
		}
		if (ret)
			continue;
		else
			break;
	}
	if (retries >= 3) {
		All_LOG("#Captest: Restore flash2reg failed");
		ret = -1;
	} else {
		All_LOG("#Captest: Restore flash2reg success");
	}

	LFREE(buff);
	LFREE(flash2reg_buff);

	return ret;
}

static int cts_check_flash2reg_buf(uint8_t *buf, size_t len)
{
	int ret, i, retries;
	uint8_t flash2reg[20];
	
	uint32_t flash2reg_init_en_addr;
	uint32_t flash2reg_length_addr;
	uint32_t flash2reg_crc_den_addr;
	uint32_t flash2reg_crc_target_addr;
	uint32_t flash2reg_start_addr;
	uint32_t read_init_en_addr = 0;
	uint32_t read_length_addr = 0;
	uint32_t read_crc_den_addr = 0;
	uint32_t read_crc_target_addr = 0;
	uint32_t read_start_addr = 0;
	
	uint8_t flash2reg_info_data[CTS_FLASH_2_REG_INFO_LAST_ADDR - CTS_FLASH_2_REG_INFO_START_ADDR];

	uint32_t flash2reg_info_len = CTS_FLASH_2_REG_INFO_LAST_ADDR - CTS_FLASH_2_REG_INFO_START_ADDR;

	for (i = 0; i < len; i++) {
		if (buf[i] != flash2reg_default[i]) {
			All_LOG("#Captest: Read from 0x%x buf[%d]:0x%x != default[%d]:0x%x default",
				CTS_FLASH_2_REG_START_ADDR, i, buf[i], i, flash2reg_default[i]);
			return 1;
		}
	}

	for (retries = 0; retries < 3; retries++) {
		/* Read flash2reg header: 20bytes */
		ret = cts_read_flash2reg_buff(flash2reg, CTS_FLASH_2_REG_ADDR, sizeof(flash2reg));
		if (ret) {
			THP_LOGE("#Captest: Read from 0x%x failed", CTS_FLASH_2_REG_ADDR);
			continue;
		}
		All_LOG("#Captest: 2. read flash2reg header, len: %d", sizeof(flash2reg));
		cts_dump_flash2reg_data(flash2reg, sizeof(flash2reg));

		/* Read flash2reg info table: 80bytes */
		ret = cts_read_flash2reg_buff(flash2reg_info_data, CTS_FLASH_2_REG_INFO_START_ADDR, flash2reg_info_len);
		if (ret) {
			All_LOG("#Captest: Read from 0x%x failed", CTS_FLASH_2_REG_INFO_START_ADDR);
			continue;
		}
		All_LOG("#Captest: 3. read flash2reg info, len: %d", flash2reg_info_len);
		cts_dump_flash2reg_data(flash2reg_info_data, flash2reg_info_len);

		flash2reg_init_en_addr = 0x0000C35A;
		flash2reg_length_addr =	flash2reg_info_len;
		flash2reg_crc_den_addr = 0xFFFFFFFF;
		flash2reg_crc_target_addr = cts_calc_crc32(flash2reg_info_data, flash2reg_info_len, 0);
		flash2reg_start_addr = CTS_FLASH_2_REG_INFO_START_ADDR;

		for (i = 0; i < 4; i++) {
			read_init_en_addr |= (uint32_t)flash2reg[i] << (i * 8);
			read_length_addr |= (uint32_t)flash2reg[i + 4] << (i * 8);
			read_crc_den_addr |= (uint32_t)flash2reg[i + 8] << (i * 8);
			read_crc_target_addr |= (uint32_t)flash2reg[i + 12] << (i * 8);
			read_start_addr |= (uint32_t)flash2reg[i + 16] << (i * 8);
		}
		if (flash2reg_init_en_addr != read_init_en_addr) {
			All_LOG("#Captest: read init_en_addr 0x%x != 0x%x",
				read_init_en_addr, flash2reg_init_en_addr);
			continue;
		}
		if (flash2reg_crc_den_addr != read_crc_den_addr) {
			All_LOG("#Captest: read crc_den_addr 0x%x != 0x%x",
				read_crc_den_addr, flash2reg_crc_den_addr);
			continue;
		}
		if (flash2reg_start_addr != read_start_addr) {
			All_LOG("#Captest: read start_addr 0x%x != 0x%x",
				read_start_addr, flash2reg_start_addr);
			continue;
		}
		if (flash2reg_length_addr != read_length_addr) {
			All_LOG("#Captest: read length_addr 0x%x != 0x%x",
				read_length_addr, flash2reg_length_addr);
			return 1;
		}
		if (flash2reg_crc_target_addr != read_crc_target_addr) {
			All_LOG("#Captest: read crc_target_addr 0x%x != 0x%x",
				read_crc_target_addr, flash2reg_crc_target_addr);
			return 1;
		}
		break;
	}

	if (retries >= 3) {
		All_LOG("#Captest: check flash2reg retries: %d", retries);
		return -1;
	}
	return 0;
}

static int cts_write_check_flash2reg(uint8_t *dst, uint8_t *src, size_t len_reg, size_t len_2k)
{
	int ret, i, j, k;
	uint8_t *buff = NULL;
	uint8_t *buff_start = NULL;
	size_t len_trim = CTS_FLASH_2_REG_START_ADDR - CTS_FLASH_2_REG_INFO_START_ADDR;
	size_t len_total = len_trim + len_2k;

	size_t flash2reg_info_len = CTS_FLASH_2_REG_INFO_LAST_ADDR - CTS_FLASH_2_REG_INFO_START_ADDR;
	uint32_t flash2reg_init_en_addr;
	uint32_t flash2reg_length_addr;
	uint32_t flash2reg_crc_den_addr;
	uint32_t flash2reg_crc_target_addr;
	uint32_t flash2reg_start_addr;
	uint32_t datalist[5];
	uint8_t buf_tmp[4];
	uint8_t flash2reg_data[20];
	int offset = 0;

	THP_LOGE("Malloc bug_tmp, size: %d", len_total);
	buff = (uint8_t *)MALLOC(len_total);
	if (!buff) {
		All_LOG("#Captest: Malloc buff failed");
		return -1;
	}

	buff_start = buff + len_trim;
	
	for (i = 0; i < 3; i++) {
		MEMSET(buff, 0xFF, len_total);
		THP_LOGE("No: %d, Read data from 0x%x, size: %d",
			i, CTS_FLASH_2_REG_INFO_START_ADDR, len_total);
		ret = cts_read_flash2reg_buff(buff, CTS_FLASH_2_REG_INFO_START_ADDR, len_total);
		if (ret) {
			All_LOG("#Captest: cts_read_flash2reg_buff failed");
			continue;
		}
		All_LOG("#Captest: 4. Read flash2reg info, len: %d", flash2reg_info_len);
		cts_dump_flash2reg_data(buff, flash2reg_info_len);

		THP_LOGE("Memcpy flash2reg info size: 0x%x", len_reg);
		MEMCPY(dst, src, len_reg);
		MEMCPY(buff_start, src, len_reg);
		/* Copy osc trim from 0x30FFD~0x30FFF to 0x3103D~0x3103F*/
		MEMCPY(dst + len_reg, buff + (len_trim - 3), 3);
		MEMCPY(buff_start + len_reg, buff + (len_trim - 3), 3);
		All_LOG("#Captest: 5. Copy flash2reg info for calc crc, len: %d", flash2reg_info_len);
		cts_dump_flash2reg_data(buff, flash2reg_info_len);

		flash2reg_init_en_addr = 0x0000C35A;
		flash2reg_length_addr = flash2reg_info_len;
		flash2reg_crc_den_addr = 0xFFFFFFFF;
		flash2reg_crc_target_addr = cts_calc_crc32(buff, flash2reg_info_len, 0);
		flash2reg_start_addr = CTS_FLASH_2_REG_INFO_START_ADDR;
		
		datalist[0] = flash2reg_init_en_addr;
		datalist[1] = flash2reg_length_addr;
		datalist[2] = flash2reg_crc_den_addr;
		datalist[3] = flash2reg_crc_target_addr;
		datalist[4] = flash2reg_start_addr;
		for (j = 0; j < 5; j++) {
			buf_tmp[0] = (uint8_t)(datalist[j]);
			buf_tmp[1] = (uint8_t)(datalist[j] >> 8);
			buf_tmp[2] = (uint8_t)(datalist[j] >> 16);
			buf_tmp[3] = (uint8_t)(datalist[j] >> 24);
			for (k = 0; k < 4; k++) {
				flash2reg_data[offset + k] = buf_tmp[k];
			}
			offset += 4;
		}

		THP_LOGE("Memcpy flash2reg header size: %d", sizeof(flash2reg_data));
		MEMCPY(dst + (len_2k - sizeof(flash2reg_data)), flash2reg_data, sizeof(flash2reg_data));

		ret = cts_write_flash2reg_buff(FLASH_CMD_ERASE_SECTOR, CTS_FLASH_2_REG_START_ADDR, 0, len_2k, dst);
		if (ret) {
			All_LOG("#Captest: cts_write_flash2reg_buff failed");
			continue;
		}

		MEMSET(buff, 0xFF, len_total);
		ret = cts_read_flash2reg_buff(buff, CTS_FLASH_2_REG_INFO_START_ADDR, len_total);
		if (ret) {
			All_LOG("#Captest: cts_read_flash2reg_buff failed");
			continue;
		}
		All_LOG("#Captest: 6. readback flash2reg info, len: %d", flash2reg_info_len);
		cts_dump_flash2reg_data(buff, flash2reg_info_len);

		for (j = 0; j < len_2k; j++) {
			if (dst[j] != buff_start[j]) {
				All_LOG("#Captest: Read from flash buf_start[%d]:0x%x != dst[%d]:0x%x",
					j, buff_start[j], j, dst[j]);
				ret = -1;
				break;
			}
		}
		if (ret)
			continue;
		else
			break;
	}
	if (i >= 3) {
		All_LOG("#Captest: Write & check flash2reg failed");
		All_LOG("#Captest: cts_dump_flash2reg_default(read):");
		cts_dump_flash2reg_data(buff, len_total);
		ret = -1;
	} else {
		All_LOG("#Captest: Write & check flash2reg success");
	}

	LFREE(buff);
	
	return ret;
}

static void cts_check_flash2reg_test(void)
{
	int ret;
	uint8_t reg_value[3] = {0, 0, 0};
	uint8_t flash_value[3] = {0, 0, 0};

	if (flash2reg_buff)
		MEMSET(flash2reg_buff, 0xFF, flash2reg_len);
	else
		flash2reg_buff = (uint8_t *)MALLOC(flash2reg_len);
	if (!flash2reg_buff) {
		All_LOG("#Captest: Malloc flash2reg_buff failed");
		goto err_return;
	}

	All_LOG("#Captest: 1. default flash2reg data, len: %d", sizeof(flash2reg_default));
	cts_dump_flash2reg_data(flash2reg_default, sizeof(flash2reg_default));

	THP_LOGE("Read flash2reg from 0x%x, size: %d", CTS_FLASH_2_REG_START_ADDR, flash2reg_len);
	ret = cts_read_flash2reg_buff(flash2reg_buff, CTS_FLASH_2_REG_START_ADDR, flash2reg_len);
	if (ret) {
		All_LOG("#Captest: Read flash2reg buff failed");
		goto err_return;
	}
 	
	ret = cts_check_flash2reg_buf(flash2reg_buff, sizeof(flash2reg_default));
	if (ret < 0) {
		THP_LOGE("#Captest: cts_check_flash2reg_buf failed");
		goto err_return;
	} else if (ret > 0) {
		ret = cts_write_check_flash2reg(flash2reg_buff, flash2reg_default, sizeof(flash2reg_default), flash2reg_len);
		if (ret) {
			THP_LOGE("#Captest: cts_write_check_flash2reg failed");
			goto err_return;
		}
		THP_LOGE("#Captest: cts_check_flash2reg_buf success");
	}

	MEMCPY(flash_value, flash2reg_buff + flash2reg_chk_len, sizeof(flash_value));

	cts_reset_device_inspect();
	cts_rw_flash_prework();
	ret = cts_drw_read_raw(0x71031, reg_value, sizeof(reg_value));
	if (ret) {
		All_LOG("#Captest: Read value from 0x%x failed", 0x71031);
	} else {
		All_LOG("#Captest: Read from 0x%x reg_value[%d]=0x%x, reg_value[%d]=0x%x, reg_value[%d]=0x%x",
			0x71031, 0, reg_value[0], 1, reg_value[1], 2, reg_value[2]);
	}
	if ((reg_value[0] != (flash_value[0] & 0x3F)) || (reg_value[1] != flash_value[1])
		|| (reg_value[2] != (flash_value[2] & 0x01))) {
		All_LOG("#Captest: osc trim value not match");
		All_LOG("#Captest: register value 0x%x 0x%x 0x%x", reg_value[0], reg_value[1], reg_value[2]);
		All_LOG("#Captest: flash value 0x%x 0x%x 0x%x", flash_value[0], flash_value[1], flash_value[2]);
	} else {
		All_LOG("#Captest: osc trim value match");
	}
	
err_return:
	LFREE(flash2reg_buff);
	return;
}

void cts_test_osc_trim(uint16_t real_sync, uint8_t trigger_ratio,
	uint8_t max_ratio, uint16_t osc_trim, uint16_t osc_trim_fine)
{
	int ret;
	uint8_t trim_value[3];
	uint8_t reg_value[3] = {0, 0, 0};

	/* Readback osc trim value from flash */
	ret = cts_set_spi_speed(SPI_SPEED_PROG);
	if (ret) {
		THP_LOGW("Set spi speed failed");
	}
	cts_rw_flash_prework();

#ifdef CTS_READ_FLASH_SECTION_ENABLE
	THP_LOGE("Read CP/FT trim value");
	cts_read_flash_info(FLASH_CMD_FAST_READ, CTS_CP_FT_TRIM_ADDR, 80);
	THP_LOGE("Read project id");
	cts_read_flash_info(FLASH_CMD_FAST_READ, CTS_PROJECT_ID_ADDR, 32);
	THP_LOGE("Read flash2reg value");
	cts_read_flash_info(FLASH_CMD_FAST_READ, CTS_FLASH_2_REG_ADDR, 20);
	THP_LOGE("Read hsync trim value");
	cts_read_flash_info(FLASH_CMD_FAST_READ, CTS_HSYNC_TRIM_ADDR, 8);
	THP_LOGE("Read CP/FT trim invert value");
	cts_read_flash_info(FLASH_CMD_FAST_READ, CTS_CP_FT_TRIM_INVERT_ADDR, 40);

	cts_read_reg_info(0x71030, 4);
	cts_read_reg_info(0x7102C, 4);
	cts_read_reg_info(0x71014, 4);
#endif

	trim_value[0] = (uint8_t)(osc_trim & 0xFF);
	trim_value[1] = (uint8_t)(osc_trim_fine & 0xFF);
	trim_value[2] = (uint8_t)((osc_trim_fine >> 8) & 0xFF);
	All_LOG("#Captest: read trim: 0x%x, trim fine: 0x%x, 0x%x",
		trim_value[0], trim_value[1], trim_value[2]);

	All_LOG("Dump flash2reg_default");
	cts_dump_flash2reg_data(flash2reg_default, sizeof(flash2reg_default));

	/* read & save flash2reg: addr 0x31000, size 2K */
	cts_restore_flash2reg(trim_value);

	cts_reset_device_inspect();
	cts_rw_flash_prework();
	ret = cts_drw_read_raw(0x71031, reg_value, sizeof(reg_value));
	if (ret) {
		All_LOG("#Captest: Read value from 0x%x failed", 0x71031);
	} else {
		All_LOG("#Captest: Read from 0x%x reg_value[%d]=0x%x, reg_value[%d]=0x%x, reg_value[%d]=0x%x",
			0x71031, 0, reg_value[0], 1, reg_value[1], 2, reg_value[2]);
	}
	if ((reg_value[0] != (trim_value[0] & 0x3F)) || (reg_value[1] != trim_value[1])
		|| (reg_value[2] != (trim_value[2] & 0x01))) {
		All_LOG("#Captest: osc trim value not match");
		All_LOG("#Captest: register value 0x%x 0x%x 0x%x", reg_value[0], reg_value[1], reg_value[2]);
		All_LOG("#Captest: flash value 0x%x 0x%x 0x%x", trim_value[0], trim_value[1], trim_value[2]);
	} else {
		All_LOG("#Captest: osc trim value match");
	}

	ret = cts_set_spi_speed(SPI_SPEED);
	if (ret) {
		THP_LOGW("Set spi speed failed");
	}

	return;
}

int cts_get_osc_trim(void)
{
	int ret;
	uint16_t osc_trim, osc_trim_fine;

	ret = cts_tcs_get_osc_trim(&osc_trim);
	if (ret) {
		THP_LOGE("Get osc trim failed");
	} else {
		All_LOG("#Captest: osc trim: 0x%x", osc_trim);
	}

	ret = cts_tcs_get_osc_trim_fine(&osc_trim_fine);
	if (ret) {
		THP_LOGE("Get osc trim fine failed");
	} else {
		All_LOG("#Captest: osc trim fine: 0x%x", osc_trim_fine);
	}

	return ret;
}

static bool cts_hsync_test_prework(void)
{
	int ret;
	uint8_t reg_value[2];
	bool do_osc_trim = false;

	if (do_osc_trim_buff)
		MEMSET(do_osc_trim_buff, 0x55, flash2reg_len);
	else
		do_osc_trim_buff = (uint8_t *)MALLOC(flash2reg_len);
	if (!do_osc_trim_buff) {
		All_LOG("Malloc do_osc_trim_buff failed");
		do_osc_trim = false;
	}

	ret = cts_set_spi_speed(SPI_SPEED_PROG);
	if (ret) {
		All_LOG("#Captest: Set spi speed failed");
	}
	//cts_reset_device_inspect();
	cts_rw_flash_prework();

	/* Get osc trim info: cali done & cali cnt */
	if (do_osc_trim_buff) {
		ret = cts_read_flash2reg_buff(do_osc_trim_buff, CTS_OSC_TRIM_CALI_INFO, flash2reg_len);
		if (ret) {
			All_LOG("#Captest: Read flash2reg_info from 0x%x failed", CTS_OSC_TRIM_CALI_INFO);
		} else {
			All_LOG("#Captest: cali_info before: %#04x, %#04x, %#04x, %#04x",
				do_osc_trim_buff[0], do_osc_trim_buff[1],
				do_osc_trim_buff[2], do_osc_trim_buff[3]);

			uint8_t cali_done = do_osc_trim_buff[0];
			uint8_t cali_cnt = do_osc_trim_buff[1];
			uint8_t rev_cali_done = do_osc_trim_buff[2];
			uint8_t rev_cali_cnt = do_osc_trim_buff[3];
			if ( ((uint8_t)cali_done) != ((uint8_t)~rev_cali_done) || 
					((uint8_t)cali_cnt) != ((uint8_t)~rev_cali_cnt))
			{
				if ((cali_done == 0xFF) && (cali_cnt == 0xFF)) {
					do_osc_trim = true;
				}
			}
		}
	}

	/******************* reload flash2reg info if 0x71031 is NULL *******************/
	ret = cts_drw_read_raw(0x71031, reg_value, 2);
	if (ret) {
		All_LOG("#Captest: Read value from 0x%x failed", 0x71031);
	} else {
		All_LOG("#Captest: Read from 0x%x reg_value[%d]=0x%x, reg_value[%d]=0x%x",
			0x71031, 0, reg_value[0], 1, reg_value[1]);
	}
	if (reg_value[0] == 0x00) {
		cts_check_flash2reg_test();
	} else {
		All_LOG("#Captest: No need retrim, read from 0x%x, value: 0x%x", 0x71031, reg_value[0]);
	}
	cts_reset_device_inspect();
	ret = cts_set_spi_speed(SPI_SPEED);
	if (ret) {
		All_LOG("#Captest: Set spi speed failed");
	}

	return do_osc_trim;
}

static void cts_write_osc_trim_cali_info(uint8_t *buf)
{
	int ret;

	ret = cts_set_spi_speed(SPI_SPEED_PROG);
	if (ret) {
		THP_LOGW("#Captest: Set spi speed failed");
	}
	cts_rw_flash_prework();
	
	ret = cts_write_flash2reg_buff(FLASH_CMD_ERASE_SECTOR, CTS_OSC_TRIM_CALI_INFO, 0,
		flash2reg_len, buf);
	if (ret) {
		All_LOG("#Captest: write flash2reg buff failed");
	}

	ret = cts_set_spi_speed(SPI_SPEED);
	if (ret) {
		THP_LOGW("#Captest: Set spi speed failed");
	}
	
	return;
}

void cts_set_cali_info(uint8_t osc_trim_cali_done, uint8_t osc_trim_cali_cnt)
{
	do_osc_trim_buff[0] = osc_trim_cali_done;
	do_osc_trim_buff[1] = osc_trim_cali_cnt;
	do_osc_trim_buff[2] = ~osc_trim_cali_done;
	do_osc_trim_buff[3] = ~osc_trim_cali_cnt;
	cts_write_osc_trim_cali_info(do_osc_trim_buff);
	All_LOG("#Captest: cali_info write %d: %X, %#04x, %#04x, %#04x", __LINE__,
		do_osc_trim_buff[0], do_osc_trim_buff[1],
		do_osc_trim_buff[2], do_osc_trim_buff[3]);
}

int cts_inspect_hsync(void)
{
	TIME_T start_time;
	int ret;
	size_t size = 2;
	uint8_t hsync_result[6];
	int retries = 0;
	uint8_t work_mode = 0;
	uint16_t value;
	int count = 2;
	int i;
	uint8_t trigger_ratio = 0;
	uint8_t trigger_set = 0;
	uint16_t max_ratio = 0;
	uint16_t real_sync;
	uint16_t osc_trim = 0xFF;
	uint16_t osc_trim_fine = 0xFF;

	int rc;
	uint32_t temp_buf;

	All_LOG("#Captest: hsync test cnt: %d", hsync_cnt);

	g_hsync_osc_trim_flag = cts_hsync_test_prework();
	if (do_osc_trim_buff) {
		osc_trim_cali_done = do_osc_trim_buff[0];
		osc_trim_cali_cnt = do_osc_trim_buff[1];
	}

	All_LOG("#Captest: hsync data begin");
	start_time = GET_CURR_TIME();

err_retry:
	/*
	 * enter factory mode
	 */
	for (retries = 0; retries < 3; retries++) {
		ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE);
		if (!ret) {
			THP_LOGI("Set firmware work mode success");
			break;
		}
		cts_mdelay(2);
			continue;
		}
	if (retries >= 3) {
		THP_LOGE("Set firmware work mode failed");
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}


	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf3:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf3 invalid");
	}
	for (retries = 0; retries < CTS_WORK_MODE_RETRY_CNT; retries++) {
		ret = cts_tcs_get_work_mode(&work_mode);
		if (ret) {
			THP_LOGE("Get firmware work mode failed %d, retries: %d", ret, retries);
		} else if (work_mode == CTS_FIRMWARE_WORK_MODE) {
			break;
		} else {
		THP_LOGI("Recv work mode %d != %d, retriest: %d",
			work_mode, CTS_FIRMWARE_WORK_MODE, retries);
		}
		cts_mdelay(CTS_WORK_MODE_RETRY_DELAY);

		rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
		THP_LOGE("ictest_buf3:0x%x",temp_buf);
		if (rc) {
			THP_LOGE("read ictest_buf3 invalid");
		}
	}
	if (work_mode != CTS_FIRMWARE_WORK_MODE) {
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	/****************** Get osc trim before calibration ******************/
	cts_get_osc_trim();
	/*********************************************************************/

	/*
	 * enable hsync test
	 */
	ret = cts_tcs_set_enable_hsync_test(CTS_ENABLE_HSYNC_TEST);
	if (ret) {
		ret = CTS_ERR_WORKMODE_FAILED;
		THP_LOGE("set enable hsync test failed %d", ret);
		goto err_reset_device;
	}

	for (i = 0; i < TEST_HSYNC_OSC_TRIM_RETRY; i++) {
		ret = cts_tcs_set_real_hsync(TEST_HSYNC_OSC_TRIM_TYPICAL);
		if (ret) {
			THP_LOGE("Set real hsync failed %d", ret);
			continue;
		}
		cts_mdelay(1);
		ret = cts_tcs_get_real_hsync(&real_sync);
		if (ret) {
			THP_LOGE("Get real hsync failed %d", ret);
			continue;
		} else if (real_sync != TEST_HSYNC_OSC_TRIM_TYPICAL) {
			THP_LOGE("Get real hsync %d != %d", real_sync, TEST_HSYNC_OSC_TRIM_TYPICAL);
			continue;
		}
	}
	if (real_sync != TEST_HSYNC_OSC_TRIM_TYPICAL) {
		THP_LOGE("Get real hsync invalid");
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	for (i = 0; i < TEST_HSYNC_OSC_TRIM_RETRY; i++) {
		ret = cts_tcs_set_osc_trim_max_ratio(TEST_HSYNC_OSC_TRIM_MAX_RATIO);
		if (ret) {
			THP_LOGE("Set osc trim max_ratio failed %d", ret);
			continue;
		}
		cts_mdelay(1);
		ret = cts_tcs_get_osc_trim_max_ratio(&max_ratio);
		if (ret) {
			THP_LOGE("Get osc trim max_ratio failed %d", ret);
			continue;
		} else if (max_ratio != TEST_HSYNC_OSC_TRIM_MAX_RATIO) {
			THP_LOGE("Get osc trim max_ratio %d != %d", max_ratio, TEST_HSYNC_OSC_TRIM_MAX_RATIO);
			continue;
		}
	}
	if (max_ratio != TEST_HSYNC_OSC_TRIM_MAX_RATIO) {
		THP_LOGE("Get osc trim max_ratio invalid");
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	if (!hsync_cnt)
		trigger_set = TEST_HSYNC_OSC_TRIM_MIN_RATIO;
	else
		trigger_set = TEST_HSYNC_OSC_TRIM_MIN_RATIO_1;
	for (i = 0; i < TEST_HSYNC_OSC_TRIM_RETRY; i++) {
		ret = cts_tcs_set_osc_trim_trigger_ratio(trigger_set);
		if (ret) {
			THP_LOGE("Set osc trim trigger_ratio failed %d", ret);
			continue;
		}
		cts_mdelay(1);
		ret = cts_tcs_get_osc_trim_trigger_ratio(&trigger_ratio);
		if (ret) {
			THP_LOGE("Get osc trim trigger_ratio failed %d", ret);
			continue;
		} else if (trigger_ratio != trigger_set) {
			THP_LOGE("Get osc trim trigger_ratio %d != %d", trigger_ratio, trigger_set);
			continue;
		}
	}
	if (trigger_ratio != trigger_set) {
		THP_LOGE("Get osc trim trigger_ratio invalid");
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	THP_LOGE("real hsync: %d, max_ratio: %d, trigger_ratio: %d",
		real_sync, max_ratio, trigger_ratio);

	/*
	 * read hsync result
	 */
	ret = cts_test_polling_hsyncdata(hsync_result, size);
	if (ret) {
		THP_LOGE("Read hsyncdata failed %d", ret);
		goto err_reset_device;
	} else {
		value = hsync_result[0] | hsync_result[1] << 8;
	}

	All_LOG("#Captest: ---------------------");
	
	if (!hsync_cnt) {
		All_LOG("#Captest: MIN: %d, MAX: %d", HSYNC_TEST_MIN_PID01, HSYNC_TEST_MAX_PID01);
	} else {
		All_LOG("#Captest: MIN: %d, MAX: %d", HSYNC_TEST_MIN_PID02, HSYNC_TEST_MAX_PID02);
	}
	All_LOG("#Captest: hsyncdata: %d", value);
	All_LOG("#Captest: osc trim: 0x%x", (hsync_result[2] | hsync_result[3] << 8));
	All_LOG("#Captest: osc trim fine: 0x%x", (hsync_result[4] | hsync_result[5] << 8));
	All_LOG("#Captest: ---------------------");
	if (!hsync_cnt) {
			if ((value >= HSYNC_TEST_MAX_PID01) || (value <= HSYNC_TEST_MIN_PID01)) {
				ret = 4;
			goto err_retrim;
		} else {
			if (g_hsync_osc_trim_flag && do_osc_trim_buff) {
				osc_trim_cali_done = OSC_TRIM_CALI_OK;
				osc_trim_cali_cnt = OSC_TRIM_CALI_CNT_DEF;
			}
			ret = 0;
			goto err_reset_device;
		}
	} else {
		if ((value > HSYNC_TEST_MAX_PID02) || (value < HSYNC_TEST_MIN_PID02)) {
			if (hsync_cnt == 2) {
				if (g_hsync_osc_trim_flag && do_osc_trim_buff) {
					osc_trim_cali_done = OSC_TRIM_CALI_NG;
					osc_trim_cali_cnt = OSC_TRIM_CALI_CNT_DEF;
				}
				ret = -1;
				goto err_reset_device;
			} else {
				ret = -1;
				goto err_retrim;
			}
		} else {
			if (g_hsync_osc_trim_flag && do_osc_trim_buff) {
				osc_trim_cali_done = OSC_TRIM_CALI_OK;
				osc_trim_cali_cnt = OSC_TRIM_CALI_CNT;
			}
			ret = 0;
			goto err_reset_device;
		}
	}

err_retrim:
	if (g_hsync_osc_trim_flag) {
		osc_trim = hsync_result[2] | hsync_result[3] << 8;
		osc_trim_fine = hsync_result[4] | hsync_result[5] << 8;
		All_LOG("#Captest: DEFAULT osc_trim: 0x%x, osc_trim_fine: 0x%x", osc_trim, osc_trim_fine);
		if ((real_sync == TEST_HSYNC_OSC_TRIM_TYPICAL) &&
			(trigger_ratio == trigger_set) &&
			(max_ratio == TEST_HSYNC_OSC_TRIM_MAX_RATIO)) {
			if ((osc_trim != 0xFFFF) && (osc_trim_fine != 0xFFFF)) {
				cts_test_osc_trim(real_sync, trigger_ratio, max_ratio, osc_trim, osc_trim_fine);
			}
			ret = 4;
		}
	} else {
		All_LOG("#Captest: no need osc trim cali, g_hsync_osc_trim_flag: %d, cali done: %X cur cnt: %d",
			g_hsync_osc_trim_flag, osc_trim_cali_done, osc_trim_cali_cnt);
	}

err_reset_device:
	cts_reset_device_inspect();
	if ((ret == CTS_ERR_DATA_FAILED || ret == CTS_ERR_WORKMODE_FAILED) && --count) {
		THP_LOGE("#Captest: Get hsyncdata, ret: %d, retries: %d", ret, count);
		goto err_retry;
	}

	THP_LOGI("Hsync test cost %ldms", ELAPSED_MS(start_time));

	All_LOG("#Captest: hsync data end");

	if (ret == CTS_ERR_RDY_TIMEOUT) {
		All_LOG("#Captest: get data rdy timeout, ret: %d(%s)", ret, "TIMEOUT_ERR");
		return 0;
	}  else if (ret == CTS_ERR_DATA_FAILED) {
		All_LOG("#Captest: hsync failed, ret: %d(%s)", ret, "DATA_ERR");
		return 0;
	} else if (ret == CTS_ERR_WORKMODE_FAILED) {
		All_LOG("#Captest: workmode failed, ret: %d(%s)", ret, "WORKMODE_ERR");
		return 0;
	}

	if (ret) {
		All_LOG("#Captest: hsync test: fail");
	} else {
		All_LOG("#Captest: hsync test: pass");
	}

	return ret;
}
#endif

#ifdef TEST_HSYNC_ONLY
#if 0
static void cts_get_cali_info(void)
{
	int ret;

	if (do_osc_trim_buff)
		MEMSET(do_osc_trim_buff, 0x55, flash2reg_len);
	else
		do_osc_trim_buff = (uint8_t *)MALLOC(flash2reg_len);
	if (!do_osc_trim_buff) {
		All_LOG("Malloc do_osc_trim_buff failed");
	}

	ret = cts_set_spi_speed(SPI_SPEED_PROG);
	if (ret) {
		All_LOG("#Captest: Set spi speed failed");
	}
	cts_rw_flash_prework();

	/* Get osc trim info: cali done & cali cnt */
	if (do_osc_trim_buff) {
		ret = cts_read_flash2reg_buff(do_osc_trim_buff, CTS_OSC_TRIM_CALI_INFO, flash2reg_len);
		if (ret) {
			All_LOG("#Captest: Read flash2reg_info from 0x%x failed", CTS_OSC_TRIM_CALI_INFO);
		} else {
			All_LOG("#Captest: cali_info %d: %X, %#04x, %#04x, %#04x", __LINE__,
				do_osc_trim_buff[0], do_osc_trim_buff[1],
				do_osc_trim_buff[2], do_osc_trim_buff[3]);
		}
	}

	cts_reset_device_inspect();
	ret = cts_set_spi_speed(SPI_SPEED);
	if (ret) {
		All_LOG("#Captest: Set spi speed failed");
	}
	return;
}
#endif

int cts_inspect_hsync_only(void)
{
	TIME_T start_time;
	int ret;
	size_t size = 2;
	uint8_t hsync_result[2] = { 0 };
	int retries = 0;
	uint8_t work_mode = 0;
	uint16_t value;
	int count = 1;

	int rc;
	uint32_t temp_buf;

	//cts_get_cali_info();

	All_LOG("#Captest: hsync data begin");
	start_time = GET_CURR_TIME();

err_retry:
	/*
	 * enter factory mode
	 */
	for (retries = 0; retries < 3; retries++) {
		ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE);
		if (!ret) {
			THP_LOGI("Set firmware work mode success");
			break;
		}
		cts_mdelay(2);
			continue;
		}
	if (retries >= 3) {
		THP_LOGE("Set firmware work mode failed");
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
	THP_LOGE("ictest_buf3:0x%x",temp_buf);
	if (rc) {
		THP_LOGE("read ictest_buf3 invalid");
	}
	for (retries = 0; retries < CTS_WORK_MODE_RETRY_CNT; retries++) {
		ret = cts_tcs_get_work_mode(&work_mode);
		if (ret) {
			THP_LOGE("Get firmware work mode failed %d, retries: %d", ret, retries);
		} else if (work_mode == CTS_FIRMWARE_WORK_MODE) {
			break;
		} else {
		THP_LOGI("Recv work mode %d != %d, retriest: %d",
			work_mode, CTS_FIRMWARE_WORK_MODE, retries);
		}
		cts_mdelay(35);
		rc = cts_tcs_read_u32attr(TP_STD_CMD_ICTEST_FLASH_CP_EN_RW, &temp_buf);
		THP_LOGE("ictest_buf3:0x%x",temp_buf);
		if (rc) {
			THP_LOGE("read ictest_buf3 invalid");
		}
	}
	if (work_mode != CTS_FIRMWARE_WORK_MODE) {
		ret = CTS_ERR_WORKMODE_FAILED;
		goto err_reset_device;
	}

	/*
	 * enable hsync test
	 */
	ret = cts_tcs_set_enable_hsync_test(CTS_ENABLE_HSYNC_TEST);
	if (ret) {
		ret = CTS_ERR_WORKMODE_FAILED;
		THP_LOGE("set enable hsync test failed %d", ret);
		goto err_reset_device;
	}

	/*
	 * read hsync result
	 */
	ret = cts_test_polling_hsyncdata(hsync_result, size);
	if (ret) {
		THP_LOGE("Read hsyncdata failed %d", ret);
		goto err_reset_device;
	} else {
		value = hsync_result[0] | hsync_result[1] << 8;
	}
#if 0
	All_LOG("#Captest: ---------------------");

	All_LOG("#Captest: MIN: %d, MAX: %d", HSYNC_TEST_MIN_PID04, HSYNC_TEST_MAX_PID04);
	All_LOG("#Captest: hsyncdata: %d", value);
	All_LOG("#Captest: ---------------------");
	if ((value > HSYNC_TEST_MAX_PID04_1) || (value < HSYNC_TEST_MIN_PID04_1)) {
		ret = -1;
		goto err_reset_device;
	}
#else
	if (!strcmp(project_id, PROJECT_ID_1)) {
		All_LOG("#Captest: MIN: %d, MAX: %d", HSYNC_TEST_MIN_PID01, HSYNC_TEST_MAX_PID01);
		All_LOG("#Captest: hsyncdata: %d", value);
		All_LOG("#Captest: ---------------------");
		if ((value > HSYNC_TEST_MAX_PID01) || (value < HSYNC_TEST_MIN_PID01)) {
			ret = -1;
			goto err_reset_device;
		} else {
			ret = 0;
			//goto err_reset_device;
		}
	} else if (!strcmp(project_id, PROJECT_ID_2)) {
		All_LOG("#Captest: MIN: %d, MAX: %d", HSYNC_TEST_MIN_PID02, HSYNC_TEST_MAX_PID02);
		All_LOG("#Captest: hsyncdata: %d", value);
		All_LOG("#Captest: ---------------------");
		if ((value > HSYNC_TEST_MAX_PID02) || (value < HSYNC_TEST_MIN_PID02)) {
			ret = -1;
			goto err_reset_device;
		} else {
			ret = 0;
			//goto err_reset_device;
		}
	} else {
	    All_LOG("#Captest:Invalid project_id: %s", project_id);
		All_LOG("#Captest: hsyncdata: %d", value);
		All_LOG("#Captest: ---------------------");
		ret = -1;
		goto err_reset_device;
	}
#endif

err_reset_device:
	cts_reset_device_inspect();
	if ((ret == CTS_ERR_DATA_FAILED || ret == CTS_ERR_WORKMODE_FAILED) && --count) {
		THP_LOGE("#Captest: Get hsyncdata, ret: %d, retries: %d", ret, count);
		goto err_retry;
	}

	THP_LOGI("Hsync test cost %ldms", ELAPSED_MS(start_time));

	All_LOG("#Captest: hsync data end");

	if (ret == CTS_ERR_RDY_TIMEOUT) {
		All_LOG("#Captest: get data rdy timeout, ret: %d(%s)", ret, "TIMEOUT_ERR");
		return 0;
	}  else if (ret == CTS_ERR_DATA_FAILED) {
		All_LOG("#Captest: hsync failed, ret: %d(%s)", ret, "DATA_ERR");
		return 0;
	} else if (ret == CTS_ERR_WORKMODE_FAILED) {
		All_LOG("#Captest: workmode failed, ret: %d(%s)", ret, "WORKMODE_ERR");
		return 0;
	}
	
	if (ret) {
		All_LOG("#Captest: hsync test: fail");
	} else {
		All_LOG("#Captest: hsync test: pass");
	}

	return ret;
}
#endif

#ifdef TEST_APCLK
int cts_inspect_ap_clk(void)
{
	int ret;
	uint8_t rdata[1];

	ret = cts_set_spi_speed(SPI_SPEED_PROG);
	if (ret) {
		All_LOG("#Captest: Set spi speed failed");
		ret = 1;
		goto err_return;
	}
	ret = cts_enter_drw_mode();
	if (ret) {
		All_LOG("#Captest: Enter prog mode failed");
		ret = 1;
		goto err_return;
	}

	ret = cts_drw_write_u8(0x73049, 0xFD);
	if (ret) {
		All_LOG("#Captest: Write 0x73049 failed");
		ret = 1;
		goto err_return;
	}

	cts_mdelay(10);
	
	ret = cts_drw_read_u8(0x73039, rdata);
	if (ret) {
		THP_LOGE("#Captest: Read addr 0x73039 failed");
		ret = 1;
		goto err_return;
	}
	All_LOG("#Captest: Read data 0x%x, result 0x%x", rdata[0], rdata[0]&BIT(1));
	if (rdata[0] & BIT(1)) {
		ret = -1;
	} else {
		ret = 0;
	}

err_return:
	if (ret) {
		if (ret < 0) {
			All_LOG("#Captest: ap clk test: fail");
		}
	} else {
		All_LOG("#Captest: ap clk test: pass");
	}
	
	cts_reset_device_inspect();
	if (cts_set_spi_speed(SPI_SPEED)) {
		All_LOG("#Captest: Set spi speed failed");
	}
	
	return ret;
}
#endif

#ifdef CTS_FOR_FFT_MODE
static void cts_dump_fftdata_s16(const char *desc, int index, const int16_t *data)
{
#define SPLIT_LINE_STR \
	"--------------------------------------------------------"\
	"--------------------------------------------------------"
#define ROW_NUM_FORMAT_STR  "%2d | "
#define COL_NUM_FORMAT_STR  "%-5hd "
#define DATA_FORMAT_STR     "%-5hd "

	int r, c, rows, cols;
	int32_t max, min, sum, average;
	int max_r, max_c, min_r, min_c;
	char line_buf[550];
	int count = 0;

	rows = 24;
	cols = 19;

	max = min = data[0];
	sum = 0;
	max_r = max_c = min_r = min_c = 0;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			int16_t val = data[r * cols + c];

			sum += val;
			if (val > max) {
				max = val;
				max_r = r;
				max_c = c;
			} else if (val < min) {
				min = val;
				min_r = r;
				min_c = c;
			}
		}
	}
	average = sum / (rows * cols);
	count = 0;
	count += snprintf(line_buf + count, sizeof(line_buf) - count,
			" %s test data frame %u MIN: [%u][%u]=%hd, MAX: [%u][%u]=%hd, AVG=%hd",
			desc, index, min_r, min_c, min, max_r, max_c, max, average);
	THP_LOGI(SPLIT_LINE_STR);
	THP_LOGE("%s", line_buf);
	THP_LOGI(SPLIT_LINE_STR);
	count = 0;
	count += snprintf(line_buf + count, sizeof(line_buf) - count, "   |  ");
	for (c = 0; c < cols; c++) {
		count += snprintf(line_buf + count, sizeof(line_buf) - count,
				COL_NUM_FORMAT_STR, c);
	}
	THP_LOGI("%s", line_buf);
	THP_LOGI(SPLIT_LINE_STR);
	for (r = 0; r < rows; r++) {
		count = 0;
		count += snprintf(line_buf + count, sizeof(line_buf) - count,
				ROW_NUM_FORMAT_STR, r);
		for (c = 0; c < cols; c++) {
			count += snprintf(line_buf + count, sizeof(line_buf) - count,
				DATA_FORMAT_STR, data[r * cols + c]);
		}
		THP_LOGE("%s", line_buf);
	}
	THP_LOGI(SPLIT_LINE_STR);
#undef SPLIT_LINE_STR
#undef ROW_NUM_FORMAT_STR
#undef COL_NUM_FORMAT_STR
#undef DATA_FORMAT_STR
}
#endif /* CTS_FOR_FFT_MODE */

uint32_t cts_inspect(void)
{
	uint32_t result = THP_AFE_INSPECT_OK;
	int ret = 0;
	int i;

	inspect_flag = 1;
	thp_dev_clear_frame_buffer(1);

	All_LOG("#Captest: %s-start afehal_inspect test", project_id);

#ifdef TEST_APCLK
	All_LOG("#Captest: ap_clk_pin test begin");
	ret = cts_inspect_ap_clk();
	if (ret < 0)
		//result |= THP_AFE_INSPECT_EPIN;
	All_LOG("#Captest: ap_clk_pin test end");
#endif

#ifdef TEST_HSYNC
	All_LOG("#Captest: hsync test begin");
	cts_mdelay(100);
	cts_reset_device_inspect();
	cts_mdelay(200);
#ifdef TEST_HSYNC_ONLY
	ret = cts_inspect_hsync_only();
	if (ret) {
		//result |= THP_AFE_INSPECT_ERC;
	}
#else
	do {
		ret = cts_inspect_hsync();
	} while ((ret >= 4) && (++hsync_cnt < CTS_HSYNC_TEST_CNT));
	if (ret) {
		result |= THP_AFE_INSPECT_ERC;
	}
	hsync_cnt = 0;
	if (g_hsync_osc_trim_flag && do_osc_trim_buff) {
		cts_set_cali_info(osc_trim_cali_done, osc_trim_cali_cnt);
	}
#endif
	LFREE(do_osc_trim_buff);
	All_LOG("#Captest: hsync test end");
#endif


#ifdef CTS_FOR_FFT_MODE
	THP_LOGE("------------------FFT MODE BEGIN------------------");

/* 451 freqs: 50K~500K */
#define CTS_FFT_DATA_SIZE		(451 * 2)
	int j;
	uint8_t workmode = -1;
	int16_t *data = NULL;

	if (data)
		MEMSET(data, 0, CTS_FFT_DATA_SIZE);
	else
		data = MALLOC(CTS_FFT_DATA_SIZE);

	for (j = 0; j < 3; j++) {
		ret = cts_tcs_set_work_mode(CTS_FFT_WORK_MODE);
		if (ret) {
			THP_LOGE("Set work mode to FFT MODE failed");
			goto reset_device;
		}
		cts_mdelay(50);

		ret = cts_tcs_get_work_mode(&workmode);
		if (ret) {
			THP_LOGE("Get current work mode failed");
			goto reset_device;
		} else if (workmode != CTS_FFT_WORK_MODE) {
			THP_LOGE("Get current workmode %d != %d, retries: %d",
				workmode, CTS_FFT_WORK_MODE, j);
			continue;
		}
		break;
	}

	THP_LOGI("Current workmode: %d, set workmode: %d", workmode, CTS_FFT_WORK_MODE);
	if (workmode != CTS_FFT_WORK_MODE)
		goto reset_device;

	cts_mdelay(120);

	for (j = 0; j < CTS_FFT_DATA_GET_TIMES; j++) {
		ret = cts_tcs_set_fft_mode(CTS_FFT_MODE_ENABLE);
		if (ret) {
			THP_LOGE("Set fft mode enable failed");
			goto reset_device;
		}
		
		ret = cts_test_polling_fftdata((uint16_t *)data, CTS_FFT_DATA_SIZE);
		if (ret) {
			THP_LOGE("Get fft data failed");
			goto reset_device;
		}

		cts_dump_fftdata_s16("fft_data", j, data);
	}

reset_device:
	cts_reset_device_inspect();

	THP_LOGE("------------------FFT MODE END------------------");
#endif	

#ifdef TEST_SPI_IIC_COMM
	All_LOG("#Captest: spi/iic comm test begin");
	ret = cts_inspect_spi_iic_comm();
	if (ret) {
		//result |= THP_AFE_INSPECT_ESPI;
	}
	All_LOG("#Captest: spi/iic comm test end");
#endif

#ifdef TEST_RESET
	All_LOG("#Captest: reset test begin");
	ret = cts_inspect_reset();
	if (ret) {
		//result |= THP_AFE_INSPECT_ESPI;
	}
	All_LOG("#Captest: reset test end");
#endif

#ifdef TEST_RAWDATA
	if (inspect_grid_data){
		//MEMSET(inspect_grid_data, 0, TOTAL_GRID_DATA_SIZE);
	}
	else
		inspect_grid_data = (uint16_t *)MALLOC(RAWDATA_TEST_FRAMES * FRAME_GRID_DATA_SIZE);

	if (inspect_line_data) {
		//MEMSET(inspect_line_data, 0, TOTAL_LINE_DATA_SIZE);
	}
	else
		inspect_line_data = (uint16_t *)MALLOC(LINEDATA_TEST_FRAMES * FRAME_LINE_DATA_SIZE);

	if (inspect_line_max_min)
		MEMSET(inspect_line_max_min, 0, TOTAL_LINE_MAX_MIN_DATA);
	else
		inspect_line_max_min = (uint16_t *)MALLOC(2 * FRAME_LINE_DATA_SIZE);
	
	All_LOG("#Captest: rawdata test begin");
	cts_mdelay(100);
	ret = cts_inspect_rawdata();
	if (ret) {
		result |= THP_AFE_INSPECT_ERAW;
	}
	All_LOG("#Captest: rawdata test end");
#endif

#ifdef TEST_NOISE
	if (inspect_noise_data) {
		MEMSET(inspect_noise_data, 0, TOTAL_NOISE_DATA_SIZE);
	} else {
		inspect_noise_data = (int16_t *)MALLOC(NOISE_TEST_FRAMES * FRAME_NOISE_DATA_SIZE);
	}
	
	All_LOG("#Captest: noise test begin");
	cts_mdelay(100);
	ret = cts_inspect_noise();
	if (ret) {
		//result |= THP_AFE_INSPECT_ENOISE;
	}
	All_LOG("#Captest: noise test end");
#endif

#ifdef TEST_SHORT
	if (inspect_short_data) {
		MEMSET(inspect_short_data, 0, TOTAL_SHORT_DATA_SIZE + TEST_SHORT_DATA_SIZE * 2);
	} else {
		inspect_short_data = (uint16_t *)MALLOC(TOTAL_SHORT_DATA_SIZE + TEST_SHORT_DATA_SIZE * 2);
	}

	All_LOG("#Captest: short test begin");
	cts_mdelay(100);
	ret = cts_inspect_short();
	if (ret) {
		result |= THP_AFE_INSPECT_ESHORT;
	}
	All_LOG("#Captest: short test end");
#endif

#ifdef TEST_OPEN
	if (inspect_open_data) {
		MEMSET(inspect_open_data, 0, TOTAL_GRID_DATA_SIZE);
	} else {
		inspect_open_data = (uint16_t *)MALLOC(RAWDATA_TEST_FRAMES * FRAME_GRID_DATA_SIZE);
	}

	All_LOG("#Captest: open test begin");
	
	cts_mdelay(100);
	
	for (i = 0; i < 3; i++) {
		ret = cts_inspect_open();
		if (!ret) {
			break;
		} else {
			All_LOG("#Captest: open test retries %d", i+1);
		}
	}
	if (ret)
		result |= THP_AFE_INSPECT_EOPEN;
	All_LOG("#Captest: open test end");
#endif

	cts_tcs_set_real_hsync(TEST_HSYNC_OSC_TRIM_TYPICAL);
	cts_mdelay(1);
	cts_tcs_set_osc_trim_max_ratio(TEST_HSYNC_OSC_TRIM_MAX_RATIO);
	cts_force_get_hw_cap();

	All_LOG("#Captest: finished afehal_inspect test, %s, 0x%x", result ? "fail" : "pass", result);

	cts_mdelay(200);
	inspect_flag = 0;

	if (stylus_enable) {
		All_LOG("Stylus need enabled");
		cts_enable_stylus_hpp3_0();
	}

	thp_dev_clear_frame_buffer(1);

	return result;
}

uint16_t *cts_get_inspect_grid_data(void)
{

	if (inspect_grid_data) {
		return inspect_grid_data;
	} else {
		return NULL;
	}
}

uint16_t *cts_get_inspect_line_data(void)
{
	if (inspect_line_data) {
		return inspect_line_data;
	} else {
		return NULL;
	}
}

int16_t *cts_get_inspect_noise(void)
{/*
	if (inspect_noise_data) {
		return inspect_noise_data + (FRAME_GRID_DATA_NODES + FRAME_LINE_DATA_NODES) * NOISE_TEST_FRAMES;
	} else {
		return NULL;
	}*/
		return NULL;
}
