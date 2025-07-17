/*
 * @file       cts_thp.c
 *
 * @authors    Chipone
 *
 * @par Description
 *   This is Touch Host Processing(THP) Analog Front End(AFE)
 *   Hardware Abstraction Layer(HAL) Chipone Implement file.
 *
 * Copyright (c) Chipone TechnoTHP_LOGIes Co., Ltd. 2021. All rights reserved.
 */

#include "thp_afe_hal.h"

///////////////////////////////////////////////////////////////////////////////
#include "cts_log.h"
#include "cts_core.h"
#include "cts_inspect.h"
#include "thp_ioctl.h"
#include "cts_utils.h"

uint8_t inspect_flag = INSPECT_NORMAL_FLAG;                   //for captest flow

// REMOVE THIS LATER
#define     TODO() \
    do { \
        CTS_THP_LOGE("WARN: unimplemented"); \
    } while (0)
///////////////////////////////////////////////////////////////////////////////


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
uint32_t thp_afe_hal_spec_version(void)
{
    CTS_THP_LOGD(".");
    return THP_AFE_HAL_SPEC_VERSION;
}

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
uint8_t thp_afe_hal_spec_major_version(void)
{
    CTS_THP_LOGD("");
    return THP_AFE_HAL_SPEC_MAJOR_VERSION;
}

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
uint8_t thp_afe_hal_spec_minor_version(void)
{
    CTS_THP_LOGD(".");
    return THP_AFE_HAL_SPEC_MINOR_VERSION;
}

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
uint8_t thp_afe_hal_spec_patch_version(void)
{
    CTS_THP_LOGI("+");
    return THP_AFE_HAL_SPEC_PATCH_VERSION;
}

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
THP_AFE_ERR_ENUM thp_afe_open(void)
{
    CTS_THP_LOGE("+");
    return cts_open() ? THP_AFE_ESTATE : THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_open_project(const char *proj_id)
{
    CTS_THP_LOGE("+, %s", proj_id);
    return cts_open_project(proj_id) ? THP_AFE_ESTATE : THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_close(void)
{
    CTS_THP_LOGE("+");
    return thp_dev_close()? THP_AFE_ESTATE:THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_start(void)
{
    CTS_THP_LOGE("+");
    if (cts_start() < 0)
    {
        return THP_AFE_EINVAL;
    }
    return THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_stop(void)
{
    CTS_THP_LOGE("+");
    if (cts_stop() < 0)
    {
        return THP_AFE_EINVAL;
    }
    return THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_screen_off(void)
{
    CTS_THP_LOGE("+");
    return cts_screen_off() ? THP_AFE_EOTHER : THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_screen_on(void)
{
    CTS_THP_LOGE("+");
    return cts_screen_on() ? THP_AFE_EOTHER : THP_AFE_OK;
}

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
THP_AFE_INFO_STRUCT *thp_afe_get_info(void)
{
    return cts_get_info();
}

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
THP_AFE_HW_CAP_STRUCT *thp_afe_get_hw_cap(void)
{
    return cts_get_hw_cap();
}

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
THP_AFE_FRAME_DATA_STRUCT *thp_afe_get_frame(void)
{
    if ( thp_dev_close_check() < 0 )
    {
        CTS_THP_LOGI("get frame , but device is closed");
    }
    return cts_get_frame();
}

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
THP_AFE_ERR_ENUM thp_afe_start_calibration(void)
{
    //yjl
    CTS_THP_LOGD("+ invalid");
    return THP_AFE_OK;
}

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
    THP_AFE_ERR_ENUM(*calibDataReadCallback)(void* dataPtr, uint32_t dataLen))
{
#if 1
    CTS_THP_LOGD("+ invalid");
    return THP_AFE_ESTATE ;

#else
    CTS_THP_LOGD("thp_afe_set_calib_data_callback_func");
    return cts_set_calib_data_callback_func( calibDataWriteCallback, calibDataReadCallback) ?  THP_AFE_ESTATE : THP_AFE_OK;
#endif
}

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
THP_AFE_ERR_ENUM thp_afe_clear_status(THP_AFE_STATUS_ENUM status)
{
    if ( inspect_flag || (!(current_afe_data_type == FRAME_TYPE_FINGER_0 || current_afe_data_type == FRAME_TYPE_FINGER_1) ) )
    {
        return cts_clear_status(status) ? THP_AFE_ESTATE : THP_AFE_OK;
    }

    if (status==THP_AFE_STATUS_CALIBRATION_DONE)
    {
        g_tcs_cmd[TCS_CLEAR_STATUS_CALIBRATE_DONE_LEVEL] = 1;
        return THP_AFE_OK;
    }

    if (  status==THP_AFE_STATUS_FREQ_SHIFT_DONE &&  last_Frame_index < 1000 )
    {
        g_tcs_cmd[TCS_CLEAR_STATUS_FREQ_SHIFT_LEVEL] = 1;
        return THP_AFE_OK;
    }

    return cts_clear_status(status) ? THP_AFE_ESTATE : THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_set_baseline_update_interval(uint16_t interval)
{
    return cts_set_baseline_update_interval(interval) ? THP_AFE_ESTATE : THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_set_idle_touch_threshold(uint16_t threshold)
{
    // CTS_THP_LOGD("idle threshold %d", threshold);
    // return cts_set_idle_touch_threshold(threshold) ? THP_AFE_ESTATE : THP_AFE_OK;
    return THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_set_idle_scan_rate(uint8_t rate)
{
    TODO();
    return THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_enter_idle(void)
{
    // CTS_THP_LOGI("enter idle");
    if ( inspect_flag || (!(current_afe_data_type == FRAME_TYPE_FINGER_0 || current_afe_data_type == FRAME_TYPE_FINGER_1) ) )
    {
        // return cts_enter_idle() ? THP_AFE_ESTATE : THP_AFE_OK;
        CTS_THP_LOGI("captest or no finger mode, exit");
        return THP_AFE_OK;
    }
    g_tcs_cmd[TCS_ENTER_IDLE_LEVEL] = 1;
    return THP_AFE_OK;
}

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
//#define  FORCE_IDLE_INVALID
THP_AFE_ERR_ENUM thp_afe_force_exit_idle(void)
{
#ifdef  FORCE_IDLE_INVALID
    CTS_THP_LOGD("+ invalid ");
    return  THP_AFE_OK;
#else

    CTS_THP_LOGD("+");
    return cts_exit_idle() ? THP_AFE_ESTATE : THP_AFE_OK;
#endif
}

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

THP_AFE_ERR_ENUM thp_afe_force_to_freq_point(uint8_t index)
{
    //yjl
    CTS_THP_LOGI("+index=%d", index);
    return cts_force_to_freq_point(index) ? THP_AFE_ESTATE : THP_AFE_OK;
}

/******************************************************************************
* Function: thp_afedisablefreq.shift()
*
* @summary
*   
* @return
*   THP_AFE_ERR_ENUM
*
* @param void
*   
* @note
*
*****************************************************************************/
//yjl
THP_AFE_ERR_ENUM thp_afe_disable_freq_shift(void)
{
    CTS_THP_LOGI("-");
    return cts_freq_shift_switch(0);
}
/******************************************************************************
* Function: thp_afe_enable_freq_shift()
*
* @summary
*   
* @return
*   THP_AFE_ERR_ENUM
*
* @param void
*   
* @note
*
*****************************************************************************/
THP_AFE_ERR_ENUM thp_afe_enable_freq_shift(void)
{

    CTS_THP_LOGI("+");
    return cts_freq_shift_switch(1);
}

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
THP_AFE_ERR_ENUM thp_afe_force_to_scan_rate(uint8_t index)
{
    // TODO();
    return THP_AFE_OK;
    // return cts_force_to_scan_rate(index) ? THP_AFE_ESTATE : THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_set_log_callback_func (void (*log_func)(const char *))
{
    CTS_THP_LOGD(".");
    if (cts_set_log_callback_func(log_func) < 0)
    {
        return THP_AFE_EINVAL;
    }

    return THP_AFE_OK;
}

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
THP_AFE_ERR_ENUM thp_afe_set_log_level(uint8_t log_level)
{
    CTS_THP_LOGD(".");
    if (cts_set_log_level(log_level) < 0)
    {
        return THP_AFE_EINVAL;
    }

    return THP_AFE_OK;
}

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
uint32_t thp_afe_inspect(void)
{
    return cts_inspect();
}

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
uint16_t *thp_afe_get_inspect_grid_data(void)
{
    return cts_get_inspect_grid_data();
}

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
uint16_t *thp_afe_get_inspect_line_data(void)
{
    return cts_get_inspect_line_data();
}

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
int16_t *thp_afe_get_inspect_noise(void)
{
#ifdef   TEST_NOISE
    return cts_get_inspect_noise();
#else
    return NULL;
#endif
}

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


// //bool thp_afe_is_esd_triggered(int nFiger)
// {

//     return cts_afe_is_esd_triggered();
//     return false;
// }



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


