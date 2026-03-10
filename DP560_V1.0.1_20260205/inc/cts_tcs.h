#ifndef CTS_TCS_H
#define CTS_TCS_H

#include "cts_hal.h"

#pragma pack(push, 1)
typedef struct {
	uint8_t  addr;
	uint16_t cmd;
	uint16_t datlen;
	uint16_t crc16;
} tcs_tx_head;

typedef struct {
	uint8_t  ecode;
	uint16_t cmd;
	uint16_t crc16;
} tcs_rx_tail;

#pragma pack(pop)

enum TcsCmdIndex
{
	TP_STD_CMD_INFO_READ_RO,
	TP_STD_CMD_INFO_HOSTCOMM_VER_RO,
	TP_STD_CMD_INFO_CHIP_HW_ID_RO,
	TP_STD_CMD_INFO_CHIP_FW_ID_RO,
	TP_STD_CMD_INFO_MANUFACTURE_ID_RO,
	TP_STD_CMD_INFO_FW_VER_RO,
	TP_STD_CMD_INFO_TCS_BUF_MAX_LEN_RO,
	TP_STD_CMD_INFO_FLASH_CONFIG_VER_RO,
	TP_STD_CMD_INFO_TCS_VER_RO,
	TP_STD_CMD_SYS_STS_READ_RO,
	TP_STD_CMD_SYS_STS_OFFSET_AND_TYPE_CFG_RW,
	TP_STD_CMD_SYS_STS_READ_START_RO,
	TP_STD_CMD_SYS_STS_WR_REG_RAM_SEQUENCE_WO,
	TP_STD_CMD_SYS_STS_WR_REG_RAM_BATCH_WO,
	TP_STD_CMD_SYS_STS_GET_COORDINATES_RO,
	TP_STD_CMD_SYS_STS_GET_DATA_BY_POLLING_RO,
	TP_STD_CMD_SYS_STS_WORK_MODE_RW,
	TP_STD_CMD_SYS_STS_SYS_BUSY_FLAG_RO,
	TP_STD_CMD_SYS_STS_DAT_RDY_FLAG_RW,
	TP_STD_CMD_SYS_STS_PWR_STATE_RW,
	TP_STD_CMD_SYS_STS_CHARGER_PLUGIN_RW,
	TP_STD_CMD_SYS_STS_UPDATE_BASE_RW,
	TP_STD_CMD_SYS_STS_HEART_BEAT_RO,
	TP_STD_CMD_SYS_STS_REPORTRATE_NUM_RO,
	TP_STD_CMD_SYS_STS_ALL_REPORTRATE_RO,
	TP_STD_CMD_SYS_STS_LOCK_REPORT_RATE_EN_RW,
	TP_STD_CMD_SYS_STS_SYSTM_PASS_RO,
	TP_STD_CMD_SYS_STS_RESET_WO,
	TP_STD_CMD_SYS_STS_CURRENT_WORKMODE_RO,
	TP_STD_CMD_SYS_STS_DATA_CAPTURE_SUPPORT_RO,
	TP_STD_CMD_SYS_STS_DATA_CAPTURE_EN_RW,
	TP_STD_CMD_SYS_STS_DATA_CAPTURE_FUNC_MAP_RW,
	TP_STD_CMD_SYS_STS_OFFSET_AND_LEN_CFG_RW,
	TP_STD_CMD_SYS_STS_REG_DAT_RDY_FLAG_RW,
	TP_STD_CMD_SYS_STS_REG_READ_START_RO,
	Resvered,
	TP_STD_CMD_SYS_STS_FFT_EN_RW,
	TP_STD_CMD_SYS_STS_FFT_DAT_RDY_FLAG_RW,
	TP_STD_CMD_SYS_STS_SUSPEND_SCAN_EN_RW,
	TP_STD_CMD_SYS_STS_POWER_ON_READ_FLAG_RW,
	TP_STD_CMD_SYS_STS_FLASH_CONFIG_VER_ERROR_RW,
	TP_STD_CMD_SYS_STS_EXT_CLK_INPUT_RW,
	TP_STD_CMD_SYS_STS_CLEAR_STYLIS_STATUS_RW,
	TP_STD_CMD_SYS_STS_SHB_EN_RW,
	TP_STD_CMD_SYS_STS_SCAN_STATE_RW,
	TP_STD_CMD_SYS_STS_START_RECALIB_RW,
	TP_STD_CMD_SYS_STS_CLEAR_FW_STATUS_RW,
	TP_STD_CMD_SYS_STS_STYLUS_PRESS_RW,
	TP_STD_CMD_SYS_STS_WRITE_HOP_FREQ_BASE_TO_FLASH_EN_RW,
	TP_STD_CMD_SYS_STS_FOLD_COMPEN_RW,
	TP_STD_CMD_SYS_STS_EAR_PHONE_EN_RW,
	TP_STD_CMD_SYS_STS_PROXIMITY_EN_RW,
	TP_STD_CMD_SYS_STS_GAME_MODE_EN_RW,
	TP_STD_CMD_ALGO_INFO_READ_RO,
	TP_STD_CMD_ALGO_DATA_FILTER_RW,
	TP_STD_CMD_ALGO_EDGE_PROC_RW,
	TP_STD_CMD_ALGO_ESD_RW,
	TP_STD_CMD_ALGO_POS_FILTER_RW,
	TP_STD_CMD_ALGO_REGION_INFO_RW,
	TP_STD_CMD_ALGO_TCH_TH_RW,
	TP_STD_CMD_ALGO_WATER_PROOF_RW,
	TP_STD_CMD_ALGO_STYLUS_CFG_RW,
	TP_STD_CMD_ALGO_EDGE_INHIBITION_RW,
	TP_STD_CMD_BASE_READ_RO,
	TP_STD_CMD_BASE_UPDATE_EN_RW,
	TP_STD_CMD_BASE_ENVIR_UPDATE_EN_RW,
	TP_STD_CMD_BASE_OFFEST_UPDATE_EN_RW,
	TP_STD_CMD_BASE_NEG_UPDATE_EN_RW,
	TP_STD_CMD_BASE_SC_UPDATE_MC_EN_RW,
	TP_STD_CMD_BASE_SC_UPDATE_EN_RW,
	TP_STD_CMD_BASE_SC_TRACE_EN_RW,
	TP_STD_CMD_BASE_MC_TRACE_EN_RW,
	TP_STD_CMD_BASE_MC_TRACE_PERIOD_RW,
	TP_STD_CMD_BASE_MC_TRACE_STEP_RW,
	TP_STD_CMD_BASE_SC_TRACE_PERIOD_RW,
	TP_STD_CMD_BASE_SC_TRACE_STEP_RW,
	TP_STD_CMD_BASE_MC_UPDATE_SC_EN_RW,
	TP_STD_CMD_MC_NEED_UPDATE_BASE,
	TP_STD_CMD_SC_NEED_UPDATE_BASE,
	TP_STD_CMD_TP_PARA_READ_RO,
	TP_STD_CMD_TP_PARA_TOUCH_INFO_RO,
	TP_STD_CMD_TP_PARA_MASTER_TX_NUM_RO,
	TP_STD_CMD_TP_PARA_MASTER_RX_NUM_RO,
	TP_STD_CMD_TP_PARA_SLAVE_TX_NUM_RO,
	TP_STD_CMD_TP_PARA_SLAVE_RX_NUM_RO,
	TP_STD_CMD_TP_PARA_MASTER_TOUCH_NUM_RO,
	TP_STD_CMD_TP_PARA_INIT_MODE_RW,
	TP_STD_CMD_TP_PARA_WAKEUP_POL_RW,
	TP_STD_CMD_TP_PARA_GPIO_VOL_RW,
	TP_STD_CMD_TP_PARA_REPORT_RATE_RW,
	TP_STD_CMD_TP_PARA_PATTERN_TYPE_RO,
	TP_STD_CMD_TP_PARA_XY_SWAP_RW,
	TP_STD_CMD_TP_PARA_X_SWAP_RW,
	TP_STD_CMD_TP_PARA_Y_SWAP_RW,
	TP_STD_CMD_TP_PARA_TX_TR_ORDER_RO,
	TP_STD_CMD_TP_PARA_RX_TR_ORDER_RO,
	TP_STD_CMD_TP_PARA_SLV_TX_TR_ORDER_RO,
	TP_STD_CMD_TP_PARA_SLV_RX_TR_ORDER_RO,
	TP_STD_CMD_TP_PARA_ALL_TX_TR_ORDER_RO,
	TP_STD_CMD_TP_PARA_ALL_RX_TR_ORDER_RO,
	TP_STD_CMD_SCAN_READ_RO,
	TP_STD_CMD_SCAN_MUTUAL_SCAN_TIME_RO,
	TP_STD_CMD_SCAN_MUTUAL_DRVING_FREQ_RO,
	TP_STD_CMD_SCAN_MUTUAL_HOPPING_FREQ_RO,
	TP_STD_CMD_SCAN_SELF_SCAN_TIME_RO,
	TP_STD_CMD_SCAN_SELF_DRVING_FREQ_RO,
	TP_STD_CMD_SCAN_STYLUS_SCAN_TIME_RO,
	TP_STD_CMD_SCAN_STYLUS_SENSING_F1_RO,
	TP_STD_CMD_SCAN_STYLUS_SENSING_F2_RO,
	TP_STD_CMD_SCAN_MUTUAL_SCAN_MODE_RW,
	TP_STD_CMD_SCAN_SELF_SCAN_MODE_RW,
	TP_STD_CMD_SCAN_STYLUS_SCAN_MODE_RW,
	TP_STD_CMD_SCAN_HSYNC_VSYNC_MODE_RW,
	TP_STD_CMD_SCAN_MUTUAL_DATA_RAW_RO,
	TP_STD_CMD_SCAN_MUTUAL_DATA_DIFF_RO,
	TP_STD_CMD_SCAN_MUTUAL_DATA_BASE_RO,
	TP_STD_CMD_SCAN_SELF_DATA_RAW_RO,
	TP_STD_CMD_SCAN_SELF_DATA_DIFF_RO,
	TP_STD_CMD_SCAN_SELF_DATA_BASE_RO,
	TP_STD_CMD_SCAN_STYLUS_DATA_RAW_RO,
	TP_STD_CMD_SCAN_STYLUS_DATA_DIFF_RO,
	TP_STD_CMD_SCAN_STYLUS_DATA_BASE_RO,
	TP_STD_CMD_SCAN_SCAN_ENABLE_SELECT,
	TP_STD_CMD_SCAN_SCAN_FREQ_TEST_EN,
	TP_STD_CMD_SCAN_SET_SCAN_FREQ,
	TP_STD_CMD_NOISE_READ_RO,
	TP_STD_CMD_NOISE_HOP_NO_TOUCH_UP_TM_RW,
	TP_STD_CMD_NOISE_FREQ_HOP_MODE_RW,
	TP_STD_CMD_NOISE_FREQ_HOP_INDEX_RW,
	TP_STD_CMD_NOISE_FREQ_HOP_TH_RW,
	TP_STD_CMD_FFT_NOISE_VAL_RO,
	TP_STD_CMD_NUM_HOP_FREQ_RO,
	TP_STD_CMD_HOP_FREQ_ClC_NUM_RW,
	TP_STD_CMD_NUM_STYLUS_HOP_FREQ_RO,
	TP_STD_CMD_STYLUS_HOP_FREQ_ClC_NUM_RO,
	TP_STD_CMD_FORCE_LOCK_HOP_FREQ_WO,
	TP_STD_CMD_FORCE_LOCK_FREQ_EN_RW,
	TP_STD_CMD_FORCE_STYLUS_HOP_FREQ_RW,
	TP_STD_CMD_HOP_FREQ_HANDL_EN_RW,
	TP_STD_CMD_GSTR_READ_RO,
	TP_STD_CMD_GSTR_ENTER_GSTR_FS_RW,
	TP_STD_CMD_GSTR_EXIT_GSTR_RW,
	TP_STD_CMD_GSTR_ENTER_GSTR_LP_RW,
	TP_STD_CMD_GSTR_ALGO_RW,
	TP_STD_CMD_GSTR_ENABLE_TYPE_RW,
	TP_STD_CMD_GSTR_DISABLE_TYPE_RW,
	TP_STD_CMD_GSTR_ClEAR_STATUS_RW,
	TP_STD_CMD_LOW_POWER_READ_RO,
	TP_STD_CMD_LOW_POWER_ENTER_MONITOR_TM_RW,
	TP_STD_CMD_LOW_POWER_MONITOR_WAKEUP_CNT_RW,
	TP_STD_CMD_LOW_POWER_IDLE_MODE_EN_RW,
	TP_STD_CMD_LOW_POWER_ENTER_IDLE_MODE_RW,
	TP_STD_CMD_LOW_POWER_IDLE_REPORT_INTERVAL_RW,
	TP_STD_CMD_LOW_POWER_EXIT_IDLE_MODE_RW,
	TP_STD_CMD_LOW_POWER_IDLE_TOUCH_THRESHOLD_RW,
	TP_STD_CMD_LOW_POWER_IDLE_SCAN_RATE_RW,
	TP_STD_CMD_LOW_POWER_RESET_IDLE_BASE_EN_RW,
	TP_STD_CMD_STYLUS_READ_RO,
	TP_STD_CMD_ICTEST_READ_RO,
	TP_STD_CMD_ICTEST_FLASH_CP_EN_RW,
	TP_STD_CMD_ICTEST_HOP_FREQ_EN_RW,
	TP_STD_CMD_ICTEST_SHORTTEST_EN_RW,
	TP_STD_CMD_ICTEST_SHORTTEST_RES_VAL_RO,
	TP_STD_CMD_ICTEST_OPENTEST_EN_RW,
	TP_STD_CMD_ICTEST_SELF_COMP_RW,
	TP_STD_CMD_ICTEST_FW_DATA_PROCESS_EN,
	TP_STD_CMD_ICTEST_SHORT_CHANNEL_TO_GND,
	TP_STD_CMD_ICTEST_MUTUAL_SHORT_CHANNEL,
	TP_STD_CMD_ICTEST_HSYNC_TEST_EN,
	TP_STD_CMD_ICTEST_HSYNC_RESULT,
	TP_STD_CMD_ICTEST_SHORT_THRESHOLD,
	TP_STD_CMD_ICTEST_REAL_HSYNC_FREQ,
	TP_STD_CMD_ICTEST_OSC_TRIM_VALUE,
	TP_STD_CMD_ICTEST_OSC_TRIM_VALUE_FINE,
	TP_STD_CMD_ICTEST_OSC_SHIFT_TRIM_TRIGGER_RATIO,
	TP_STD_CMD_ICTEST_AP_CLK_TRIM_OSC,
	TP_STD_CMD_ICTEST_OSC_SHIFT_TRIM_MAX_RATIO,
    TP_STD_CMD_ICTEST_VSYNC_TEST_EN,
    TP_STD_CMD_ICTEST_VSYNC_PASS_FLAG,
    TP_STD_CMD_ICTEST_MUTUAL_RAWDATA_PREPRO_EN,
    TP_STD_CMD_ICTEST_SELF_DCAP_ADJUSTEN,
};
// public const ushort TCS_VERSION = 0x0312;
static const unsigned char TcsCmdValue[][7] =
{
	//-----------------------------------------------------------
	// baseFlage, classID, cmdID, isRead, isWrite, isData, retLen
	//-----------------------------------------------------------
	{ 0, 16,  0,  1,  0,  1,  0 },    // TP_STD_CMD_INFO_READ_RO,
	{ 0, 16,  1,  1,  0,  0,  2 },    // TP_STD_CMD_INFO_HOSTCOMM_VER_RO,
	{ 0, 16,  2,  1,  0,  0,  5 },    // TP_STD_CMD_INFO_CHIP_HW_ID_RO,
	{ 0, 16,  3,  1,  0,  0,  4 },    // TP_STD_CMD_INFO_CHIP_FW_ID_RO,
	{ 0, 16,  4,  1,  0,  0,  4 },    // TP_STD_CMD_INFO_MANUFACTURE_ID_RO,
	{ 0, 16,  5,  1,  0,  0,  4 },    // TP_STD_CMD_INFO_FW_VER_RO,
	{ 0, 16,  6,  1,  0,  0,  4 },    // TP_STD_CMD_INFO_TCS_BUF_MAX_LEN_RO,
	{ 0, 16,  7,  1,  0,  0,  2 },    // TP_STD_CMD_INFO_FLASH_CONFIG_VER_RO,
	{ 0, 16,  8,  1,  0,  0,  2 },    // TP_STD_CMD_INFO_TCS_VER_RO,
	{ 0, 17,  0,  1,  0,  1,  0 },    // TP_STD_CMD_SYS_STS_READ_RO,
	{ 0, 17,  1,  1,  1,  0,  4 },    // TP_STD_CMD_SYS_STS_OFFSET_AND_TYPE_CFG_RW,
	{ 0, 17,  2,  1,  0,  1,  0 },    // TP_STD_CMD_SYS_STS_READ_START_RO,
	{ 0, 17,  3,  0,  1,  0,  0 },    // TP_STD_CMD_SYS_STS_WR_REG_RAM_SEQUENCE_WO,
	{ 0, 17,  4,  0,  1,  0,  0 },    // TP_STD_CMD_SYS_STS_WR_REG_RAM_BATCH_WO,
	{ 0, 17,  5,  1,  0,  1,  0 },    // TP_STD_CMD_SYS_STS_GET_COORDINATES_RO,
	{ 0, 17,  6,  1,  0,  0,  0 },    // TP_STD_CMD_SYS_STS_GET_DATA_BY_POLLING_RO,
	{ 0, 17,  7,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_WORK_MODE_RW,
	{ 0, 17,  8,  1,  0,  0,  1 },    // TP_STD_CMD_SYS_STS_SYS_BUSY_FLAG_RO,
	{ 0, 17,  9,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_DAT_RDY_FLAG_RW,
	{ 0, 17, 10,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_PWR_STATE_RW,
	{ 0, 17, 11,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_CHARGER_PLUGIN_RW,
	{ 0, 17, 12,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_UPDATE_BASE_RW,
	{ 0, 17, 13,  1,  0,  0,  4 },    // TP_STD_CMD_SYS_STS_HEART_BEAT_RO,
	{ 0, 17, 14,  1,  0,  0,  1 },    // TP_STD_CMD_SYS_STS_REPORTRATE_NUM_RO,
	{ 0, 17, 15,  1,  0,  0,  0 },    // TP_STD_CMD_SYS_STS_ALL_REPORTRATE_RO,
	{ 0, 17, 16,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_LOCK_REPORT_RATE_EN_RW,
	{ 0, 17, 17,  1,  0,  0,  1 },    // TP_STD_CMD_SYS_STS_SYSTM_PASS_RO,
	{ 0, 17, 18,  0,  1,  0,  2 },    // TP_STD_CMD_SYS_STS_RESET_WO,
	{ 0, 17, 19,  1,  0,  0,  1 },    // TP_STD_CMD_SYS_STS_CURRENT_WORKMODE_RO,
	{ 0, 17, 20,  1,  0,  0,  1 },    // TP_STD_CMD_SYS_STS_DATA_CAPTURE_SUPPORT_RO,
	{ 0, 17, 21,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_DATA_CAPTURE_EN_RW,
	{ 0, 17, 22,  1,  1,  0,  2 },    // TP_STD_CMD_SYS_STS_DATA_CAPTURE_FUNC_MAP_RW,
	{ 0, 17, 23,  1,  1,  0,  5 },    // TP_STD_CMD_SYS_STS_OFFSET_AND_LEN_CFG_RW,
	{ 0, 17, 24,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_REG_DAT_RDY_FLAG_RW,
	{ 0, 17, 25,  1,  0,  1,  0 },    // TP_STD_CMD_SYS_STS_REG_READ_START_RO,
	{ 0, 17, 26,  1,  1,  0,  1 },    // Resvered,
	{ 0, 17, 27,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_FFT_EN_RW,
	{ 0, 17, 28,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_FFT_DAT_RDY_FLAG_RW,
	{ 0, 17, 29,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_SUSPEND_SCAN_EN_RW,
	{ 0, 17, 30,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_POWER_ON_READ_FLAG_RW,
	{ 0, 17, 31,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_FLASH_CONFIG_VER_ERROR_RW,
	{ 0, 17, 32,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_EXT_CLK_INPUT_RW,
	{ 0, 17, 33,  1,  1,  0,  2 },    // TP_STD_CMD_SYS_STS_CLEAR_STYLIS_STATUS_RW,
	{ 0, 17, 34,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_SHB_EN_RW,
	{ 0, 17, 35,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_SCAN_STATE_RW,
	{ 0, 17, 36,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_START_RECALIB_RW,
	{ 0, 17, 37,  1,  1,  0,  4 },    // TP_STD_CMD_SYS_STS_CLEAR_FW_STATUS_RW,
	{ 0, 17, 38,  1,  1,  0,  2 },    // TP_STD_CMD_SYS_STS_STYLUS_PRESS_RW,
	{ 0, 17, 39,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_WRITE_HOP_FREQ_BASE_TO_FLASH_EN_RW,
	{ 0, 17, 40,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_FOLD_COMPEN_RW
	{ 0, 17, 41,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_EAR_PHONE_EN_RW
	{ 0, 17, 42,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_PROXIMITY_EN_RW
	{ 0, 17, 43,  1,  1,  0,  1 },    // TP_STD_CMD_SYS_STS_GAME_MODE_EN_RW
	{ 0, 18,  0,  1,  0,  1,  0 },    // TP_STD_CMD_ALGO_INFO_READ_RO,
	{ 0, 18,  1,  1,  1,  0, 18 },    // TP_STD_CMD_ALGO_DATA_FILTER_RW,
	{ 0, 18,  2,  1,  1,  0, 13 },    // TP_STD_CMD_ALGO_EDGE_PROC_RW,
	{ 0, 18,  3,  1,  1,  0,  4 },    // TP_STD_CMD_ALGO_ESD_RW,
	{ 0, 18,  4,  1,  1,  0, 17 },    // TP_STD_CMD_ALGO_POS_FILTER_RW,
	{ 0, 18,  5,  1,  1,  0, 20 },    // TP_STD_CMD_ALGO_REGION_INFO_RW,
	{ 0, 18,  6,  1,  1,  0, 16 },    // TP_STD_CMD_ALGO_TCH_TH_RW,
	{ 0, 18,  7,  1,  1,  0,  3 },    // TP_STD_CMD_ALGO_WATER_PROOF_RW,
	{ 0, 18,  8,  1,  1,  0, 32 },    // TP_STD_CMD_ALGO_STYLUS_CFG_RW,
	{ 0, 18,  9,  1,  1,  0, 85 },    // TP_STD_CMD_ALGO_EDGE_INHIBITION_RW,
	{ 0, 19,  0,  1,  0,  1,  0 },    // TP_STD_CMD_BASE_READ_RO,
	{ 0, 19,  1,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_UPDATE_EN_RW,
	{ 0, 19,  2,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_ENVIR_UPDATE_EN_RW,
	{ 0, 19,  3,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_OFFEST_UPDATE_EN_RW,
	{ 0, 19,  4,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_NEG_UPDATE_EN_RW,
	{ 0, 19,  5,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_SC_UPDATE_MC_EN_RW,
	{ 0, 19,  6,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_SC_UPDATE_EN_RW,
	{ 0, 19,  7,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_SC_TRACE_EN_RW,
	{ 0, 19,  8,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_MC_TRACE_EN_RW,
	{ 0, 19,  9,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_MC_TRACE_PERIOD_RW,
	{ 0, 19, 10,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_MC_TRACE_STEP_RW,
	{ 0, 19, 11,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_SC_TRACE_PERIOD_RW,
	{ 0, 19, 12,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_SC_TRACE_STEP_RW,
	{ 0, 19, 13,  1,  1,  0,  1 },    // TP_STD_CMD_BASE_MC_UPDATE_SC_EN_RW,
	{ 0, 19, 14,  1,  1,  0,  1 },    // TP_STD_CMD_MC_NEED_UPDATE_BASE,
	{ 0, 19, 15,  1,  1,  0,  1 },    // TP_STD_CMD_SC_NEED_UPDATE_BASE,
	{ 0, 20,  0,  1,  0,  1,  0 },    // TP_STD_CMD_TP_PARA_READ_RO,
	{ 0, 20,  1,  1,  0,  0,  6 },    // TP_STD_CMD_TP_PARA_TOUCH_INFO_RO,
	{ 0, 20,  2,  1,  0,  0,  1 },    // TP_STD_CMD_TP_PARA_MASTER_TX_NUM_RO,
	{ 0, 20,  3,  1,  0,  0,  1 },    // TP_STD_CMD_TP_PARA_MASTER_RX_NUM_RO,
	{ 0, 20,  4,  1,  0,  0,  1 },    // TP_STD_CMD_TP_PARA_SLAVE_TX_NUM_RO,
	{ 0, 20,  5,  1,  0,  0,  1 },    // TP_STD_CMD_TP_PARA_SLAVE_RX_NUM_RO,
	{ 0, 20,  6,  1,  0,  0,  1 },    // TP_STD_CMD_TP_PARA_MASTER_TOUCH_NUM_RO,
	{ 0, 20,  7,  1,  1,  0,  1 },    // TP_STD_CMD_TP_PARA_INIT_MODE_RW,
	{ 0, 20,  8,  1,  1,  0,  1 },    // TP_STD_CMD_TP_PARA_WAKEUP_POL_RW,
	{ 0, 20,  9,  1,  1,  0,  1 },    // TP_STD_CMD_TP_PARA_GPIO_VOL_RW,
	{ 0, 20, 10,  1,  1,  0,  2 },    // TP_STD_CMD_TP_PARA_REPORT_RATE_RW,
	{ 0, 20, 11,  1,  0,  0,  1 },    // TP_STD_CMD_TP_PARA_PATTERN_TYPE_RO,
	{ 0, 20, 12,  1,  1,  0,  1 },    // TP_STD_CMD_TP_PARA_XY_SWAP_RW,
	{ 0, 20, 13,  1,  1,  0,  1 },    // TP_STD_CMD_TP_PARA_X_SWAP_RW,
	{ 0, 20, 14,  1,  1,  0,  1 },    // TP_STD_CMD_TP_PARA_Y_SWAP_RW,
	{ 0, 20, 15,  1,  0,  0, 84 },    // TP_STD_CMD_TP_PARA_TX_TR_ORDER_RO,
	{ 0, 20, 16,  1,  0,  0, 84 },    // TP_STD_CMD_TP_PARA_RX_TR_ORDER_RO,
	{ 0, 20, 17,  1,  0,  0, 84 },    // TP_STD_CMD_TP_PARA_SLV_TX_TR_ORDER_RO,
	{ 0, 20, 18,  1,  0,  0, 84 },    // TP_STD_CMD_TP_PARA_SLV_RX_TR_ORDER_RO,
	{ 0, 20, 19,  1,  0,  0, 84 },    // TP_STD_CMD_TP_PARA_ALL_TX_TR_ORDER_RO,
	{ 0, 20, 20,  1,  0,  0, 84 },    // TP_STD_CMD_TP_PARA_ALL_RX_TR_ORDER_RO,
	{ 0, 21,  0,  1,  0,  1,  0 },    // TP_STD_CMD_SCAN_READ_RO,
	{ 0, 21,  1,  1,  0,  0,  2 },    // TP_STD_CMD_SCAN_MUTUAL_SCAN_TIME_RO,
	{ 0, 21,  2,  1,  0,  0,  2 },    // TP_STD_CMD_SCAN_MUTUAL_DRVING_FREQ_RO,
	{ 0, 21,  3,  1,  0,  0,  2 },    // TP_STD_CMD_SCAN_MUTUAL_HOPPING_FREQ_RO,
	{ 0, 21,  4,  1,  0,  0,  2 },    // TP_STD_CMD_SCAN_SELF_SCAN_TIME_RO,
	{ 0, 21,  5,  1,  0,  0,  2 },    // TP_STD_CMD_SCAN_SELF_DRVING_FREQ_RO,
	{ 0, 21,  6,  1,  0,  0,  2 },    // TP_STD_CMD_SCAN_STYLUS_SCAN_TIME_RO,
	{ 0, 21,  7,  1,  0,  0,  2 },    // TP_STD_CMD_SCAN_STYLUS_SENSING_F1_RO,
	{ 0, 21,  8,  1,  0,  0,  2 },    // TP_STD_CMD_SCAN_STYLUS_SENSING_F2_RO,
	{ 0, 21,  9,  1,  1,  0,  1 },    // TP_STD_CMD_SCAN_MUTUAL_SCAN_MODE_RW,
	{ 0, 21, 10,  1,  1,  0,  1 },    // TP_STD_CMD_SCAN_SELF_SCAN_MODE_RW,
	{ 0, 21, 11,  1,  1,  0,  1 },    // TP_STD_CMD_SCAN_STYLUS_SCAN_MODE_RW,
	{ 0, 21, 12,  1,  1,  0,  1 },    // TP_STD_CMD_SCAN_HSYNC_VSYNC_MODE_RW,
	{ 0, 21, 13,  1,  0,  1,  0 },    // TP_STD_CMD_SCAN_MUTUAL_DATA_RAW_RO,
	{ 0, 21, 14,  1,  0,  1,  0 },    // TP_STD_CMD_SCAN_MUTUAL_DATA_DIFF_RO,
	{ 0, 21, 15,  1,  0,  1,  0 },    // TP_STD_CMD_SCAN_MUTUAL_DATA_BASE_RO,
	{ 0, 21, 16,  1,  0,  1,  0 },    // TP_STD_CMD_SCAN_SELF_DATA_RAW_RO,
	{ 0, 21, 17,  1,  0,  1,  0 },    // TP_STD_CMD_SCAN_SELF_DATA_DIFF_RO,
	{ 0, 21, 18,  1,  0,  1,  0 },    // TP_STD_CMD_SCAN_SELF_DATA_BASE_RO,
	{ 0, 21, 19,  1,  0,  1,  0 },    // TP_STD_CMD_SCAN_STYLUS_DATA_RAW_RO,
	{ 0, 21, 20,  1,  0,  1,  0 },    // TP_STD_CMD_SCAN_STYLUS_DATA_DIFF_RO,
	{ 0, 21, 21,  1,  0,  1,  0 },    // TP_STD_CMD_SCAN_STYLUS_DATA_BASE_RO,
	{ 0, 21, 22,  1,  1,  0,  1 },    // TP_STD_CMD_SCAN_SCAN_ENABLE_SELECT,
	{ 0, 21, 23,  1,  1,  0,  1 },    // TP_STD_CMD_SCAN_SCAN_FREQ_TEST_EN,
	{ 0, 21, 24,  1,  1,  0,  2 },    // TP_STD_CMD_SCAN_SET_SCAN_FREQ,
	{ 0, 22,  0,  1,  0,  1,  0 },    // TP_STD_CMD_NOISE_READ_RO,
	{ 0, 22,  1,  1,  1,  0,  2 },    // TP_STD_CMD_NOISE_HOP_NO_TOUCH_UP_TM_RW,
	{ 0, 22,  2,  1,  1,  0,  1 },    // TP_STD_CMD_NOISE_FREQ_HOP_MODE_RW,
	{ 0, 22,  3,  1,  1,  0,  1 },    // TP_STD_CMD_NOISE_FREQ_HOP_INDEX_RW,
	{ 0, 22,  4,  1,  1,  0,  1 },    // TP_STD_CMD_NOISE_FREQ_HOP_TH_RW,
	{ 0, 22,  5,  1,  0,  0,  0 },    // TP_STD_CMD_FFT_NOISE_VAL_RO,
	{ 0, 22,  6,  1,  0,  0,  1 },    // TP_STD_CMD_NUM_HOP_FREQ_RO,
	{ 0, 22,  7,  1,  1,  0,  0 },    // TP_STD_CMD_HOP_FREQ_ClC_NUM_RW,
	{ 0, 22,  8,  1,  1,  0,  1 },    // TP_STD_CMD_NUM_STYLUS_HOP_FREQ_RO,
	{ 0, 22,  9,  1,  1,  0,  0 },    // TP_STD_CMD_STYLUS_HOP_FREQ_ClC_NUM_RO,
	{ 0, 22, 10,  0,  1,  0,  1 },    // TP_STD_CMD_FORCE_LOCK_HOP_FREQ_WO,
	{ 0, 22, 11,  1,  1,  0,  1 },    // TP_STD_CMD_FORCE_LOCK_FREQ_EN_RW,
	{ 0, 22, 12,  1,  1,  0,  4 },    // TP_STD_CMD_FORCE_STYLUS_HOP_FREQ_RW,
	{ 0, 22, 13,  1,  1,  0,  1 },    // TP_STD_CMD_HOP_FREQ_HANDL_EN_RW,
	{ 0, 23,  0,  1,  0,  1,  0 },    // TP_STD_CMD_GSTR_READ_RO,
	{ 0, 23,  1,  1,  1,  0,  2 },    // TP_STD_CMD_GSTR_ENTER_GSTR_FS_RW,
	{ 0, 23,  2,  1,  1,  0,  1 },    // TP_STD_CMD_GSTR_EXIT_GSTR_RW,
	{ 0, 23,  3,  1,  1,  0,  1 },    // TP_STD_CMD_GSTR_ENTER_GSTR_LP_RW,
	{ 0, 23,  4,  1,  1,  0, 23 },    // TP_STD_CMD_GSTR_ALGO_RW,
	{ 0, 23,  5,  1,  1,  0,  2 },    // TP_STD_CMD_GSTR_ENABLE_TYPE_RW,
	{ 0, 23,  6,  1,  1,  0,  2 },    // TP_STD_CMD_GSTR_DISABLE_TYPE_RW,
	{ 0, 23,  7,  1,  1,  0,  2 },    // TP_STD_CMD_GSTR_ClEAR_STATUS_RW,
	{ 0, 24,  0,  1,  0,  1,  0 },    // TP_STD_CMD_LOW_POWER_READ_RO,
	{ 0, 24,  1,  1,  1,  0,  1 },    // TP_STD_CMD_LOW_POWER_ENTER_MONITOR_TM_RW,
	{ 0, 24,  2,  1,  1,  0,  1 },    // TP_STD_CMD_LOW_POWER_MONITOR_WAKEUP_CNT_RW,
	{ 0, 24,  3,  1,  1,  0,  1 },    // TP_STD_CMD_LOW_POWER_IDLE_MODE_EN_RW,
	{ 0, 24,  4,  1,  1,  0,  1 },    // TP_STD_CMD_LOW_POWER_ENTER_IDLE_MODE_RW,
	{ 0, 24,  5,  1,  1,  0,  2 },    // TP_STD_CMD_LOW_POWER_IDLE_REPORT_INTERVAL_RW,
	{ 0, 24,  6,  1,  1,  0,  1 },    // TP_STD_CMD_LOW_POWER_EXIT_IDLE_MODE_RW,
	{ 0, 24,  7,  1,  1,  0,  2 },    // TP_STD_CMD_LOW_POWER_IDLE_TOUCH_THRESHOLD_RW,
	{ 0, 24,  8,  1,  1,  0,  2 },    // TP_STD_CMD_LOW_POWER_IDLE_SCAN_RATE_RW,
	{ 0, 24,  9,  1,  1,  0,  1 },    // TP_STD_CMD_LOW_POWER_RESET_IDLE_BASE_EN_RW,
	{ 0, 25,  0,  1,  0,  1,  0 },    // TP_STD_CMD_STYLUS_READ_RO,
	{ 0, 26,  0,  1,  0,  1,  0 },    // TP_STD_CMD_ICTEST_READ_RO,
	{ 0, 26,  1,  1,  1,  0,  4 },    // TP_STD_CMD_ICTEST_FLASH_CP_EN_RW,
	{ 0, 26,  2,  1,  1,  0,  1 },    // TP_STD_CMD_ICTEST_HOP_FREQ_EN_RW,
	{ 0, 26,  3,  1,  1,  0,  1 },    // TP_STD_CMD_ICTEST_SHORTTEST_EN_RW,
	{ 0, 26,  4,  1,  1,  0, 126 },    // TP_STD_CMD_ICTEST_SHORTTEST_RES_VAL_RO,
	{ 0, 26,  5,  1,  1,  0,  1 },    // TP_STD_CMD_ICTEST_OPENTEST_EN_RW,
	{ 0, 26,  6,  1,  1,  0,  1 },    // TP_STD_CMD_ICTEST_SELF_COMP_RW,
	{ 0, 26,  7,  1,  1,  0,  5 },    // TP_STD_CMD_ICTEST_FW_DATA_PROCESS_EN,
	{ 0, 26,  8,  1,  1,  0, 128 },    // TP_STD_CMD_ICTEST_SHORT_CHANNEL_TO_GND,
	{ 0, 26,  9,  1,  1,  0, 128 },    // TP_STD_CMD_ICTEST_MUTUAL_SHORT_CHANNEL,
	{ 0, 26, 10,  1,  1,  0,  1 },    // TP_STD_CMD_ICTEST_HSYNC_TEST_EN,
	{ 0, 26, 11,  1,  1,  0,  2 },    // TP_STD_CMD_ICTEST_HSYNC_RESULT,
	{ 0, 26, 12,  1,  1,  0,  2 },    // TP_STD_CMD_ICTEST_SHORT_THRESHOLD,
	{ 0, 26, 13,  1,  1,  0,  2 },    // TP_STD_CMD_ICTEST_REAL_HSYNC_FREQ,
	{ 0, 26, 14,  1,  1,  0,  2 },    // TP_STD_CMD_ICTEST_OSC_TRIM_VALUE,
	{ 0, 26, 15,  1,  1,  0,  2 },    // TP_STD_CMD_ICTEST_OSC_TRIM_VALUE_FINE,
	{ 0, 26, 16,  1,  1,  0,  1 },    // TP_STD_CMD_ICTEST_OSC_SHIFT_TRIM_TRIGGER_RATIO,
	{ 0, 26, 17,  1,  1,  0,  1 },    // TP_STD_CMD_ICTEST_AP_CLK_TRIM_OSC,
	{ 0, 26, 18,  1,  1,  0,  1 },    // TP_STD_CMD_ICTEST_OSC_SHIFT_TRIM_MAX_RATIO,
    { 0, 26, 19,  1,  1,  0,  1 },      //TP_STD_CMD_ICTEST_VSYNC_TEST_EN,
    { 0, 26, 20,  1,  0,  0,  1 },      //TP_STD_CMD_ICTEST_VSYNC_PASS_FLAG,
    { 0, 26, 21,  1,  1,  0,  1 },      //TP_STD_CMD_ICTEST_MUTUAL_RAWDATA_PREPRO_EN,
    { 0, 26, 24,  1,  1,  0,  1 },      //TP_STD_CMD_ICTEST_SELF_DCAP_ADJUSTEN,
};



#define BIT(x)				(1 << x)

enum int_data_type {
	INT_DATA_TYPE_NONE					= 0,
	INT_DATA_TYPE_MUTUAL_RAWDATA 		= 1,
	INT_DATA_TYPE_MUTUAL_BASEDATA		= 2,
	INT_DATA_TYPE_MUTUAL_DIFFDATA		= 3,
	INT_DATA_TYPE_LINE_RAWDATA			= 4,
	INT_DATA_TYPE_LINE_BASEDATA			= 5,
	INT_DATA_TYPE_LINE_DIFFDATA			= 6,
	INT_DATA_TYPE_STYLUS_F1_I_RAWDATA	= 7,
	INT_DATA_TYPE_STYLUS_F1_Q_RAWDATA	= 8,
	INT_DATA_TYPE_STYLUS_F2_I_RAWDATA	= 9,
	INT_DATA_TYPE_STYLUS_F2_Q_RAWDATA	= 10,
	INT_DATA_TYPE_STYLUS_F1_DIFFDATA	= 11,
	INT_DATA_TYPE_STYLUS_F2_DIFFDATA	= 12,
	INT_DATA_TYPE_STYLUS_F1_BASEDATA	= 13,
	INT_DATA_TYPE_STYLUS_F2_BASEDATA	= 14,
	INT_DATA_TYPE_MONITOR_RAWDATA		= 15,
	INT_DATA_TYPE_MONITOR_DIFFDATA		= 16,
	INT_DATA_TYPE_MONITOR_BASEDATA		= 17,
	INT_DATA_TYPE_NOISE_SENSE_DATA		= 18,
	INT_DATA_TYPE_RAWDATA				= 19,
	INT_DATA_TYPE_DIFFDATA				= 20,
	INT_DATA_TYPE_THP_DATA				= 21,
	INT_DATA_TYPE_MASK					= 0x3F,
};

enum int_data_method {
    INT_DATA_METHOD_NONE	= 0,
    INT_DATA_METHOD_HOST	= 1,
    INT_DATA_METHOD_POLLING	= 2,
    INT_DATA_METHOD_DEBUG	= 3,
    INT_DATA_METHOD_CNT		= 4,
};

enum line_data_type {
	LINE_DATA_NONE		= 0,
	LINE_DATA_MAX		= 1,
	LINE_DATA_MIN		= 2,
};

/* raw touch info without data */
#define TOUCH_INFO_SIZ                      (112)
/* tcs reply tail: (errcode + cmd + crc) */
#define TCS_REPLY_TAIL_SIZ                  (sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t))

#define INT_DATA_VALID_SIZ					(62)
#define INT_DATA_INFO_SIZ                   (64)
#define INT_DATA_TYPE_LEN_SIZ               (4)

#if PATTERN_TYPE_1
#define FRAME_GRID_DATA_NODES				(ROWS_PATTERN * COLS_PATTERN)
#else
#define FRAME_GRID_DATA_NODES				(ROWS * COLS)
#endif
#define FRAME_GRID_DATA_SIZE \
	(FRAME_GRID_DATA_NODES * sizeof(uint16_t))
#define FRAME_NOISE_DATA_SIZE \
	(FRAME_GRID_DATA_NODES * sizeof(uint16_t))


#define FRAME_LINE_DATA_NODES				(ROWS + COLS)
#define FRAME_LINE_DATA_SIZE \
	(FRAME_LINE_DATA_NODES * sizeof(uint16_t))
#define FRAME_LINE_NOISE_DATA_SIZE \
	(FRAME_LINE_DATA_NODES * sizeof(uint16_t))

#if IC_TYPE_ICNT92X8
#define FRAME_SHORT_DATA_NODES				(126)
#define FRAME_SHORT_DATA_SIZE \
	(FRAME_SHORT_DATA_NODES * sizeof(uint16_t))
#define TEST_SHORT_DATA_SIZE				(128)

#elif IC_TYPE_ICNT9268S
#define FRAME_SHORT_DATA_NODES				(63)
#define FRAME_SHORT_DATA_SIZE \
	(FRAME_SHORT_DATA_NODES * sizeof(uint16_t))
#define TEST_SHORT_DATA_SIZE				(65)

#elif IC_TYPE_ICNT93XX
#define FRAME_SHORT_DATA_NODES				(50 + 85)
#define FRAME_SHORT_DATA_SIZE \
	(FRAME_SHORT_DATA_NODES * sizeof(uint16_t))
#define TEST_SHORT_DATA_SIZE				(50 + 85 + 2)

#endif

int cts_tcs_read_buff(enum TcsCmdIndex cmdIdx, uint8_t *rdata, size_t rdatalen);
int cts_tcs_write_buff(enum TcsCmdIndex cmdIdx, uint8_t *wdata, size_t wdatalen);
int cts_tcs_read_attr(enum TcsCmdIndex cmdIdx, uint8_t *rdata, size_t rdatalen);
int cts_tcs_write_attr(enum TcsCmdIndex cmdIdx, uint8_t *wdata, size_t wdatalen);
int cts_tcs_read_u8attr(enum TcsCmdIndex cmdIdx, uint8_t *u8attr);
int cts_tcs_read_u16attr(enum TcsCmdIndex cmdIdx, uint16_t *u16attr);
int cts_tcs_read_u32attr(enum TcsCmdIndex cmdIdx, uint32_t *u32attr);
int cts_tcs_write_u8attr(enum TcsCmdIndex cmdIdx, uint8_t u8attr);
int cts_tcs_write_u16attr(enum TcsCmdIndex cmdIdx, uint16_t u16attr);
int cts_tcs_write_u32attr(enum TcsCmdIndex cmdIdx, uint32_t u32attr);


int cts_tcs_get_fw_ver(uint32_t *fw_ver);
int cts_tcs_get_hw_id(uint32_t *hwid);

int cts_tcs_get_mst_tx_num(uint8_t *tx_num);
int cts_tcs_get_mst_rx_num(uint8_t *rx_num);
int cts_tcs_get_slv_tx_num(uint8_t *tx_num);
int cts_tcs_get_slv_rx_num(uint8_t *rx_num);
int cts_tcs_get_tx_tr_order(uint8_t *order);
int cts_tcs_get_rx_tr_order(uint8_t *order);

int cts_tcs_get_rows_cols(uint8_t *rx_num, uint8_t *tx_num);
int cts_tcs_get_pattern_type(uint8_t *type);
int cts_tcs_get_scan_freqs(uint8_t *scan_freq_num, uint16_t *scan_freqs);
int cts_tcs_get_stylus_scan_freqs(uint8_t *stylus_scan_freq_num, uint16_t *stylus_scan_freqs);
int cts_tcs_get_scan_rate(uint8_t *num_scan_rate, uint16_t *scan_rate);

int cts_tcs_set_hw_cap_ready(uint8_t value);

int cts_tcs_enable_freq(void);
int cts_tcs_disable_freq(void);
int cts_tcs_start_calibration(void);
int cts_tcs_set_freq_points(uint8_t index);
int cts_tcs_set_scan_rate(uint16_t rate);
int cts_tcs_set_scan_state(THP_AFE_SCAN_STATE_ENUM state);

int cts_tcs_stylus_freq_immediately(void);
int cts_tcs_stylus_freq_next_uplink(void);
int cts_tcs_enable_stylus(void);
int cts_tcs_disable_stylus(void);
int cts_tcs_set_afe_suspend(void);
int cts_tcs_set_afe_resume(void);

int cts_tcs_enable_wakeup_gesture(uint16_t gesture);
int cts_tcs_disable_wakeup_gesture(uint16_t gesture);
int cts_tcs_clear_gesture_status(uint16_t gesture);

int cts_tcs_clr_afe_status(uint32_t status);
int cts_tcs_clr_stylus_status(uint16_t status);

int cts_tcs_test_enable_freq(void);

int cts_tcs_set_mnt_touch_threshold(uint16_t threshold);
int cts_tcs_set_mnt_baseline_update_interval(uint16_t interval);
int cts_tcs_reset_idle_baseline(void);
int cts_tcs_force_enter_mnt(void);
int cts_tcs_force_exit_mnt(void);

int cts_tcs_get_int_data_types(uint16_t *int_data_types);
int cts_tcs_set_int_data_types(uint16_t int_data_types);
int cts_tcs_get_int_data_method(uint8_t *int_data_method);
int cts_tcs_set_int_data_method(uint8_t int_data_method);
int cts_tcs_set_line_data_type(uint8_t line_data_type);
int cts_tcs_get_self_dcap_adj_enable(uint8_t *enable);
int cts_tcs_set_self_dcap_adj_enable(uint8_t enable);
int cts_tcs_get_data_ready_flag(uint8_t *ready);
int cts_tcs_clr_data_ready_flag(void);
int cts_tcs_polling_rawdata(uint8_t *buf, size_t size);
int cts_tcs_polling_shortdata(uint8_t *buf, size_t size);
int cts_tcs_polling_hsyncdata(uint8_t *buf, size_t size);
int cts_get_hsyncdata(void);

int cts_tcs_get_data_capture_support(uint8_t *support);

int cts_tcs_get_work_mode(uint8_t *work_mode);
int cts_tcs_set_work_mode(uint8_t work_mode);

int cts_tcs_set_short_thresh(uint16_t thresh);

#ifdef CTS_FOR_FFT_MODE
int cts_tcs_set_fft_mode(uint8_t fft_mode);
int cts_tcs_get_fft_data_ready_flag(uint8_t *ready);
int cts_tcs_clr_fft_data_ready_flag(void);
#endif
int cts_tcs_get_enable_short_test(uint8_t *enable);
int cts_tcs_set_enable_short_test(uint8_t enable);

int cts_tcs_get_enable_open_test(uint8_t *enable);
int cts_tcs_set_enable_open_test(uint8_t enable);
 
int cts_tcs_set_enable_hsync_test(uint8_t enable);
int cts_tcs_get_real_hsync(uint16_t *hsync);
int cts_tcs_set_real_hsync(uint16_t hsync);
int cts_tcs_get_osc_trim_trigger_ratio(uint8_t *value);
int cts_tcs_set_osc_trim_trigger_ratio(uint8_t value);
int cts_tcs_set_osc_trim_max_ratio(uint16_t value);
int cts_tcs_get_osc_trim_max_ratio(uint16_t *value);
int cts_tcs_get_osc_trim(uint16_t *value);
int cts_tcs_get_osc_trim_fine(uint16_t *value);

int cts_tcs_get_scan_mode(uint8_t *scan_mode);
int cts_tcs_set_scan_mode(uint8_t scan_mode);

int cts_tcs_get_cneg(uint8_t *buf, size_t size);

int cts_test_polling_rawdata(uint16_t *buf, size_t size);
int cts_test_polling_shortdata(uint16_t *buf, size_t size);
int cts_test_polling_hsyncdata(uint8_t *buf, size_t size);

int cts_tcs_update_base_to_flash(void);

int cts_tcs_get_scan_enable_select(uint8_t *value);
int cts_tcs_set_scan_enable_select(uint8_t value);
int cts_tcs_get_scan_freq_test_en(uint8_t *enable);
int cts_tcs_set_scan_freq_test_en(uint8_t enable);
int cts_tcs_get_scan_freq(uint16_t *value);
int cts_tcs_set_scan_freq(uint16_t value);

int cts_tcs_set_charger(uint8_t enable);

int cts_tcs_get_compen_enable(uint8_t *enable);
int cts_tcs_set_compen_enable(uint8_t enable);

int cts_tcs_get_shb_enable(uint8_t *enable);
int cts_tcs_set_shb_enable(uint8_t enable);
#ifdef CTS_FOR_FFT_MODE
int cts_test_polling_fftdata(uint16_t *buf, size_t size);
int cts_tcs_polling_fftdata(uint8_t *buf, size_t size);
#endif

int cts_tcs_set_cur_clock(uint8_t clock);
int cts_tcs_get_cur_clock(uint8_t *clock);

extern void cts_dump_spi_tx_rx(uint8_t *tx, uint8_t *rx, size_t tx_len, size_t rx_len);

int cts_tcs_read_pack(uint8_t *tx, enum TcsCmdIndex index, uint16_t rdatalen);
int cts_tcs_write_pack(uint8_t *tx, enum TcsCmdIndex index, uint8_t *wdata, uint16_t wdatalen);
int cts_tcs_spi_xing(uint8_t *tx, uint8_t *rx, size_t total_len);
#endif /* CTS_TCS_H */
