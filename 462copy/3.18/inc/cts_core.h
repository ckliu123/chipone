#ifndef CTS_CORE_H
#define CTS_CORE_H

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include "thp_afe_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PROJECT_ID				"W462CF1300"

#define CTS_DEBUG
#ifdef CTS_DEBUG
/*** DEBUG PART ***/
#define DEBUG_SOCKET_TOOL
// #define DEBUG_DUMP_IOCTL_SPI_FULL_DATA
// #define DISABLE_IDLE
#define  FIRMWARE_UPDATE_SDCARD
// #define  FIRMWARE_SDCARD_DUAL_PATH
#endif

//donnot change
#define DECREASE_MNT_CMD
#define VENDOR_NAME                "chipone"
#define PRODUCT_NAME               "icnl9952"

//yjl  version
#define AFE_VERSION                "1.0.07_dbg3"//last   "1.0.9_0114"   20230926

#define ROWS                       40
#define COLS                       60
#define ROWS_STYLUS_TIED           8
#define COLS_STYLUS_TIED           12
#define MAX_NUM_SCAN_FREQ          5
#define MAX_NUM_SCAN_RATE          5

//add yjl
#define USED_NUM_SCAN_FREQ         3
#define USED_NUM_SCAN_RATE         2
#define FS_RAW_DEST_VALUE          2500

//#define  F_FREQ1 100
//#define  F_FREQ2 125
//#define  F_FREQ3 148
#define  F_FREQ1					97
#define  F_FREQ2					119
#define  F_FREQ3					83


#define  F_SCAN_RATE1  120
#define  F_SCAN_RATE2  60

//#define MNT_EXIT_THD_DEFINED_BY_FW
#define DAEMON_GET_DATA_TIME_OUT_ACTIVE    100
#define DAEMON_GET_DATA_TIME_OUT_IDLE      1500

#define GET_FRAME_TIMEOUT_MS               4500
#define CURRENT_FRAME_MIN_SIZ           11

typedef enum
{
    FRAME_TYPE_NONE                     = 0x00,
    FRAME_TYPE_FINGER_0                 = 0x01,
    FRAME_TYPE_FINGER_1                 = 0x02,
    FRAME_TYPE_STYLUS_0MERGE1           = 0x04,
    FRAME_TYPE_STYLUS_2MERGE3           = 0x08,
    FRAME_TYPE_STYLUS_0               = 0x10,
    FRAME_TYPE_STYLUS_1               = 0x20,
    FRAME_TYPE_STYLUS_2               = 0x40,
    FRAME_TYPE_STYLUS_3               = 0x80,
} CTS_FRAME_TYPE_ENUM;

extern uint16_t  current_afe_data_type;
extern uint16_t  last_Frame_index;
extern uint16_t  last_afe_status;
extern uint16_t  last_stylus_status;
extern uint16_t  last_finger_freq;
extern uint16_t  g_version;

#define SCREEN_ON_FLAG   0
#define SCREEN_OFF_FLAG  1
extern int nonblock; //for screenon/screenoff
extern void cts_screen_on_off_setflag(int flag);
extern void cts_normal_mode_init(void);

#pragma pack(push, 1)
typedef struct
{
    uint8_t  addr;
    uint16_t cmd;
    uint16_t datlen;
    uint16_t crc16;
} tcs_tx_head;

typedef struct
{
    uint8_t  ecode;
    uint16_t cmd;
    uint16_t crc16;
} tcs_rx_tail;

#define POS_BUF_LEN  112               // for DP416
#define TX_CHANNELS 40
#define RX_CHANNELS 64
// #define FINGER_MODE_DUMMY_LENGTH   320 // for DP416
#define FINGER_MODE_DUMMY_LENGTH   (((TX_CHANNELS*RX_CHANNELS) - (ROWS * COLS))*sizeof(uint16_t))     //2025/01/16 modified by mbteng
#define CTS_FW_DUMP_INFO
#ifdef CTS_FW_DUMP_INFO
#define CTS_ID_NUM 10
#define FRAME_NUM_TO_PRINT 1//100 2024/12/04 modified by mbteng
typedef struct
{
    uint8_t dbg_cnt;
    uint8_t job_id;
    uint16_t ddi_line_num;
    uint8_t ddi_vsync_cnt;
    uint8_t ddi_tps_cnt;
} CTS_ID_INFO_STRUCT;

typedef struct
{
    CTS_ID_INFO_STRUCT      mstrID0;
    CTS_ID_INFO_STRUCT      mstrID1;
    CTS_ID_INFO_STRUCT      mstrID2;
    CTS_ID_INFO_STRUCT      mstrID3;
    CTS_ID_INFO_STRUCT      mstrID4;
    CTS_ID_INFO_STRUCT      slaveID0;
    CTS_ID_INFO_STRUCT      slaveID1;
    CTS_ID_INFO_STRUCT      slaveID2;
    CTS_ID_INFO_STRUCT      slaveID3;
    CTS_ID_INFO_STRUCT      slaveID4;
    uint32_t                mstr_scan_go_err0_sts;
    uint32_t                mstr_scan_go_err1_sts;
    uint32_t                slave_scan_go_err0_sts;
    uint32_t                slave_scan_go_err1_sts;
    uint32_t                mstr_dmct_go_err0;
    uint32_t                mstr_dmct_go_err1;
    uint32_t                slave_dmct_go_err0;
    uint32_t                slave_dmct_go_err1;
    uint8_t                 ddi_r_0A;
    uint8_t                 ddi_fsm_state;
} CTS_FW_DUMP_INFO_STRUCT;
#endif
typedef struct
{
    uint8_t                        points[POS_BUF_LEN];
    uint8_t                        index;
    uint8_t                        version;
    uint32_t                       magic_number;
    uint16_t                       curr_frame_size;
    uint16_t                       next_frame_size;
    uint16_t                       frame_type;
    uint16_t                       frame_index;
    uint16_t                       finger_scan_freq;
    uint16_t                       finger_noise[5];
    uint16_t                       finger_scan_rate;
    uint16_t                       stylus_tx1_curr_scan_freq;
    uint16_t                       stylus_tx2_curr_scan_freq;
    uint16_t                       stylus_tx1_next_scan_freq;
    uint16_t                       stylus_tx2_next_scan_freq;
    uint16_t                       stylus_noise[4];
    uint16_t                       stylus_scan_rate;
    uint32_t                       afe_status;
    uint16_t                       gesture_status;
    uint16_t                       stylus_status;
    uint16_t                       fw_status;
    uint16_t                       krang_index;
    uint8_t                        krang_frm_num;
    uint8_t                        krang_frm_count;
    uint16_t                       debug_frame_type;
    uint8_t                        u8Res1;
    uint8_t                        u8Res2;
    uint16_t                       data_type;
    uint16_t                       data_len;
    uint16_t                       rawdata[ROWS*COLS]; // for DP416
    uint8_t                        dummyFinger[FINGER_MODE_DUMMY_LENGTH]; // for DP416
} CTS_FRAME_STRUCT;

typedef struct
{
    uint8_t                        points[POS_BUF_LEN];
    uint8_t                        index;
    uint8_t                        version;
    uint32_t                       magic_number;
    uint16_t                       curr_frame_size;
    uint16_t                       next_frame_size;
    uint16_t                       frame_type;
    uint16_t                       frame_index;
    uint16_t                       finger_scan_freq;
    uint16_t                       finger_noise[5];
    uint16_t                       finger_scan_rate;
    uint16_t                       stylus_tx1_curr_scan_freq;
    uint16_t                       stylus_tx2_curr_scan_freq;
    uint16_t                       stylus_tx1_next_scan_freq;
    uint16_t                       stylus_tx2_next_scan_freq;
    uint16_t                       stylus_noise[4];
    uint16_t                       stylus_scan_rate;
    uint32_t                       afe_status;
    uint16_t                       gesture_status;
    uint16_t                       stylus_status;
    uint16_t                       fw_status;
    uint16_t                       krang_index;
    uint8_t                        krang_frm_num;
    uint8_t                        krang_frm_count;
    uint16_t                       debug_frame_type;
    uint8_t                        u8Res1;
    uint8_t                        u8Res2;
    uint16_t                       data_type;
    uint16_t                       data_len;
    uint16_t                       stylusdata[ (ROWS*COLS_STYLUS_TIED + ROWS_STYLUS_TIED*COLS)*2 ];
    uint16_t                       rawdata[ROWS*COLS/4]; // for DP416
    uint8_t                        dummyFingreStylus[80];// for DP416
} CTS_FRAME_FINGER_STYLUS_STRUCT;

#define RAWDATA_NODES              ((ROWS + 2) * COLS)
#define RAWDATA_SIZE               (RAWDATA_NODES * sizeof(uint16_t))
#define FRAME_SIZE                 sizeof(CTS_FRAME_STRUCT)
#define FRAME_RAWDATA_INDEX        (FRAME_SIZE - (ROWS)*COLS*sizeof(uint16_t) - FINGER_MODE_DUMMY_LENGTH )
#define TCS_HEAD_LEN               (sizeof(tcs_tx_head))
#define TCS_TAIL_LEN               (sizeof(tcs_rx_tail))
#define FRAME_SIZE_HAS_TAIL        (FRAME_SIZE + TCS_TAIL_LEN)
/*
extern uint16_t num_col_used;
extern uint16_t num_row_used;
extern uint16_t num_col_tied_used;
extern uint16_t num_row_tied_used;
extern uint16_t s_Fs_raw_dest_value;
*/
#pragma pack(pop)

typedef struct
{
    struct timeval                 tv;
    size_t                         framelen;
    uint8_t                        frame[FRAME_SIZE_HAS_TAIL];
} CTS_IOCTL_FRAME_STRUCT;

//extern  int isActive;
/* match thp api */
int cts_open(void);
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
int cts_enter_idle(void);
int cts_exit_idle(void);

//int cts_exit_idle_set_timeout(uint16_t timeout);
int cts_clear_status(THP_AFE_STATUS_ENUM status);
//yjl
int cts_tcs_clear_status(THP_AFE_STATUS_ENUM status);
int cts_tcs_Calib_update(void);

int cts_screen_off(void);
int cts_screen_on(void);
int cts_force_to_freq_point(uint8_t index);
//yjl
int cts_freq_shift_switch(uint8_t enable);
int cts_tcs_freq_shift_switch(uint8_t enable);

/* internal */
void cts_reset_device(void);

int wait_to_norm(uint16_t retryMax, uint16_t  time1msDelay);
int wait_to_curr_mode(uint16_t retryMax, uint16_t  time1msDelay);
int wait_to_cfg_mode(void);
int cts_force_get_hw_cap(void);

bool cts_afe_is_esd_triggered(void);
#ifdef __cplusplus
}
#endif

#endif /* CTS_CORE_H */

