#ifndef CTS_CORE_H
#define CTS_CORE_H

#include "cts_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////////
#define HAL_VERSION						"1.0.2"		// Common version, don't change
///////////////////////////////////////////////////////////////////////////////////


#define MAX_NUM_SCAN_FREQ				10
#define MAX_NUM_SCAN_RATE				5


#define BIT(x)							(1 << x)
enum clear_stylus_status {
	HPP2_X_PROTOCAL				= BIT(0),
	HPP3_X_PROTOCAL				= BIT(1),
	STYLUS_DETECT_MODE			= BIT(2),
	STYLUS_CONFIRM_MODE			= BIT(3),
	STYLUS_ACTIVE_MODE			= BIT(4),
	STYLUS_FREQ_SHIFT_DONE		= BIT(5),
	STYLUS_FREQ_SHIFT_REQUEST	= BIT(6),
	STYLUS_ALL_FREQ_NOISY		= BIT(7),
};

enum debug_info_14 {
	DEBUG_CHARGER_BIT			= BIT(0),
	DEBUG_PALM_BIT				= BIT(1),
	DEBUG_H_SYNC_BIT			= BIT(2),
	DEBUG_STYLUS_BIT			= BIT(3),
	DEBUG_DYNAIC_IA_GAIN_BIT	= BIT(4),
};

#pragma pack(push, 1)
typedef struct {
	uint64_t	dummy;
	uint16_t	curr_size;
	uint16_t	next_size;
	uint16_t	frame_index;
	uint16_t	frame_type;
	uint16_t	noise_state;
	uint16_t	scan_freq;
	uint16_t	scan_rate;
	uint16_t	scan_state;
	uint16_t	afe_status;
	uint16_t	gesture_status;
	uint16_t	stylus_status;
	uint16_t	stylus_scan_freq_f1;
	uint16_t	stylus_scan_freq_f2;
	uint16_t	stylus_new_scan_freq_f1;
	uint16_t	stylus_new_scan_freq_f2;
} CTS_FRAME_HEADER;					// 38bytes

typedef struct {
	uint16_t	data[32];
} CTS_DEBUG_INFO_STRUCT;

typedef struct {
#if PATTERN_TYPE_1
	uint16_t	mutual[ROWS_PATTERN * COLS_PATTERN];
#else
	uint16_t	mutual[ROWS * COLS];
#endif
	uint16_t	self_rx[RX_NUM];
	uint16_t	self_tx[TX_NUM];
	uint16_t	noise[10];
} CTS_DATA_TYPE0;

//#define GRID_ALIGN					(5)							// 字节对齐，可变更；和固件确认，防止数据越界；
//#define GIRD_NUM					(ROWS * COLS * 2 / 5)		// 手笔同时，手互容分5次传完；不可变更；
typedef struct {
	uint16_t	stylus_rx_f1_I[RX_NUM];
	uint16_t	stylus_tx_f1_I[TX_NUM];
	uint16_t	stylus_rx_f1_Q[RX_NUM];
	uint16_t	stylus_tx_f1_Q[TX_NUM];
	uint16_t	stylus_rx_f2_I[RX_NUM];
	uint16_t	stylus_tx_f2_I[TX_NUM];
	uint16_t	stylus_rx_f2_Q[RX_NUM];
	uint16_t	stylus_tx_f2_Q[TX_NUM];
	//uint16_t	mutual[((GIRD_NUM+GRID_ALIGN-1)/GRID_ALIGN)*GRID_ALIGN];
	uint16_t	mutual[ROWS * COLS * 2 / 5];
} CTS_DATA_TYPE1;

typedef struct {
	uint16_t	stylus_rx_f1_I[RX_NUM];
	uint16_t	stylus_tx_f1_I[TX_NUM];
	uint16_t	stylus_rx_f1_Q[RX_NUM];
	uint16_t	stylus_tx_f1_Q[TX_NUM];
	uint16_t	stylus_rx_f2_I[RX_NUM];
	uint16_t	stylus_tx_f2_I[TX_NUM];
	uint16_t	stylus_rx_f2_Q[RX_NUM];
	uint16_t	stylus_tx_f2_Q[TX_NUM];
	uint16_t	self_rx[RX_NUM];
	uint16_t	self_tx[TX_NUM];
	uint16_t	noise[10];
} CTS_DATA_TYPE2;

typedef struct {
	uint8_t		retcode;
	uint16_t	cmd;
	uint16_t	crc16;
} CTS_TCS_RET_STRUCT;

typedef struct {
	CTS_FRAME_HEADER					header;
	CTS_DEBUG_INFO_STRUCT				debug_info;
	CTS_DATA_TYPE0						data;
	CTS_TCS_RET_STRUCT					tcs;
} CTS_FRAME_STRUCT0;

typedef struct {
	CTS_FRAME_HEADER					header;
	CTS_DEBUG_INFO_STRUCT				debug_info;
	CTS_DATA_TYPE1						data;
	CTS_TCS_RET_STRUCT					tcs;
} CTS_FRAME_STRUCT1;

typedef struct {
	CTS_FRAME_HEADER					header;
	CTS_DEBUG_INFO_STRUCT				debug_info;
	CTS_DATA_TYPE2						data;
	CTS_TCS_RET_STRUCT					tcs;
} CTS_FRAME_STRUCT2;

typedef struct {
	CTS_FRAME_HEADER					header;
	CTS_DEBUG_INFO_STRUCT				debug_info;
} CTS_FRAME_STRUCT;

//#define CTS_FRAME_MAX_SIZ				(5 * 1024)	//4096
/* data size + (header + debuginfo) + noise */
//#define CTS_FRAME_VALIDE_SIZ			\
	//((ROWS * COLS + ROWS + COLS) * 2 + 102 + 20)
#define CTS_FRAME_VALIDE_SIZ			\
	((ROWS_PATTERN * COLS_PATTERN + ROWS + COLS) * 2 + sizeof(CTS_FRAME_HEADER) + 20)
#define CTS_FRAME_MAX_SIZ				\
	((CTS_FRAME_VALIDE_SIZ > (5 * 1024)) ? CTS_FRAME_VALIDE_SIZ + 5 : (5 * 1024))

#pragma pack(pop)

/* match thp api */
int cts_open(void);
int cts_close(void);
int cts_open_project(const char *proj_id);
int cts_set_calib_data_callback_func(
    THP_AFE_ERR_ENUM(*calibDataWriteCallback)(void* dataPtr, uint32_t dataLen),
    THP_AFE_ERR_ENUM(*calibDataReadCallback)(void* dataPtr, uint32_t dataLen));
int cts_start(void);
THP_AFE_INFO_STRUCT *cts_get_info(void);
THP_AFE_HW_CAP_STRUCT *cts_get_hw_cap(void);
THP_AFE_FRAME_DATA_STRUCT *cts_get_frame(void);
int cts_stop(void);
int cts_set_idle_touch_threshold(uint16_t threshold);
int cts_set_baseline_update_interval(uint16_t interval);
int cts_reset_idle_baseline(void);
int cts_enter_idle(void);
int cts_exit_idle(void);
int cts_clear_status(THP_AFE_STATUS_ENUM status);
int cts_set_scan_state(THP_AFE_SCAN_STATE_ENUM state);
int cts_clear_stylus_status(THP_AFE_STYLUS_STATUS_ENUM status);
int cts_screen_off(void);
int cts_screen_on(void);
int cts_afe_enter_tui(void);
int cts_afe_exit_tui(void);
int cts_afe_enable_wakeup_gesture(THP_AFE_GESTURE_ENUM gesture);
int cts_afe_disable_wakeup_gesture(THP_AFE_GESTURE_ENUM gesture);
int cts_clear_gesture_status(THP_AFE_GESTURE_ENUM gesture);


int cts_enable_freq_shift(void);
int cts_disable_freq_shift(void);
int cts_start_calibration(void);
int cts_start_fold_calibration(void);
int cts_force_to_freq_point(uint8_t index);
int cts_force_to_scan_rate(uint8_t index);
int cts_stylus_freq_immediately(void);
int cts_stylus_freq_next_uplink(void);
int cts_enable_stylus_hpp3_0(void);
int cts_disable_stylus_hpp3_0(void);
int cts_afe_suspend(void);
int cts_afe_resume(void);

uint8_t cts_xtal_change_request(void);
uint8_t cts_get_xtal_enable(void);

void cts_afe_vendor_call(uint32_t value);

int cts_afe_set_charger(bool plugin);

int cts_enable_update_compen();
int cts_disable_update_compen();

void cts_set_cur_clock(uint8_t clock);
uint8_t cts_get_cur_clock(void);

struct cts_dev_info {
	uint8_t mst_tx_num;
	uint8_t mst_rx_num;
	uint8_t slv_tx_num;
	uint8_t slv_rx_num;
	uint8_t tx_tr_order[TR_ORDER_MAX];
	uint8_t rx_tr_order[TR_ORDER_MAX];

	bool pattern_type;
};

int cts_reset_device(void);
int cts_reset_device_inspect(void);
int wait_to_norm(void);
int cts_force_get_hw_cap(void);

enum cts_vendor_call {
	CTS_VENDOR_DEFAULT					= 0,
	CTS_VENDOR_ONLY_GRID 				= BIT(0),
	CTS_VENDOR_ONLY_SELF				= BIT(1),
	CTS_VENDOR_ONLY_UPLINK				= BIT(2),
};
enum vendor_call_index {
	VENDOR_DEFAULT_RST					= 0,
	VENDOR_SET_SCAN_SELECT_GRID 		= 1,
	VENDOR_SET_SCAN_SELECT_SELF			= 2,
	VENDOR_SET_SCAN_SELECT_UPLINK		= 3,
	VENDOR_SET_SCAN_FREQ_TEST_EN		= 4,
	VENDOR_SET_SCAN_FREQ_TEST_DIS		= 5,
	VENDOR_SET_SCAN_FREQ				= 6,
	VENDOR_GET_SCAN_SELECT				= 7,
	VENDOR_GET_SCAN_FREQ_TEST			= 8,
	VENDOR_GET_SCAN_FREQ				= 9,
	VENDOR_START_CALI					= 10,
	VENDOR_UPDATE_BASE_TO_FLASH			= 11,
	VENDOR_DUMP_RAWDATA_TABLE			= 12,
	VENDOR_SET_SCAN_STATE				= 13,

	VENDOR_SET_PROJECT_ID				= 20,
	VENDOR_GET_PROJECT_ID				= 21,
};

int cts_flash_erase(uint8_t cmd, uint32_t flash_addr, uint32_t len);
int cts_efctrl_program_flash(uint8_t cmd, uint32_t flash_addr,
	uint32_t sram_addr, uint32_t len);

int cts_nvr_unlock(void);
int cts_nvr_lock(void);

#ifdef __cplusplus
}
#endif

#endif /* CTS_CORE_H */

