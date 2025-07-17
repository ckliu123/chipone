/*
 * @file       thp_afe_hal.h
 *
 * @authors    caiweigang@huawei.com
 *
 * @par Description
 *   This is Touch Host Processing(THP) Analog Front End(AFE)
 *   Hardware Abstraction Layer(HAL) header file.
 *   All the touch IC suppliers should implement AFE driver based on it.
 *
 * Copyright (c) Huawei TechnoTHP_LOGIes Co., Ltd. 2016-2019. All rights reserved.
 */

#ifndef THP_AFE_HAL_H
#define THP_AFE_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>


#ifdef __cplusplus
extern "C" {
#endif

#define THP_AFE_HAL_SPEC_MAJOR_VERSION 1  // defines THP AFE HAL spec major version
#define THP_AFE_HAL_SPEC_MINOR_VERSION 23 // defines THP AFE HAL spec minor version
#define THP_AFE_HAL_SPEC_PATCH_VERSION 0  // defines THP AFE HAL spec patch version
#define THP_AFE_HAL_SPEC_VERSION               \
    (THP_AFE_HAL_SPEC_MAJOR_VERSION * 65536 +  \
        THP_AFE_HAL_SPEC_MINOR_VERSION * 256 + \
        THP_AFE_HAL_SPEC_PATCH_VERSION)

#define THP_AFE_INFO_LEN  32
#define TCS_CLEAR_STATUS_CALIBRATE_DONE_LEVEL    0
#define TCS_CLEAR_STATUS_FREQ_SHIFT_LEVEL        1
#define TCS_ENTER_IDLE_LEVEL                     2

#define MAX_TCS_CMD_NUM                          3
extern  uint8_t g_tcs_cmd[MAX_TCS_CMD_NUM];

typedef enum
{
    INSPECT_NORMAL_FLAG      = 0,
    INSPECT_TEST_INIT_FLAG   = 1,
} INSPECT_FLAG_TYPE_ENUM;

extern  uint8_t inspect_flag;                   //for captest flow
// Error code of AFE
typedef enum
{
    THP_AFE_OK = 0,
    THP_AFE_EINVAL,   /* invalid argument/parameter */
    THP_AFE_ENOMEM,   /* out of memory */
    THP_AFE_EIO,      /* driver/AFE error */
    THP_AFE_ESTATE,   /* AFE HAL state error */
    THP_AFE_ETIMEOUT, /* get frame timeout */
    THP_AFE_EDATA,    /* bad frame data */
    THP_AFE_EOTHER    /* all other errors */
} THP_AFE_ERR_ENUM;

/* Error code of AFE Inspection */
typedef enum
{
    THP_AFE_INSPECT_OK = 0,            /* OK */
    THP_AFE_INSPECT_ESPI = (1 << 0),   // 0:0x0001, SPI communication error
    THP_AFE_INSPECT_ERAW = (1 << 1),   // 1:0x0002, Raw data error
    THP_AFE_INSPECT_ENOISE = (1 << 2), // 2:0x0004, Noise error
    THP_AFE_INSPECT_EOPEN = (1 << 3),  // 3:0x0008, Sensor open error
    THP_AFE_INSPECT_ESHORT = (1 << 4), // 4:0x0010, Sensor short error
    THP_AFE_INSPECT_ERC = (1 << 5),    // 5:0x0020, Sensor RC error
    THP_AFE_INSPECT_EPIN = (1 << 6),   // 6:0x0040,
    /* Errors of TSVD/TSHD/TRCST/TRCRQ and other PINs
       when Report Rate Switching between 60 Hz and 120 Hz */
    THP_AFE_INSPECT_EOTHER = (1 << 7)  // 7:0x0080, All other errors
} THP_AFE_INSPECT_ERR_ENUM;

/* Info of feature */
typedef enum
{
    THP_AFE_FEATURE_NOT_SUPPORTED = 0, /* feature is not supported */
    THP_AFE_FEATURE_SUPPORTED,         /* Feature is supported but not automatically */
    THP_AFE_FEATURE_AUTO               /* Feature is supported automatically */
} THP_AFE_FEATURE_ENUM;

/* Sensor architecture */
typedef enum
{
    THP_AFE_SA_ONCELL = 1,    /* On cell sensor */
    THP_AFE_SA_HYBRID_INCELL, /* Hybrid in cell sensor */
    THP_AFE_SA_FULL_INCELL    /* Full in cell sensor */
} THP_AFE_SENSOR_ARCH_ENUM;

/* Sensor pattern */
typedef enum
{
    THP_AFE_SP_SSD = 1, /* SSD sensor pattern */
    THP_AFE_SP_DSD,     /* DSD sensor pattern */
    THP_AFE_SP_MH3,     /* MH3 sensor pattern */
    THP_AFE_SP_PE1,     /* PE1 sensor pattern */
    THP_AFE_SP_PE2,     /* PE2 sensor pattern */
    THP_AFE_SP_AIT      /* Full in cell self sensor pattern */
} THP_AFE_SENSOR_PATTERN_ENUM;

/* AFE status */
typedef enum
{
    THP_AFE_STATUS_NONE = 0,
    THP_AFE_STATUS_IDLE_MODE = (1 << 0),                // 0:0x000001 /* Indicate AFE is running in IDLE mode */
    THP_AFE_STATUS_ACTIVE_MODE = (1 << 1),              // 1:0x000002 /* Indicate AFE is running in Active mode */
    THP_AFE_STATUS_FREQ_SHIFT_DONE = (1 << 2),          // 2:0x000004 /* Indicate frequency shift has done */
    THP_AFE_STATUS_CALIBRATION_DONE = (1 << 3),         // 3:0x000008 /* Indicate calibration has done */
    THP_AFE_STATUS_GESTURE_DETECTED = (1 << 4),         // 4:0x000010 /* Indicate wakeup gesture is detected */
    THP_AFE_STATUS_ALL_FREQ_NOISY = (1 << 5),           // 5:0x000020 /* Indicate all scan frequencies are noisy */
    THP_AFE_STATUS_SOS = (1 << 6),                      // 6:0x000040
    // Indicate AFE HAL run into unknown state
    // and can't recover by itself,
    // need help from caller which will normally reset AFE HAL
    THP_AFE_STATUS_RECAL_REQUEST = (1 << 7),            // 7:0x000080
    /* Indicate AFE request host to do recalibration */
    THP_AFE_STATUS_IDLE_BASELINE_RESET_DONE = (1 << 8), // 8:0x000100 /* Indicate IDLE baseline has been reset */
    THP_AFE_STATUS_FREQ_SHIFT_ENABLED = (1 << 9),       // 9:0x000200 /* Indicate auto frequency shift is enabled */
    THP_AFE_STATUS_PROXIMITY = (1 << 10),               // 10:0x000400 /* Indicate AFE has entered proximity mode */
    THP_AFE_STATUS_DISPLAY_SYNC = (1 << 11),            // 11:0x000800 /* Indicate AFE has entered display sync mode */
    THP_AFE_STATUS_SUSPEND = (1 << 12),                 // 12:0x001000
    THP_AFE_STATUS_SCREEN_OFF_MODE = (1 << 13),     // 13:0x002000 /* Indicate AFE is running in Screen Off mode */
    THP_AFE_STATUS_FORCE_ABNORMAL = (1 << 14),      // 14:0x004000 /* Indicate force sensor is broken */
    THP_AFE_STATUS_SIDE_IDLE_MODE = (1 << 15),      // 15:0x008000 /* Indicate AFE is running in SideIdle mode */
    THP_AFE_STATUS_SIDE_ACTIVE_MODE = (1 << 16),    // 16:0x010000 /* Indicate AFE is running in SideActive mode */
    THP_AFE_STATUS_RAW_JUMP = (1 << 17),            // 17:0x020000
    /* Indicate grid data jump caused by
      change of AFE settings */
    THP_AFE_STATUS_RAW_UNSTABLE = (1 << 18),        // 18:0x040000
    /* Indicate grid data is unstable caused by
      change of AFE settings */
    THP_AFE_STATUS_SIDE_TOUCH_DETECTED = (1 << 19), // 19:0x080000 /* Indicate touch is detected in side region */
    THP_AFE_STATUS_CORE_TOUCH_DETECTED = (1 << 20), // 20:0x100000 /* Indicate touch is detected in core region */
} THP_AFE_STATUS_ENUM;

/* Wake up gesture */
typedef enum
{
    THP_AFE_GESTURE_NONE = 0,                             // No gesture */
    THP_AFE_GESTURE_SINGLE_CLICK = (1 << 0),              // 0:0x0001, Single Click Gesture
    THP_AFE_GESTURE_DOUBLE_CLICK = (1 << 1),              // 1:0x0002, Double Click Gesture
    THP_AFE_GESTURE_HOVER = (1 << 2),                     // 2:0x0004, Hover Gesture
    THP_AFE_GESTURE_SWIPE = (1 << 3),                     // 3:0x0008, Swipe Gesture
    THP_AFE_GESTURE_STYLUS_DETECT_INK_NO_BTN = (1 << 4),  // 4:0x0010
    /* Detect stylus on panel with ink Gesture
     (for HPP2.X)
     When TPIC detect stylus approaching panel,
     and decode the pressure coding signal,
     confirm ink flag (ink1-ink4 in HPP3.0 protocol spec) set,
     but button flag
     (a12,b12 in HPP3.0 protocol spec) not set. */
    THP_AFE_GESTURE_STYLUS_DETECT_INK_WITH_BTN = (1 << 5) // 5:0x0020,
            /* Detect stylus on panel with ink Gesture
             (for HPP2.X)
             When TPIC detect stylus approaching panel,
             and decode the pressure coding signal,
             confirm ink flag (ink1-ink4 in HPP3.0 protocol spec) set,
             and button flag (a12, b12 in HPP3.0 protocol spec) set. */
} THP_AFE_GESTURE_ENUM;

/* Hardware Noise Detection Capabilities */
typedef enum
{
    THP_AFE_NOISE_DETECT_NOT_SUPPORTED = 0, /* Hardware noise detection is not supported */
    THP_AFE_NOISE_DETECT_CURR_FREQ,         /* AFE has the capability to detect noise level of
                                               current scan frequency in each frame */
    THP_AFE_NOISE_DETECT_ALL_FREQ           /* AFE has the capability to detect noise level of
                                               all scan frequencies in each frame */
} THP_AFE_NOISE_DETECT_ENUM;

typedef enum
{
    THP_AFE_SCAN_STATE_WHOLE = 0,   /* Scan whole panel */
    THP_AFE_SCAN_STATE_MAJOR_ONLY,  /* Only scan major part panel */
    THP_AFE_SCAN_STATE_MINOR_ONLY,  /* Only scan minor part panel */
    THP_AFE_SCAN_STATE_NUM,
} THP_AFE_SCAN_STATE_ENUM;

/* AFE STYLUS STATUS */
typedef enum
{
    THP_AFE_STATUS_NO_STYLUS_PROTOCOL = 0,               /* Indicate AFE is in scan mode
                                                            without active stylus protocol */
    THP_AFE_STATUS_HPP2_X_PROTOCOL = (1 << 0),           // 0:0x0001,
    /* Indicate AFE is in scan mode with
       hpp 2.x protocol */
    THP_AFE_STATUS_HPP3_0_PROTOCOL = (1 << 1),           // 1:0x0002,
    /* Indicate AFE is in scan mode with
       hpp 3.0 protocol */
    THP_AFE_STATUS_STYLUS_DETECT_MODE = (1 << 2),        // 2:0x0004,
    /* Indicate AFE is in STYLUS DETECT mode
       In which mode 600HZ active stylus detection
       and 120HZ/60HZ finger detection.
       The AFE report only finger data! */
    THP_AFE_STATUS_STYLUS_CONFIRM_MODE = (1 << 3),       // 3:0x0008
    /* Indicate AFE is running in STYLUS CONFIRM mode.
       When in detect mode, AFE detect strong stylus signal,
       and then enter this mode to make sure
       AFE synchronize with active stylus pen.
       The AFE will report stylus data when confirm successful!
       Otherwise report finger data only or nothing!
       Depend on TPIC design. */
    THP_AFE_STATUS_STYLUS_ACTIVE_MODE = (1 << 4),        // 4:0x0010,
    /* Indicate AFE is running in STYLUS ACTIVE mode.
       This mode, AFE is always synchronize with active stylus pen.
       360HZ stylus coordinate detection and
       60HZ finger detection @HPP3.0.
       240HZ coordinate detection and 60HZ pressure and
       button detection @HPP2.X.
       The AFE report stylus data or finger data! */
    THP_AFE_STATUS_STYLUS_FREQ_SHIFT_DONE = (1 << 5),    // 5:0x0020,
    /* Indicate active stylus
       scan frequency shift has done. */
    THP_AFE_STATUS_STYLUS_FREQ_SHIFT_REQUEST = (1 << 6), // 6:0x0040
    /* Indicate TPIC want to shift frequency because
       TPIC detect current bandwidth is too noisy,
       and find some other frequency bandwidth is clear;
       TPIC will inform HOST the two new frequency by
       THP_AFE_STYLUS_FRAME_DATA_STRUCT
       (tx1_new_scan_freq and tx2_new_scan_freq);
       TPIC do not shift the frequency until HOST call the API
       (thp_afe_set_hpp3_0_detect_freq_next_uplink or
       thp_afe_set_hpp3_0_detect_freq_immediately) */
    THP_AFE_STATUS_STYLUS_ALL_FREQ_NOISY = (1 << 7),     /* 7:0x0080, Indicate all stylus scan frequencies are noisy. */
} THP_AFE_STYLUS_STATUS_ENUM;

/* AFE STYLUS PROTOCOL */
typedef enum
{
    THP_AFE_STYLUS_PROTOCOL_NONE = 0,          /* Indicate AFE do not support STYLUS protocol */
    THP_AFE_STYLUS_PROTOCOL_HPP2_X = (1 << 0), /* Indicate AFE can support hpp 2.x protocol */
    THP_AFE_STYTUS_PROTOCOL_HPP3_0 = (1 << 1)  /* Indicate AFE can support hpp 3.0 protocol */
} THP_AFE_STYLUS_PROTOCOL_ENUM;

/* AFE STYLUS button info */
typedef enum
{
    THP_AFE_STYLUS_BTN_NONE = 0,     /* Indicate AFE detect NO button press */
    THP_AFE_STYLUS_BTN_1 = (1 << 0), /* Indicate AFE detect button 1 press */
} THP_AFE_STYLUS_BTN_ENUM;

typedef enum
{
    THP_AFE_STYLUS_NORMAL_LINE_DATA = 0,  // Indicate AFE report stylus data in normal line data type
    THP_AFE_STYLUS_IQ_LINE_DATA,          // Indicate AFE report stylus data in IQ line data type
    THP_AFE_STYLUS_NORMAL_GRID_DATA,      // Indicate AFE report stylus data in normal grid data type
    THP_AFE_STYLUS_TIED_GRID_DATA,        // Indicate AFE report stylus data in tied grid data type
} THP_AFE_STYLUS_DATA_TYPE_ENUM;

/* Information about the AFE and AFE library */
typedef struct
{
    char vendor_name[THP_AFE_INFO_LEN];  /* Vendor name of Touch Panel and TPIC, separated with '/' */
    char product_name[THP_AFE_INFO_LEN]; /* Produce name */
    char version[THP_AFE_INFO_LEN];      /* AFE version */
} THP_AFE_INFO_STRUCT;

/* Capabilities of the AFE */
typedef struct
{
    uint16_t num_col;                               /* Number of sensors along column */
    uint16_t num_row;                               /* Number of sensors along row */
    uint8_t num_button;                             /* Number of buttons */
    uint8_t rx_direction;                           /* Direction of rx sensor:
                                                       0: rx is along column direction
                                                       1: rx is along row direction */
    uint8_t rx_channel;                             /* Number of rx channel supported by silicon
                                                       which could be scanned at the same time with
                                                       same noise background */
    uint8_t rx_slot_layout;                         /* Layout of rx slot;
                                                       rx slot indicates the number of scan to finish all rx sensor scan
                                                       which equal to numRxSensor/numRxChannel normally;
                                                       0: normal layout,
                                                       the rx sensors in the same rx slot is physically grouped;
                                                       1: interlace layout,
                                                       the rx sensors in the same rx slot is interlaced
                                                       instead of being physically grouped;
                                                       In case of numRxChannel >= numRxSensor,
                                                       the layout should be always normal; */
    uint16_t pitch_size_um;                         /* sensor pitch size in um */
    uint8_t num_scan_freq;                          /* Number of frequencies for hopping */
    uint16_t *scan_freq;                            /* Pointer to array of all the scan frequencies;
                                                       NOTE: scan_freq[0] should hold the default scan frequency
                                                       in normal working mode; */
    uint8_t num_scan_rate;                          /* Number of scan rate choices in active mode;
                                                       for example: 60Hz and 120Hz are both supported,
                                                       then the num_active_scan_rate is 2; */
    uint16_t *scan_rate;                            /* Pointer to array of all the scan rates;
                                                       NOTE: scan_rate[0] should hold the default scan rate
                                                       in normal working mode; */
    THP_AFE_NOISE_DETECT_ENUM feature_noise_detect; /* Info of hardware noise detection capability in each frame;
                                                      different types of hardware noise detection capability
                                                      requires different noise_data in THP_AFE_FRAME_DATA_STRUCT */
    THP_AFE_FEATURE_ENUM feature_freq_hop;          /* Info of frequency hopping feature */
    THP_AFE_FEATURE_ENUM feature_calibration;       /* Info of dynamic hardware calibration feature:
                                                       THP_AFE_FEATURE_NOT_SUPPORTED indicates
                                                            TPIC has no hardware calibration capability
                                                       THP_AFE_FEATURE_SUPPORTED indicates
                                                            TPIC has hardware calibration capability
                                                            but need host algorithm to detect and trigger
                                                            hardware calibration
                                                       THP_AFE_FEATURE_AUTO indicates
                                                            TPIC has hardware calibration capability and
                                                            TPIC will detect whether need do hardware calibration
                                                            by itself,
                                                            if need, TPIC will report THP_AFE_STATUS_RECAL_REQUEST and
                                                            host algorithm will determine
                                                            when to trigger hardware calibration
                                                       NOTE: dynamic hardware calibration is different from
                                                            power-on hardware calibration,
                                                            for TPIC with hardware calibration,
                                                            power-on hardware calibration is triggered by
                                                            TPIC itself during power-on and
                                                            dynamic hardware calibration is triggered by
                                                            host during run-time */
    THP_AFE_FEATURE_ENUM feature_wakeup_gesture;    /* Info of wakeup gesture feature */
    THP_AFE_SENSOR_ARCH_ENUM sensor_arch;           /* Sensor Architecture */
    THP_AFE_SENSOR_PATTERN_ENUM sensor_pattern;     /* Sensor Pattern */
    THP_AFE_STYLUS_PROTOCOL_ENUM stylus_protocol;   /* Info of active stylus protocol support feature. */
    uint8_t stylus_scan_freq_num;                   /* 1.Number of frequencies for HPP3.0 stylus hopping
                                                       2.for HPP2.X, default stylus_scan_freq_num is 2
                                                       3.for HPP3.0, stylus_scan_freq_num indicate
                                                            the frequency num TPIC can use */
    uint16_t *stylus_scan_freq;                     /* 1. Pointer to Array of all stylus scan frequencies.
                                                       2. for HPP2.X, stylus_scan_freq[0] is 176,meaning 176.37Khz,
                                                            stylus_scan_freq[1] is 252 meaning 252.49Khz
                                                       3. for HPP3.0, the value of stylus_scan_freq [n] is
                                                            frequency index,
                                                            refer to the frequency table in
                                                            <THP_AFE_HAL_SPEC> section 3.11.2.
                                                       for example,set to 0 for 100Khz,and 1 for 100.31Khz etc. */
    THP_AFE_FEATURE_ENUM feature_side_touch;        /* Info of side touch feature;
                                                       please refer to Side Touch for more information */
    uint8_t force_num;                              /* Number of force unit supported */
    THP_AFE_STYLUS_DATA_TYPE_ENUM stylus_data_type; // Info of stylus data type
    uint8_t num_col_after_tied;                     // Number of sensors along column after tied together.
    // Only used in THP_AFE_STYLUS_TIED_GRID_DATA type stylus data.
    uint8_t num_row_after_tied;                     // Number of sensors along row after tied together.
    // Only used in THP_AFE_STYLUS_TIED_GRID_DATA type stylus data.
} THP_AFE_HW_CAP_STRUCT;

/* Stylus frame data structure */
typedef struct
{
    uint16_t *tx1_line_data;           /* pointer to active stylus tx1 data;
                                          HPP2.X stylus data and HPP3.0 tx1 stylus data.
                                          data format:
                                            THP_AFE_STYLUS_NORMAL_LINE_DATA: [ col+ row],col first,then row
                                            THP_AFE_STYLUS_IQ_LINE_DATA: [colI+rowI+colQ+rowQ],
                                              colI,colQ is number of col IQ data, equal to col. same as rowI,rawQ.
                                              colI first,then rowI,then colQ,then rowQ
                                            THP_AFE_STYLUS_GRID_DATA: [col][row]
                                            THP_AFE_STYLUS_TIED_GRID_DATA: [col_after_tied*row+col*row_after_tied]
                                              First col direction tied, then row direction.
                                          col/row is defined in THP_AFE_HW_CAP_STRUCT
                                          if no data available, the value should be set to NULL */
    uint16_t *tx2_line_data;           /* pointer to active stylus tx2 data;just for HPP3.0 tx2 stylus data.
                                          data format:refer to tx1_linedata
                                          for HPP2.X the value should be set to NULL
                                          if no data available, the value should be set to NULL */
    uint16_t tx1_scan_freq;            /* 1, Indicate current tx1 detect frequency
                                          2, for HPP2.X,the value should be set to
                                             176 for 176.37Khz and 252 for 252.49Khz
                                          3, for HPP3.0, set the value by index of frequency table in section 3.11.2
                                          for example ,set to 0 for 100Khz,and 1 for 100.31Khz etc. */
    uint16_t tx2_scan_freq;            /* 1, Indicate current detect tx2 frequency
                                          2, for HPP2.X the value should be set to NULL
                                          3, for HPP3.0, set the value by index of frequency table in section 3.11.2
                                          for example ,set to 0 for 100Khz,and 1 for 100.31Khz etc. */
    uint16_t pressure;                 /* Active stylus pen pressure data.
                                          Used ONLY in HPP2.X
                                          0:no pressure
                                          1-4095: valid pressure data */
    THP_AFE_STYLUS_BTN_ENUM button;    /* Active stylus pen button info.
                                          Used ONLY in HPP2.X */
    THP_AFE_STYLUS_STATUS_ENUM status; /* Stylus scan status */
    uint16_t *stylus_noise;            /* 1, Pointer to unified noise data detected by AFE hardware,
                                          the unit of noise data is same as unit of stylus line data;
                                          2, Length of stylus_noise equals to stylus_scan_freq_num in
                                          THP_AFE_HW_CAP_STRUCT;
                                          3, stylus_noise[n] is the noise level of stylus_scan_freq[n];
                                          4, if no noise data available, the value should be set to NULL; */
    uint16_t tx1_new_scan_freq;        /* 1, Indicate which tx1 frequency TPIC want shift to;
                                          2, If no freq shift request it should keep same with tx1_scan_freq
                                          3, for HPP2.X the value should be set to NULL
                                          4, for HPP3.0, set the value by index of frequency table in section 3.11.2
                                          for example ,set to 0 for 100Khz,and 1 for 100.31Khz etc. */
    uint16_t tx2_new_scan_freq;        /* 1, Indicate which tx2 frequency TPIC want shift to;
                                          2, If no freq shift request it should keep same with tx2_scan_freq
                                          3, for HPP2.X the value should be set to NULL
                                          4, for HPP3.0, set the value by index of frequency table in section 3.11.2
                                          for example ,set to 0 for 100Khz,and 1 for 100.31Khz etc. */
} THP_AFE_STYLUS_FRAME_DATA_STRUCT;

/* Frame data structure */
/*typedef struct
{
    long tv_sec;
    long tv_usec;
} TIMEVAL_STRUCT;*/
typedef struct timeval TIMEVAL_STRUCT;

typedef struct
{
    TIMEVAL_STRUCT time_stamp;                /* time_stamp records the system timeval info which
                                                 could be obtained from system */
    uint16_t frame_index;                     /* frame index, start from 1 after reset
                                                 and should be increased 1 by 1 for each frame
                                                 by TPIC instead of AFEHAL;
                                                 this info will be used for abnormal reset or
                                                 frame missing detection. */
    uint16_t *grid_data;                      /* pointer to grid data,
                                                 which could be mutual data or self data in full incell architecture;
                                                 data format: [row][col], col first, then row
                                                 col/row is defined in THP_AFE_HW_CAP_STRUCT
                                                 if no grid data available, the value should be set to NULL */
    uint16_t *line_data;                      /* pointer to line data, which could be self data or others;
                                                 data format: [row+col], col first, then row
                                                 col/row is defined in THP_AFE_HW_CAP_STRUCT
                                                 if no line data available, the value should be set to NULL */
    uint16_t *button_data;                    /* pointer to button data
                                                 the length of button data is defined in THP_AFE_HW_CAP_STRUCT;
                                                 if no button data available, the value should be set to NULL */
    uint16_t *noise_data;                     /* Pointer to unified noise data detected by hardware,
                                                 the unit of noise data is same as unit of grid data;
                                                 the length of noise data is determined by feature_noise_detect type
                                                 in THP_AFE_HW_CAP_STRUCT;
                                                 if no noise data available, the value should be set to NULL */
    uint16_t scan_freq;                       /* Current scan frequency, unit in KHz */
    uint16_t scan_rate;                       /* Current scan rate, unit in Hz */
    THP_AFE_STATUS_ENUM status;               /* Status of AFE */
    THP_AFE_GESTURE_ENUM gesture;             /* The detected wake up gesture */
    THP_AFE_STYLUS_FRAME_DATA_STRUCT *stylus; /* Pointer to structure of Stylus data . */
    uint16_t *side_data;                      /* pointer to side data;
                                                 the length of side data is determined by thp_afe_set_side_region;
                                                 the order of side data is determined by column index and
                                                 start from column 0;
                                                 if no side data available, the value should be set to NULL;
                                                 please refer to Side Touch for more information; */
    uint16_t *force_data;                     /* pointer to force data;
                                                 the length of side data is determined by force_num
                                                 in THP_AFE_HW_CAP_STRUCT;
                                                 if no force data available, the value should be set to NULL; */
    THP_AFE_SCAN_STATE_ENUM scan_state;       /* Actual scan state, this state should match with actual valid data
                                                 range */
} THP_AFE_FRAME_DATA_STRUCT;

/******************************************************************************
* Function: thp_afe_hal_spec_version()
*
* @summary
*   Return THP AFE HAL spec version.
*
* @return
*   THP AFE HAL spec version which is defined by THP_AFE_HAL_SPEC_VERSION macro
*
* @par Notes
*   THP AFE HAL spec version follows Semantic Versioning 2.0.0 spec,
*   refer to http:// semver.org/ for details.
*   formula:version = major*65536 + minor*256 + patch
*
*****************************************************************************/
uint32_t thp_afe_hal_spec_version(void);

/******************************************************************************
* Function: thp_afe_hal_spec_major_version()
*
* @summary
*   Return THP AFE HAL spec major version.
*
* @return
*   THP AFE HAL spec major version which is defined by
*   THP_AFE_HAL_SPEC_MAJOR_VERSION macro
*
* @par Notes
*   THP AFE HAL spec version follows Semantic Versioning 2.0.0 spec,
*   refer to http:// semver.org/ for details.
*   formula:version = major*65536 + minor*256 + patch
*
*****************************************************************************/
uint8_t thp_afe_hal_spec_major_version(void);

/******************************************************************************
* Function: thp_afe_hal_spec_minor_version()
*
* @summary
*   Return THP AFE HAL spec minor version.
*
* @return
*   THP AFE HAL spec minor version which is defined by
*   THP_AFE_HAL_SPEC_MINOR_VERSION macro
*
* @par Notes
*   THP AFE HAL spec version follows Semantic Versioning 2.0.0 spec,
*   refer to http:// semver.org/ for details.
*   formula:version = major*65536 + minor*256 + patch
*
*****************************************************************************/
uint8_t thp_afe_hal_spec_minor_version(void);

/******************************************************************************
* Function: thp_afe_hal_spec_patch_version()
*
* @summary
*   Return THP AFE HAL spec patch version.
*
* @return
*   THP AFE HAL spec patch version which is defined by
*   THP_AFE_HAL_SPEC_PATCH_VERSION macro
*
* @par Notes
*   THP AFE HAL spec version follows Semantic Versioning 2.0.0 spec,
*   refer to http:// semver.org/ for details.
*   formula:version = major*65536 + minor*256 + patch
*
*****************************************************************************/
uint8_t thp_afe_hal_spec_patch_version(void);

/******************************************************************************
* Function: thp_afe_open()
*
* @summary
*   Creates an instance of AFE library for default project.
*   This is the entry point to AFE library.
*   No other APIs can be used before this API call.
*
* @return
*   THP_AFE_ERR_ENUM
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_open(void);

/******************************************************************************
* Function: thp_afe_open_project()
*
* @summary
*   Creates an instance of AFE library for project specified by proj_id.
*   This is the entry point to AFE library.
*   No other APIs can be used before this API call.
*
* @param proj_id
*   proj_id is 10 characters string to identify different TP modules.
*
* @return
*   THP_AFE_ERR_ENUM
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_open_project(const char *proj_id);

/******************************************************************************
* Function: thp_afe_close()
*
* @summary
*   Releases AFE.
*
* @return
*   THP_AFE_ERR_ENUM
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_close(void);

/******************************************************************************
* Function: thp_afe_start()
*
* @summary
*   Start AFE. Upon successful return, AFE is powered on and initialized to
*   work in default mode.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   Before calling the_afe_start, thp_afe_set_calib_data_callback_func()
*   should be called properly.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_start(void);

/******************************************************************************
* Function: thp_afe_stop()
*
* @summary
*   Stop AFE operation and power off AFE.
*
* @return
*   THP_AFE_ERR_ENUM
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_stop(void);

/******************************************************************************
* Function: thp_afe_screen_off()
*
* @summary
*   Notifies AFE that display goes off.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   AFE change to WorkingScreenOff state after this API being called.
*   If AFE is kept being powered on:
*   1. AFE should work in gesture detect mode if wakeup gesture is enabled;
*   2. AFE should work in idle mode if wakeup gesture is not enabled.
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_screen_off(void);

/******************************************************************************
* Function: thp_afe_screen_on()
*
* @summary
*   Notifies AFE that display turns on.
*
* @return
*   THP_AFE_ERR_ENUM
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_screen_on(void);

/******************************************************************************
* Function: thp_afe_get_info()
*
* @summary
*   Returns information about the AFE and AFE library, such as vendor name,
*   product name and AFE version.
*
* @return
*   pointer to AFE information
*
*****************************************************************************/
THP_AFE_INFO_STRUCT *thp_afe_get_info(void);

/******************************************************************************
* Function: thp_afe_get_hw_cap()
*
* @summary
*   Returns capabilities of the AFE, such as number of col/row, pitch size,
*    whether frequency shift is supported, etc.
*
* @return
*   pointer to hardware capability information
*
*****************************************************************************/
THP_AFE_HW_CAP_STRUCT *thp_afe_get_hw_cap(void);

/******************************************************************************
* Function: thp_afe_get_frame()
*
* @summary
*   Retrieves grid data, button data, AFE status etc.
*
* @return
*   pointer to frame data
*
* @par Notes
*   This API is a blocking call, AFE should provide a timeout mechanism.
*   When this API is blocking, if thp_afe_screen_off is called,
*   this API should return NULL immediately;
*   Please note that the content of frame data buffer should not be changed
*   until thp_afe_get_frame() getting called next time.
*****************************************************************************/
THP_AFE_FRAME_DATA_STRUCT *thp_afe_get_frame(void);

/******************************************************************************
* Function: thp_afe_enable_freq_shift()
*
* @summary
*   Enable frequency shift feature
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   1. if auto freq hopping feature supported, freq shift feature should be
*      enable by default unless thp_afe_disable_freq_shift() get called
*   2. if manual freq hopping feature supported,
*      thp_afe_enable_freq_shift should be ignored
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_enable_freq_shift(void);

/******************************************************************************
* Function: thp_afe_disable_freq_shift()
*
* @summary
*   Disable freq shift feature until thp_afe_start() get called again
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   1. if auto freq hopping feature supported, the feature should be disabled
*   2. if manual freq hopping feature supported, thp_afe_start_freq_shift should be ignored
*   3. thp_afe_force_to_freq_point/ thp_afe_force_to_scan_rate should be supported even though freq shift disabled
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_disable_freq_shift(void);

/******************************************************************************
 * Function: thp_afe_start_calibration()
 *
 * @summary
 *   Start to calibrate raw data.
 *
 * @return
 *   THP_AFE_ERR_ENUM
 *
 * @par Notes
 *   Below notes are for AFE with manual hardware calibration only:
 *   1. The calibration means hardware calibration which will calibrate hardware resource
 *      to compensate sensor to sensor raw data variation;
 *      When calibration done, AFE should inform caller with THP_AFE_STATUS_CALIBRATION_DONE status in
 *      THP_AFE_FRAME_DATA_STRUCT; THP_AFE_STATUS_CALIBRATION_DONE status should be kept until being cleared by caller
 *      with thp_afe_clear_status() API.
 *   2. AFE should load default calibration data instead of doing calibration after power on;
 *   3. Supplier should make sure validity of default calibration data;
 *   4. This API is designed to force re-calibration dynamically if needed;
 *   5. Each time this API get called, AFEHAL should record the new calibration data internally
 *      and maintain the life cycle until thp_afe_start get called;
 *   6. Each time power on , AFEHAL should check the availability of new calibration data
 *      and load it if available to override default calibration data;
 *   7. AFE could also detect whether re-calibration is needed or not,
 *      if yes, report the event with THP_AFE_STATUS_RECAL_REQUEST status;
 *
 *****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_start_calibration(void);

/******************************************************************************
* Function: thp_afe_set_calib_data_callback_func()
*
* @summary
*   Set call back function to save calibration data, after each calibration,
*   AFE should call the callback function to save calibration data.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param
*   calibDataWriteCallback: call back funtion to save calibration data
*   calibDataReadCallback: call back funtion to load calibration data
*
* @par Notes
*   1. This API is designed for the AFE which has no space
*      to save calibration data on silicon;
*   2. this API should be called before thp_afe_start;
*   3. if the return value of calibDataReadCallback() is not THP_AFE_OK,
*      which indicates the calibration data is broken or missing,
*      AFE need re-do calibration and save calibration data;
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_calib_data_callback_func(
    THP_AFE_ERR_ENUM(*calibDataWriteCallback)(void* dataPtr, uint32_t dataLen),
    THP_AFE_ERR_ENUM(*calibDataReadCallback)(void* dataPtr, uint32_t dataLen));

/******************************************************************************
* Function: thp_afe_clear_status()
*
* @summary
*   Clear the specified status.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param status
*   Specify which status to clear
*
* @par Notes
*   Refer to thp_afe_start_calibration() and thp_afe_start_freq_shift() for more info.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_clear_status(THP_AFE_STATUS_ENUM status);

/******************************************************************************
* Function: thp_afe_clear_gesture_status()
*
* @summary
*   Clear the specified gesture status.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param status
*   Specify which gesture status to clear
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_clear_gesture_status(THP_AFE_GESTURE_ENUM gesture);

/******************************************************************************
* Function: thp_afe_set_baseline_update_interval()
*
* @summary
*   Set time interval to update grid data baseline in IDLE mode, unit in ms.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param interval
*   Time interval to update grid data baseline in IDLE mode
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_baseline_update_interval(uint16_t interval);

/******************************************************************************
* Function: thp_afe_set_idle_touch_threshold()
*
* @summary
*   Set line data touch threshold in idle mode.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param threshold
*   line data touch threshold
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_idle_touch_threshold(uint16_t threshold);

/******************************************************************************
* Function: thp_afe_set_idle_scan_rate()
*
* @summary
*   Set idle scan rate, unit in Hz.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param rate
*   Idle scan rate
*
* @par Notes
*   The higher idle scan rate, the better 1st touch response time,
*   but the more power consumption in idle mode.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_idle_scan_rate(uint8_t rate);

/******************************************************************************
 * Function: thp_afe_reset_idle_baseline()
 *
 * @summary
 *   Inform AFE to reset IDLE baseline.
 *
 * @return
 *   THP_AFE_ERR_ENUM
 *
 * @par Notes
 *   IDLE baseline reset is mostly control by host, host will reset IDLE baseline when necessary;
 *   AFE is responsible for baseline update in IDLE mode only and
 *   AFE should judge to exit IDLE mode based on absolute IDLE signal instead of positive signal only;
 *   THP_AFE_STATUS_IDLE_BASELINE_RESET_DONE should be set on the next frame of grid data after IDLE baseline being
 *   reset.
 *****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_reset_idle_baseline(void);

/******************************************************************************
* Function: thp_afe_enter_idle()
*
* @summary
*   Inform AFE that it is allowed to enter IDLE mode.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   Refer to thp_afe_set_idle_interval() for more info.
*   This API may be called multi-time even AFE is working in IDLE mode.
*   The judgment to exit IDLE should base on line data of columns of core region only;
*   refer to Side Touch for details.
*   When TPIC receives IDLE command, it should return one or more valid frames data
*   with THP_AFE_STATUS_IDLE_MODE status before entering IDLE mode
*   to let host know it is ready to enter idle.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_enter_idle(void);

/******************************************************************************
* Function: thp_afe_force_exit_idle()
*
* @summary
*   Force AFE to exit IDLE mode.
*
* @return
*   THP_AFE_ERR_ENUM
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_force_exit_idle(void);

/******************************************************************************
* Function: thp_afe_force_to_freq_point()
*
* @summary
*   Force AFE active mode to work in the indexed scan frequency,
*   and disable auto freq shift feature until thp_afe_start() or
*   thp_afe_enable_freq_shift() get called
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param index
*   Specify the index of frequency, range from 0 to num_freq-1,
*   num_freq is defined in THP_AFE_HW_CAP_STRUCT
*
* @note
*   1. if the value of index equals to current index, do nothing;
*      orelse change to the indexed frequency and set THP_AFE_STATUS_FREQ_SHIFT_DONE status
*   2. no matter this API is called or not, IDLE mode need be supported;
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_force_to_freq_point(uint8_t index);

//yjl
THP_AFE_ERR_ENUM thp_afe_disable_freq_shift(void);

THP_AFE_ERR_ENUM thp_afe_anble_freq_shift(void);

/******************************************************************************
* Function: thp_afe_force_to_scan_rate()
*
* @summary
*   Force AFE active mode to keep working in the indexed scan rate until being re-started
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param index
*   Specify the index of scan rate in active mode, range from 0 to num_active_scan_rate-1,
*   num_active_scan_rate is defined in THP_AFE_HW_CAP_STRUCT
*
* @note
*   1. if the value of index equals to current index, do nothing;
*      orelse change to the indexed scan rate and set THP_AFE_STATUS_FREQ_SHIFT_DONE status
*   2. no matter freq shift is enabled or not, scan rate is fully control by host
*   3. no matter this API is called or not, IDLE mode need be supported;
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_force_to_scan_rate(uint8_t index);

/******************************************************************************
* Function: thp_afe_set_log_callback_func()
*
* @summary
*   Set log call back function to AFE, AFE should output all the log
*   to the log callback function.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param void(*)(const char *)
*   AFE log call back funtion
*
* @par Notes
*   The suggested log format is: LogLevel+TimeStamp+FunctionName+LogContent,
*   for example: [I][1473384419770][thp_afe_hal_funcion] log-content;
*   formula:TimeStamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
*   tv is the system timeval which could be obtained from system.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_log_callback_func (void (*log_func)(const char *));

/******************************************************************************
* Function: thp_afe_set_shblog_callback_func()
*
* @summary
*   Set log call back function to AFE, AFE can print number in sensorhub.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param void(*)((int32_t*, uint32_t))
*   AFE log call back function
*
* @par Notes
*   The second parameter maximizes support to 8;
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_shblog_callback_func (void (*log_func)(int32_t *data, uint32_t data_number));

/******************************************************************************
* Function: thp_afe_set_log_level()
*
* @summary
*   Set log call back function to AFE, AFE should output all the log
*   to the log callback function.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   If this API is not called, AFE should have a default log level;
*   log level is defined as below:
*   THP_AFE_LOG_LEVEL_ERROR(1)
*   THP_AFE_LOG_LEVEL_WARNING(2)
*   THP_AFE_LOG_LEVEL_INFO(3)
*   THP_AFE_LOG_LEVEL_DEBUG(4)
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_log_level(uint8_t log_level);

/******************************************************************************
* Function: thp_afe_enable_wakeup_gesture_v1()
*
* @summary
*   Inform AFE to enable wakeup gesture.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param gesture
*   Specific wake up gesture
*
* @par Notes
*   After wakeup gesture being enabled, AFE should enter gesture mode automatically when screen off;
*   It is AFE HAL/AFE's responsibility to detect gesture instead of caller;
*   if wake up gesture detected, the detected gesture type along with
*   THP_AFE_STATUS_GESTURE_DETECTED status should be set in THP_AFE_FRAME_DATA_STRUCT.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_enable_wakeup_gesture_v1(THP_AFE_GESTURE_ENUM gesture);

/******************************************************************************
* Function: thp_afe_disable_wakeup_gesture_v1()
*
* @summary
*   Inform AFE to disable specific wakeup gesture.
*
* @return
*   THP_AFE_ERR_ENUM
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_disable_wakeup_gesture_v1(THP_AFE_GESTURE_ENUM gesture);

/******************************************************************************
* Function: thp_afe_set_wakeup_gesture_scan_rate_v1()
*
* @summary
*   Set specific wakeup gesture scan rate, unit in Hz.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param rate
*   Wakeup gesture scan rate
*
* @par Notes
*   The higher scan rate, the better gesture response time,
*   but the more power consumed.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_wakeup_gesture_scan_rate_v1(THP_AFE_GESTURE_ENUM gesture, uint8_t rate);

/******************************************************************************
* Function: thp_afe_inspect()
*
* @summary
*   TP inspection.
*
* @return
*   combination of THP_AFE_INSPECT_ERR_ENUM
*
* @par Notes
*   Refer to THP_AFE_INSPECT_ERR_ENUM for details.
*****************************************************************************/
uint32_t thp_afe_inspect(void);

/******************************************************************************
 * Function: thp_afe_get_inspect_grid_data()
 *
 * @summary
 *   Retrieves grid data for TP inspections.
 *
 * @return
 *   uint16_t*
 *
 * @par Notes
 *   Return pointer to inspect_grid_data.
 *   The structure of returned data is the same with grid_data, refer to THP_AFE_FRAME_DATA_STRUCT.
 *   Please note that the content of inspect_grid_data buffer should not be changed until thp_afe_inspect() getting
 *   called next time.
 *****************************************************************************/
uint16_t *thp_afe_get_inspect_grid_data(void);

/******************************************************************************
 * Function: thp_afe_get_inspect_line_data()
 *
 * @summary
 *   Retrieves line data for TP inspections.
 *
 * @return
 *   uint16_t*
 *
 * @par Notes
 *   Return pointer to inspect_line_data.
 *   The structure of returned data is the same with line_data, refer to THP_AFE_FRAME_DATA_STRUCT.
 *   Please note that the content of inspect_line_data buffer should not be changed until thp_afe_inspect() getting
 *   called next time.
 *****************************************************************************/
uint16_t *thp_afe_get_inspect_line_data(void);

/******************************************************************************
 * Function: thp_afe_get_inspect_noise()
 *
 * @summary
 *   Retrieves noise data for TP inspections.
 *
 * @return
 *   int16_t*
 *
 * @par Notes
 *   Return pointer to inspect_noise.
 *   The structure of returned data is the same with grid_data, refer to THP_AFE_FRAME_DATA_STRUCT.
 *   Please note that the content of inspect_noise buffer should not be changed until thp_afe_inspect() getting called
 *   next time.
 *****************************************************************************/
int16_t *thp_afe_get_inspect_noise(void);

/******************************************************************************
* Function: thp_afe_enter_tui()
*
* @summary
*   Inform AFE that system will switch from Anroid to TEE.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   AFE HAL need preserve current AFE working status.
*   Note: at this moment, Android has lost the power of AFE hardware control.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_enter_tui(void);

/******************************************************************************
* Function: thp_afe_exit_tui()
*
* @summary
*   Inform AFE that system will switch from TEE to Android.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   AFE HAL need restore the AFE working status before entering TUI.
*   Note: at this moment, Android has the power to control AFE hardware.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_exit_tui(void);

/******************************************************************************
* Function: thp_afe_enter_proximity()
*
* @summary
*   Inform AFE to enter proximity mode.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   From normal mode to proximity mode, TPIC need ensure:
*   1. work with display off
*   2. raw data shift < 100
*   3. mode switching timing < 150ms
*   4. set THP_AFE_STATUS_PROXIMITY status
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_enter_proximity(void);

/******************************************************************************
* Function: thp_afe_exit_proximity()
*
* @summary
*   Inform AFE to exit proximity mode.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   From proximity mode to normal mode, TPIC need ensure:
*   1. raw data shift < 100
*   2. mode switching timing < 300ms
*   3. clear THP_AFE_STATUS_PROXIMITY status
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_exit_proximity(void);

/******************************************************************************
* Function: thp_afe_enter_display_sync()
*
* @summary
*   Inform AFE to enter display sync mode.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   For TDDI solution, this API could be ignored.
*   From normal mode to display sync mode, TPIC need ensure:
*   1. In display sync mode, SNR with any display pattern noise should be
*      the same as SNR without display noise, and report rate should be >= 60Hz
*   2. raw data shift < 100
*   3. mode switching timing < 100ms
*   4. set THP_AFE_STATUS_DISPLAY_SYNC status
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_enter_display_sync(void);

/******************************************************************************
* Function: thp_afe_exit_display_sync()
*
* @summary
*   Inform AFE to exit display sync mode.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   For TDDI solution, this API could be ignored.
*   From display sync mode to normal mode, TPIC need ensure:
*   1. raw data shift < 100
*   2. mode switching timing < 100ms
*   3. clear THP_AFE_STATUS_DISPLAY_SYNC status
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_exit_display_sync(void);

/******************************************************************************
* Function: thp_afe_suspend()
*
* @summary
*   Inform AFE to enter suspend state.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   1. This API is designed to co-work with thp_afe_resume() in-pair;
*   2. THP_AFE_STATUS_SUSPEND status should be set
*      and return in frame data before entering suspend state;
*   3. In suspend state, AFE does not scan until any of below APIs get called
*      a. thp_afe_resume()
*      b. thp_afe_start()
*      c. thp_afe_exit_proximity()
*   4. In suspend state, AFE timeout THP_LOGIc should be disabled;
*   5. In suspend state, AFE power consumption should be less than 1mA;
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_suspend(void);

/******************************************************************************
* Function: thp_afe_resume()
*
* @summary
*   Inform AFE to exit suspend state.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   1. This API is designed to co-work with thp_afe_suspend() in-pair;
*   2. THP_AFE_STATUS_SUSPEND status should be cleared
*      and return in frame data after exiting suspend state;
*   3. After exiting suspend state, AFE scan should be restored;
*   4. After exiting suspend state, AFE timeout THP_LOGIc should be restored;
*   5. Period from thp_afe_resume() API executed
*      to IRQ of 1st frame data asserted should be less than 50ms
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_resume(void);

/******************************************************************************
* Function: thp_afe_enable_hpp2_x()
*
* @summary
*   Inform AFE to detect active stylus with hpp2_x protocol.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*    1. The three APIs thp_afe_enable_hpp2_x and thp_afe_enable_hpp3_0
*        and thp_afe_disable_active_stylus are designed to work together.
*    2. The default mode is thp_afe_disable_active_stylus!
*    3. The three status THP_AFE_NO_STYLUS_PROTOCOL / THP_AFE_STATUS_HPP2_X_PROTOCOL
*      / THP_AFE_STATUS_HPP3_0_PROTOCOL indicate the AFE working mode!
*    4. After the API called, the AFE should change scan mode to support HPP2.X protocol,
*       Including but not limited to active mode, idle mode, even screen off mode
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_enable_hpp2_x(void);

/******************************************************************************
* Function: thp_afe_enable_hpp3_0()
*
* @summary
*   Inform AFE to detect active stylus with hpp3_0 protocol.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*    1. The three APIs thp_afe_enable_hpp2_x and thp_afe_enable_hpp3_0
*        and thp_afe_disable_active_stylus are designed to work together.
*    2. The default mode is thp_afe_disable_active_stylus!
*    3. The three status THP_AFE_NO_STYLUS_PROTOCOL / THP_AFE_STATUS_HPP2_X_PROTOCOL
*      / THP_AFE_STATUS_HPP3_0_PROTOCOL indicate the AFE working mode!
*    4. After the API called, the AFE should change scan mode to support HPP3.0 protocol,
*       Including but not limited to active mode, idle mode, even screen off mode
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_enable_hpp3_0(void);

/******************************************************************************
* Function: thp_afe_disable_active_stylus()
*
* @summary
*   Inform AFE back to the scan mode without detecting active stylus.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*    1. The three APIs thp_afe_enable_hpp2_x and thp_afe_enable_hpp3_0
*        and thp_afe_disable_active_stylus are designed to work together.
*    2. The default mode is thp_afe_disable_active_stylus!
*    3. The three status THP_AFE_NO_STYLUS_PROTOCOL / THP_AFE_STATUS_HPP2_X_PROTOCOL
*      / THP_AFE_STATUS_HPP3_0_PROTOCOL indicate the AFE working mode!
*    4. After the API called, the AFE should change scan mode to
*       default mode without active stylus protocol,
*       Including but not limited to active mode, idle mode, even screen off mode
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_disable_active_stylus(void);

/******************************************************************************
* Function: thp_afe_set_hpp3_0_detect_freq_immediately()
*
* @summary
*   Inform AFE set HPP3.0 scan frequency to new value.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   After this API called, AFE should set active stylus scan frequency to the setting value immediately.
*   Frequency table refer to section 3.11.2 <THP_AFE_HAL_SPEC>
*   Frequency shift refer to section 3.11.3 in <THP_AFE_HAL_SPEC>
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_hpp3_0_detect_freq_immediately(uint16_t freq1Index, uint16_t freq2Index);

/******************************************************************************
* Function: thp_afe_set_hpp3_0_detect_freq_next_uplink()
*
* @summary
*   Inform AFE set HPP3.0 scan frequency to new value.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   After this API called, AFE should buffer the two new setting frequency.
*   One specific bit in next UPLINK(see HPP3.0 spec) should be inversed to inform HPP3.0 pen.
*   And then AFE set active stylus scan frequency to the setting value after the UPLINK.
*   Frequency table refer to section 3.11.2 <THP_AFE_HAL_SPEC>
*   Frequency shift refer to section 3.11.3 in <THP_AFE_HAL_SPEC>
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_hpp3_0_detect_freq_next_uplink(uint16_t freq1Index, uint16_t freq2Index);

/*
 * THP_AFE_STATUS_STYLUS_FREQ_SHIFT_DONE, THP_AFE_STATUS_STYLUS_FREQ_SHIFT_REQUEST status should be kept until
 * being cleared by caller with thp_afe_clear_stylus_status() API.
 */
THP_AFE_ERR_ENUM thp_afe_clear_stylus_status(THP_AFE_STYLUS_STATUS_ENUM status);


/******************************************************************************
* Function: thp_afe_set_charger(bool plugin)
*
* @summary
*   Inform AFE charger state when it change.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*    1. This API is designed for AFE getting charger state so that AFE can use different IC config in charger state.
*   2. AFE using different config is effective for optimizing the SNR in charger state.
*   3. This API is an optional implementation interface according to actual needs.

*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_charger(bool plugin);

/******************************************************************************
* Function: thp_afe_set_udfp_mode(bool is_udfp_enabled)
*
* @summary
*   Inform AFE under display fingerprint state.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   1. udfp is an abbreviation of under display fingerprint.
*   2. After power on, AFE can determine if it should give IC a command according to whether the under display
*   fingerprint has been entered or not.
*   3. The command for IC could be for the purpose of reduce power consumption.
*   4. This API is an optional implementation interface according to actual needs.

*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_udfp_mode(bool is_udfp_enabled);

/******************************************************************************
* Function: thp_afe_is_dmd_triggered(void)
*
* @summary
*   AFE inform daemon that it need to trigger a DMD log.
*
* @return
*   bool
*
* @par Notes
*    1. This API will trigger DMDI_THP_AFE_LOG_TRIGGER.
*   2. This API return should be reset to false after a call.
*   3. The DMD trigger mechanism set twice DMD triggered interval exceed one day and three times.
*   4. The DMD could be used to count checksum error or IC FW abnormal reset.

*****************************************************************************/
bool thp_afe_is_dmd_triggered(void);

/******************************************************************************
* Function: thp_afe_set_side_region(uint32_t colBitMap)
*
* @summary
*   Inform AFE the area of side region.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param colBitMap
*   Each bit is mapped to 1 column, bit-0 is mapped to column-0;
*   bit value indicates whether the mapped column belongs to side region or not:
*   1: yes
*   0: no
*
* @note
*   please refer to Side Touch for more information
*
*****************************************************************************/


bool thp_afe_is_esd_triggered(int nFiger);


THP_AFE_ERR_ENUM thp_afe_set_side_region(uint32_t colBitMap);

/******************************************************************************
* Function: thp_afe_set_side_region(uint32_t colBitMap)
*
* @summary
*   Inform AFE the area of side region.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param colBitMap
*   Each bit is mapped to 1 column, bit-0 is mapped to column-0;
*   bit value indicates whether the mapped column belongs to side region or not:
*   1: yes
*   0: no
*
* @note
*   please refer to Side Touch for more information
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_scan_state(THP_AFE_SCAN_STATE_ENUM state);
/******************************************************************************
* Function: thp_afe_set_scan_state(THP_AFE_SCAN_STATE_ENUM state);
*
* @summary
*   Inform AFE to switch scan state.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param state
*   Refer to THP_AFE_SCAN_STATE_ENUM.
*
* @note
*   Please refer to scan state for more information.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_core_region(uint32_t colBitMap);

/******************************************************************************
* Function: thp_afe_enter_side_idle()
*
* @summary
*   Inform AFE to enter side idle state.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @note
*   The judgment to exit SideIdle state should base on line data of columns of side region only;
*   Please refer to Side Touch for details.
*   When TPIC receives SideIdle command, it should return one or more valid frames data with
*   THP_AFE_STATUS_SIDE_IDLE_MODE status before entering SideIdle mode
*   to let host know it is ready to enter SideIdle.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_enter_side_idle(void);

/******************************************************************************
* Function: thp_afe_force_exit_side_idle()
*
* @summary
*   Force AFE to exit side idle mode.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @note
*   There are several different exit states depending on scenarios,
*   please refer to Side Touch for more information
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_force_exit_side_idle(void);

/******************************************************************************
* Function: thp_afe_set_side_scan_rate
*
* @summary
*   Set AFE side touch active scan rate
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param rate
*   side touch active scan rate, 0 means side scan disabled;
*
* @note
*   please refer to Side Touch for more information
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_set_side_scan_rate(uint16_t rate);

/******************************************************************************
* Function: thp_afe_idle_line_data_report
*
* @summary
*   This API is designed to control line data report in idle mode
*
* @return
*   THP_AFE_ERR_ENUM
*
* @param enabled
*   0: disable idle line data report
*   1: enable idle line data report in line_data field of THP_AFE_FRAME_DATA_STRUCT;
*
* @par Notes
*   Normally line data in idle mode is not reported to host for power consumption saving.
*   But in some scenarios, host may want to get each frame of idle line data,
*   for example idle mode debugging or line data qualification.
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_idle_line_data_report(uint8_t enabled);

/******************************************************************************
* Function: thp_afe_enable_force()
*
* @summary
*   Inform AFE to enable force feature which is disable by default.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   Below list the possible user scenarios to support force:
*   1. under display finger print
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_enable_force(void);

/******************************************************************************
* Function: thp_afe_disable_force()
*
* @summary
*   Inform AFE to disable force feature.
*
* @return
*   THP_AFE_ERR_ENUM
*
* @par Notes
*   Below list the possible user scenarios to support force:
*   1. under display finger print
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_disable_force(void);

#ifdef __cplusplus
}
#endif

#endif /* THP_AFE_HAL_H */
