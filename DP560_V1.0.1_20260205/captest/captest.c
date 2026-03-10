#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <time.h>
#include <unistd.h>

#define THP_LOG_TAG		"afehal_captest"

//#include "cts_tcs.h"
#include "thp/thp_afe_hal.h"
#include "cts_hal.h"

/*
#ifndef __MUSL__
#include <android/log.h>
#else
#include <hilog/log.h>
#undef LOG_DOMAIN 
#define LOG_DOMAIN		0X3101
#define HILOGI(...) OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, LOG_TAG, __VA_ARGS__)
#endif
*/
const char *func_names[] = {
	"thp_afe_hal_spec_major_version",
	"thp_afe_hal_spec_minor_version",
	"thp_afe_hal_spec_patch_version",
	"thp_afe_open_project",
	"thp_afe_set_log_callback_func",
	"thp_afe_set_log_level",

	"thp_afe_set_calib_data_callback_func",

	"thp_afe_start",
	"thp_afe_get_info",
	"thp_afe_get_hw_cap",
	"thp_afe_get_frame",

	"thp_afe_inspect",
	"thp_afe_get_inspect_grid_data",
	"thp_afe_get_inspect_line_data",
	"thp_afe_get_inspect_noise",
};

struct func_protos {
	uint8_t								(*spec_major_version)(void);
	uint8_t								(*spec_minor_version)(void);
	uint8_t								(*spec_patch_version)(void);
	THP_AFE_ERR_ENUM					(*open_project)(const char *proj_id);
	THP_AFE_ERR_ENUM					(*set_log_callback_func)(void (*log_func)(const char *));
	THP_AFE_ERR_ENUM					(*set_log_level)(uint8_t log_level);

	THP_AFE_ERR_ENUM					(*set_calib_data_callback_func)(
    		THP_AFE_ERR_ENUM(*calibDataWriteCallback)(void* dataPtr, uint32_t dataLen),
    		THP_AFE_ERR_ENUM(*calibDataReadCallback)(void* dataPtr, uint32_t dataLen));

	THP_AFE_ERR_ENUM				(*start)(void);
	THP_AFE_INFO_STRUCT *			(*get_info)(void);
	THP_AFE_HW_CAP_STRUCT *			(*get_hw_cap)(void);
	THP_AFE_FRAME_DATA_STRUCT *		(*get_frame)(void);

	uint32_t						(*inspect)(void);
	uint16_t *						(*get_inspect_grid_data)(void);
	uint16_t *						(*get_inspect_line_data)(void);
	int16_t *						(*get_inspect_noise)(void);
};


#define PROJECT_ID		"P519B71300"
//#define AFEHAL_SO_PATH	      "/odm/etc/firmware/ts/libafehalP315B71300.so"
#define AFEHAL_SO_PATH	      "/odm/etc/firmware/ts/libafehalP382B71300.so"

char *projid;
char sopath[128];
void *sohdlr;
void *funcs[sizeof(func_names) / sizeof(const char *)];

uint16_t grid_data[42 * 68];
uint16_t line_data[42 + 68];
uint16_t noise_data[3];
THP_AFE_STYLUS_FRAME_DATA_STRUCT stylus;
uint16_t tx1_line_data[42 + 68];
uint16_t tx2_line_data[42 + 68];
uint16_t stylus_noise[6];


static void log_func(const char *msg) {
/*#ifndef __MUSL__
	__android_log_print(ANDROID_LOG_ERROR, THP_LOG_TAG, "%s", msg);
#else
	HILOGI("%{public}s", msg);
#endif*/
	return;
}

int save_data = 1;


int save_info(FILE *filep, THP_AFE_INFO_STRUCT *info) {
	fwrite(info, sizeof(*info), 1, filep);
	fflush(filep);

	return 0;
}

int main(int argc, char *argv[]) {
	struct func_protos *pfn = (struct func_protos *)funcs;
	uint32_t inspect_result;

	// prepare
	projid = PROJECT_ID;
	snprintf(sopath, sizeof(sopath), "/data/libafehal%s.so", projid);
	printf("Project ID: %s, so file path: %s\n", projid, sopath);

	// load hal so file
	sohdlr = dlopen(sopath, RTLD_NOW);
	if (!sohdlr) {
		fprintf(stderr, "Load '%s' failed: %s", sopath, strerror(errno));
		return -1;
	}

	// lookup api
	for (int i = 0; i < sizeof(func_names) / sizeof(const char *); i++) {
		funcs[i] = dlsym(sohdlr, func_names[i]);
		if (!funcs[i]) {
			fprintf(stderr, "Lookup %s func failed: %s", func_names[i], strerror(errno));
			return -2;
		}
	}

	// call sequence
	printf("major: %d\n", pfn->spec_major_version());
	printf("minor: %d\n", pfn->spec_minor_version());
	printf("patch: %d\n", pfn->spec_patch_version());
	printf("open: %d\n", pfn->open_project(projid));
	printf("set_log_callback_func: %d\n", pfn->set_log_callback_func(log_func));
	printf("set_log_level: %d\n", pfn->set_log_level(4));
	printf("set_calib_data_callback_func: %d\n", pfn->set_calib_data_callback_func(NULL, NULL));

	// start afehal
	printf("start: %d\n",pfn->start());

	// get info
	THP_AFE_INFO_STRUCT *info = pfn->get_info();
	if (!info) {
		fprintf(stderr, "Get info failed\n");
		return -4;
	}
	printf("info: vendor: %s, product: %s, version: %s\n", info->vendor_name, info->product_name, info->version);

	// get hw_cap
	THP_AFE_HW_CAP_STRUCT *hw_cap = pfn->get_hw_cap();
	if (!hw_cap) {
		fprintf(stderr, "Get hw cap failed");
		return -5;
	}
	printf("hw_cap->num_col				= %d\n", hw_cap->num_col);
	printf("hw_cap->num_row				= %d\n", hw_cap->num_row);
	printf("hw_cap->num_button			= %d\n", hw_cap->num_button);
	printf("hw_cap->rx_direction			= %d\n", hw_cap->rx_direction);
	printf("hw_cap->rx_channel			= %d\n", hw_cap->rx_channel);
	printf("hw_cap->rx_slot_layout			= %d\n", hw_cap->rx_slot_layout);
	printf("hw_cap->pitch_size_um			= %d\n", hw_cap->pitch_size_um);
	printf("hw_cap->num_scan_freq			= %d\n", hw_cap->num_scan_freq);
	printf("hw_cap->num_scan_rate			= %d\n", hw_cap->num_scan_rate);
	printf("hw_cap->feature_noise_detect			= %d\n", hw_cap->feature_noise_detect);
	printf("hw_cap->feature_freq_hop			= %d\n", hw_cap->feature_freq_hop);
	printf("hw_cap->feature_calibration			= %d\n", hw_cap->feature_calibration);
	printf("hw_cap->feature_wakeup_gesture			= %d\n", hw_cap->feature_wakeup_gesture);
	printf("hw_cap->sensor_arch			= %d\n", hw_cap->sensor_arch);
	printf("hw_cap->sensor_pattern			= %d\n", hw_cap->sensor_pattern);
	printf("hw_cap->stylus_protocol			= %d\n", hw_cap->stylus_protocol);
	printf("hw_cap->stylus_scan_freq_num			= %d\n", hw_cap->stylus_scan_freq_num);
	printf("hw_cap->feature_side_touch			= %d\n", hw_cap->feature_side_touch);
	printf("hw_cap->force_num			= %d\n", hw_cap->force_num);
	printf("hw_cap->stylus_data_type			= %d\n", hw_cap->stylus_data_type);
	printf("hw_cap->num_col_after_tied			= %d\n", hw_cap->num_col_after_tied);
	printf("hw_cap->num_row_after_tied			= %d\n", hw_cap->num_row_after_tied);


	inspect_result = pfn->inspect();
	printf("inspect: %#010x\n", inspect_result);




	dlclose(sohdlr);

	return 0;
}

