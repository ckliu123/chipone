#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <time.h>

//#include <android/log.h>

#include "thp/thp_afe_hal.h"
#include "cts_hal.h"


#ifdef __MUSL__
#include <hilog/log.h>
#undef LOG_DOMAIN 
#define LOG_DOMAIN		0X3101
#define HILOGI(...) OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, LOG_TAG, __VA_ARGS__)
#else
#include <android/log.h>
#endif

struct fn_entry {
	THP_AFE_ERR_ENUM					(*set_calib_data_callback_func)(
    		THP_AFE_ERR_ENUM(*calibDataWriteCallback)(void* dataPtr, uint32_t dataLen),
    		THP_AFE_ERR_ENUM(*calibDataReadCallback)(void* dataPtr, uint32_t dataLen));

	uint8_t								(*spec_major_version)(void);
	uint8_t								(*spec_minor_version)(void);
	uint8_t								(*spec_patch_version)(void);
	THP_AFE_ERR_ENUM					(*open_project)(const char *proj_id);
	THP_AFE_ERR_ENUM					(*set_log_callback_func)(void (*log_func)(const char *));
	THP_AFE_ERR_ENUM					(*set_log_level)(uint8_t log_level);
	THP_AFE_ERR_ENUM					(*start)(void);

	THP_AFE_INFO_STRUCT *				(*get_info)(void);
	THP_AFE_HW_CAP_STRUCT *				(*get_hw_cap)(void);
	THP_AFE_FRAME_DATA_STRUCT *			(*get_frame)(void);
};

static const char *fn_name[] = {
	"thp_afe_set_calib_data_callback_func",

	"thp_afe_hal_spec_major_version",
	"thp_afe_hal_spec_minor_version",
	"thp_afe_hal_spec_patch_version",
	"thp_afe_open_project",
	"thp_afe_set_log_callback_func",
	"thp_afe_set_log_level",
	"thp_afe_start",

	"thp_afe_get_info",
	"thp_afe_get_hw_cap",
	"thp_afe_get_frame",
};

#define	ENTRY_SIZ						(sizeof(struct fn_entry) / sizeof(void *))

#define	ROWS_F							32
#define	COLS_F							18
#define	DMYLEN							8

struct info_pkg {
	THP_AFE_INFO_STRUCT					info;
};

struct hw_cap_pkg {
	THP_AFE_HW_CAP_STRUCT				hw_cap;
	uint16_t							scan_freq[DMYLEN];
	uint16_t							scan_rate[DMYLEN];
	uint16_t							stylus_scan_freq[DMYLEN];
};

struct frame_pkg {
	THP_AFE_FRAME_DATA_STRUCT			frame;
	uint16_t							grid_data[ROWS_F * COLS_F];
	uint16_t							line_data[ROWS_F + COLS_F];
	uint16_t							button_data[DMYLEN];
	uint16_t							noise_data[DMYLEN];
	uint16_t							side_data[DMYLEN];
	uint16_t							force_data[DMYLEN];

	THP_AFE_STYLUS_FRAME_DATA_STRUCT	stylus;
	uint16_t							tx1_line_data[(ROWS_F + COLS_F) * 2];
	uint16_t							tx2_line_data[(ROWS_F + COLS_F) * 2];
	uint16_t							stylus_noise[DMYLEN];
};

struct fdaemon {
	void								*handler;
	struct fn_entry						entry;
	FILE								*filep;

	char								projid[128];
	char								sopath[128];
	char								datapath[128];

	struct info_pkg						info_pkg;
	struct hw_cap_pkg					hw_cap_pkg;
	struct frame_pkg					frame_pkg;
};

#define THP_LOG_TAG		"aptouch_fdaemon"

static void log_func(const char *msg) {
/*#ifndef __MUSL__
	__android_log_print(ANDROID_LOG_ERROR, THP_LOG_TAG, "%s", msg);
#else
	HILOGI("%{public}s", msg);
#endif*/
	return;
}

static int run_afehal(struct fdaemon *pdaemon) {
	struct fn_entry *pentry = &pdaemon->entry;
	uint8_t major, minor, patch;
	int ret;

	major = pentry->spec_major_version();
	minor = pentry->spec_minor_version();
	patch = pentry->spec_patch_version();
	printf("major:%d, minor:%d, patch:%d\n", major, minor, patch);

	ret = pentry->open_project(pdaemon->projid);
	if (ret != THP_AFE_OK) {
		fprintf(stderr, "open project failed: %d\n", ret);
		return -1;
	}

	ret = pentry->set_log_callback_func(log_func);
	if (ret != THP_AFE_OK) {
		fprintf(stderr, "set log callback failed: %d\n", ret);
		return -1;
	}

	ret = pentry->set_log_level(4);
	if (ret != THP_AFE_OK) {
		fprintf(stderr, "set log level failed: %d\n", ret);
		return -1;
	}

	ret = pentry->set_calib_data_callback_func(NULL, NULL);
	if (ret != THP_AFE_OK) {
		fprintf(stderr, "set calib data callback failed: %d\n", ret);
		return -1;
	}

	ret = pentry->start();
	if (ret != THP_AFE_OK) {
		fprintf(stderr, "start failed: %d\n", ret);
		return -1;
	}

	return 0;
}

static int start_daemon(struct fdaemon *pdaemon) {
	void **pfn;
	time_t now;
	struct tm *ptm;
	int i;

	if (!pdaemon) {
		fprintf(stderr, "Invalid daemon!\n");
		return -1;
	}

	/*if (!pdaemon->projid) {
		fprintf(stderr, "Project id is empty!\n");
		return -1;
	}*/
	snprintf(pdaemon->sopath, sizeof(pdaemon->sopath),
			"/data/libafehal%s.so", pdaemon->projid);
	
	printf("Loading %s ...", pdaemon->sopath);

	pdaemon->handler = dlopen(pdaemon->sopath, RTLD_NOW);
	if (!pdaemon->handler) {
		fprintf(stderr, "Load %s failed: %s\n", pdaemon->sopath, strerror(errno));
		return -1;
	}

	pfn = (void **)&pdaemon->entry;
	for (i = 0; i < ENTRY_SIZ; i++) {
		pfn[i] = dlsym(pdaemon->handler, fn_name[i]);
		if (!pfn[i]) {
			fprintf(stderr, "Find symbol %s failed\n", fn_name[i]);
			return -1;
		}
	}

	now = time(&now);
	ptm = localtime(&now);
	snprintf(pdaemon->datapath, sizeof(pdaemon->datapath),
			"/data/thp_data_%04d%02d%02d_%02d%02d%02d.dat",
			ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
			ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	pdaemon->filep = fopen(pdaemon->datapath, "w+");
	if (!pdaemon->filep) {
		fprintf(stderr, "Open file %s failed: %s\n", pdaemon->datapath, strerror(errno));
		return -1;
	}

	printf("Daemon started\n");
	return run_afehal(pdaemon);
}

static void stop_daemon(struct fdaemon *pdaemon) {
	//dlclose(pdaemon->handler);
	printf("Stoped\n");
}

static void show_info(THP_AFE_INFO_STRUCT *pinfo) {
	printf(
		"info->vendor  = \"%s\";\n"
		"info->product = \"%s\";\n"
		"info->version = \"%s\";\n",
		pinfo->vendor_name,
		pinfo->product_name,
		pinfo->version);
}

static int save_info(struct fdaemon *pdaemon) {
	if (1 != fwrite(&pdaemon->info_pkg, sizeof(struct info_pkg), 1, pdaemon->filep)) {
		fprintf(stderr, "Save info failed");
		return -1;
	}
	fflush(pdaemon->filep);
	return 0;
}

static int process_info(struct fdaemon *pdaemon) {
	struct fn_entry *pentry = &pdaemon->entry;
	struct info_pkg *ppkg = &pdaemon->info_pkg;
	THP_AFE_INFO_STRUCT *pinfo = pentry->get_info();
	if (!pinfo) {
		fprintf(stderr, "Get info failed\n");
		return -1;
	}

	show_info(pinfo);
	memset(ppkg, 0, sizeof(*ppkg));
	memcpy(&ppkg->info, pinfo, sizeof(THP_AFE_INFO_STRUCT));

	return save_info(pdaemon);
}

static void show_hw_cap(THP_AFE_HW_CAP_STRUCT *phw_cap) {
	printf(
		"hw_cap->num_col				= %d;\n"
		"hw_cap->num_row				= %d;\n"
		"hw_cap->num_button			= %d;\n"
		"hw_cap->rx_direction			= %d;\n"
		"hw_cap->rx_channel			= %d;\n"
		"hw_cap->rx_slot_layout			= %d;\n"
		"hw_cap->pitch_size_um			= %d;\n"
		"hw_cap->num_scan_freq			= %d;\n"
		"hw_cap->num_scan_rate			= %d;\n"
		"hw_cap->feature_noise_detect		= %d;\n"
		"hw_cap->feature_freq_hop		= %d;\n"
		"hw_cap->feature_calibration		= %d;\n"
		"hw_cap->feature_wakeup_gesture		= %d;\n"
		"hw_cap->sensor_arch			= %d;\n"
		"hw_cap->sensor_pattern			= %d;\n"
		"hw_cap->stylus_protocol			= %d;\n"
		"hw_cap->stylus_scan_freq_num		= %d;\n"
		"hw_cap->feature_side_touch		= %d;\n"
		"hw_cap->force_num			= %d;\n"
		"hw_cap->stylus_data_type		= %d;\n"
		"hw_cap->num_col_after_tied		= %d;\n"
		"hw_cap->num_row_after_tied		= %d;\n",
		 phw_cap->num_col,
		 phw_cap->num_row,
		 phw_cap->num_button,
		 phw_cap->rx_direction,
		 phw_cap->rx_channel,
		 phw_cap->rx_slot_layout,
		 phw_cap->pitch_size_um,      
		 phw_cap->num_scan_freq,
		 phw_cap->num_scan_rate,      
		 phw_cap->feature_noise_detect,
		 phw_cap->feature_freq_hop,
		 phw_cap->feature_calibration,
		 phw_cap->feature_wakeup_gesture,
		 phw_cap->sensor_arch,        
		 phw_cap->sensor_pattern,
		 phw_cap->stylus_protocol,    
		 phw_cap->stylus_scan_freq_num,
		 phw_cap->feature_side_touch, 
		 phw_cap->force_num,          
		 phw_cap->stylus_data_type,   
		 phw_cap->num_col_after_tied,
		 phw_cap->num_row_after_tied); 
}

static int save_hw_cap(struct fdaemon *pdaemon) {
	if (1 != fwrite(&pdaemon->hw_cap_pkg, sizeof(struct hw_cap_pkg), 1, pdaemon->filep)) {
		fprintf(stderr, "Save hw_cap failed");
		return -1;
	}
	fflush(pdaemon->filep);
	return 0;
}

static int process_hw_cap(struct fdaemon *pdaemon) {
	struct fn_entry *pentry = &pdaemon->entry;
	struct hw_cap_pkg *ppkg = &pdaemon->hw_cap_pkg;
	THP_AFE_HW_CAP_STRUCT *phw_cap = pentry->get_hw_cap();
	if (!phw_cap) {
		fprintf(stderr, "Get hw_cap failed\n");
		return -1;
	}

	show_hw_cap(phw_cap);
	memset(ppkg, 0, sizeof(*ppkg));
	memcpy(&ppkg->hw_cap, phw_cap, sizeof(THP_AFE_HW_CAP_STRUCT));
	memcpy(ppkg->scan_freq, phw_cap->scan_freq, sizeof(uint16_t) * phw_cap->num_scan_freq);
	memcpy(ppkg->scan_rate, phw_cap->scan_freq, sizeof(uint16_t) * phw_cap->num_scan_rate);
	memcpy(ppkg->stylus_scan_freq, phw_cap->stylus_scan_freq, sizeof(uint16_t) * phw_cap->stylus_scan_freq_num);

	return save_hw_cap(pdaemon);
}

static void show_frame(THP_AFE_FRAME_DATA_STRUCT *pframe) {
		printf(
			"time_stamp=%0ld, "
			"frame_index=%6d, "
			"grid_data=%p, "
			"line_data=%p, "
			"button_data=%p, "
			"noise_data=%p, "
			"scan_freq=%3d, "
			"scan_rate=%3d, "
			"status=%08x, "
			"gesture=%08x, "
			"side_data=%p, "
			"force_data=%p, "
			"scan_state=%08x, ",
			pframe->time_stamp.tv_sec * 1000 + pframe->time_stamp.tv_usec / 1000,
			pframe->frame_index,
			(void *)pframe->grid_data,
			(void *)pframe->line_data,
			(void *)pframe->button_data,
			(void *)pframe->noise_data,
			pframe->scan_freq,
			pframe->scan_rate,
			pframe->status,
			pframe->gesture,
			(void *)pframe->side_data,
			(void *)pframe->force_data,
			pframe->scan_state);

		printf(
			"stylus=%p, { "
			"tx1_line_data=%p, "
			"tx2_lin2_data=%p, "
			"tx1_scan_freq=%3d, "
			"tx2_scan_freq=%3d, "
			"pressure=%4d, "
			"button=%08x, "
			"status=%08x, "
			"stylus_noise=%p, "
			"tx1_new_scan_freq=%3d, "
			"tx2_new_scan_freq=%3d, "
			"}\n",
			(void *)pframe->stylus,
			(void *)pframe->stylus->tx1_line_data,
			(void *)pframe->stylus->tx2_line_data,
			pframe->stylus->tx1_scan_freq,
			pframe->stylus->tx2_scan_freq,
			pframe->stylus->pressure,
			pframe->stylus->button,
			pframe->stylus->status,
			(void *)pframe->stylus->stylus_noise,
			pframe->stylus->tx1_new_scan_freq,
			pframe->stylus->tx2_new_scan_freq);
}

static int save_frame(struct fdaemon *pdaemon) {
	if (1 != fwrite(&pdaemon->frame_pkg, sizeof(struct frame_pkg), 1, pdaemon->filep)) {
		fprintf(stderr, "Save frame failed");
		return -1;
	}
	fflush(pdaemon->filep);
	return 0;
}

static int process_frame(struct fdaemon *pdaemon) {
	struct fn_entry *pentry = &pdaemon->entry;
	struct frame_pkg *ppkg = &pdaemon->frame_pkg;
	THP_AFE_FRAME_DATA_STRUCT *pframe;
	int i;

	for (i = 0; i < 10; i++) {
		pframe = pentry->get_frame();
		if (!pframe) {
			fprintf(stderr, "Get frame failed, cnt=%d\n", i);
		} else {
			break;
		}
	}

	if (!pframe) {
		return -1;
	}

	show_frame(pframe);

	memset(ppkg, 0, sizeof(*ppkg));
	memcpy(&ppkg->frame, pframe, sizeof(THP_AFE_FRAME_DATA_STRUCT));
	if (pframe->grid_data) {
		memcpy(ppkg->grid_data, pframe->grid_data, sizeof(ppkg->grid_data));
	}
	if (pframe->line_data) {
		memcpy(ppkg->line_data, pframe->line_data, sizeof(ppkg->line_data));
	}
	if (pframe->button_data) {
		memcpy(ppkg->button_data, pframe->button_data, sizeof(ppkg->button_data));
	}
	if (pframe->noise_data) {
		memcpy(ppkg->noise_data, pframe->noise_data, sizeof(ppkg->noise_data));
	}
	if (pframe->side_data) {
		memcpy(ppkg->side_data, pframe->side_data, sizeof(ppkg->side_data));
	}
	if (pframe->force_data) {
		memcpy(ppkg->force_data, pframe->force_data, sizeof(ppkg->force_data));
	}
	if (pframe->stylus) {
		memcpy(&ppkg->stylus, pframe->stylus, sizeof(THP_AFE_STYLUS_FRAME_DATA_STRUCT));
		if (pframe->stylus->tx1_line_data) {

			for (int i = 0; i < 68; i++) {
				printf("%05d ", pframe->stylus->tx1_line_data[i]);
				if (i == 23) {
					printf("\n");
				}
			}
			printf("\n");

			memcpy(ppkg->tx1_line_data, pframe->stylus->tx1_line_data, sizeof(ppkg->tx1_line_data));
		}
		if (pframe->stylus->tx2_line_data) {
			memcpy(ppkg->tx2_line_data, pframe->stylus->tx2_line_data, sizeof(ppkg->tx2_line_data));
		}
		if (pframe->stylus->stylus_noise) {
			memcpy(ppkg->stylus_noise, pframe->stylus->stylus_noise, sizeof(ppkg->stylus_noise));
		}
	}

	return save_frame(pdaemon);
}

int main(int argc, char *argv[]) {
	int ret;
	struct fdaemon daemon;

#if 0
	if (argc < 2) {
		printf("Usage: %s <PROJECT_ID>\n", argv[0]);
		return 0;
	}
#endif

	printf("info size: %ld, hw_cap size: %ld, frame size: %ld\n",
			sizeof(struct info_pkg),
			sizeof(struct hw_cap_pkg),
			sizeof(struct frame_pkg));

	memset(&daemon, 0, sizeof(struct fdaemon));
	snprintf(daemon.projid, sizeof(daemon.projid), "%s", "P519B71300");//argv[1]);
	//snprintf(daemon.projid, sizeof(daemon.projid), "%s", "P472B62900");//argv[1]);
	//snprintf(daemon.projid, sizeof(daemon.projid), "%s", "P315B71300");//argv[1]);

	ret = start_daemon(&daemon);
	if (ret < 0) {
		return -1;
	}

	ret = process_info(&daemon);
	if (ret < 0) {
		return -1;
	}

	ret = process_hw_cap(&daemon);
	if (ret < 0) {
		return -1;
	}

	do {
		ret = process_frame(&daemon);
		if (ret < 0) {
			break;
		}
	} while (true);

	stop_daemon(&daemon);
	return 0;
}
