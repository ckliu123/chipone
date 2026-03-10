#include "cts_hal.h"

#include <string.h>
#include <stdio.h>

#include "thp/thp_afe_hal.h"
#include "thp/thp_dev_itf.h"
#include "cts_utils.h"
#include "cts_spi.h"
#include "cts_tcs.h"
#include "cts_upfw.h"
#include "cts_core.h"
#include "cts_tool.h"
#include "cts_drw.h"

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
	FRAME_TYPE_6			= 0x0060,
	FRAME_TYPE_SUB_61		= 0x0061,
	FRAME_TYPE_SUB_62		= 0x0062,
	FRAME_TYPE_SUB_63		= 0x0063,
	FRAME_TYPE_SUB_64		= 0x0064,
} CTS_FRAME_TYPE_ENUM;

typedef struct {
	TIMEVAL_STRUCT			tv;
	size_t					framelen;
	uint8_t					frame[CTS_FRAME_MAX_SIZ];
} CTS_IOCTL_FRAME_STRUCT;

#ifdef CTS_SWITCH_SCAN_STATE_TRACK
struct scan_state_tracker {
	uint16_t	begin_tracking;
	uint16_t	required_scan_state;
	uint16_t	another_retry;
#define SCAN_STATE_TRACK_FRAME_MAX		4
	uint16_t	checked_frames;
};
static struct scan_state_tracker scan_state_tracker = { 0 };
#endif

unsigned char g_lock				= 0;			// mutex lock
uint16_t last_scan_state			= 0;
extern uint16_t s_scan_state;

char *project_id = PROJECT_ID_1;

static int nonblock					= 0;
static int suspend_flag				= 0;			//for shb mode, psensor, pocket palm
static int tui_flag					= 0;			//for tui mode
static int vendor_call_flag			= 0;			//for debug
static int sos_flag					= 0;			//for sos flow
int inspect_flag					= 0;			//for captest flow
bool stylus_enable					= false;		//for enable stylus after captest
uint16_t g_ext_clk_request			= 0;			//for external clock
bool dmd_trigger					= false;		//for DMDI trigger
bool dynamic_gain_triggered			= false;		//for firmware change gain value
bool dynamic_gain_flag				= false;		//for firmware change gain value

uint16_t g_noise[5]					= { 0, 0, 0, 0, 0 };
uint8_t debug_fw_fode_done			= 0; 
uint8_t rawdata_table_recive_cnt	= 0;
static int last_frame_in_idle		= 0;

#define CTS_FOLD_COMPEN_ON					0
#define CTS_NUM_SCAN_FREQ					4
#define TCS_RET_CODE_OK						0
#define TCS_CMD_VALUE						0x5105

#define RAW_DEST_DATA_PID02					11000//11300//13000//24000
//#define SINGLE_COFF_PID02                     220//320//4
#define RAW_DEST_DATA_PID03					10000//11300//13000//24000
#define SINGLE_COFF_PID03					750//320//4
#define SINGLE_COFF_PID03_DIFF				0//320//4
#define RAW_COMP

#define IDLE_GET_FRAME_TIMEOUT_MS			1500
#define ACTIVE_GET_FRAME_TIMEOUT_MS			250
#define CURRENT_FRAME_MIN_SIZ				11

static THP_AFE_FRAME_DATA_STRUCT			g_thp_frame;
static THP_AFE_STYLUS_FRAME_DATA_STRUCT		g_thp_stylus_frame;
static CTS_IOCTL_FRAME_STRUCT				g_ioctl_frame;
static uint32_t g_get_frame_timeout = 		IDLE_GET_FRAME_TIMEOUT_MS;

//#define MAX_NUM_SCAN_FREQ					10
//#define MAX_NUM_SCAN_RATE					5
extern uint16_t s_scan_freq[MAX_NUM_SCAN_FREQ];
extern uint16_t s_scan_rate[MAX_NUM_SCAN_RATE];
extern uint8_t s_scan_rate_num;
extern uint8_t s_curr_scan_rate;
uint16_t g_todo_scan_rate;

static int16_t *rawdata_table		= NULL;
bool compen_done					= false;
int fold_cnt						= 0;
static bool flag_0 = false, flag_1 = false, flag_2 = false, flag_3 = false;
#define CTS_FOLD_COMPEN_CNT_MAX				1
extern THP_AFE_SCAN_STATE_ENUM afe_state_store;

/* 
 * 0-> enter idle;			1-> reset_idle_baseline;
 * 2-> scan_rate;			3-> freq_point;
 */
uint8_t g_tcs_cmd[MAX_TCS_CMD_NUM]	= { 0 };
uint8_t g_freq_index				= 0;
uint8_t g_scan_rate					= 0;

/* 'T'-0x54, 'o'-0x6F, 'u'-0x75, 'c'-0x63, 'h'-0x68, 'T'-0x54, 'H'-0x48, 'P'-0x50 */
#define DUMMY								0x0102546863756F54
#define FINGER_NOISE_MAX_SIZ				5
#define STYLUS_NOISE_MAX_SIZ				4

static uint16_t coff_freq_pid_02[] = {

};

static uint16_t coff_freq_pid_03[] = {

};


#define RAW_DEST_DATA			15000
static uint16_t coff_freq[] = {

};

int cts_reset_device(void)
{
	All_LOG("Reset the device");
	thp_dev_reset(1);
	cts_mdelay(2);
	thp_dev_reset(0);
	cts_mdelay(2);
	thp_dev_reset(1);
	cts_mdelay(50);
	
	// cts_tcs_set_real_hsync(TEST_HSYNC_OSC_TRIM_TYPICAL);
	cts_mdelay(1);
	// cts_tcs_set_osc_trim_max_ratio(TEST_HSYNC_OSC_TRIM_MAX_RATIO);

	thp_dev_clear_frame_buffer(1);

	return 0;
}

int cts_reset_device_inspect(void)
{
	All_LOG("Reset the device");
	thp_dev_reset(1);
	cts_mdelay(2);
	thp_dev_reset(0);
	cts_mdelay(2);
	thp_dev_reset(1);
	cts_mdelay(250);

	thp_dev_clear_frame_buffer(1);

	return 0;
}

#ifdef CTS_READ_WAFERID_ENABLE
static int cts_get_wafer_id(void)
{
	int retries = 0;
	int ret, i;
	uint8_t status;
	uint8_t wafer_id[24];
	uint32_t flash_addr = 0x00030000;
	uint32_t sram_addr = 0x00000000;
	uint32_t data_length = 0x00000200;

	All_LOG("Read wafer id");
	MEMSET(wafer_id, 0, sizeof(wafer_id));
	ret = cts_set_spi_speed(SPI_SPEED_PROG);
	if (ret < 0) {
		THP_LOGW("Set spi speed failed");
		return -1;
	}
	ret = cts_enter_drw_mode();
	if (ret < 0) {
		THP_LOGE("Enter prog mode failed");
		return -1;
	}

	ret = cts_drw_write_u8(0x78000, 0x0B);
	if (ret) {
		THP_LOGE("Set SPI TxForword failed");
		return -1;
	}
	ret = cts_drw_write_u8(0x78003, 0x8F);
	if (ret) {
		THP_LOGE("Set EXT mode failed");
		return -1;
	}
	cts_mdelay(1);

	ret = cts_drw_write_u8(0x71034, 0x00);
	if (ret) {
		THP_LOGE("Reset flash voltage reg1 failed");
		return -1;
	}
	ret = cts_drw_write_u8(0x74084, 0x00);
	if (ret) {
		THP_LOGE("Reset flash voltage reg2 failed");
		return -1;
	}
	cts_mdelay(10);
/*
	ret = cts_drw_write_u8(0x74075, 0x00);
	if (ret) {
		THP_LOGE("Write 0x74075 failed");
		return -1;
	}
	ret = cts_drw_write_u8(0x74074, 0x00);
	if (ret) {
		THP_LOGE("Write 0x74074 failed");
		return -1;
	}
*/
	for (i = 0; i < 2; i++) {
		ret = cts_drw_write_u8(SFCTL_CMD_SEL, FLASH_CMD_FAST_READ);
		if (ret) {
			THP_LOGE("Set mass read failed");
			return -1;
		}
		ret = cts_drw_write_u32(SFCTL_FLASH_ADDR, flash_addr);
		if (ret) {
			THP_LOGE("Set flash addr failed");		
			return -1;
		}
		ret = cts_drw_write_u32(SFCTL_SRAM_ADDR, sram_addr);
		if (ret) {
			THP_LOGE("Set sram addr failed");
			return -1;
		}
		ret = cts_drw_write_u32(SFCTL_DATA_LENGTH, data_length);
		if (ret) {
			THP_LOGE("Set data length failed");
			return -1;
		}
	
		ret = cts_drw_write_u8(SFCTL_START_DEXC, 1);
		if (ret) {
			THP_LOGE("Start data transsion failed");
			return -1;
		}

		cts_mdelay(10);
		All_LOG("Read wafer id retries: %d", i+1);
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
		return -1;
	}

	ret = cts_drw_read_raw(sram_addr, wafer_id, sizeof(wafer_id));
	if (ret < 0) {
		THP_LOGE("Read wafer id failed");
		return -1;
	}
	
	/* foundry id, x/y, wafer num */
	All_LOG("waferid: 0x%x%x%x%x%x%x%x%x%x, 0x%x 0x%x, 0x%x",
		wafer_id[0], wafer_id[1], wafer_id[2], wafer_id[3], wafer_id[4],
		wafer_id[5], wafer_id[6], wafer_id[7], wafer_id[8],
		wafer_id[9], wafer_id[10],
		wafer_id[11]);
	All_LOG("invert waferid: 0x%x%x%x%x%x%x%x%x%x, 0x%x 0x%x, 0x%x",
		wafer_id[12], wafer_id[13], wafer_id[14], wafer_id[15], wafer_id[16],
		wafer_id[17], wafer_id[18], wafer_id[19], wafer_id[20],
		wafer_id[21], wafer_id[22],
		wafer_id[23]);

	return ret;
}
#endif

static int cts_prework(void)
{
	int ret = -1;
	uint32_t cur_ver = 0;
	uint32_t hwid = 0;

	/* Soft reset, double check */
	All_LOG("soft reset");
	if (cts_drw_write_u8(REGDEF_RSTCFG, 0xFE)) {
		THP_LOGE("Reset chip failed");
	}
	/****************************/
	cts_reset_device();

	if (!rawdata_table)
		rawdata_table = (int16_t *)MALLOC(FRAME_GRID_DATA_SIZE * CTS_NUM_SCAN_FREQ);

#ifdef CTS_READ_WAFERID_ENABLE
	int retries = 0;
	/*****************Read wafer id****************/
	do {
		ret = cts_get_wafer_id();
		if (ret) {
			All_LOG("Read wafer id failed, retries %d", retries);
			cts_reset_device();
		}
	} while(ret && (++retries < 5));
	All_LOG("soft reset after wafer id");
	if (cts_drw_write_u8(REGDEF_RSTCFG, 0xFE)) {
		THP_LOGE("Reset chip failed");
	}
	cts_reset_device();
	/**********************************************/
#endif

	ret = cts_set_spi_speed(SPI_SPEED);
	if (ret < 0) {
		THP_LOGW("Set spi speed failed");
	}

	cts_tcs_get_hw_id(&hwid);
	if ((hwid & CTS_IC_HWID_MASK) == CTS_IC_HWID) {
		goto get_fw_ver;
	} else {
		THP_LOGI("hwid = %#06x", hwid);
		goto update_firmware;
	}

get_fw_ver:
	ret = cts_tcs_get_fw_ver(&cur_ver);
	if (ret) {
		THP_LOGW("Get fw version failed");
		cur_ver = 0;
	}

update_firmware:
	THP_LOGI("Get firmware version: 0x%x", cur_ver);
	ret = cts_update_firmware(cur_ver);
	if (ret < 0) {
		THP_LOGE("Update firmware failed");
		return -1;
	} else if (ret == 1) {
		THP_LOGI("No need set spi speed");
		goto no_set_spi_speed;
	}

	compen_done = false;
	ret = cts_set_spi_speed(SPI_SPEED);
	if (ret < 0) {
		THP_LOGW("Set spi speed failed");
	}

no_set_spi_speed:
	ret = thp_dev_set_timeout(1500);
	if (ret) {
		THP_LOGE("Set timeout failed failed: rc=%d", ret);
		return -1;
	}

	ret = cts_force_get_hw_cap();
	if (ret < 0) {
		THP_LOGE("Get hw_cap failed");
		return -1;
	}

	return 0;
}

static int cts_postwork(void)
{
	THP_LOGI("postwork");
	LFREE(rawdata_table);
	return 0;
}

static void cts_dump_frame(CTS_FRAME_STRUCT *frame)
{
	uint8_t *debug = NULL;
	debug = (uint8_t *)&frame->debug_info.data[0];

	THP_LOGI("index:   %d, type:    %d", frame->header.frame_index, frame->header.frame_type);
	THP_LOGI("cur:     %04d, next:    %04d, freq:    %04d, rate:    %04d, state:   %04d, afe:     0x%x",
		frame->header.curr_size, frame->header.next_size, frame->header.scan_freq,
		frame->header.scan_rate, frame->header.scan_state, frame->header.afe_status);
	THP_LOGI("gesture: %04d, stylus:  %04d, f1:      %04d, f2:      %04d, n_f1:    %04d, n_f2:    %04d",
		frame->header.gesture_status, frame->header.stylus_status, frame->header.stylus_scan_freq_f1,
		frame->header.stylus_scan_freq_f2, frame->header.stylus_new_scan_freq_f1,
		frame->header.stylus_new_scan_freq_f2);
	THP_LOGI("TouchNum: %d, GstNum: %d, CurGstID: %d, GstFailInfo: %d, PosX: %d, PosY: %d",
		debug[0], debug[1], debug[2], debug[3], (debug[4] | (debug[5] << 8)), (debug[6] | (debug[7] << 8)));
	THP_LOGI("CurState: %d, LastStateFlag: %d, FrameMax: %d, FrameMin: %d",
		debug[8], debug[9], (int16_t)(debug[10] | (debug[11] << 8)), (int16_t)(debug[12] | (debug[13] << 8)));
	THP_LOGI("MultiFlag: %d[Charger_BIT0, PALM_BIT1, HSYNC_BIT2, STYLUS_BIT3, GAIN_BIT4]", debug[14]);
	THP_LOGI("Palm: %d, SelfSyncMode: %d, StylusStatus: %d", debug[15], debug[16], debug[17]);
	THP_LOGI("FrameInfo: [ %d, %d, %d, %d, %d, %d, %d, %d ]",
		debug[18], debug[19], debug[20], debug[21], debug[22], debug[23], debug[24], debug[25]);
	THP_LOGI("NoiseBaseAvg: [ %d, %d, %d, %d ]",
		(int16_t)(debug[26] | (debug[27] << 8)), (int16_t)(debug[28] | (debug[29] << 8)),
		(int16_t)(debug[30] | (debug[31] << 8)), (int16_t)(debug[32] | (debug[33] << 8)));
	THP_LOGI("NoiseBaseMax: [ %d, %d, %d, %d ]",
		(int16_t)(debug[34] | (debug[35] << 8)), (int16_t)(debug[36] | (debug[37] << 8)),
		(int16_t)(debug[38] | (debug[39] << 8)), (int16_t)(debug[40] | (debug[41] << 8)));
	THP_LOGI("NoiseValue: [ %d, %d, %d, %d ]",
		(int16_t)(debug[42] | (debug[43] << 8)), (int16_t)(debug[44] | (debug[45] << 8)),
		(int16_t)(debug[46] | (debug[47] << 8)), (int16_t)(debug[48] | (debug[49] << 8)));
	THP_LOGI("FlashHaveCal: %d, CaliCnt: %d, UpdateBaseToFlash: %d", debug[50], debug[51], debug[52]);
	THP_LOGI("IsUseFlashBase: %d, PoweronCaliDone: %d, CaliDone: %d", debug[53], debug[54], debug[55]);
	THP_LOGI("HopBaseThr: %d, CaliRun: %d, CaliFrameMax: %d, CaliFrameMin: %d",
		(debug[56] | (debug[57] << 8)), (debug[58] | (debug[59] << 8)),
		(int16_t)(debug[60] | (debug[61] << 8)), (int16_t)(debug[62] | (debug[63] << 8)));
	g_noise[0] = (int16_t)(debug[4] | (debug[5] << 8));		//(int16_t)debug[22];
	g_noise[1] = (int16_t)(debug[6] | (debug[7] << 8));
	debug_fw_fode_done = debug[15];
	if (debug[14] & DEBUG_DYNAIC_IA_GAIN_BIT) {
		//All_LOG("@@ Triggered dynamic gain, dynamic_gain_flag: %d, dynamic_gain_triggered: %d",
			//dynamic_gain_flag, dynamic_gain_triggered);
		dynamic_gain_flag = true;
	}
}

static void cts_sos_dump_frame(CTS_FRAME_STRUCT* frame)
{
	uint8_t* debug = NULL;
	debug = (uint8_t*)&frame->debug_info.data[0];

	All_LOG("index:   %d, type:    %d", frame->header.frame_index, frame->header.frame_type);
	All_LOG("cur:     %04d, next:    %04d, freq:    %04d, rate:    %04d, state:   %04d, afe:     0x%x",
		frame->header.curr_size, frame->header.next_size, frame->header.scan_freq,
		frame->header.scan_rate, frame->header.scan_state, frame->header.afe_status);
	All_LOG("gesture: %04d, stylus:  %04d, f1:      %04d, f2:      %04d, n_f1:    %04d, n_f2:    %04d",
		frame->header.gesture_status, frame->header.stylus_status, frame->header.stylus_scan_freq_f1,
		frame->header.stylus_scan_freq_f2, frame->header.stylus_new_scan_freq_f1,
		frame->header.stylus_new_scan_freq_f2);
	All_LOG("TouchNum: %d, GstNum: %d, CurGstID: %d, GstFailInfo: %d, PosX: %d, PosY: %d",
		debug[0], debug[1], debug[2], debug[3], (debug[4] | (debug[5] << 8)), (debug[6] | (debug[7] << 8)));
	All_LOG("CurState: %d, LastStateFlag: %d, FrameMax: %d, FrameMin: %d",
		debug[8], debug[9], (int16_t)(debug[10] | (debug[11] << 8)), (int16_t)(debug[12] | (debug[13] << 8)));
	All_LOG("MultiFlag: %d[Charger_BIT0, PALM_BIT1, HSYNC_BIT2, STYLUS_BIT3, GAIN_BIT4]", debug[14]);
	All_LOG("Palm: %d, SelfSyncMode: %d, StylusStatus: %d", debug[15], debug[16], debug[17]);
	All_LOG("FrameInfo: [ %d, %d, %d, %d, %d, %d, %d, %d ]",
		debug[18], debug[19], debug[20], debug[21], debug[22], debug[23], debug[24], debug[25]);
	All_LOG("NoiseBaseAvg: [ %d, %d, %d, %d ]",
		(int16_t)(debug[26] | (debug[27] << 8)), (int16_t)(debug[28] | (debug[29] << 8)),
		(int16_t)(debug[30] | (debug[31] << 8)), (int16_t)(debug[32] | (debug[33] << 8)));
	All_LOG("NoiseBaseMax: [ %d, %d, %d, %d ]",
		(int16_t)(debug[34] | (debug[35] << 8)), (int16_t)(debug[36] | (debug[37] << 8)),
		(int16_t)(debug[38] | (debug[39] << 8)), (int16_t)(debug[40] | (debug[41] << 8)));
	All_LOG("NoiseValue: [ %d, %d, %d, %d ]",
		(int16_t)(debug[42] | (debug[43] << 8)), (int16_t)(debug[44] | (debug[45] << 8)),
		(int16_t)(debug[46] | (debug[47] << 8)), (int16_t)(debug[48] | (debug[49] << 8)));
	All_LOG("FlashHaveCal: %d, CaliCnt: %d, UpdateBaseToFlash: %d", debug[50], debug[51], debug[52]);
	All_LOG("IsUseFlashBase: %d, PoweronCaliDone: %d, CaliDone: %d", debug[53], debug[54], debug[55]);
	All_LOG("HopBaseThr: %d, CaliRun: %d, CaliFrameMax: %d, CaliFrameMin: %d",
		(debug[56] | (debug[57] << 8)), (debug[58] | (debug[59] << 8)),
		(int16_t)(debug[60] | (debug[61] << 8)), (int16_t)(debug[62] | (debug[63] << 8)));
	g_noise[0] = (int16_t)(debug[4] | (debug[5] << 8));		//(int16_t)debug[22];
	g_noise[1] = (int16_t)(debug[6] | (debug[7] << 8));
	debug_fw_fode_done = debug[15];
}

static void cts_dump_rawdata_table(int16_t *table)
{
#define SPLIT_LINE_STR \
	"--------------------------------------------------------"\
	"--------------------------------------------------------"
#define ROW_NUM_FORMAT_STR  "%2d | "
#define COL_NUM_FORMAT_STR  "%-5d "
#define DATA_FORMAT_STR     "%-5hd "

	int r, c, freqs;
	char line_buf[550];
	int count = 0;
#if PATTERN_TYPE_1
	uint8_t rows = ROWS_PATTERN, cols = COLS_PATTERN;
#else
	uint8_t rows = ROWS, cols = COLS;
#endif

	if (!table)
		return;

	for (freqs = 0; freqs < 4; freqs++) {
		count = 0;
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count, "Freq: %d", freqs);
		APP_LOGI("%s", line_buf);
		count = 0;
		count += SNPRINTF(line_buf + count, sizeof(line_buf) - count, "   |  ");
		for (c = 0; c < cols; c++) {
			count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
					COL_NUM_FORMAT_STR, c);
		}
		APP_LOGI("%s", line_buf);
		APP_LOGI(SPLIT_LINE_STR);
		for (r = 0; r < rows; r++) {
			count = 0;
			count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
					ROW_NUM_FORMAT_STR, r);
			for (c = 0; c < cols; c++) {
				count += SNPRINTF(line_buf + count, sizeof(line_buf) - count,
					DATA_FORMAT_STR, table[r * cols + c + freqs * FRAME_GRID_DATA_NODES]);
			}
			APP_LOGI("%s", line_buf);
		}
		APP_LOGI(SPLIT_LINE_STR);
		APP_LOGI("\n");
	}
#undef SPLIT_LINE_STR
#undef ROW_NUM_FORMAT_STR
#undef COL_NUM_FORMAT_STR
#undef DATA_FORMAT_STR
}

static int cts_ioctl_get_frame(CTS_IOCTL_FRAME_STRUCT *ioctl_frame)
{
	int ret = -1;
	uint8_t *frame = ioctl_frame->frame;
	size_t framelen = sizeof(ioctl_frame->frame);
	TIMEVAL_STRUCT *tv = &ioctl_frame->tv;

	MEMSET(ioctl_frame, 0, sizeof(*ioctl_frame));
	THP_LOGI("+");
	ret = thp_dev_get_frame(frame, framelen, tv);
	if (ret != 0) {
		THP_LOGE("Ioctl get frame failed: %s(%d)", strerror(errno), errno);
		return ret;
	}
	THP_LOGI("-, %s", ret ? "NG" : "OK");
	ioctl_frame->framelen = framelen;

		cts_dump_frame((CTS_FRAME_STRUCT *)frame);

	return 0;
}

static int cts_check_ioctl_frame(CTS_IOCTL_FRAME_STRUCT *ioctl_frame)
{
	uint16_t crc16_calc;
	uint16_t frame_type;

	CTS_FRAME_STRUCT *cts_frame = (CTS_FRAME_STRUCT *)ioctl_frame->frame;
	CTS_TCS_RET_STRUCT *cts_tcs_ret;

	if (cts_frame->header.dummy != DUMMY) {
		//THP_LOGE("Invalid magic number: recv %#018x calc %#018x", cts_frame->header.dummy, DUMMY);
		THP_LOGI("Invalid magic number: recv 0x%x/0x%x calc 0x%x/0x%x", cts_frame->header.dummy,
			cts_frame->header.dummy >> 32, DUMMY, DUMMY >> 32);
		//return -1;
	}

	THP_LOGI("+");

	if (cts_frame->header.curr_size > CTS_FRAME_MAX_SIZ) {

			THP_LOGE("Invalid current frame size: %d > %d", cts_frame->header.curr_size, CTS_FRAME_MAX_SIZ);

		goto err_free;
	}

	if (cts_frame->header.curr_size < 14) {

			THP_LOGE("Invalid current frame size: %d < %d", cts_frame->header.curr_size, 14);

		goto err_free;
	}

	if (cts_frame->header.next_size > CTS_FRAME_MAX_SIZ) {

			THP_LOGE("Invalid next frame size: %d", cts_frame->header.next_size);

		goto err_free;
	}

	cts_tcs_ret = (CTS_TCS_RET_STRUCT *)(ioctl_frame->frame + cts_frame->header.curr_size);
	crc16_calc = cts_crc16((const uint8_t *)cts_frame, cts_frame->header.curr_size + 3);
	if (cts_tcs_ret->crc16 != crc16_calc) {

			THP_LOGE("Invalid crc16: recv %#06x calc %#06x", cts_tcs_ret->crc16, crc16_calc);

		goto err_free;
	}

	if (cts_tcs_ret->cmd != TCS_CMD_VALUE) {

			THP_LOGE("Invalid cmd value: recv %#06x expect %#06x", cts_tcs_ret->cmd, TCS_CMD_VALUE);

		goto err_free;
	}

	if (cts_tcs_ret->retcode != TCS_RET_CODE_OK) {

			THP_LOGE("Invalid ret code: recv %d expect %d", cts_tcs_ret->retcode, TCS_RET_CODE_OK);

		goto err_free;
	}

	frame_type = cts_frame->header.frame_type & FRAME_TYPE_MASK;
	if ((frame_type != FRAME_TYPE_1) &&
		(frame_type != FRAME_TYPE_2) &&
		(frame_type != FRAME_TYPE_3) &&
		(frame_type != FRAME_TYPE_6)) {

			THP_LOGE("Invalid frame type: %#06x", cts_frame->header.frame_type);

		goto err_free;
	}
	
	THP_LOGI("-, OK");
	return 0;

err_free:
	THP_LOGI("-, NG");
	return -1;
}

#ifdef CTS_DIFF_SCREEN_STATE_DATA_SIZE
static int cts_convert_grid_data_major(uint16_t freq, uint16_t *thp_data, int nrows, int ncols)
{
	int i;
	int pro_id;
	
	int16_t self_col[COLS];
	int16_t self_row[ROWS];
	uint16_t mutual_nodes = ROWS_PATTERN * COLS_PATTERN;

	uint16_t *coff;
	uint16_t *coff_temp;
	if (!strcmp(project_id, PROJECT_ID_1) || !strcmp(project_id, PROJECT_ID_3) || !strcmp(project_id, PROJECT_ID_7) ) {
		coff_temp = coff_freq_pid_02;
		pro_id = 0;
	} else {
		coff_temp = coff_freq_pid_03;
		pro_id = 1;
	}

	if (freq == s_scan_freq[0]) {
		coff = coff_temp;
	} else if (freq == s_scan_freq[1]) {
		coff = coff_temp;//coff_freq_1;
	} else if (freq == s_scan_freq[2]) {
		coff = coff_temp;//coff_freq_2;
	}else if (freq == s_scan_freq[3]) {
		coff = coff_temp;//coff_freq_3;
	} else {
		coff = coff_temp;
		APP_LOGE("BUG!! Unexpected scan freq %d", freq);
		//return -1;
	}

	THP_LOGI("scan freq %d, scan_state:%d", freq, s_scan_state);

	/* selfdata normalization */
	MEMCPY(self_col, thp_data + nrows*ncols, ncols*2);
	MEMSET(self_col + ncols, 0, ncols*2);
	MEMCPY(self_row, thp_data + nrows*ncols + ncols, nrows*2);
	MEMSET(self_row + nrows, 0, nrows*2*2);
	MEMCPY(thp_data + mutual_nodes, self_col, ncols*2);
	MEMCPY(thp_data + mutual_nodes + ncols, self_row, ROWS*2);
	MEMCPY(thp_data + mutual_nodes + ncols + ROWS, self_col + ncols, ncols*2);
		
	/* mutualdata normalization */
	for (i = 0; i < ROWS_PATTERN * COLS_PATTERN; i++) {
		if (0 == pro_id) {
#ifdef SINGLE_COFF_PID02
			thp_data[i] = thp_data[i] * SINGLE_COFF_PID02/100 + RAW_DEST_DATA_PID02;
#else
			thp_data[i] = thp_data[i] * coff[i]/1024 - RAW_DEST_DATA_PID02;
#endif
		} else if (1 == pro_id) {
#ifdef SINGLE_COFF_PID03
			if(i < nrows * ncols)
				thp_data[i] = thp_data[i] * SINGLE_COFF_PID03/100 - RAW_DEST_DATA_PID03;
#else
			if(i < nrows * ncols)
				thp_data[i] = thp_data[i] * coff[i]/1024 - RAW_DEST_DATA_PID03;
#endif
		}
	}
	MEMSET(thp_data + nrows*ncols, 0, nrows*2*ncols*2);

	return 0;
}

static int cts_convert_grid_data_minor(uint16_t freq, uint16_t *thp_data, int nrows, int ncols)
{
	int i;
	int pro_id;
	
	int16_t self_col[COLS];
	int16_t self_row[ROWS];
	uint16_t mutual_nodes = ROWS_PATTERN * COLS_PATTERN;

	uint16_t *coff;
	uint16_t *coff_temp;
	if (!strcmp(project_id, PROJECT_ID_1) || !strcmp(project_id, PROJECT_ID_3) || !strcmp(project_id, PROJECT_ID_7) ) {
		coff_temp = coff_freq_pid_02;
		pro_id = 0;
	} else {
		coff_temp = coff_freq_pid_03;
		pro_id = 1;
	}

	if (freq == s_scan_freq[0]) {
		coff = coff_temp;
	} else if (freq == s_scan_freq[1]) {
		coff = coff_temp;//coff_freq_1;
	} else if (freq == s_scan_freq[2]) {
		coff = coff_temp;//coff_freq_2;
	}else if (freq == s_scan_freq[3]) {
		coff = coff_temp;//coff_freq_3;
	} else {
		coff = coff_temp;
		APP_LOGE("BUG!! Unexpected scan freq %d", freq);
		//return -1;
	}

	THP_LOGI("scan freq %d, scan_state:%d", freq, s_scan_state);

	/* selfdata normalization */
	MEMSET(self_col, 0, ncols*2);
	MEMCPY(self_col + ncols, thp_data + nrows*ncols, ncols*2);
	MEMSET(self_row, 0, nrows/2*2);
	MEMCPY(self_row + nrows/2, thp_data + nrows*ncols + ncols, nrows*2);
	MEMCPY(thp_data + mutual_nodes, self_col, ncols*2);
	MEMCPY(thp_data + mutual_nodes + ncols, self_row, ROWS*2);
	MEMCPY(thp_data + mutual_nodes + ncols + ROWS, self_col + ncols, ncols*2);
	
	/* mutualdata normalization */
	MEMCPY(thp_data+nrows/2*ncols, thp_data, nrows*ncols*2);
	for (i = 0; i < ROWS_PATTERN * COLS_PATTERN; i++) {
		if (0 == pro_id) {
#ifdef SINGLE_COFF_PID02
			thp_data[i] = thp_data[i] * SINGLE_COFF_PID02/100 + RAW_DEST_DATA_PID02;
#else
			thp_data[i] = thp_data[i] * coff[i]/1024 - RAW_DEST_DATA_PID02;
#endif
		} else if (1 == pro_id) {
#ifdef SINGLE_COFF_PID03
			if(i >= nrows/2 * ncols)
				thp_data[i] = thp_data[i] * SINGLE_COFF_PID03/100 - RAW_DEST_DATA_PID03;
#else
			if(i >= nrows/2 * ncols)
				thp_data[i] = thp_data[i] * coff[i]/1024 - RAW_DEST_DATA_PID03;
#endif
		}
	}
	MEMSET(thp_data, 0, nrows/2*ncols*2);

	return 0;
}
#endif

#if PATTERN_TYPE_1
uint16_t g_whole_frame[ROWS_PATTERN * COLS_PATTERN];
static void cts_convert_pattern_1_grid_data(uint16_t *thp_data, uint16_t freq, uint16_t *coff, bool flag)
{
	int i, j, k;
	int pro_id = 1;
	int16_t *raw_comp;

	uint16_t mutual_nodes = ROWS_PATTERN * COLS_PATTERN;

	if (freq == s_scan_freq[0]) {
		raw_comp = rawdata_table;
	} else if (freq == s_scan_freq[1]) {
		raw_comp = rawdata_table + FRAME_GRID_DATA_NODES * 1;
	} else if (freq == s_scan_freq[2]) {
		raw_comp = rawdata_table + FRAME_GRID_DATA_NODES * 2;
	}else if (freq == s_scan_freq[3]) {
		raw_comp = rawdata_table + FRAME_GRID_DATA_NODES * 3;
	} else {
		raw_comp = rawdata_table;
		APP_LOGE("BUG!! Unexpected scan freq %d", freq);
		//return;
	}

	if (flag) {
		for (i = 0; i < ROWS_PATTERN * COLS_PATTERN; i++) {
			if (0 == pro_id) {
#ifdef SINGLE_COFF_PID02
				thp_data[i] = thp_data[i] * SINGLE_COFF_PID02/100 + RAW_DEST_DATA_PID02;
#else
				thp_data[i] = thp_data[i] * coff[i]/1024 - RAW_DEST_DATA_PID02;
#endif
			} else if (1 == pro_id) {
#ifdef SINGLE_COFF_PID03
#ifdef RAW_COMP
				if (s_scan_state == 3)
					thp_data[i] = (uint16_t)((int16_t)thp_data[i] + raw_comp[i]);
#endif
				if ((s_scan_state == 1) && (i < ROWS_PATTERN / 3 * COLS_PATTERN)) {
					thp_data[i] = thp_data[i] * (SINGLE_COFF_PID03 + SINGLE_COFF_PID03_DIFF)/100 - RAW_DEST_DATA_PID03;
				} else if ((s_scan_state == 2) && (i >= ROWS_PATTERN / 3 * COLS_PATTERN)) {
					thp_data[i] = thp_data[i] * SINGLE_COFF_PID03/100 - RAW_DEST_DATA_PID03;
				} else if ((s_scan_state != 1) && (s_scan_state != 2)) {
					if(i < ROWS_PATTERN / 3 * COLS_PATTERN)
						thp_data[i] = thp_data[i] * (SINGLE_COFF_PID03 + SINGLE_COFF_PID03_DIFF)/100 - RAW_DEST_DATA_PID03;
					else
						thp_data[i] = thp_data[i] * SINGLE_COFF_PID03/100 - RAW_DEST_DATA_PID03;
				}
#else
				thp_data[i] = thp_data[i] * coff[i]/1024 - RAW_DEST_DATA_PID03;
#endif
			}
		}
	
		/**** save whole screen griddata to g_whole_frame buff ****/
		if ((s_scan_state == 0) || (s_scan_state == 3))
			MEMCPY(g_whole_frame, thp_data, sizeof(g_whole_frame));
		else if (s_scan_state == 1)
			MEMCPY(g_whole_frame, thp_data, mutual_nodes/3*2);
		else if (s_scan_state == 2)
			MEMCPY(g_whole_frame + mutual_nodes/3, thp_data + mutual_nodes/3, mutual_nodes/3*2*2);
		else
			THP_LOGE("Invalid s_scan_state: %d", s_scan_state);
		/***********************************************************/
	
		/**** fill 2/3 screen griddata under 1/3 screen state ****/
		if (s_scan_state == 1)
			MEMCPY(thp_data + mutual_nodes/3, g_whole_frame + mutual_nodes/3, mutual_nodes/3*2*2);
		/**** fill 1/3 screen griddata under 2/3 screen state ****/
		if (s_scan_state == 2)
			MEMCPY(thp_data, g_whole_frame, mutual_nodes/3*2);
		/*********************************************************/
	} else if (!flag) {
		for (i = 0; i < COLS_PATTERN * ROWS_PATTERN; i++) {
			if (0 == pro_id) {
#ifdef SINGLE_COFF_PID02
				thp_data[i] = thp_data[i] * SINGLE_COFF_PID02/100 + RAW_DEST_DATA_PID02;
#else
				thp_data[i] = thp_data[i] * coff[i]/1024 - RAW_DEST_DATA_PID02;
#endif
			}  else if (1 == pro_id) {
#ifdef SINGLE_COFF_PID03
				if(i < ROWS_PATTERN / 3 * COLS_PATTERN)
					thp_data[i] = thp_data[i] * (SINGLE_COFF_PID03 + 100)/100 - RAW_DEST_DATA_PID03;
				else
					thp_data[i] = thp_data[i] * SINGLE_COFF_PID03/100 - RAW_DEST_DATA_PID03;
#else
				thp_data[i] = thp_data[i] * coff[i]/1024 - RAW_DEST_DATA_PID03;
#endif
			}
		}
		cts_exchange_xy1(thp_data, ROWS_PATTERN, COLS_PATTERN);
	}


	return;
}
#endif

static int cts_convert_grid_data(uint16_t freq, uint16_t *thp_data, int nrows, int ncols)
{
	int i, j, k;
	bool flag = false;
	
	int16_t self_col[ncols];
	int16_t self_row[nrows];
#if PATTERN_TYPE_1
	uint16_t tmp[ROWS_PATTERN * COLS_PATTERN];
	uint16_t mutual_nodes = ROWS_PATTERN * COLS_PATTERN;
#else
	uint16_t tmp[ncols * nrows];
	uint16_t mutual_nodes = ncols * nrows;
#endif
	uint16_t *coff;
	uint16_t *coff_temp;
    if (!strcmp(project_id, PROJECT_ID_1)) {
         coff_temp = coff_freq;
	} else if (!strcmp(project_id, PROJECT_ID_2)) {
       coff_temp = coff_freq;  
	} else {
        All_LOG("coff_freq_vxn ———— unknow proj_id ----"); //
        coff_temp = coff_freq;
	}

	if (freq == s_scan_freq[0]) {
		coff = coff_temp;
	} else if (freq == s_scan_freq[1]) {
		coff = coff_temp;
	} else if (freq == s_scan_freq[2]) {
		coff = coff_temp;
	}else if (freq == s_scan_freq[3]) {
		coff = coff_temp;
	} else {
		coff = coff_temp;
		All_LOG("BUG!! Unexpected scan freq %d", freq);
		//return -1;
	}

	THP_LOGI("scan freq %d,scan_state:%d", freq, s_scan_state);

	if (RX_NUM == COLS) {
		flag = true;
		
		/* selfdata normalization */
	/*
		for (i = 0; i < ncols; i++) {
			self_tmp[i] = (int16_t)thp_data[mutual_nodes + i];
			thp_data[mutual_nodes + i] = (uint16_t)self_tmp[i];
		}

		for (i = 0; i < nrows; i++) {
			self_tmp[i] = (int16_t)thp_data[mutual_nodes + ncols + i];
			thp_data[mutual_nodes + ncols + i] = (uint16_t)self_tmp[i];
		}*/
			
		/* mutualdata normalization */
#if PATTERN_TYPE_1
		cts_convert_pattern_1_grid_data(thp_data, freq, coff, flag);
#else
		for (i = 0; i < nrows * ncols; i++) {
			thp_data[i] = thp_data[i] * coff[i]/1024 + RAW_DEST_DATA;
			//thp_data[i] = thp_data[i] ;
		}
#endif
	} else {
		flag = false;
		
		/* selfdata normalization */
		for (i = 0; i < ncols; i++)
			self_col[i] = (int16_t)thp_data[mutual_nodes + nrows + i];
		for (i = 0; i < nrows; i++)
			self_row[i] = (int16_t)thp_data[mutual_nodes + i] * 1;
		for (i = 0; i < ncols; i++)
			thp_data[mutual_nodes + i] = (uint16_t)self_col[i];
		for (i = 0; i < nrows; i++)
			thp_data[mutual_nodes + ncols + i] = (uint16_t)self_row[i];

		/* mutualdata normalization */
		cts_exchange_xy1(thp_data, nrows, ncols);
#if PATTERN_TYPE_1
		cts_convert_pattern_1_grid_data(thp_data, freq, coff, flag);
#else
		for (i = 0; i < nrows * ncols; i++) {
			thp_data[i] = thp_data[i] * coff[i]/1024 + RAW_DEST_DATA;
			tmp[i] = thp_data[i];
		}
		//cts_exchange_xy1(thp_data, nrows, ncols);
#endif
	}

	return 0;
}

static void cts_convert_pattern_1_pen_data(int16_t *thp_data1,
	int16_t *thp_data2, int nrows, int ncols)
{
	int i;

	for (i = 0; i < ncols; i++) {
		if (s_scan_state == 1) {		// 只扫1/3屏
			if (i >= COLS_PATTERN) {
			  thp_data1[i] = 0;
			  thp_data1[nrows + ncols + i] = 0;
			  thp_data2[i] = 0;
			  thp_data2[nrows + ncols + i] = 0;
			} else {
				if (i == 0) {
					thp_data1[i] = thp_data1[i] * 200 / 200;	
					thp_data1[nrows + ncols + i] = thp_data1[nrows + ncols + i] * 200 / 200;
					thp_data2[i] = thp_data2[i] * 200 / 200;
					thp_data2[nrows + ncols + i] = thp_data2[nrows + ncols + i] * 200 /200;
				} else {
					thp_data1[i] = thp_data1[i] * 150 / 200;	
					thp_data1[nrows + ncols + i] = thp_data1[nrows + ncols + i] * 150 / 200;
					thp_data2[i] = thp_data2[i] * 150 / 200;
					thp_data2[nrows + ncols + i] = thp_data2[nrows + ncols + i] * 150 /200;
				}
			}
		} else if(s_scan_state == 2) {		// 只扫2/3屏
			if (i < COLS_PATTERN) {
			  thp_data1[i] = 0;
			  thp_data1[nrows + ncols + i] = 0;
			  thp_data2[i] = 0;
			  thp_data2[nrows + ncols + i] = 0;
			} else {
				if (i == (ncols - 1)) {
					thp_data1[i] = thp_data1[i] * 200 / 200;	
					thp_data1[nrows + ncols + i] = thp_data1[nrows + ncols + i] * 200 / 200;
					thp_data2[i] = thp_data2[i] * 200 / 200;
					thp_data2[nrows + ncols + i] = thp_data2[nrows + ncols + i] * 200 /200;
				} else {
					thp_data1[i] = thp_data1[i] * 150 / 200;	
					thp_data1[nrows + ncols + i] = thp_data1[nrows + ncols + i] * 150 / 200;
					thp_data2[i] = thp_data2[i] * 150 / 200;
					thp_data2[nrows + ncols + i] = thp_data2[nrows + ncols + i] * 150 / 200;
				}
			}
		} else {		//全屏
			if ((i == 0) || (i == (ncols - 1))) {
				thp_data1[i] = thp_data1[i] * 150 / 200;
				thp_data1[nrows + ncols + i] = thp_data1[nrows + ncols + i] * 150 / 200;
				thp_data2[i] = thp_data2[i] * 150 / 200;
				thp_data2[nrows + ncols + i] = thp_data2[nrows + ncols + i] * 150 /200;
			} else {
				thp_data1[i] = thp_data1[i] * 150 / 200;
				thp_data1[nrows + ncols + i] = thp_data1[nrows + ncols + i] * 150 / 200;
				thp_data2[i] = thp_data2[i] * 150 / 200;
				thp_data2[nrows + ncols + i] = thp_data2[nrows + ncols + i] * 150 /200;
			}
		}
	}
	for (i = 0; i < nrows; i++) {
		if(s_scan_state == 1) { 	// 只扫1/3屏
			if (i >= (ROWS_PATTERN / 3))  {
			  thp_data1[ncols + i] = 0;
			  thp_data1[ncols * 2 + nrows + i] = 0;
			  thp_data2[ncols + i] = 0;
			  thp_data2[ncols * 2 + nrows + i] = 0;
			} else {
				if (i == 0) {
					thp_data1[ncols + i] = thp_data1[ncols + i] * 200/ 200;
					thp_data1[ncols * 2 + nrows + i] = thp_data1[ncols * 2 + nrows + i] * 200 / 200;
					thp_data2[ncols + i] = thp_data2[ncols + i] * 200 / 200;
					thp_data2[ncols * 2 + nrows + i] = thp_data2[ncols * 2 + nrows + i] * 200 / 200;
				} else {
					thp_data1[ncols + i] = thp_data1[ncols + i] * 150 / 200;
					thp_data1[ncols * 2 + nrows + i] = thp_data1[ncols * 2 + nrows + i] * 150 / 200;
					thp_data2[ncols + i] = thp_data2[ncols + i] * 150 / 200;
					thp_data2[ncols * 2 + nrows + i] = thp_data2[ncols * 2 + nrows + i] * 150 / 200;
				}
			}
		} else if (s_scan_state == 2) { 	// 只扫2/3屏
			if (i < (ROWS_PATTERN / 3)) {
			  thp_data1[ncols + i] = 0;
			  thp_data1[ncols * 2 + nrows + i] = 0;
			  thp_data2[ncols + i] = 0;
			  thp_data2[ncols * 2 + nrows + i] = 0;
			} else {
				if (i == (nrows - 1)) {
					thp_data1[ncols + i] = thp_data1[ncols + i] * 200 / 200;
					thp_data1[ncols * 2 + nrows + i] = thp_data1[ncols * 2 + nrows + i] * 200 / 200;
					thp_data2[ncols + i] = thp_data2[ncols + i] * 200 / 200;
					thp_data2[ncols * 2 + nrows + i] = thp_data2[ncols * 2 + nrows + i] * 200 / 200;
				} else {
					thp_data1[ncols + i] = thp_data1[ncols + i] * 150 / 200;
					thp_data1[ncols * 2 + nrows + i] = thp_data1[ncols * 2 + nrows + i] * 150 / 200;
					thp_data2[ncols + i] = thp_data2[ncols + i] * 150 / 200;
					thp_data2[ncols * 2 + nrows + i] = thp_data2[ncols * 2 + nrows + i] * 150 / 200;
				}
			}
		}else { //全屏
			if ((i == 0) || (i == (nrows - 1))) {
				thp_data1[ncols + i] = thp_data1[ncols + i] * 200 / 200;
				thp_data1[ncols * 2 + nrows + i] = thp_data1[ncols * 2 + nrows + i] * 200 / 200;
				thp_data2[ncols + i] = thp_data2[ncols + i] * 200 / 200;
				thp_data2[ncols * 2 + nrows + i] = thp_data2[ncols * 2 + nrows + i] * 200 / 200;
			} else {
				thp_data1[ncols + i] = thp_data1[ncols + i] * 150 / 200;
				thp_data1[ncols * 2 + nrows + i] = thp_data1[ncols * 2 + nrows + i] * 150 / 200;
				thp_data2[ncols + i] = thp_data2[ncols + i] * 150 / 200;
				thp_data2[ncols * 2 + nrows + i] = thp_data2[ncols * 2 + nrows + i] * 150 / 200;
			}
		}
	}
	
	return;
}

static int cts_convert_pen_data(uint16_t freq, int16_t *thp_data1,
	int16_t *thp_data2, int nrows, int ncols)
{
	int i;
	/* tmp1: tx, tmp2: rx */
	int16_t tmp1_f1_I[ncols];
	int16_t tmp2_f1_I[nrows];
	int16_t tmp1_f1_Q[ncols];
	int16_t tmp2_f1_Q[nrows];
	int16_t tmp1_f2_I[ncols];
	int16_t tmp2_f2_I[nrows];
	int16_t tmp1_f2_Q[ncols];
	int16_t tmp2_f2_Q[nrows];
	
	if (RX_NUM == COLS) {
#if PATTERN_TYPE_1
	cts_convert_pattern_1_pen_data(thp_data1, thp_data2, nrows, ncols);
#else
	for (i = 0; i < ncols; i++) {
		thp_data1[i] = thp_data1[i] * 90 / 200;
		thp_data1[nrows + ncols + i] = thp_data1[nrows + ncols + i] * 90 / 200;
		thp_data2[i] = thp_data2[i] * 90 / 200;
		thp_data2[nrows + ncols + i] = thp_data2[nrows + ncols + i] * 90 /200;
	}

	for (i = 0; i < nrows; i++) {
		thp_data1[ncols + i] = thp_data1[ncols + i] * 90 / 200;
		thp_data1[ncols * 2 + nrows + i] = thp_data1[ncols * 2 + nrows + i] * 90 / 200;
		thp_data2[ncols + i] = thp_data2[ncols + i] * 90 / 200;
		thp_data2[ncols * 2 + nrows + i] = thp_data2[ncols * 2 + nrows + i] * 90 / 200;
	}
#endif
	} else {
		for (i = 0; i < ncols; i++) {
			tmp1_f1_I[i] = thp_data1[nrows + i] * 100 / 200;				//tx_f1_I
			tmp1_f1_Q[i] = thp_data1[nrows * 2 + ncols + i] * 100 / 200;	//tx_f1_Q
		
			tmp1_f2_I[i] = thp_data2[nrows + i] * 100 / 200;				//tx_f2_I
			tmp1_f2_Q[i] = thp_data2[nrows * 2 + ncols + i] * 100 / 200;	//tx_f2_Q
		}
		for (i = 0; i < nrows; i++) {
			tmp2_f1_I[i] = thp_data1[i] * 100 / 200;						//rx_f1_I
			tmp2_f1_Q[i] = thp_data1[nrows + ncols + i] * 100 / 200;		//rx_f1_Q

			tmp2_f2_I[i] = thp_data2[i] * 100 / 200;						//rx_f2_I
			tmp2_f2_Q[i] = thp_data2[nrows + ncols + i] * 100 / 200;		//rx_f2_Q
		}
		
		for (i = 0; i < ncols; i++) {
			thp_data1[i] = tmp1_f1_I[i];
			thp_data2[i] = tmp1_f2_I[i];
		}
		for (i = 0; i < nrows; i++) {
			thp_data1[ncols + i] = tmp2_f1_I[i];
			thp_data2[ncols + i] = tmp2_f2_I[i];
		}
		for (i = 0; i < ncols; i++) {
			thp_data1[ncols + nrows + i] = tmp1_f1_Q[i];
			thp_data2[ncols + nrows + i] = tmp1_f2_Q[i];
		}
		for (i = 0; i < nrows; i++) {
			thp_data1[ncols * 2 + nrows + i] = tmp2_f1_Q[i];
			thp_data2[ncols * 2 + nrows + i] = tmp2_f2_Q[i];
		}
	}
	//cts_dump_pendata(thp_data1, thp_data2, ROWS, COLS);

	return 0;
}

static void cts_osc_trim_trigger(void)
{
	cts_get_hsyncdata();
	cts_tcs_set_cur_clock(OSC_TRIM_INFO);
	dmd_trigger = true;
	All_LOG("cts_osc_trim_trigger, dmd_trigger: %d", dmd_trigger);

	return;
}

static void cts_dynamic_gain_trigger()
{
	dynamic_gain_triggered = true;
	dmd_trigger = true;
	All_LOG("cts_dynamic_gain_trigger, dmd_trigger: %d", dmd_trigger);
}

#ifdef CTS_SWITCH_SCAN_STATE_TRACK
static void cts_begin_track_scan_state(uint16_t required_scan_state)
{
	scan_state_tracker.begin_tracking = 1;
	scan_state_tracker.required_scan_state = required_scan_state;
	scan_state_tracker.checked_frames = 0;
	scan_state_tracker.another_retry = 0;
}

static void cts_end_track_scan_state(void)
{
	MEMSET(&scan_state_tracker, 0, sizeof(scan_state_tracker));
}

static void cts_track_scan_state(uint16_t current_scan_state)
{
	// NOT traking, skip
	if (0 == scan_state_tracker.begin_tracking) {
		return;
	}

	// set scan state ok
	if (scan_state_tracker.required_scan_state == current_scan_state) {
		All_LOG("Set scan state to %d OK.", current_scan_state);
		cts_end_track_scan_state();
		return;
	}

	All_LOG("Set scan state to %d, while %d received, retry %d, %d frame.",
			scan_state_tracker.required_scan_state, current_scan_state,
			scan_state_tracker.another_retry, scan_state_tracker.checked_frames);

	// has more frames to check
	if (scan_state_tracker.checked_frames < SCAN_STATE_TRACK_FRAME_MAX) {
		scan_state_tracker.checked_frames++;
		return;
	}

	// another retry already
	if (scan_state_tracker.another_retry > 0) {
		All_LOG("Set scan state to %d Failed again, let it go!!",
				scan_state_tracker.required_scan_state);
		cts_end_track_scan_state();
		return;
	}

	// first retry
	All_LOG("Set scan state to %d Failed after %d frames, try again.",
			scan_state_tracker.required_scan_state, scan_state_tracker.checked_frames);
	cts_set_scan_state(scan_state_tracker.required_scan_state);
	scan_state_tracker.another_retry = 1;
}
#endif

static int cts_convert_frame(TIMEVAL_STRUCT *tv, CTS_FRAME_STRUCT *cts_frame,
	THP_AFE_FRAME_DATA_STRUCT *thp_frame)
{
	CTS_FRAME_STRUCT0 *cts_frame0;
	CTS_FRAME_STRUCT1 *cts_frame1;

	MEMSET(thp_frame, 0, sizeof(*thp_frame));
	THP_LOGI("g_noise: %d, %d, %d, %d, %d", g_noise[0], g_noise[1], g_noise[2], g_noise[3], g_noise[4]);

#ifdef CTS_FOLD_COMPEN_ON
	if ((cts_frame->header.frame_type & FRAME_TYPE_MASK)  == FRAME_TYPE_6) {
		cts_frame0 = (CTS_FRAME_STRUCT0 *)cts_frame;
		rawdata_table_recive_cnt++;
		All_LOG("Frame type: %d,current cnt:%d", cts_frame->header.frame_type,rawdata_table_recive_cnt);
		if (cts_frame->header.frame_type == FRAME_TYPE_SUB_61) {
			MEMCPY(rawdata_table, cts_frame0->data.mutual, FRAME_GRID_DATA_SIZE);
			flag_0 = true;
			cts_disable_update_compen();
		} else if (cts_frame->header.frame_type == FRAME_TYPE_SUB_62) {
			MEMCPY(rawdata_table + FRAME_GRID_DATA_NODES * 1, cts_frame0->data.mutual, FRAME_GRID_DATA_SIZE);
			flag_1 = true;
		} else if (cts_frame->header.frame_type == FRAME_TYPE_SUB_63) {
			MEMCPY(rawdata_table + FRAME_GRID_DATA_NODES * 2, cts_frame0->data.mutual, FRAME_GRID_DATA_SIZE);
			flag_2 = true;
		} else if (cts_frame->header.frame_type == FRAME_TYPE_SUB_64) {
			MEMCPY(rawdata_table + FRAME_GRID_DATA_NODES * 3, cts_frame0->data.mutual, FRAME_GRID_DATA_SIZE);
			flag_3 = true;
		} else
			THP_LOGE("Invalid frame type: %d", cts_frame->header.frame_type);
		return 1;
	}
#endif

	// finger data
	if (cts_frame->header.frame_type == FRAME_TYPE_SUB_11) {
		cts_frame0 = (CTS_FRAME_STRUCT0 *)cts_frame;
		s_scan_state =  cts_frame0->header.scan_state;
		s_curr_scan_rate = cts_frame0->header.scan_rate;
		g_ext_clk_request = (cts_frame0->header.afe_status & THP_AFE_STATUS_XTAL_CLK);

#ifdef CTS_DIFF_SCREEN_STATE_DATA_SIZE
		if (s_scan_state == 0 || s_scan_state == 3)
			cts_convert_grid_data(cts_frame0->header.scan_freq, cts_frame0->data.mutual, ROWS, COLS);
		else if (s_scan_state == 1 || s_scan_state == 4)
			cts_convert_grid_data_major(cts_frame0->header.scan_freq, cts_frame0->data.mutual, ROWS/3, COLS/2);
		else if (s_scan_state == 2)
			cts_convert_grid_data_minor(cts_frame0->header.scan_freq, cts_frame0->data.mutual, ROWS*2/3, COLS/2);
		else
			THP_LOGE("Invalid s_scan_state: %d", s_scan_state);
#else
		cts_convert_grid_data(cts_frame0->header.scan_freq, cts_frame0->data.mutual, ROWS, COLS);
#endif

		MEMSET(&g_thp_stylus_frame, 0, sizeof(g_thp_stylus_frame));

		thp_frame->time_stamp.tv_sec = tv->tv_sec;
		thp_frame->time_stamp.tv_usec = tv->tv_usec;
		thp_frame->frame_index = cts_frame0->header.frame_index;
		thp_frame->grid_data = cts_frame0->data.mutual;
		thp_frame->line_data = cts_frame0->data.self_rx;//NULL;
		thp_frame->button_data = NULL;
		thp_frame->noise_data = g_noise;//cts_frame->noise;
		thp_frame->scan_freq = cts_frame0->header.scan_freq; //161;
		thp_frame->scan_rate = cts_frame0->header.scan_rate; //120;
		if (cts_frame0->header.afe_status & THP_AFE_STATUS_FOLD_COMPEN_DONE) {
			cts_frame0->header.afe_status |= THP_AFE_STATUS_CALIBRATION_DONE;
		}

		/* 20240311-判断固件是否触发动态gain调整 */
		if (dynamic_gain_flag && !dynamic_gain_triggered) {
			cts_dynamic_gain_trigger();
		}

		/* 20240228
		 * RESET后，固件需校准时钟时，将BIT(13)举起，HAL将校准后的hsync值回读
		 * 并下发OSC_TRIM_INFO告诉固件，已回读hsync，固件清BIT(13)
		 * HAL接收到后，清BIT(13)，上层不感知 */
		if (cts_frame0->header.afe_status & THP_AFE_STATUS_OSC_TRIM_TRIGGER) {
			//compen_done = false;
			cts_frame0->header.afe_status &= (~THP_AFE_STATUS_OSC_TRIM_TRIGGER);
			cts_osc_trim_trigger();
		}

		/* 固件复用BIT(4)&BIT(15)，用于开机4帧校准rawdata table标志位
		 * 固件复用BIT(14)，用于申请开关外部时钟
		 * HAL接收到后，清BIT(4)&BIT(14)&BIT(15)，上层不感知 */
		thp_frame->status = (cts_frame0->header.afe_status & (~THP_AFE_STATUS_FOLD_COMPEN_DONE)) &(~THP_AFE_STATUS_FOLD_COMPEN_REQUEST);
		thp_frame->status = (thp_frame->status & (~THP_AFE_STATUS_XTAL_CLK));
		if (g_ext_clk_request && (!stylus_enable))
			thp_frame->status |= THP_AFE_STATUS_XTAL_REQUEST;
		else
			thp_frame->status = (thp_frame->status & (~THP_AFE_STATUS_XTAL_REQUEST));
		
		if (cts_frame->header.afe_status & THP_AFE_STATUS_FOLD_COMPEN_REQUEST) {
			 thp_frame->status |= THP_AFE_STATUS_FOLD_COMPEN_REQUEST_FOR_HAL;
		}
		if (cts_frame->header.afe_status & THP_AFE_STATUS_FOLD_COMPEN_DONE) {
			 thp_frame->status = thp_frame->status | THP_AFE_STATUS_FOLD_COMPEN_DONE_FOR_HAL;
		}
		if (1 == debug_fw_fode_done) {
			thp_frame->status |= THP_AFE_STATUS_FOLD_COMPEN_DONE_FW_FLAG;
		}
		if (compen_done) {
			 thp_frame->status = thp_frame->status | THP_AFE_STATUS_FOLD_COMPEN_DONE_HAL_FLAG;
		}
		thp_frame->gesture = cts_frame0->header.gesture_status;
		thp_frame->stylus = &g_thp_stylus_frame;
		thp_frame->stylus->status = cts_frame0->header.stylus_status;
		thp_frame->stylus->tx1_scan_freq = cts_frame0->header.stylus_scan_freq_f1;
		thp_frame->stylus->tx2_scan_freq = cts_frame0->header.stylus_scan_freq_f2;
		thp_frame->stylus->tx1_new_scan_freq = cts_frame0->header.stylus_new_scan_freq_f1;
		thp_frame->stylus->tx2_new_scan_freq = cts_frame0->header.stylus_new_scan_freq_f2;
		thp_frame->side_data = NULL;
		thp_frame->force_data = NULL;
		
#ifdef CTS_SWITCH_SCAN_STATE_TRACK
		cts_track_scan_state(cts_frame0->header.scan_state);
#endif

		if (cts_frame0->header.scan_state == 3)
			thp_frame->scan_state = THP_AFE_SCAN_STATE_WHOLE;
		else
			thp_frame->scan_state = cts_frame0->header.scan_state;//THP_AFE_SCAN_STATE_WHOLE;
		
		if ((compen_done == false) && (last_scan_state != cts_frame0->header.scan_state)) {
			All_LOG("detect scan state change,forece calibration");
			thp_frame->status = thp_frame->status | THP_AFE_STATUS_RECAL_REQUEST;
		}
		last_scan_state = cts_frame0->header.scan_state;
		
		if (!last_frame_in_idle && ((thp_frame->status & (1 << 0)) == THP_AFE_STATUS_IDLE_MODE)) {
			last_frame_in_idle = 1;
			g_get_frame_timeout = IDLE_GET_FRAME_TIMEOUT_MS; 
			thp_dev_set_timeout(1500);
		} else if (last_frame_in_idle && ((thp_frame->status & (1 << 1)) == THP_AFE_STATUS_ACTIVE_MODE)) {
			last_frame_in_idle = 0;
			g_get_frame_timeout = ACTIVE_GET_FRAME_TIMEOUT_MS; 
			thp_dev_set_timeout(250);
		}

		return 0;
	}

	if ((cts_frame->header.frame_type == FRAME_TYPE_SUB_21) ||
		(cts_frame->header.frame_type == FRAME_TYPE_SUB_22) ||
		(cts_frame->header.frame_type == FRAME_TYPE_SUB_23) ||
		(cts_frame->header.frame_type == FRAME_TYPE_SUB_24) ||
		(cts_frame->header.frame_type == FRAME_TYPE_SUB_25) ||
		(cts_frame->header.frame_type == FRAME_TYPE_SUB_31)) {
			cts_frame1 = (CTS_FRAME_STRUCT1 *)cts_frame;
			s_scan_state =  cts_frame1->header.scan_state;
			s_curr_scan_rate = cts_frame1->header.scan_rate;
			cts_convert_pen_data(cts_frame1->header.scan_freq,
				(int16_t *)cts_frame1->data.stylus_rx_f1_I,
				(int16_t *)cts_frame1->data.stylus_rx_f2_I, ROWS, COLS);

			thp_frame->time_stamp.tv_sec = tv->tv_sec;
			thp_frame->time_stamp.tv_usec = tv->tv_usec;

			MEMSET(&g_thp_stylus_frame, 0, sizeof(g_thp_stylus_frame));
			thp_frame->frame_index = cts_frame1->header.frame_index;
			thp_frame->scan_freq = cts_frame1->header.scan_freq;
			thp_frame->scan_rate = cts_frame1->header.scan_rate;
			//thp_frame->status = cts_frame1->header.afe_status;
			thp_frame->status = (cts_frame1->header.afe_status & (~THP_AFE_STATUS_FOLD_COMPEN_DONE)) &(~THP_AFE_STATUS_FOLD_COMPEN_REQUEST);
			thp_frame->gesture = cts_frame1->header.gesture_status;
			thp_frame->stylus = &g_thp_stylus_frame;
			thp_frame->stylus->tx1_line_data = cts_frame1->data.stylus_rx_f1_I;
			thp_frame->stylus->tx2_line_data = cts_frame1->data.stylus_rx_f2_I;
			thp_frame->stylus->tx1_scan_freq = cts_frame1->header.stylus_scan_freq_f1; //139;		
			thp_frame->stylus->tx2_scan_freq = cts_frame1->header.stylus_scan_freq_f2; //194;		
			thp_frame->stylus->pressure = 0;
			thp_frame->stylus->button = 0;
			thp_frame->stylus->status = cts_frame1->header.stylus_status;
			thp_frame->stylus->stylus_noise = g_noise;	
			thp_frame->stylus->tx1_new_scan_freq = cts_frame1->header.stylus_new_scan_freq_f1; //139;	
			thp_frame->stylus->tx2_new_scan_freq = cts_frame1->header.stylus_new_scan_freq_f2; //194;
			
#ifdef CTS_SWITCH_SCAN_STATE_TRACK
			cts_track_scan_state(cts_frame1->header.scan_state);
#endif
	
			if (cts_frame1->header.scan_state == 3)
				thp_frame->scan_state = THP_AFE_SCAN_STATE_WHOLE;
			else
				thp_frame->scan_state = cts_frame1->header.scan_state;
			
			if((compen_done == false)&&(last_scan_state != cts_frame1->header.scan_state)){
				All_LOG("detect scan state change,forece calibration");
				thp_frame->status = thp_frame->status | THP_AFE_STATUS_RECAL_REQUEST;
			}		
			last_scan_state = cts_frame1->header.scan_state;

			return 0;
	}
	THP_LOGE("ERROR!! Should not be here!!");
	return 0;
}

/* thp */
int cts_open(void)
{
	return cts_open_project(PROJECT_ID_1);
}

int cts_open_project(const char *proj_id)
{
	int ret = -1;
	All_LOG("cts_open +");
	All_LOG("proj_id: %s, PID_1: %s, PID_2: %s", proj_id, PROJECT_ID_2);
	ret = thp_dev_open();
	if (ret < 0) {
		All_LOG("Open thp device failed");
		return -1;
	}
	if (!strcmp(PROJECT_ID_1, proj_id)) {
		project_id = PROJECT_ID_1;		
	} else if (!strcmp(PROJECT_ID_2, proj_id)) {
		project_id = PROJECT_ID_2;
	}
	All_LOG("cts_open -");
	return strcmp(project_id, proj_id);
}
int cts_close(void)
{
	int ret = -1;

	All_LOG("cts_close +");
	ret = thp_dev_close();
	if (ret < 0) {
		THP_LOGE("Close thp device failed");
	}
	All_LOG("cts_close -");
	return 0;
}

int cts_set_calib_data_callback_func(
    THP_AFE_ERR_ENUM(*calibDataWriteCallback)(void* dataPtr, uint32_t dataLen),
    THP_AFE_ERR_ENUM(*calibDataReadCallback)(void* dataPtr, uint32_t dataLen))
{
	THP_LOGI("cts_set_calib_data_callback_func +");
	return 0;
}

int cts_start(void)
{
	int ret = -1;
	All_LOG("cts_start +");

	ret = cts_prework();
	if (ret < 0) {
		THP_LOGE("Do prework failed");
		LFREE(rawdata_table);
		return -1;
	}

#ifdef DEBUG_SOCKET_TOOL
	cts_tool_start_thread();
#endif
	

	All_LOG("cts_start -");
	return 0;
}

int cts_stop(void)
{
	int ret = -1;
	All_LOG("cts_stop +");

	ret = cts_postwork();
	if (ret < 0) {
		THP_LOGE("Do postwork failed");
	}

	All_LOG("cts_stop -");
	return 0;
}

static void cts_index_is_line(CTS_FRAME_STRUCT *cts_frame)
{
	static int index = 0;
	static bool first = true;
	// All_LOG("frame body checksum error: last frame_index: %d & current frame_index: %d is not line",
	// 		index, cts_frame->header.frame_index);

	if (first) {
		THP_LOGI("first time to set frame_index");
		index = cts_frame->header.frame_index;
		first = false;
		return;
	}
	if (cts_frame->header.frame_index == 0xFFFF) {
		THP_LOGI("frame_index to max: 0xFFFF");
		index = -1;
		return;
	} else if (cts_frame->header.frame_index - index == 1) {
		index = cts_frame->header.frame_index;
		return;
	} else {
		All_LOG("frame body checksum error: last frame_index: %d & current frame_index: %d is not line",
			index, cts_frame->header.frame_index);
		index = cts_frame->header.frame_index;
	}
}

static bool cts_enter_spec_mode(void)
{
	static bool first_time = true;
	if (nonblock || tui_flag) {
		THP_LOGI("Get frame with nonblock or tui mode");
		return true;
	} else if (vendor_call_flag) {
		THP_LOGI("Get frame with vendor_call");
		first_time = true;
		thp_dev_set_timeout(0);
		return true;
	} else if (inspect_flag) {
		THP_LOGI("Get frame with inspect mode");
		return true;
	}

	if (suspend_flag) {
		THP_LOGI("Get frame suspend flag set");
		first_time = true;
		thp_dev_set_timeout(0);
		return true;
	}

	if (first_time) {
		if (last_frame_in_idle)
			thp_dev_set_timeout(1500);
		else
			thp_dev_set_timeout(250);
		first_time = false;
	}
	return false;
}

extern uint8_t txbuf[SPI_MAX_SIZ];
extern uint8_t rxbuf[SPI_MAX_SIZ];
static void cts_recovery_sos(CTS_FRAME_STRUCT *cts_frame)
{
	int ret;
	size_t txlen;
	size_t rxlen = TOUCH_INFO_SIZ;
	int i = 0;
	CTS_FRAME_STRUCT cts_frame_dump;
	size_t size;

	size = sizeof(CTS_FRAME_STRUCT);
	THP_LOGE("Dump frame size: %d", size);

	cts_exit_idle();

	cts_sos_dump_frame(cts_frame);
	cts_force_get_hw_cap();
	for (i = 0; i < 6; i++) {
		txlen = cts_tcs_read_pack(txbuf, TP_STD_CMD_SYS_STS_GET_COORDINATES_RO,
			rxlen - TCS_REPLY_TAIL_SIZ);
		ret = cts_tcs_spi_xing(txbuf, rxbuf, txlen > rxlen ? txlen : rxlen);
		if (ret)
			All_LOG("Get frame failed");

		MEMCPY(&cts_frame_dump, rxbuf, size);

		All_LOG("cts_sos, get fw info %d times\n", i);
		All_LOG("dummy: 0x%x/0x%x", cts_frame_dump.header.dummy, (cts_frame_dump.header.dummy >> 32));
		All_LOG("index:   %d, type:	%d", cts_frame_dump.header.frame_index, cts_frame_dump.header.frame_type);
		All_LOG("cur:	   %04d, next:	  %04d, freq:	 %04d, rate:	%04d, state:   %04d, afe:	  0x%x",
			cts_frame_dump.header.curr_size, cts_frame_dump.header.next_size, cts_frame_dump.header.scan_freq,
			cts_frame_dump.header.scan_rate, cts_frame_dump.header.scan_state, cts_frame_dump.header.afe_status);
		All_LOG("gesture: %04d, stylus:  %04d, f1: 	 %04d, f2:		%04d, n_f1:    %04d, n_f2:	  %04d",
			cts_frame_dump.header.gesture_status, cts_frame_dump.header.stylus_status, cts_frame_dump.header.stylus_scan_freq_f1,
			cts_frame_dump.header.stylus_scan_freq_f2, cts_frame_dump.header.stylus_new_scan_freq_f1,
			cts_frame_dump.header.stylus_new_scan_freq_f2);
		cts_mdelay(20);
	}

	cts_reset_device();
	cts_force_get_hw_cap();
	sos_flag = 1;
}

static bool cts_last_frame_sos()
{
	if (sos_flag) {
		MEMSET(&g_thp_frame, 0, sizeof(g_thp_frame));
		g_thp_frame.status |= THP_AFE_STATUS_SOS;
		sos_flag = 0;
		return true;
	}
	return false;
}

void cts_switch_mode(uint8_t i)
{
    switch (i) {
    case 0:
		cts_enter_idle();
		g_tcs_cmd[i] = 0;
		break;
    case 1:
		cts_reset_idle_baseline();
		g_tcs_cmd[i] = 0;
		break;
	case 2:
		cts_set_scan_state(afe_state_store);
		g_tcs_cmd[i] = 0;
		break;
		
    default:
        THP_LOGI("INVALID VALUE: %d", i);
		return;
    }
}

#if CTS_FOLD_COMPEN_ON
static void cts_set_fold_compen(CTS_FRAME_STRUCT *cts_frame)
{
	if (cts_frame->header.afe_status & THP_AFE_STATUS_FOLD_COMPEN_REQUEST) {
		THP_LOGI("request fold compensation");

		if (!compen_done) {
			cts_enable_update_compen();
			if (!flag_0 && !flag_1 && !flag_2 && !flag_3 && (fold_cnt < CTS_FOLD_COMPEN_CNT_MAX)) {
				fold_cnt++;
			    cts_start_fold_calibration();
			} else {
				cts_frame->header.afe_status = cts_frame->header.afe_status | THP_AFE_STATUS_RECAL_REQUEST;
			}
		} else {
			if (cts_frame->header.afe_status & THP_AFE_STATUS_OSC_TRIM_TRIGGER) {
				All_LOG("Osc trim triggered");
			} else {
				All_LOG("Rawdata compesation already done, ignore!");
				cts_tcs_set_compen_enable(2);
			}
		}
		if (flag_0 || flag_1 || flag_2 || flag_3) {
			if (rawdata_table_recive_cnt >= 16) {
				THP_LOGI("fw request fold compensation times:%d, ingore current request", rawdata_table_recive_cnt);
				cts_frame->header.afe_status |= THP_AFE_STATUS_FOLD_COMPEN_NOT_DONE_FLAG;
				return;
			}
			THP_LOGI("get rawdata_table ok!, but wait fold done flag timeout, set afe request flag");
			cts_frame->header.afe_status = cts_frame->header.afe_status | THP_AFE_STATUS_RECAL_REQUEST;
		}
	} else {
		if ((cts_frame->header.afe_status & THP_AFE_STATUS_FOLD_COMPEN_DONE) &&
			!compen_done && (!flag_0 || !flag_1 || !flag_2 || !flag_3)) {
			THP_LOGE("get rawdata_table failed, need restart calibration");
			cts_start_fold_calibration();
			cts_mdelay(2);
			cts_tcs_set_compen_enable(4);
		}
	}
}

static void cts_clr_fold_compen(CTS_FRAME_STRUCT *cts_frame)
{
	if (flag_0 && flag_1 && flag_2 && flag_3) {
		if (cts_frame->header.afe_status & THP_AFE_STATUS_FOLD_COMPEN_DONE) {
			All_LOG("fold compensation done");
			compen_done = true;
			flag_0 = flag_1 = flag_2 = flag_3 = false;
			//cts_disable_update_compen();
			cts_tcs_set_compen_enable(2);
			//cts_dump_rawdata_table(rawdata_table);
		}
	}
}
#endif

THP_AFE_FRAME_DATA_STRUCT *cts_get_frame(void)
{
	int ret = -1;
	int i;
	uint32_t count = 0;
	TIME_T start_tv;
	long elapsed_ms;
	CTS_FRAME_STRUCT *cts_frame;

	start_tv = GET_CURR_TIME();

	if (cts_last_frame_sos())
		return &g_thp_frame;

	for (i = 0; i < MAX_TCS_CMD_NUM; i++) {
		if (g_tcs_cmd[i])
			cts_switch_mode(i);
	}

	THP_LOGI("Enter");

	thp_dev_get_frame_count(&count);

start_get_frame:
	ret = cts_ioctl_get_frame(&g_ioctl_frame);
	if (ret != 0) {
		//THP_LOGE("Get frame from kernel failed %d", ret);
		All_LOG("Get frame from kernel failed %d", ret);
		goto end_get_frame;
	}

	ret = cts_check_ioctl_frame(&g_ioctl_frame);
	if (ret < 0) {

			THP_LOGE("Invalid frame");

		goto end_get_frame;
	}

	cts_frame = (CTS_FRAME_STRUCT *)&g_ioctl_frame.frame;

#if CTS_FOLD_COMPEN_ON
	cts_set_fold_compen(cts_frame);
#endif

	ret = cts_convert_frame(&g_ioctl_frame.tv, (CTS_FRAME_STRUCT *)g_ioctl_frame.frame, &g_thp_frame);
	if (ret < 0) {
		THP_LOGE("Convert frames failed");
		goto end_get_frame;
	}

#if CTS_FOLD_COMPEN_ON
	if (ret == 1) {
		if (flag_0 && flag_1 && flag_2 && flag_3) {
			All_LOG("get rawdata_table ok!, wait fold compensation done flag ");
			//cts_dump_rawdata_table(rawdata_table);
		}
			return NULL;
	}
	cts_clr_fold_compen(cts_frame);
#endif
	
#ifdef DEBUG_SOCKET_TOOL
	cts_tool_send_to_client(cts_frame);
	cts_tool_save_frame_data(cts_frame);
#endif

	cts_index_is_line(cts_frame);

end_get_frame:
	THP_LOGI("Exit");

	elapsed_ms = ELAPSED_MS(start_tv);
	if (ret == 0) {
		THP_LOGI("Get frame cost %lldms", elapsed_ms);
		return &g_thp_frame;
	} else if (ret == ENODATA) {
			return NULL;
	} else {
		All_LOG("Get frame failed timeout");

		/* 1. firstly judge spec mode, if yes, ignore 3s timeout */
		if (cts_enter_spec_mode())
			return NULL;

		/* 2. Secondly, if not spec mode, reset device & set sos_flag */
		cts_recovery_sos(cts_frame);

		return NULL;
	}
}

int cts_set_idle_touch_threshold(uint16_t threshold)
{
	//THP_LOGD("cts_set_idle_touch_threshold +");
#if 0
	THP_LOGD("Set idle touch threshold: %d", threshold);
	rc = cts_tcs_set_mnt_touch_threshold(threshold);
	if (rc) {
		THP_LOGE("Set idle touch threshold failed: rc=%d", rc);
		return -1;
	}
#endif
	return 0;
}

int cts_set_baseline_update_interval(uint16_t interval)
{
	//int rc;

	//THP_LOGI("cts_set_baseline_update_interval +");

	//THP_LOGI("Set idle baseline update interval: %d", interval);
/*
	rc = cts_tcs_set_mnt_baseline_update_interval(interval);
	if (rc) {
		THP_LOGE("set monitor baseline update interval: rc=%d", rc);
		return -1;
	}
*/
	return 0;
}

int cts_reset_idle_baseline(void)
{
	int rc;

	All_LOG("cts_reset_idle_baseline +");
	
	rc = cts_tcs_reset_idle_baseline();
	if (rc) {
		THP_LOGE("set monitor baseline update interval: rc=%d", rc);
		return -1;
	}

	return 0;
}

int cts_enter_idle(void)
{
	int rc;

	All_LOG("cts_enter_idle +");

	rc = thp_dev_clear_frame_buffer(1);

	rc = cts_tcs_force_enter_mnt();
	if (rc) {
		THP_LOGE("Enter monitor mode failed: rc=%d", rc);
		return -1;
	}

	return 0;
}

int cts_exit_idle(void)
{
	int rc;

	All_LOG("cts_exit_idle +");

	rc = cts_tcs_force_exit_mnt();
	if (rc) {
		THP_LOGE("Exit monitor mode failed: rc=%d", rc);
		return -1;
	}

	return 0;
}

int cts_clear_status(THP_AFE_STATUS_ENUM status)
{
	int ret = -1;

	All_LOG("cts_clear_status +");

	THP_LOGI("Clear status: 0x%x", status);
		
	ret = cts_tcs_clr_afe_status(status);
	if (ret) {
		THP_LOGE("Clear afe status failed: %d", ret);
	}
		
	return ret;
}

int cts_set_scan_state(THP_AFE_SCAN_STATE_ENUM state)
{
	int ret;
	THP_AFE_SCAN_STATE_ENUM cur_state = state;

	ret = thp_dev_clear_frame_buffer(1);

#if CTS_FOLD_COMPEN_ON
	if (compen_done && (g_todo_scan_rate == 240) && (state == 0))
		cur_state = 3;
#endif

	All_LOG("cts_set_scan_state, state: %d", cur_state);
	
	ret = cts_tcs_set_scan_state(cur_state);
	if (ret) {
		All_LOG("Set scan state failed: %d", ret);
		return -1;
	}

#ifdef CTS_SWITCH_SCAN_STATE_TRACK
	cts_begin_track_scan_state((uint16_t)cur_state);
#endif
	return 0;
}


int cts_screen_off(void)
{
	int ret;
	All_LOG("cts_screen_off +");

	nonblock = 1;
	ret = thp_dev_set_block(0);
	if (ret < 0) {
		THP_LOGE("Set block 0 failed");
		return -1;
	}

	if (s_scan_state == 3) {
		All_LOG("Scan state: %d, set to %d", s_scan_state, 0);
		if (cts_tcs_set_scan_state(0)) {
			THP_LOGE("Set scan state to 0 failed");
			return -1;
		}
	}

	return 0;
}

int cts_screen_on(void)
{
	int ret;
	uint32_t cur_ver = 0;
	uint32_t hwid = 0;

	All_LOG("cts_screen_on +");

	/* Soft reset, double check */
	All_LOG("soft reset");
	if (cts_drw_write_u8(REGDEF_RSTCFG, 0xFE)) {
		THP_LOGE("Reset chip failed");
	}
	/****************************/
	cts_reset_device();
	//thp_dev_clear_frame_buffer(1);

	/**** clear fold_cnt for fold compen flow ****/
	fold_cnt = 0;
	/*********************************************/

	if (dynamic_gain_flag) {
		All_LOG("Firmware triggered dynamic gain");
	}

	cts_tcs_get_hw_id(&hwid);
	if ((hwid & CTS_IC_HWID_MASK) == CTS_IC_HWID) {
		goto get_fw_ver;
	} else {
		THP_LOGI("hwid = %#06x", hwid);
		goto update_firmware;
	}
	
get_fw_ver:
	ret = cts_tcs_get_fw_ver(&cur_ver);
	if (ret) {
		THP_LOGW("Get fw version failed");
		cur_ver = 0;
	}

update_firmware:
	THP_LOGI("Get firmware version: 0x%x", cur_ver);
	ret = cts_update_firmware(cur_ver);
	if (ret < 0) {
		THP_LOGE("Update firmware failed");
		ret = -1;
	} else if (ret == 1) {
		THP_LOGI("No need set spi speed & Re-write power on read flag");
		ret = 0;
		goto no_set_spi_speed;
	}

	if (cts_set_spi_speed(SPI_SPEED)) {
		THP_LOGW("Set spi speed failed");
		ret = -1;
	}

no_set_spi_speed:
	last_frame_in_idle = 0;
	nonblock = 0;

	if (cts_force_get_hw_cap()) {
		THP_LOGE("Get hw_cap failed");
		ret = -1;
	}
	
	if (thp_dev_set_block(1)) {
		THP_LOGE("Set block 1 failed");
		ret = -1;
	}
	if (thp_dev_set_timeout(1500)) {
		THP_LOGE("Set timeout failed failed: rc=%d", ret);
		ret = -1;
	}
	if (thp_dev_set_afe_status()) {
		THP_LOGE("Set afe status failed: rc=%d", ret);
		ret = -1;
	}

	return ret;
}

int cts_afe_enter_tui(void)
{
	All_LOG("cts_afe_enter_tui +");
	tui_flag = 1;
	return 0;
}

int cts_afe_exit_tui(void)
{
	All_LOG("cts_afe_exit_tui +");
	tui_flag = 0;
	return 0;
}

int cts_enable_freq_shift(void)
{
	All_LOG("cts_enable_freq_shift +");
	return cts_tcs_enable_freq();
}

int cts_disable_freq_shift(void)
{
	//All_LOG("cts_disable_freq_shift +");
	//return cts_tcs_disable_freq();
	return 0;
}

int cts_start_calibration(void)
{
	All_LOG("cts_start_calibration +");
	return cts_tcs_start_calibration();
}

int cts_start_fold_calibration(void)
{
	All_LOG("cts_start_fold_calibration +");
	return cts_tcs_start_calibration();
}

int cts_force_to_freq_point(uint8_t index)
{
	int ret = -1;

	All_LOG("cts_force_to_freq_point +,index = %d",index);

	if (cts_exit_idle())
		THP_LOGE("Exit idle failed");

	cts_mdelay(5);
	
	if (index < MAX_NUM_SCAN_FREQ) {
		ret = cts_tcs_set_freq_points(index);
	}
	return ret;
}

int cts_force_to_scan_rate(uint8_t index)
{
	int ret = -1;

	All_LOG("cts_force_to_scan_rate +");

	//if (cts_exit_idle())
		//THP_LOGE("Exit idle failed");

	//cts_mdelay(5);

	if (index < s_scan_rate_num) {	
		THP_LOGI("index: %d, s_scan_rate_num: %d, report_rate: %d, curr_rate: %d",
			index, s_scan_rate_num, s_scan_rate[index], s_curr_scan_rate);
		ret = cts_tcs_set_scan_rate(s_scan_rate[index]);
		if (ret) {
			THP_LOGE("Force to scan rate failed!");
			return ret;
		}
	} else {
		THP_LOGE("Invalid rate index: %d", index);
		return ret;
	}

	g_todo_scan_rate = s_scan_rate[index];

#if CTS_FOLD_COMPEN_ON
	if (compen_done && (s_scan_rate[index] == 240) && (s_scan_state == 0))
		cts_set_scan_state(3);
	else if ((s_scan_rate[index] != 240) && (s_scan_state == 3))
		cts_set_scan_state(0);
#endif
	
	return ret;
}

int cts_stylus_freq_immediately(void)
{
	All_LOG("cts_stylus_freq_immediately +");
	return cts_tcs_stylus_freq_immediately();
}

int cts_stylus_freq_next_uplink(void)
{
	All_LOG("cts_stylus_freq_next_uplink +");
	return cts_tcs_stylus_freq_next_uplink();
}

int cts_enable_stylus_hpp3_0(void)
{
	All_LOG("cts_enable_stylus_hpp3_0 +");
	stylus_enable = true;
	return cts_tcs_enable_stylus();
}

int cts_disable_stylus_hpp3_0(void)
{
	All_LOG("cts_disable_stylus_hpp3_0 +");
	stylus_enable = false;
	return cts_tcs_disable_stylus();
}

int cts_afe_suspend(void)
{
	SHB_LOG("cts_afe_suspend +");
	THP_LOGI("cts_afe_suspend +");
	suspend_flag = 1;
	return cts_tcs_set_afe_suspend();
}

int cts_afe_resume(void)
{
	SHB_LOG("cts_afe_resume +");
	THP_LOGI("cts_afe_resume +");
	suspend_flag = 0;
	return cts_tcs_set_afe_resume();
}

int cts_afe_enable_wakeup_gesture(THP_AFE_GESTURE_ENUM gesture)
{
	return cts_tcs_enable_wakeup_gesture(gesture);
}


int cts_afe_disable_wakeup_gesture(THP_AFE_GESTURE_ENUM gesture)
{
	return cts_tcs_disable_wakeup_gesture(gesture);
}

int cts_clear_gesture_status(THP_AFE_GESTURE_ENUM gesture)
{
	return cts_tcs_clear_gesture_status(gesture);
}


int cts_clear_stylus_status(THP_AFE_STYLUS_STATUS_ENUM status)
{
	int ret = -1;

	All_LOG("cts_clear_stylus_status +");
	
	THP_LOGI("Clear stylus status=%d(%s)", status, 
		(status & HPP2_X_PROTOCAL) ? "HPP2_X_PROTOCAL" :
		(status & HPP3_X_PROTOCAL) ? "HPP3_X_PROTOCAL" : 
		(status & STYLUS_DETECT_MODE) ? "STYLUS_DETECT_MODE" :
		(status & STYLUS_CONFIRM_MODE) ? "STYLUS_CONFIRM_MODE" :
		(status & STYLUS_ACTIVE_MODE) ? "STYLUS_ACTIVE_MODE" :
		(status & STYLUS_FREQ_SHIFT_DONE) ? "STYLUS_FREQ_SHIFT_DONE" :
		(status & STYLUS_FREQ_SHIFT_REQUEST) ? "STYLUS_FREQ_SHIFT_REQUEST" :
		(status & STYLUS_ALL_FREQ_NOISY) ? "STYLUS_ALL_FREQ_NOISY" : "ERR_CLEAR_STATUS");
		
	if (status & HPP2_X_PROTOCAL) {
		ret = cts_tcs_clr_stylus_status(HPP2_X_PROTOCAL);
		if (ret) {
			THP_LOGE("Clear HPP2_X_PROTOCAL failed: %d", ret);
			goto err_clr_status;
		}
	} else if (status & HPP3_X_PROTOCAL) {
		ret = cts_tcs_clr_stylus_status(HPP3_X_PROTOCAL);
		if (ret) {
			THP_LOGE("Clear HPP3_X_PROTOCAL failed: %d", ret);
			goto err_clr_status;
		}
	} else if (status & STYLUS_DETECT_MODE) {
		ret = cts_tcs_clr_stylus_status(STYLUS_DETECT_MODE);
		if (ret) {
			THP_LOGE("Clear STYLUS_DETECT_MODE failed: %d", ret);
			goto err_clr_status;
		}
	} else if (status & STYLUS_CONFIRM_MODE) {
		ret = cts_tcs_clr_stylus_status(STYLUS_CONFIRM_MODE);
		if (ret) {
			THP_LOGE("Clear STYLUS_CONFIRM_MODE failed: %d", ret);
			goto err_clr_status;
		}
	} else if (status & STYLUS_ACTIVE_MODE) {
		ret = cts_tcs_clr_stylus_status(STYLUS_ACTIVE_MODE);
		if (ret) {
			THP_LOGE("Clear STYLUS_ACTIVE_MODE failed: %d", ret);
			goto err_clr_status;
		}
	} else if (status & STYLUS_FREQ_SHIFT_DONE) {
		ret = cts_tcs_clr_stylus_status(STYLUS_FREQ_SHIFT_DONE);
		if (ret) {
			THP_LOGE("Clear STYLUS_FREQ_SHIFT_DONE failed: %d", ret);
			goto err_clr_status;
		}
	} else if (status & STYLUS_FREQ_SHIFT_REQUEST) {
		ret = cts_tcs_clr_stylus_status(STYLUS_FREQ_SHIFT_REQUEST);
		if (ret) {
			THP_LOGE("Clear STYLUS_FREQ_SHIFT_REQUEST failed: %d", ret);
			goto err_clr_status;
		}
	} else if (status & STYLUS_ALL_FREQ_NOISY) {
		ret = cts_tcs_clr_stylus_status(STYLUS_ALL_FREQ_NOISY);
		if (ret) {
			THP_LOGE("Clear STYLUS_ALL_FREQ_NOISY failed: %d", ret);
			goto err_clr_status;
		}
	} else {
		THP_LOGE("Clear_stylus_status(%d) not match", status);
		goto err_clr_status;
	}
	return 0;

err_clr_status:
	return ret;
}

#define CTS_SCAN_FREQ_START			100	//kHz
#define CTS_SCAN_FREQ_END			500	//kHz
void cts_afe_vendor_call(uint32_t value)
{
	uint8_t data = 0xFF;
	uint16_t scan_freq_value = 0;
	char proj_id_buf[128];

	All_LOG("cts_afe_vendor_call +");
	
	if ((value >= CTS_SCAN_FREQ_START) && (value <= CTS_SCAN_FREQ_END)) {
		scan_freq_value = value;
		value = VENDOR_SET_SCAN_FREQ;
	}
    switch (value) {
    case VENDOR_DEFAULT_RST:
		All_LOG("Vendor call: %d(%s)", value, "Set Scan Enable Select: CTS_VENDOR_DEFAULT");
		cts_reset_device();
		cts_force_get_hw_cap();
		cts_tcs_set_scan_enable_select(CTS_VENDOR_DEFAULT);
		vendor_call_flag = 0;
		break;

    case VENDOR_SET_SCAN_SELECT_GRID:
		All_LOG("Vendor call: %d(%s)", value, "Set Scan Enable Select: CTS_VENDOR_ONLY_GRID");
		vendor_call_flag = 1;
		cts_tcs_set_scan_enable_select(CTS_VENDOR_ONLY_GRID);
		break;
    case VENDOR_SET_SCAN_SELECT_SELF:
		All_LOG("Vendor call: %d(%s)", value, "Set Scan Enable Select: CTS_VENDOR_ONLY_SELF");
		vendor_call_flag = 1;
		cts_tcs_set_scan_enable_select(CTS_VENDOR_ONLY_SELF);
		break;
    case VENDOR_SET_SCAN_SELECT_UPLINK:
		All_LOG("Vendor call: %d(%s)", value, "Set Scan Enable Select: CTS_VENDOR_ONLY_UPLINK");
		vendor_call_flag = 1;
		cts_tcs_set_scan_enable_select(CTS_VENDOR_ONLY_UPLINK);
		break;
	case VENDOR_SET_SCAN_FREQ_TEST_EN:
		All_LOG("Vendor call: %d(%s)", value, "Set Freq Test Enable: 1-ON");
		data = 1;
		cts_tcs_set_scan_freq_test_en(data);
		break;
	case VENDOR_SET_SCAN_FREQ_TEST_DIS:
		All_LOG("Vendor call: %d(%s)", value, "Set Freq Test Enable: 0-OFF");
		data = 0;
		cts_tcs_set_scan_freq_test_en(data);
		break;
	case VENDOR_SET_SCAN_FREQ:
		All_LOG("Set Scan Freq: %d", scan_freq_value);
		cts_tcs_set_scan_freq(scan_freq_value);
		break;
	case VENDOR_GET_SCAN_SELECT:
		All_LOG("Vendor call: %d(%s)", value, "Get Scan Enable Select: 1-Grid, 2-Self, 4-Uplink");
		cts_tcs_get_scan_enable_select(&data);
		All_LOG("Scan enable select value = %d", data);
		break;
	case VENDOR_GET_SCAN_FREQ_TEST:
		All_LOG("Vendor call: %d(%s)", value, "Get Freq Test Enable: 0-OFF, 1-ON");
		cts_tcs_get_scan_freq_test_en(&data);
		All_LOG("Freq Test Enable = %d", data);
		break;
	case VENDOR_GET_SCAN_FREQ:
		All_LOG("Vendor call: %d(%s)", value, "Get Scan Freq: 100 ~ 350 kHz");
		cts_tcs_get_scan_freq(&scan_freq_value);
		All_LOG("Scan Freq = %d", scan_freq_value);
		break;

    case VENDOR_START_CALI:
		All_LOG("Vendor call: %d(%s)", value, "CALIBRATION");
		cts_tcs_start_calibration();
		break;
    case VENDOR_UPDATE_BASE_TO_FLASH:
		All_LOG("Vendor call: %d(%s)", value, "Update baseline to flash");
		cts_tcs_update_base_to_flash();
		break;
    case VENDOR_DUMP_RAWDATA_TABLE:
		All_LOG("Vendor call: Print 4 freqs rawdata-compensation tables");
		vendor_call_flag = 1;
		cts_dump_rawdata_table(rawdata_table);
		vendor_call_flag = 0;
		break;
    case VENDOR_SET_SCAN_STATE:
		All_LOG("Vendor call: set_scan_state,3");
		g_tcs_cmd[2] = 1;		
		afe_state_store = 3;
		break;

	case VENDOR_SET_PROJECT_ID:
		All_LOG("Vendor call: %d(%s)", value, "Read ProjectID");
#if IC_TYPE_ICNT92X8
		All_LOG("Get Project ID, todo...");
#else
		GetProjectID(proj_id_buf);
		All_LOG("ProjectID: %.10s", proj_id_buf);
		cts_reset_device();
#endif
	case VENDOR_GET_PROJECT_ID:
#if IC_TYPE_ICNT92X8
		All_LOG("Set Project ID, todo...");
#else
		All_LOG("Vendor call: %d(%s)", value, "Write ProjectID");
		SetProjectID(project_id);
		cts_reset_device();
#endif
		
    default:
    /*
        All_LOG("Invlid value %d! Need echo: ", value);
        All_LOG(
			"	0  --> DEFAULT_RST\n"
			"	1  --> SET_SCAN_SELECT_GRID\n"
			"	2  --> SET_SCAN_SELECT_SELF\n"
			"	3  --> SET_SCAN_SELECT_UPLINK\n"
			"	4  --> SET_SCAN_FREQ_TEST_EN\n"
			"	5  --> SET_SCAN_FREQ_TEST_DIS\n"
			"	7  --> GET_SCAN_SELECT\n"
			"	8  --> GET_SCAN_FREQ_TEST\n"
			"	9  --> GET_SCAN_FREQ\n"
			"	10 --> START_CALI\n"
			"	11 --> UPDATE_BASE_TO_FLASH\n"
			"	12 --> DUMP_RAWDATA_TABLE\n"
			"	13 --> SET_SCAN_STATE\n"
			"	20 --> SET_PROJECT_ID\n"
			"	21 --> GET_PROJECT_ID\n"
			"	100 ~ 500 --> SET_SCAN_FREQ\n"
		);
	*/
		All_LOG("%-12d--> %s", VENDOR_DEFAULT_RST, "DEFAULT_RST");
		All_LOG("%-12d--> %s", VENDOR_SET_SCAN_SELECT_GRID, "SET_SCAN_SELECT_GRID");
		All_LOG("%-12d--> %s", VENDOR_SET_SCAN_SELECT_SELF, "SET_SCAN_SELECT_SELF");
		All_LOG("%-12d--> %s", VENDOR_SET_SCAN_SELECT_UPLINK, "SET_SCAN_SELECT_UPLINK");
		All_LOG("%-12d--> %s", VENDOR_SET_SCAN_FREQ_TEST_EN, "SET_SCAN_FREQ_TEST_EN");
		All_LOG("%-12d--> %s", VENDOR_SET_SCAN_FREQ_TEST_DIS, "SET_SCAN_FREQ_TEST_DIS");
		All_LOG("%-12d--> %s", VENDOR_SET_SCAN_FREQ, "SET_SCAN_FREQ");
		All_LOG("%-12d--> %s", VENDOR_GET_SCAN_SELECT, "GET_SCAN_SELECT");
		All_LOG("%-12d--> %s", VENDOR_GET_SCAN_FREQ_TEST, "GET_SCAN_FREQ_TEST");
		All_LOG("%-12d--> %s", VENDOR_GET_SCAN_FREQ, "GET_SCAN_FREQ");
		All_LOG("%-12d--> %s", VENDOR_START_CALI, "START_CALI");
		All_LOG("%-12d--> %s", VENDOR_UPDATE_BASE_TO_FLASH, "UPDATE_BASE_TO_FLASH");
		All_LOG("%-12d--> %s", VENDOR_DUMP_RAWDATA_TABLE, "DUMP_RAWDATA_TABLE");
		All_LOG("%-12d--> %s", VENDOR_SET_SCAN_STATE, "SET_SCAN_STATE");
		All_LOG("%-12d--> %s", VENDOR_SET_PROJECT_ID, "SET_PROJECT_ID");
		All_LOG("%-12d--> %s", VENDOR_GET_PROJECT_ID, "GET_PROJECT_ID");
		All_LOG("%-12s--> %s", "100 ~ 500", "SET_SCAN_FREQ");
		return;
    }
	return;
}



int cts_afe_set_charger(bool plugin)
{
	int ret = -1;
	uint8_t enable = plugin ? 1 : 0;

	All_LOG("cts_afe_set_charger +");
	THP_LOGI("Set charger: %d", enable);
	ret = cts_tcs_set_charger(enable);
	return ret;
}

int cts_enable_update_compen()
{
	THP_LOGI("cts_enable_update_compen +");
	return cts_tcs_set_compen_enable(1);
}

int cts_disable_update_compen()
{
	THP_LOGI("cts_disable_update_compen +");
	return cts_tcs_set_compen_enable(0);
}

uint8_t cts_get_cur_clock(void)
{
	uint8_t clock = 0;
	int ret;
	
	All_LOG("cts_get_cur_clock +");
	ret = cts_tcs_get_cur_clock(&clock);
	if (!ret) {
		THP_LOGI("Get current clock: %d", clock);
	}
	return ret;
}

void cts_set_cur_clock(uint8_t clock)
{
	All_LOG("cts_set_cur_clock +, %d", clock);
	cts_tcs_set_cur_clock(clock);
}

int cts_flash_erase(uint8_t cmd, uint32_t flash_addr, uint32_t len)
{
	int ret, retries = 0;
	uint8_t status = 0;

	ret = cts_drw_write_u8(SFCTL_CMD_SEL, cmd);
	if (ret < 0) {
		THP_LOGE("Sector erase failed");
		return ret;
	}
	ret = cts_drw_write_u32(SFCTL_FLASH_ADDR, flash_addr);
	if (ret < 0) {
		THP_LOGE("Send flash start addr failed");
		return ret;
	}
	ret = cts_drw_write_u8(SFCTL_START_DEXC, 1);
	if (ret < 0) {
		THP_LOGE("start data transsion failed");
		return ret;
	}
	do {
		cts_drw_read_u8(SFCTL_SF_BUSY, &status);
		if (status == 0)
			break;
		cts_mdelay(10);
	} while (status && retries++ < 100);
	if (status) {
		THP_LOGE("read sfctl busy failed");
		return -1;
	}

	return ret;
}

int cts_efctrl_program_flash(uint8_t cmd, uint32_t flash_addr,
	uint32_t sram_addr, uint32_t len)
{
	int ret, retries = 0;
	uint8_t status = 1;

	ret = cts_drw_write_u32(SFCTL_FLASH_ADDR, flash_addr);
	if (ret < 0) {
		THP_LOGE("Send flash start addr failed");
		return ret;
	}
	ret = cts_drw_write_u32(SFCTL_SRAM_ADDR, sram_addr);
	if (ret < 0) {
		THP_LOGE("Send sram start addr failed");
		return ret;
	}
	ret = cts_drw_write_u32(SFCTL_DATA_LENGTH, len);
	if (ret < 0) {
		THP_LOGE("Write data len failed");
		return ret;
	}
	ret = cts_drw_write_u8(SFCTL_CMD_SEL, cmd);
	if (ret < 0) {
		THP_LOGE("Write cmd(%d) failed", cmd);
		return ret;
	}
	ret = cts_drw_write_u8(SFCTL_START_DEXC, 1);
	if (ret < 0) {
		THP_LOGE("start data transsion failed");
		return ret;
	}
	do {
		ret = cts_drw_read_u8(SFCTL_SF_BUSY, &status);
		if (!ret && (status == 0))
			break;
		cts_mdelay(10);
	} while (status && retries++ < 100);
	if (status) {
		THP_LOGE("read sfctl busy failed");
		return -1;
	}

	return ret;
}

int cts_nvr_unlock(void)
{
	int ret = -1;

	All_LOG("UN-LOCK NVR AREA");
	
	ret = cts_drw_write_u8(0x74075, 0x00);
	if (ret) {
		All_LOG("NVR 0x%x un-lock failed", 0x74075);
		return ret;
	}
	ret = cts_drw_write_u8(0x74074, 0x00);
	if (ret) {
		All_LOG("NVR 0x%x un-lock failed", 0x74074);
	}
	return ret;
}

int cts_nvr_lock(void)
{
	int ret = -1;

	All_LOG("LOCK NVR AREA");
	
	ret = cts_drw_write_u8(0x74075, 0x01);
	if (ret) {
		All_LOG("NVR 0x%x lock failed", 0x74075);
		return ret;
	}
	ret = cts_drw_write_u8(0x74074, 0x01);
	if (ret) {
		All_LOG("NVR 0x%x lock failed", 0x74074);
	}
	return ret;
}

