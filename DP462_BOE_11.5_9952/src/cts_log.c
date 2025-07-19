#include "cts_hal.h"
#include <stdio.h>
#include <sys/time.h>
//#include <android/log.h>
#include "cts_utils.h"
#include "cts_log.h"

#define CTS_LOG_BUF_SIZ		(1024 * 8)

#define _BUF_PTR				(buf + offset)
#define _BUF_SIZ				(sizeof(buf) - offset)

#ifndef __MUSL__
const char THP_LOG_LEVEL_MAP[] = {
	ANDROID_LOG_VERBOSE,		// THP_LOG_ERROR,
	ANDROID_LOG_ERROR,			// THP_LOG_WARN,
	ANDROID_LOG_WARN,			// THP_LOG_INFO,
	ANDROID_LOG_INFO,			// THP_LOG_DEBUG,
	ANDROID_LOG_DEBUG,		
};
#else
const char THP_LOG_LEVEL_MAP[] = {
	LOG_FATAL,
	LOG_ERROR,
	LOG_WARN,
	LOG_INFO,
	LOG_DEBUG,
};
#endif

static const char THP_LOG_LEVEL_MARK[] = {
	'N',	// PLACE_HOLDER,
	'E',	// THP_LOG_ERROR,
	'W',	// THP_LOG_WARN,
	'I',	// THP_LOG_INFO,
	'D',	// THP_LOG_DEBUG,
};

static uint8_t _log_level = THP_LOG_DEBUG;
static void (*_log_func)(const char *) = NULL;

int cts_set_log_callback_func(void (*log_func)(const char *))
{
	_log_func = log_func;
	return 0;
}

int cts_set_log_level(uint8_t log_level)
{
	_log_level = log_level;
	return 0;
}

int cts_thp_log(THP_LOG_LEVEL_ENUM log_level, \
		const char *func, const char *fmt, ...)
{
	va_list args;
	TIME_T tv;
	off_t offset = 0;
	char mark;
	char buf[CTS_LOG_BUF_SIZ];

#ifdef RELEASE

	if (!_log_func || (log_level > _log_level)) {
		return 0;
	}
#endif

	tv = GET_CURR_TIME();

	if ((log_level >= THP_LOG_ERROR) && (log_level <= THP_LOG_DEBUG)) {
		mark =  THP_LOG_LEVEL_MARK[log_level];
		offset += SNPRINTF(_BUF_PTR, _BUF_SIZ, "[%c]", mark);
	} else {
		offset += SNPRINTF(_BUF_PTR, _BUF_SIZ, "[%d]", log_level);
	}
	if (_BUF_SIZ > 0) {
		offset += SNPRINTF(_BUF_PTR, _BUF_SIZ, "[%ld]", TM2MS(tv));
	}
	if (_BUF_SIZ > 0) {
		offset += SNPRINTF(_BUF_PTR, _BUF_SIZ, "[%-s] ", func);
	}
	if (_BUF_SIZ > 0) {
		va_start(args, fmt);
		offset += vsnprintf(_BUF_PTR, _BUF_SIZ, fmt, args);
		va_end(args);
	}

#ifndef RELEASE
#ifndef __MUSL__
	int android_log_level;
	if ((log_level >= THP_LOG_ERROR) && (log_level <= THP_LOG_DEBUG)) {
		android_log_level = THP_LOG_LEVEL_MAP[log_level];
	} else {
		android_log_level = ANDROID_LOG_VERBOSE;
	}
	__android_log_print(android_log_level, LOG_TAG, "%s", buf);

	if (!_log_func) {
		return 0;
	}
#else	/*__MUSL__*/
#if 0
	int android_log_level;
	if ((log_level >= THP_LOG_ERROR) && (log_level <= THP_LOG_DEBUG)) {
		android_log_level = THP_LOG_LEVEL_MAP[log_level];
	} else {
		android_log_level = LOG_FATAL;
	}
#endif
	HILOGI("%{public}s", buf);

	if (!_log_func) {
		return 0;
	}
#endif	/*__MUSL__*/
#endif	/* RELEASE */

	if (_BUF_SIZ >= 2) {
		offset += SNPRINTF(_BUF_PTR, _BUF_SIZ, "%s", "\n");
	} else {
		*(_BUF_PTR - 1) = '\n';
	}

	_log_func(buf);

	return 0;
}

int cts_app_log(int prio, const char *func, const char *fmt, ...)
{
	va_list args;
	TIME_T tv;
	off_t offset = 0;
	char buf[CTS_LOG_BUF_SIZ];

	tv = GET_CURR_TIME();

	offset += SNPRINTF(_BUF_PTR, _BUF_SIZ, "[%ld][%-32s] ", TM2MS(tv), func);
	if (_BUF_SIZ > 0) {
		va_start(args, fmt);
		offset += vsnprintf(_BUF_PTR, _BUF_SIZ, fmt, args);
		va_end(args);
	}

#ifndef __MUSL__
	__android_log_print(prio, LOG_TAG, "%s", buf);
#else
	HILOGI("%{public}s", buf);
#endif
	return 0;
}

