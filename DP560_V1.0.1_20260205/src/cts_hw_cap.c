#include "cts_hal.h"
#include <string.h>
#include "thp/thp_afe_hal.h"
#include "cts_tcs.h"
#include "cts_core.h"

uint16_t s_scan_freq[MAX_NUM_SCAN_FREQ];
uint16_t s_scan_rate[MAX_NUM_SCAN_RATE];
uint8_t s_scan_rate_num = 0;
uint8_t s_curr_scan_rate;
uint16_t s_scan_state = 0;

static uint16_t s_stylus_scan_freq[MAX_NUM_SCAN_FREQ];
static THP_AFE_HW_CAP_STRUCT s_thp_hw_cap;

struct cts_dev_info cts_dev_info;
uint32_t g_hwid;
extern char *project_id;

extern bool compen_done;

static void dump_tx_rx_order(uint8_t *order, char *name)
{
	int i;

	//THP_LOGI("TRX_oder_failed, data:");
	for (i = 0; i < TR_ORDER_MAX; i++)
		THP_LOGD("  %-19s[%d] = %d", name, i, order[i]);
}

int cts_force_get_hw_cap(void)
{
	uint8_t rx_num, tx_num;
	uint8_t cols, rows;
	uint8_t rx_direction;
	uint8_t rx_channel;
	uint8_t num_scan_freq;
	uint8_t num_scan_rate;
	uint8_t stylus_scan_freq_num;
	int ret;

	MEMSET(s_scan_freq, 0, sizeof(s_scan_freq));
	MEMSET(s_scan_rate, 0, sizeof(s_scan_rate));
	MEMSET(s_stylus_scan_freq, 0, sizeof(s_stylus_scan_freq));
	MEMSET(&s_thp_hw_cap, 0, sizeof(s_thp_hw_cap));

	ret = cts_tcs_set_hw_cap_ready(0);
	if (ret) {
		THP_LOGE("Set hw cap ready flag failed");
		goto err_return;
	}

	/*
	 *	Get chipone device tx/rx information
	 */
	ret = cts_tcs_get_mst_tx_num(&cts_dev_info.mst_tx_num);
	if (ret) {
		THP_LOGE("Get master tx num failed");
		goto err_return;
	}
	ret = cts_tcs_get_mst_rx_num(&cts_dev_info.mst_rx_num);
	if (ret) {
		THP_LOGE("Get master rx num failed");
		goto err_return;
	}
	ret = cts_tcs_get_slv_tx_num(&cts_dev_info.slv_tx_num);
	if (ret) {
		THP_LOGE("Get slave tx num failed");
		goto err_return;
	}
	ret = cts_tcs_get_slv_rx_num(&cts_dev_info.slv_rx_num);
	if (ret) {
		THP_LOGE("Get slave rx num failed");
		goto err_return;
	}
	ret = cts_tcs_get_tx_tr_order(cts_dev_info.tx_tr_order);
	if (ret) {
		THP_LOGE("Get Tx_tr_order failed");
		goto err_return;
	}
	ret = cts_tcs_get_rx_tr_order(cts_dev_info.rx_tr_order);
	if (ret) {
		THP_LOGE("Get Rx_tr_order failed");
		goto err_return;
	}

	ret = cts_tcs_get_rows_cols(&rx_num, &tx_num);
	if (ret) {
		THP_LOGE("Get rows and cols failed");
		goto err_return;
	}
	if (rx_num == COLS) {
#if PATTERN_TYPE_1
		cols = rx_num / 2;
		rows = tx_num;
#else
		cols = rx_num;
		rows = tx_num;
#endif
	} else {
#if PATTERN_TYPE_1
		cols = tx_num;
		rows = rx_num / 2;
#else
		cols = tx_num;
		rows = rx_num;
#endif
	}
	THP_LOGD("rows = %d, cols = %d", rows, cols);

	rx_direction = 0;
	rx_channel = rx_num;
	ret = cts_tcs_get_scan_freqs(&num_scan_freq, s_scan_freq);
	if (ret) {
		THP_LOGE("Get scan freqs failed");
		goto err_return;
	}

	THP_LOGI("scan_freq_num = %d", num_scan_freq);
	THP_LOGI("scan_freqs = { %d, %d, %d, %d, %d, %d, %d, %d, %d, %d }",
			s_scan_freq[0], s_scan_freq[1], s_scan_freq[2], s_scan_freq[3], s_scan_freq[4],
			s_scan_freq[5], s_scan_freq[6], s_scan_freq[7], s_scan_freq[8], s_scan_freq[9]);

	ret = cts_tcs_get_stylus_scan_freqs(&stylus_scan_freq_num, s_stylus_scan_freq);
	if (ret) {
		THP_LOGE("Get stylus scan freqs failed");
		goto err_return;
	}
	THP_LOGI("stylus_scan_freq_num = %d", stylus_scan_freq_num);
	THP_LOGI("stylus_scan_freqs = { %d, %d, %d, %d, %d, %d, %d, %d, %d, %d }",
			s_stylus_scan_freq[0], s_stylus_scan_freq[1], s_stylus_scan_freq[2], s_stylus_scan_freq[3], s_stylus_scan_freq[4],
			s_stylus_scan_freq[5], s_stylus_scan_freq[6], s_stylus_scan_freq[7], s_stylus_scan_freq[8], s_stylus_scan_freq[9]);

	ret = cts_tcs_get_scan_rate(&num_scan_rate, s_scan_rate);
	if (ret) {
		THP_LOGE("Get scan rate failed");
		goto err_return;
	}
	s_scan_rate_num = 5;
	s_scan_rate[0] = 140;
	s_scan_rate[1] = 70;
	s_scan_rate[2] = 65;
	s_scan_rate[3] = 120;
	s_scan_rate[4] = 300;
	s_curr_scan_rate = s_scan_rate[0];
	THP_LOGI("scan_rate_num = %d", s_scan_rate_num);
	THP_LOGI("scan_rates = { %d, %d, %d, %d, %d }", s_scan_rate[0], s_scan_rate[1],
		s_scan_rate[2], s_scan_rate[3], s_scan_rate[4]);

	ret = cts_tcs_get_hw_id(&g_hwid);
	if (ret) {
		THP_LOGW("Read hwid failed");
		goto err_return;
	}

	THP_LOGI("%-22s", "chipone tx/rx info:");
	THP_LOGI("  %-22s = %d", "mst_tx_num", cts_dev_info.mst_tx_num);
	THP_LOGI("  %-22s = %d", "mst_rx_num", cts_dev_info.mst_rx_num);
	THP_LOGI("  %-22s = %d", "slv_tx_num", cts_dev_info.slv_tx_num);
	THP_LOGI("  %-22s = %d", "slv_rx_num", cts_dev_info.slv_rx_num);
#ifndef CTS_DEBUG_MODE
	dump_tx_rx_order(cts_dev_info.tx_tr_order, "tx_tr_order");
	dump_tx_rx_order(cts_dev_info.rx_tr_order, "rx_tr_order");
#endif

#ifdef CTS_DEBUG_MODE
	int i, rc;
	uint8_t rx_order[] = {86, 87, 83, 75, 112, 79, 109, 81, 84, 106, 77, 82,
						  103, 85, 100, 110, 89, 104, 98, 88, 101, 91, 96, 107,
						  99, 90, 113, 95, 92, 105, 102, 94, 111, 97, 93, 108,
						  22, 18, 24, 25, 27, 34, 31, 36, 41, 37, 35, 28,
						  32, 26, 29, 50, 49, 20, 52, 38, 23, 51, 33, 14,
						  30, 12, 16, 42, 48, 43, 47, 40, 45, 44, 46, 39};
	uint8_t tx_order[] = {80, 117, 76, 72, 123, 120, 116, 124, 122, 71, 73, 118, 121,
						  67, 68, 66, 70, 69, 65, 64, 63, 74, 119, 125, 62, 61,
						  60, 7, 58, 10, 9, 57, 55, 54, 6, 5, 0, 1, 2,
						  4, 3, 19, 21, 17, 15, 11, 59, 56, 53, 13, 8};
	for (i = 0; i < COLS; i++) {
		if (cts_dev_info.rx_tr_order[i] == rx_order[i]) {
		} else {
			THP_LOGI("TRX_oder_failed");
			dump_tx_rx_order(cts_dev_info.tx_tr_order, "tx_tr_order");
			dump_tx_rx_order(cts_dev_info.rx_tr_order, "rx_tr_order");
			rc = -1;
			break;
		}
	}
	if (rc)
		goto err_jump;
	for (i = 0; i < ROWS; i++) {
		if (cts_dev_info.tx_tr_order[i] == tx_order[i]) {
		} else {
			THP_LOGI("TRX_oder_failed");
			dump_tx_rx_order(cts_dev_info.tx_tr_order, "tx_tr_order");
			dump_tx_rx_order(cts_dev_info.rx_tr_order, "rx_tr_order");
			break;
		}
	}
err_jump:
#endif

	s_thp_hw_cap.num_col = cols;
	s_thp_hw_cap.num_row = rows;
	s_thp_hw_cap.num_button = 0;

	/* 0: col 1: row */
	s_thp_hw_cap.rx_direction = !!rx_direction;
	s_thp_hw_cap.rx_channel = rx_channel;
	/* 0: normal 1: interlace */
	s_thp_hw_cap.rx_slot_layout = 0;

	s_thp_hw_cap.pitch_size_um = 1000;

	//s_thp_hw_cap.num_scan_freq = 1;//num_scan_freq;
	//s_thp_hw_cap.scan_freq = &s_scan_freq_temp;//s_scan_freq;
	s_thp_hw_cap.num_scan_freq = num_scan_freq;
	s_thp_hw_cap.scan_freq = s_scan_freq;
	s_thp_hw_cap.num_scan_rate = s_scan_rate_num;
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

	s_thp_hw_cap.stylus_data_type = THP_AFE_STYLUS_IQ_LINE_DATA;
	s_thp_hw_cap.num_col_after_tied = 0;
	s_thp_hw_cap.num_row_after_tied = 0;

	THP_LOGI("%-22s", "hw_cap:");
	THP_LOGI("  %-22s = %d", "num_col", s_thp_hw_cap.num_col);
	THP_LOGI("  %-22s = %d", "num_row", s_thp_hw_cap.num_row);
	THP_LOGI("  %-22s = %d", "num_button", s_thp_hw_cap.num_button);
	THP_LOGI("  %-22s = %d", "rx_direction", s_thp_hw_cap.rx_direction);
	THP_LOGI("  %-22s = %d", "rx_channel", s_thp_hw_cap.rx_channel);
	THP_LOGI("  %-22s = %d", "rx_slot_layout", s_thp_hw_cap.rx_slot_layout);
	THP_LOGI("  %-22s = %d", "pitch_size_um", s_thp_hw_cap.pitch_size_um);
	THP_LOGI("  %-22s = %d", "num_scan_freq", s_thp_hw_cap.num_scan_freq);
	THP_LOGI("  %-22s = %d", "num_scan_rate", s_thp_hw_cap.num_scan_rate);
	THP_LOGI("  %-22s = %d", "feature_noise_detect", s_thp_hw_cap.feature_noise_detect);
	THP_LOGI("  %-22s = %d", "feature_freq_hop", s_thp_hw_cap.feature_freq_hop);
	THP_LOGI("  %-22s = %d", "feature_calibration", s_thp_hw_cap.feature_calibration);
	THP_LOGI("  %-22s = %d", "feature_wakeup_gesture", s_thp_hw_cap.feature_wakeup_gesture);
	THP_LOGI("  %-22s = %d", "sensor_arch", s_thp_hw_cap.sensor_arch);
	THP_LOGI("  %-22s = %d", "sensor_pattern", s_thp_hw_cap.sensor_pattern);
	THP_LOGI("  %-22s = %d", "stylus_protocol", s_thp_hw_cap.stylus_protocol);
	THP_LOGI("  %-22s = %d", "stylus_scan_freq_num", s_thp_hw_cap.stylus_scan_freq_num);
	THP_LOGI("  %-22s = %d", "feature_side_touch", s_thp_hw_cap.feature_side_touch);
	THP_LOGI("  %-22s = %d", "force_num", s_thp_hw_cap.force_num);
	THP_LOGI("  %-22s = %d", "stylus_data_type", s_thp_hw_cap.stylus_data_type);
	THP_LOGI("  %-22s = %d", "num_col_after_tied", s_thp_hw_cap.num_col_after_tied);
	THP_LOGI("  %-22s = %d", "num_row_after_tied", s_thp_hw_cap.num_row_after_tied);

err_return:
	/*
	 * tell firmware get hw cap done, avoid reading error
	 */
	if (!ret) {
		if (cts_tcs_set_hw_cap_ready(1)) {
			THP_LOGE("Set hw cap ready flag failed");
			ret = -1;
		}
	}

	return ret;
}

THP_AFE_HW_CAP_STRUCT *cts_get_hw_cap(void)
{
	THP_LOGI("cts_get_hw_cap +");
	return &s_thp_hw_cap;
}
