#include "thp_afe_hal.h"
#include "cts_core.h"
#include "cts_log.h"
#include "cts_utils.h"
#include "thp_ioctl.h"
#include "cts_tcs.h"

#define ATTR_NUM_ROW                    0x8004
#define ATTR_NUM_COL                    0x8005
#define ATTR_RX_DIRECTION               0x8200
#define ATTR_RX_CHANNEL                 0x8201
#define ATTR_NUM_SCAN_FREQ              0x8202
#define ATTR_SCAN_FREQ                  0x8203

uint16_t s_Fs_raw_dest_value = FS_RAW_DEST_VALUE;
uint16_t s_scan_freq[MAX_NUM_SCAN_FREQ];
uint16_t s_scan_rate[MAX_NUM_SCAN_RATE];
static uint16_t s_stylus_scan_freq[MAX_NUM_SCAN_FREQ];
THP_AFE_HW_CAP_STRUCT s_thp_hw_cap;

//uint16_t num_col_used=COLS;
//uint16_t num_row_used=ROWS;
//uint16_t num_col_tied_used=COLS_STYLUS_TIED;
//uint16_t num_row_tied_used=ROWS_STYLUS_TIED;
uint8_t num_scan_freq_order[MAX_NUM_SCAN_FREQ];

int cts_force_get_hw_cap(void)
{
    uint8_t num_col;
    uint8_t num_row;
    uint8_t rx_direction;
    uint8_t rx_channel;
    uint8_t num_scan_freq;
    uint8_t num_scan_rate;
    uint8_t stylus_scan_freq_num;
    int ret;

    memset(s_scan_freq, 0, sizeof(s_scan_freq));
    memset(s_scan_rate, 0, sizeof(s_scan_rate));
    memset(s_stylus_scan_freq, 0, sizeof(s_stylus_scan_freq));
    memset(&s_thp_hw_cap, 0, sizeof(s_thp_hw_cap));
    CTS_THP_LOGE("cts_force_get_hw_cap++");

    ret = cts_tcs_get_rows_cols(&num_row, &num_col);

    if (ret)
    {
        CTS_THP_LOGE("Get rows and cols failed");
        return -1;
    }
    CTS_THP_LOGI("rows = %d, cols = %d", num_row, num_col);
    rx_direction = 0;
    rx_channel = num_col;

    if ( USED_NUM_SCAN_FREQ > MAX_NUM_SCAN_FREQ )
    {
        num_scan_freq = MAX_NUM_SCAN_FREQ;
        CTS_THP_LOGE("freq num error---check hal setting");
    }
    else
    {
        num_scan_freq = USED_NUM_SCAN_FREQ;
    }

    ret = cts_tcs_get_scan_freqs(&num_scan_freq, s_scan_freq);
    if (ret)
    {
        CTS_THP_LOGE("Get scan freqs failed");
        return -1;
    }

    if (0)
    {
        uint8_t i;
        CTS_THP_LOGD("freq num=%d", num_scan_freq);
        for (i=0; i<num_scan_freq; i++)
        {
            CTS_THP_LOGD("freq[%d]=%d", i,s_scan_freq[i]);
            if ( (s_scan_freq[i] != F_FREQ1) && (s_scan_freq[i] != F_FREQ2 ) && (s_scan_freq[i] != F_FREQ3))
            {
                CTS_THP_LOGE("scan freq---check hal setting");
            }
        }
        if (s_scan_freq[0] == 0 || s_scan_freq[0] == 0xFF)
        {
            s_scan_freq[0] = F_FREQ1;
        }
        if (s_scan_freq[1] == 0 || s_scan_freq[1] == 0xFF)
        {
            s_scan_freq[1] = F_FREQ2;
        }
        if (s_scan_freq[2] == 0 || s_scan_freq[2] == 0xFF)
        {
            s_scan_freq[2] = F_FREQ3;
        }
    }

    if ( USED_NUM_SCAN_RATE > MAX_NUM_SCAN_RATE )
    {
        num_scan_rate = MAX_NUM_SCAN_RATE;
        CTS_THP_LOGE("scan rate num error---check hal setting");
    }
    else
    {
        num_scan_rate = USED_NUM_SCAN_RATE;
    }

    s_scan_rate[0] = F_SCAN_RATE1;

    if (0)
    {
        uint8_t i;
        CTS_THP_LOGD("scan rate num=%d", num_scan_rate);
        for (i=0; i<num_scan_rate; i++)
        {
            CTS_THP_LOGD("scan rate[%d]=%d", i,s_scan_rate[i]);
        }
    }

    ret = cts_tcs_get_Normal_Fs_Raw_Dest_Value(&s_Fs_raw_dest_value);
    if (ret)
    {
        CTS_THP_LOGE("Fs_raw_dest_value failed");
        s_Fs_raw_dest_value = FS_RAW_DEST_VALUE;
    }
    else
    {
        if ( s_Fs_raw_dest_value != FS_RAW_DEST_VALUE )
        {
            CTS_THP_LOGE("s_Fs_raw_dest_value error---check hal setting");
        }
    }

    //CTS_THP_LOGE("s_Fs_raw_dest_value=  %4d", s_Fs_raw_dest_value);
    ret = cts_tcs_get_fw_ver(&g_version);
    if (ret)
    {
        CTS_THP_LOGE("Get fw version failed");
    }
    CTS_THP_LOGI("curr_fw version = 0x%04x", g_version);

    stylus_scan_freq_num = 2;
    s_stylus_scan_freq[0] = 139;
    s_stylus_scan_freq[1] = 194;

    s_thp_hw_cap.num_col = num_row;
    s_thp_hw_cap.num_row = num_col;
    

    // num_col_used = s_thp_hw_cap.num_col;
    //num_row_used = s_thp_hw_cap.num_row;


    s_thp_hw_cap.num_button = 0;

    /* 0: col 1: row */
    s_thp_hw_cap.rx_direction = !!rx_direction;
    s_thp_hw_cap.rx_channel = rx_channel;
    /* 0: normal 1: interlace */
    s_thp_hw_cap.rx_slot_layout = 0;

    s_thp_hw_cap.pitch_size_um = 1000;

    s_thp_hw_cap.num_scan_freq = num_scan_freq;
    s_thp_hw_cap.scan_freq = s_scan_freq;
    s_thp_hw_cap.num_scan_rate = num_scan_rate;
    s_thp_hw_cap.scan_rate = s_scan_rate;

    s_thp_hw_cap.feature_noise_detect = THP_AFE_NOISE_DETECT_ALL_FREQ;
    s_thp_hw_cap.feature_freq_hop = THP_AFE_FEATURE_AUTO;
    s_thp_hw_cap.feature_calibration = THP_AFE_FEATURE_AUTO;
    s_thp_hw_cap.feature_wakeup_gesture = THP_AFE_FEATURE_SUPPORTED;

    /* TODO: Check these options */
    s_thp_hw_cap.sensor_arch = THP_AFE_SA_FULL_INCELL;
    s_thp_hw_cap.sensor_pattern = THP_AFE_SP_AIT;

    /* TODO: Check these options */
    s_thp_hw_cap.stylus_protocol = THP_AFE_STYTUS_PROTOCOL_HPP3_0;
    s_thp_hw_cap.stylus_scan_freq_num = stylus_scan_freq_num;
    s_thp_hw_cap.stylus_scan_freq = s_stylus_scan_freq;

    s_thp_hw_cap.feature_side_touch = THP_AFE_FEATURE_NOT_SUPPORTED;
    s_thp_hw_cap.force_num = 0;

    s_thp_hw_cap.stylus_data_type = THP_AFE_STYLUS_TIED_GRID_DATA;
    s_thp_hw_cap.num_col_after_tied = ROWS_STYLUS_TIED;
    s_thp_hw_cap.num_row_after_tied = COLS_STYLUS_TIED;

    //num_col_tied_used=s_thp_hw_cap.num_col_after_tied;
    // num_row_tied_used=s_thp_hw_cap.num_row_after_tied;
    CTS_THP_LOGE("COL=%d,ROW=%d,TIED_COL=%d,TIED_row=%d",s_thp_hw_cap.num_col,s_thp_hw_cap.num_row,  s_thp_hw_cap.num_col_after_tied, s_thp_hw_cap.num_row_after_tied);
    return 0;
}

THP_AFE_HW_CAP_STRUCT *cts_get_hw_cap(void)
{
#if 1
    CTS_THP_LOGI("%-22s", "cts_get_hw_cap  hw_cap:");
    CTS_THP_LOGI("  %-22s = %d", "num_col", s_thp_hw_cap.num_col);
    CTS_THP_LOGI("  %-22s = %d", "num_row", s_thp_hw_cap.num_row);
    CTS_THP_LOGI("  %-22s = %d", "num_button", s_thp_hw_cap.num_button);
    CTS_THP_LOGI("  %-22s = %d", "rx_direction", s_thp_hw_cap.rx_direction);
    CTS_THP_LOGI("  %-22s = %d", "rx_channel", s_thp_hw_cap.rx_channel);
    CTS_THP_LOGI("  %-22s = %d", "rx_slot_layout", s_thp_hw_cap.rx_slot_layout);
    CTS_THP_LOGI("  %-22s = %d", "pitch_size_um", s_thp_hw_cap.pitch_size_um);
    CTS_THP_LOGI("  %-22s = %d", "num_scan_freq", s_thp_hw_cap.num_scan_freq);
    CTS_THP_LOGI("  %-22s = %d", "num_scan_rate", s_thp_hw_cap.num_scan_rate);
    CTS_THP_LOGI("  %-22s = %d", "feature_noise_detect", s_thp_hw_cap.feature_noise_detect);
    CTS_THP_LOGI("  %-22s = %d", "feature_freq_hop", s_thp_hw_cap.feature_freq_hop);
    CTS_THP_LOGI("  %-22s = %d", "feature_calibration", s_thp_hw_cap.feature_calibration);
    CTS_THP_LOGI("  %-22s = %d", "feature_wakeup_gesture", s_thp_hw_cap.feature_wakeup_gesture);
    CTS_THP_LOGI("  %-22s = %d", "sensor_arch", s_thp_hw_cap.sensor_arch);
    CTS_THP_LOGI("  %-22s = %d", "sensor_pattern", s_thp_hw_cap.sensor_pattern);
    CTS_THP_LOGI("  %-22s = %d", "stylus_protocol", s_thp_hw_cap.stylus_protocol);
    CTS_THP_LOGI("  %-22s = %d", "stylus_scan_freq_num", s_thp_hw_cap.stylus_scan_freq_num);
    CTS_THP_LOGI("  %-22s = %d", "feature_side_touch", s_thp_hw_cap.feature_side_touch);
    CTS_THP_LOGI("  %-22s = %d", "force_num", s_thp_hw_cap.force_num);
    CTS_THP_LOGI("  %-22s = %d", "stylus_data_type", s_thp_hw_cap.stylus_data_type);
    CTS_THP_LOGI("  %-22s = %d", "num_col_after_tied", s_thp_hw_cap.num_col_after_tied);
    CTS_THP_LOGI("  %-22s = %d", "num_row_after_tied", s_thp_hw_cap.num_row_after_tied);
#endif
    return &s_thp_hw_cap;
}
