#ifndef CTS_LOG_H
#define CTS_LOG_H

#include "cts_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

int cts_set_log_callback_func(void (*log_func)(const char *));
int cts_set_log_level(uint8_t log_level);

typedef enum {
	THP_LOG_ERROR		= 1,
	THP_LOG_WARNING		= 2,
	THP_LOG_INFO		= 3,
	THP_LOG_DEBUG		= 4,
} THP_LOG_LEVEL_ENUM;

int cts_thp_log(THP_LOG_LEVEL_ENUM log_level, \
		const char *func, const char *fmt, ...);

#define CTS_THP_LOGE(...) cts_thp_log(THP_LOG_ERROR, __func__, ## __VA_ARGS__)
#define CTS_THP_LOGW(...) cts_thp_log(THP_LOG_WARNING, __func__, ## __VA_ARGS__)
#define CTS_THP_LOGI(...) cts_thp_log(THP_LOG_INFO, __func__, ## __VA_ARGS__)
#define CTS_THP_LOGD(...) cts_thp_log(THP_LOG_DEBUG, __func__, ## __VA_ARGS__)

//#define RELEASE

#ifndef LOG_TAG
#ifdef  RELEASE
#define LOG_TAG         "afehal"
#else
#define LOG_TAG         "afehal_chipone" //"aptouch_chipone"
#endif
#endif

#ifndef __MUSL__
int cts_app_log(int prio, const char *func, const char *fmt, ...);

#define APP_LOGE(...) cts_app_log(ANDROID_LOG_ERROR, __func__, ## __VA_ARGS__)
#define APP_LOGW(...) cts_app_log(ANDROID_LOG_WARN, __func__, ## __VA_ARGS__)
#define APP_LOGI(...) cts_app_log(ANDROID_LOG_INFO, __func__, ## __VA_ARGS__)
#define APP_LOGD(...) cts_app_log(ANDROID_LOG_DEBUG, __func__, ## __VA_ARGS__)

#define All_LOG(...) do { \
	cts_thp_log(THP_LOG_ERROR, __func__, ## __VA_ARGS__); \
	cts_app_log(ANDROID_LOG_ERROR, __func__, ## __VA_ARGS__); \
} while (0);

#else	/* __MUSL__ */

#include <hilog/log.h>
#undef LOG_DOMAIN 
#define LOG_DOMAIN 0X3101
//#define HILOGI(...) OH_LOG_Print(LOG_TAG, LOG_INFO, LOG_DOMAIN, LOG_TAG, __VA_ARGS__)
#define HILOGI(...) OH_LOG_Print(LOG_APP, LOG_INFO, LOG_DOMAIN, LOG_TAG, __VA_ARGS__)

#define APP_LOGE(...) cts_app_log(LOG_ERROR, __func__, ## __VA_ARGS__)
#define APP_LOGW(...) cts_app_log(LOG_WARN, __func__, ## __VA_ARGS__)
#define APP_LOGI(...) cts_app_log(LOG_INFO, __func__, ## __VA_ARGS__)
#define APP_LOGD(...) cts_app_log(LOG_DEBUG, __func__, ## __VA_ARGS__)

#define All_LOG(...) do { \
	cts_thp_log(THP_LOG_ERROR, __func__, ## __VA_ARGS__); \
	cts_app_log(LOG_ERROR, __func__, ## __VA_ARGS__); \
} while (0);
#endif	/* __MUSL__ */

#define SHB_LOG(...)

#endif  /* CTS_LOG_H */

