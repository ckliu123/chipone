#ifndef CTS_TCS_H
#define CTS_TCS_H

#include <string.h>
#include <errno.h>

#define TCS_RD_ADDR                        (0xF1)
#define TCS_WR_ADDR                        (0xF0)

#define CTS_FIRMWARE_WORK_MODE_NORM        (0x00)
#define CTS_FIRMWARE_WORK_MODE_TEST        (0x03)
#define CTS_FIRMWARE_WORK_MODE_OPEN_SHORT  (0x05)
#define CTS_FIRMWARE_WORK_MODE_CFG         (0x06)

/*
#if ((IC_PACKAGE == ICNL9916) || (IC_PACKAGE == ICNL9916C) || (IC_PACKAGE == ICNL9922C))
    #define NORMAL_MODE                          0U
    #define CFG_MODE                             1U
    #define OPEN_SHORT_DET_MODE                  2U
    #define DEF_MODE                             0x0FU
#elif ((IC_PACKAGE == ICNL9951R) || (IC_PACKAGE == ICNL9952) || (IC_PACKAGE == ICNL9971))
    #define NORMAL_MODE                         0U
    #define STYLUS_DBG_MODE                     1U
    #define FINGER_DBG_MODE                     2U
    #define MNT_DBG_MODE                        3U
    #define GSTR_DBG_MODE                       4U
    #define OPEN_SHORT_DET_MODE                 5U
    #define CFG_MODE                            6U
    #define TEST_MODE                           7U
    #define IDLE_MODE                           8U
    #define DEF_MODE                            0xFU
#endif
*/

#define CTS_TEST_SHORT                     (0x01)
#define CTS_TEST_OPEN                      (0x02)

#define CTS_SHORT_TEST_UNDEFINED           (0x00)
#define CTS_SHORT_TEST_BETWEEN_COLS        (0x01)
#define CTS_SHORT_TEST_BETWEEN_ROWS        (0x02)
#define CTS_SHORT_TEST_BETWEEN_GND         (0x03)

typedef struct
{
    uint8_t BoMstrSwFlag;  //b_SW_FLAG
    uint8_t BoMstrKrangEn; //SCAN_KRANG_EN
    uint8_t    u8MstrDdiR0A;  //Master DDI_R_0A
    uint8_t    u8SlvDdiR0A;   //Slave DDI_R_0A, None
    uint32_t  u32MstrGoErr0; //SCAN_GO_ERR0_STS
    uint32_t  u32MstrGoErr1; //SCAN_GO_ERR1_STS
    uint16_t   u16FwVer;      //FIRMWARE_VER
    uint8_t    u8MstrDdiState;//GetDdiStatus(), DDI_FSM_STATE
    uint8_t    u8DdiFrame;    //60/90/120
    uint8_t    u8Dbg0;        //WorkMode1
    uint8_t    u8Dbg1;        //WorkMode2
    uint8_t    u8Dbg2;        //LibScan
    uint8_t    u8Dbg3;        //SpiSlave

	uint8_t		u8MstrDdiRAC; //Master DDI_R_AC
	uint8_t		u8SlvDdiRAC; //Slave DDI_R_AC, None
	uint8_t		u8MstrDdiRD7; //Master DDI_R_D7
	uint8_t		u8SlvDdiRD7; //Slave DDI_R_D7, None
	uint8_t		u8MstrDdiR7A; //Master DDI_R_7A
	uint8_t		u8SlvDdiR7A; //Slave DDI_R_7A, None
	uint8_t		u8MstrDdiRD3; //Master DDI_R_D3
	uint8_t		u8SlvDdiRD3; //Slave DDI_R_D3, None
} SYS_STS_DBG;

#pragma pack(push,1)
typedef struct
{
    uint8_t baseFlag;
    uint8_t classID;
    uint8_t cmdID;
    uint8_t isRead;
    uint8_t isWrite;
    uint8_t isData;
} TcsCmdValue_t;

typedef struct
{
    uint8_t  baseTraceMntEn;
    uint8_t  reportRateDownRatio;
    uint8_t  CfbAdjMnt;
    uint8_t  CycleNumMnt;
    uint16_t MntTpTh;
    uint16_t EnterMntCnt;
    uint8_t  Vstim0LevelMnt;
    uint8_t  ShiftNumMnt;
    uint16_t MntToNormalCnt;
} MntOptions;
#pragma pack(pop)

enum TcsCmdIndex
{
    TP_STD_CMD_INFO_CHIP_FW_ID_RO,
    TP_STD_CMD_INFO_FW_VER_RO,
    TP_STD_CMD_INFO_TOUCH_XY_INFO_RO,
    TP_STD_CMD_INFO_PAD_PANEL_INFO_RO,
    TP_STD_CMD_INFO_MODULE_ID_RO,

    TP_STD_CMD_TP_DATA_OFFSET_AND_TYPE_CFG_RW,
    TP_STD_CMD_TP_DATA_READ_START_RO,
    TP_STD_CMD_TP_DATA_COORDINATES_RO,
    TP_STD_CMD_TP_DATA_RAW_RO,
    TP_STD_CMD_TP_DATA_DIFF_RO,
    TP_STD_CMD_TP_DATA_BASE_RO,
    TP_STD_CMD_TP_DATA_CNEG_RO,
    TP_STD_CMD_TP_DATA_WR_REG_RAM_SEQUENCE_WO,
    TP_STD_CMD_TP_DATA_WR_REG_RAM_BATCH_WO,
    TP_STD_CMD_TP_DATA_WR_DDI_REG_SEQUENCE_WO,
    TP_STD_CMD_GET_DATA_BY_POLLING_RO,

    TP_STD_CMD_SYS_STS_READ_RO,
    TP_STD_CMD_SYS_STS_WORK_MODE_RW,
    TP_STD_CMD_SYS_STS_DAT_RDY_FLAG_RW,
    TP_STD_CMD_SYS_STS_PWR_STATE_RW,
    TP_STD_CMD_SYS_STS_CHARGER_PLUGIN_RW,
    TP_STD_CMD_SYS_STS_DDI_CODE_VER_RO,
    TP_STD_CMD_SYS_STS_DAT_TRANS_IN_NORMAL_RW,
    TP_STD_CMD_SYS_STS_VSTIM_LVL_RW,
    TP_STD_CMD_SYS_STS_CNEG_RDY_FLAG_RW,
    TP_STD_CMD_SYS_STS_TP_REPORT_RATE_RO,
    TP_STD_CMD_SYS_STS_RESET_WO,
    TP_STD_CMD_SYS_STS_INT_TEST_EN_RW,
    TP_STD_CMD_SYS_STS_SET_INT_PIN_RW,
    TP_STD_CMD_SYS_STS_CNEG_RD_EN_RW,
    TP_STD_CMD_SYS_STS_INT_MODE_RW,
    TP_STD_CMD_SYS_STS_INT_KEEP_TIME_RW,
    TP_STD_CMD_SYS_STS_CURRENT_WORKMODE_RO,
    TP_STD_CMD_SYS_STS_DATA_CAPTURE_SUPPORT_RO,
    TP_STD_CMD_SYS_STS_DATA_CAPTURE_EN_RW,
    TP_STD_CMD_SYS_STS_DATA_CAPTURE_FUNC_MAP_RW,
    TP_STD_CMD_SYS_STS_PANEL_DIRECTION_RW,
    TP_STD_CMD_SYS_STS_KRANG_WORK_MODE_RW,
    TP_STD_CMD_SYS_STS_KRANG_CURRENT_WORKMODE_RO,
    TP_STD_CMD_SET_KRANG_STOP,
    TP_STD_CMD_SYS_STS_GAME_MODE_RW,
    TP_STD_CMD_SYS_STS_PRODUCTION_TEST_EN_RW,

    TP_STD_CMD_AFE_STATUS_CLEAR_WO,
    TP_STD_CMD_SYS_STS_KRANG_MODE_SW_RW,
    TP_STD_CMD_SYS_STS_SET_HPP_RW,
    TP_STD_CMD_SYS_STS_DBG,
    TP_STD_CMD_THP_SET_STY_DET_FREQ,

    TP_STD_CMD_GSTR_WAKEUP_EN_RW,
    TP_STD_CMD_GSTR_DAT_RDY_FLAG_GSTR_RW,
    TP_STD_CMD_GSTR_ENTER_MAP_RW,

    TP_STD_CMD_MNT_EN_RW,
    TP_STD_CMD_MNT_FORCE_ENTER_MNT_WO,
    TP_STD_CMD_MNT_FORCE_EXIT_MNT_WO,
    TP_STD_CMD_MNT_OPTIONS_MNT_RW,

    TP_STD_CMD_DDI_ESD_EN_RW,
    TP_STD_CMD_DDI_ESD_OPTIONS_RW,

    TP_STD_CMD_DIFF_ESD_EN_RW,


    TP_STD_CMD_CNEG_EN_RW,
    TP_STD_CMD_CNEG_OPTIONS_RW,

    TP_STD_CMD_COORD_FLIP_X_EN_RW,
    TP_STD_CMD_COORD_FLIP_Y_EN_RW,
    TP_STD_CMD_COORD_SWAP_AXES_EN_RW,

    TP_STD_CMD_NOI_SENSE_FREQ_RW,
    TP_STD_CMD_SET_NOISE_RW,

    TP_STD_CMD_FREQ_SHIFT_ENABLE_WO,
    TP_STD_CMD_FREQ_SHIFT_FORCE_WO,

    TP_STD_CMD_OPENSHORT_EN_RW,
    TP_STD_CMD_OPENSHORT_MODE_SEL_RW,
    TP_STD_CMD_OPENSHORT_SHORT_SEL_RW,
    TP_STD_CMD_OPENSHORT_SHORT_DISP_ON_EN_RW,

    TP_STD_CMD_DDI_DISP_FRM_RATE_RO,
};

#ifdef _CTS_TCS_C_
static TcsCmdValue_t TcsCmdValue[] =
{
    /*---------------------------------------------------
     *baseFlage, classID, cmdID, isRead, isWrite, isData
     *---------------------------------------------------
     */
    { 0, 0,  3, 1, 0, 0 },    /* TP_STD_CMD_INFO_CHIP_FW_ID_RO */
    { 0, 0,  5, 1, 0, 0 },    /* TP_STD_CMD_INFO_FW_VER_RO */
    { 0, 0,  7, 1, 0, 0 },    /* TP_STD_CMD_INFO_TOUCH_XY_INFO_RO */
    { 0, 0, 10, 1, 0, 0 },    /* TP_STD_CMD_INFO_PAD_PANEL_INFO_RO */
    { 0, 0, 17, 1, 0, 0 },    /* TP_STD_CMD_INFO_MODULE_ID_RO */

    { 0, 1,  1, 1, 1, 0 },    /* TP_STD_CMD_TP_DATA_OFFSET_AND_TYPE_CFG_RW */
    { 0, 1,  2, 1, 0, 1 },    /* TP_STD_CMD_TP_DATA_READ_START_RO */
    { 0, 1,  3, 1, 0, 1 },    /* TP_STD_CMD_TP_DATA_COORDINATES_RO */
    { 0, 1,  4, 1, 0, 1 },    /* TP_STD_CMD_TP_DATA_RAW_RO */
    { 0, 1,  5, 1, 0, 1 },    /* TP_STD_CMD_TP_DATA_DIFF_RO */
    { 0, 1,  6, 1, 0, 1 },    /* TP_STD_CMD_TP_DATA_BASE_RO */
    { 0, 1, 10, 1, 0, 1 },    /* TP_STD_CMD_TP_DATA_CNEG_RO */
    { 0, 1, 20, 0, 1, 1 },    /* TP_STD_CMD_TP_DATA_WR_REG_RAM_SEQUENCE_WO */
    { 0, 1, 21, 0, 1, 1 },    /* TP_STD_CMD_TP_DATA_WR_REG_RAM_BATCH_WO */
    { 0, 1, 22, 0, 1, 1 },    /* TP_STD_CMD_TP_DATA_WR_DDI_REG_SEQUENCE_WO */
    { 0, 1, 35, 1, 0, 0 },    /* TP_STD_CMD_GET_DATA_BY_POLLING_RO */

    { 0, 2,  0, 1, 0, 1 },    /* TP_STD_CMD_SYS_STS_READ_RO *//*CHECK!!*/
    { 0, 2,  1, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_WORK_MODE_RW */
    { 0, 2,  3, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_DAT_RDY_FLAG_RW */
    { 0, 2,  4, 1, 1, 1 },    /* TP_STD_CMD_SYS_STS_PWR_STATE_RW */
    { 0, 2,  5, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_CHARGER_PLUGIN_RW */
    { 0, 2,  6, 1, 0, 0 },    /* TP_STD_CMD_SYS_STS_DDI_CODE_VER_RO */
    { 0, 2,  7, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_DAT_TRANS_IN_NORMAL_RW */
    { 0, 2,  8, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_VSTIM_LVL_RW */
    { 0, 2, 17, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_CNEG_RDY_FLAG_RW */
    { 0, 2, 20, 1, 0, 0 },    /* TP_STD_CMD_SYS_STS_TP_REPORT_RATE_RO */
    { 0, 2, 22, 0, 1, 0 },    /* TP_STD_CMD_SYS_STS_RESET_WO */
    { 0, 2, 23, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_INT_TEST_EN_RW */
    { 0, 2, 24, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_SET_INT_PIN_RW */
    { 0, 2, 25, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_CNEG_RD_EN_RW */
    { 0, 2, 35, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_INT_MODE_RW */
    { 0, 2, 36, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_INT_KEEP_TIME_RW */
    { 0, 2, 51, 1, 0, 0 },    /* TP_STD_CMD_SYS_STS_CURRENT_WORKMODE_RO */
    { 0, 2, 63, 1, 0, 0 },    /* TP_STD_CMD_SYS_STS_DATA_CAPTURE_SUPPORT_RO */
    { 0, 2, 64, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_DATA_CAPTURE_EN_RW */
    { 0, 2, 65, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_DATA_CAPTURE_FUNC_MAP_RW */
    { 0, 2, 66, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_PANEL_DIRECTION_RW */
    { 0, 2, 67, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_KRANG_WORK_MODE_RW */
    { 0, 2, 68, 1, 0, 0 },    /* TP_STD_CMD_SYS_STS_KRANG_CURRENT_WORKMODE_RO */
    { 0, 2, 74, 0, 1, 0 },    /* TP_STD_CMD_SET_KRANG_STOP */
    { 0, 2, 78, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_GAME_MODE_RW */
    { 0, 2, 82, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_PRODUCTION_TEST_EN_RW */

    { 0, 2, 90, 0, 1, 0 },    /* TP_STD_CMD_AFE_STATUS_CLEAR_WO */
    { 0, 2, 93, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_KRANG_MODE_SW_RW */
    { 0, 2, 94, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_SET_HPP_RW */
    { 0, 2, 95, 1, 1, 0 },    /* TP_STD_CMD_SYS_STS_DBG*/
    { 0, 2, 101, 1, 1, 0 },    /* TP_STD_CMD_THP_SET_STY_DET_FREQ */

    { 0, 3,  1, 1, 1, 0 },    /* TP_STD_CMD_GSTR_WAKEUP_EN_RW */
    { 0, 3, 30, 1, 1, 0 },    /* TP_STD_CMD_GSTR_DAT_RDY_FLAG_GSTR_RW */
    { 0, 3, 40, 1, 1, 0 },    /* TP_STD_CMD_GSTR_ENTER_MAP_RW */


    /*---------------------------------------------------
      *baseFlage, classID, cmdID, isRead, isWrite, isData
      *---------------------------------------------------
      */

    { 0, 4,  1, 1, 1, 0 },    /* TP_STD_CMD_MNT_EN_RW */
    { 0, 4,  2, 0, 1, 0 },    /* TP_STD_CMD_MNT_FORCE_ENTER_MNT_WO */
    { 0, 4,  3, 0, 1, 0 },    /* TP_STD_CMD_MNT_FORCE_EXIT_MNT_WO */
    { 0, 4,  4, 1, 1, 0 },    /* TP_STD_CMD_MNT_OPTIONS_MNT_RW */

    { 0, 5,  1, 1, 1, 0 },    /* TP_STD_CMD_DDI_ESD_EN_RW */
    { 0, 5,  2, 1, 1, 0 },    /* TP_STD_CMD_DDI_ESD_OPTIONS_RW */

    { 0, 5,  5, 1, 1, 0 },    /* TP_STD_CMD_DIFF_ESD_EN_RW */


    { 0, 6,  1, 1, 1, 0 },    /* TP_STD_CMD_CNEG_EN_RW */
    { 0, 6,  2, 1, 1, 0 },    /* TP_STD_CMD_CNEG_OPTIONS_RW */

    { 0, 7,  2, 1, 1, 0 },    /* TP_STD_CMD_COORD_FLIP_X_EN_RW */
    { 0, 7,  3, 1, 1, 0 },    /* TP_STD_CMD_COORD_FLIP_Y_EN_RW */
    { 0, 7,  4, 1, 1, 0 },    /* TP_STD_CMD_COORD_SWAP_AXES_EN_RW */

    { 0, 10,  2, 1, 1, 0 },   /* TP_STD_CMD_NOI_SENSE_FREQ_RW */
    { 0, 10, 12, 1, 1, 0 },   /* TP_STD_CMD_SET_NOISE_RW */


    { 0, 10, 13, 0, 1, 1 },   /* TP_STD_CMD_FREQ_SHIFT_ENABLE_WO */
    { 0, 10, 14, 0, 1, 0 },   /* TP_STD_CMD_FREQ_SHIFT_FORCE_WO */


    { 0, 11, 1, 1, 1, 0 },    /* TP_STD_CMD_OPENSHORT_EN_RW */
    { 0, 11, 2, 1, 1, 0 },    /* TP_STD_CMD_OPENSHORT_MODE_SEL_RW */
    { 0, 11, 3, 1, 1, 0 },    /* TP_STD_CMD_OPENSHORT_SHORT_SEL_RW */
    { 0, 11, 4, 1, 1, 0 },    /* TP_STD_CMD_OPENSHORT_SHORT_DISP_ON_EN_RW */

    { 0, 12, 4, 1, 0, 0 },    /* TP_STD_CMD_DDI_DISP_FRM_RATE_RO */

    /*---------------------------------------------------
     *baseFlage, classID, cmdID, isRead, isWrite, isData
     *---------------------------------------------------
     */
};
#endif /* _CTS_TCS_C_ */

#define BIT(x)                (1 << x)
enum int_data_type
{
    INT_DATA_TYPE_NONE = 0,
    INT_DATA_TYPE_RAWDATA = BIT(0),
    INT_DATA_TYPE_MANUAL_DIFF = BIT(1),
    INT_DATA_TYPE_REAL_DIFF = BIT(2),
    INT_DATA_TYPE_NOISE_DIFF = BIT(3),
    INT_DATA_TYPE_BASEDATA = BIT(4),
    INT_DATA_TYPE_CNEGDATA = BIT(5),
    INT_DATA_TYPE_MASK = 0x3F,
};

enum int_data_method
{
    INT_DATA_METHOD_NONE = 0,
    INT_DATA_METHOD_HOST = 1,
    INT_DATA_METHOD_POLLING = 2,
    INT_DATA_METHOD_DEBUG = 3,
    INT_DATA_METHOD_CNT = 4,
};

int cts_tcs_read_buff(enum TcsCmdIndex cmdIdx, uint8_t *rdata, size_t rdatalen);
int cts_tcs_write_buff(enum TcsCmdIndex cmdIdx, uint8_t *wdata, size_t wdatalen);
int cts_tcs_read_hw_reg(uint32_t addr, uint8_t *data, size_t size);
int cts_tcs_write_hw_reg(uint32_t addr, uint8_t *wbuf, size_t len);

int cts_tcs_spi_xtrans_for_tool(uint8_t *tx_buf, int tx_len, uint8_t *rx_buf, int rx_len);
int cts_tcs_read_spi_for_tool(uint8_t classID, uint8_t cmdID, uint8_t *buf, size_t len);
int cts_tcs_write_spi_for_tool(uint8_t classID, uint8_t cmdID, uint8_t *buf, size_t len);

int cts_tcs_get_fw_ver(uint16_t *fw_ver);
int cts_tcs_get_res(uint16_t *res_x, uint16_t *res_y);
int cts_tcs_get_module_id(uint16_t *module_id);
int cts_tcs_get_rows_cols(uint8_t *rows, uint8_t *cols);
int cts_tcs_get_scan_freqs(uint8_t *scan_freq_num, uint16_t *scan_freqs);

int cts_tcs_get_debug_info();
int cts_tcs_get_scan_rate(uint16_t *scan_rate);
int cts_tcs_set_scan_freq(uint8_t freq);
int cts_tcs_set_scan_rate(uint8_t rate);
int cts_tcs_set_krang_stop(void);
int cts_tcs_set_product_en(void);

//add yjl
int cts_tcs_get_Normal_Fs_Raw_Dest_Value(uint16_t *NormalFsRawDestValue);

int cts_tcs_get_mnt_options(MntOptions *options);
int cts_tcs_set_mnt_options(MntOptions *options);
int cts_tcs_force_enter_mnt(void);
int cts_tcs_force_exit_mnt(void);
int cts_tcs_set_pwr_mode(uint8_t pwr_mode);

int cts_tcs_get_int_data_types(uint16_t *int_data_types);
int cts_tcs_set_int_data_types(uint16_t int_data_types);
int cts_tcs_get_int_data_method(uint8_t *int_data_method);
int cts_tcs_set_int_data_method(uint8_t int_data_method);
int cts_tcs_get_data_ready_flag(uint8_t *ready);
int cts_tcs_clr_data_ready_flag(void);
int cts_tcs_polling_rawdata(uint8_t *buf, size_t size);

int cts_tcs_get_curr_mode(uint8_t *curr_mode);
int cts_tcs_get_work_mode(uint8_t *work_mode);
int cts_tcs_set_work_mode(uint8_t work_mode);

//yjl add
int cts_tcs_disable_CalibratonCheck(void);
int cts_tcs_enable_CalibratonCheck(void);
int cts_tcs_disable_esd_ddi_check(void);
int cts_tcs_disable_esd_diff_check(void);
int cts_tcs_disable_mnt(void);
int cts_tcs_enable_mnt(void);
int cts_tcs_disable_cneg(void);
int cts_tcs_set_openshort_mode(uint8_t openshort_mode);
int cts_tcs_set_short_test_type(uint8_t short_test_type);

int cts_tcs_enable_get_cneg(void);
int cts_tcs_disable_get_cneg(void);
int cts_tcs_get_cneg_ready(uint8_t *ready);
int cts_tcs_get_cneg(uint8_t *buf, size_t size);
int cts_tcs_polling_cnegdata(uint8_t *buf, size_t size);
int cts_test_polling_rawdata(uint16_t *buf, size_t size);
int cts_test_polling_cnegdata(uint8_t *buf, size_t size);
int cts_tcs_polling_data(int type, uint8_t *buf, size_t size);

#endif /* CTS_TCS_H */
