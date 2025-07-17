#include "thp_afe_hal.h"
#include "thp_ioctl.h"
#include "cts_core.h"
#include "cts_log.h"
#include "cts_tcs.h"
#include "cts_utils.h"
#include "cts_inspect.h"

#define TEST_RAWDATA
#define TEST_OPEN
#define TEST_SHORT
#define TEST_NOISE
#define TEST_COMPCAP

#ifdef __MUSL__
#define	INT_MAX		0x7fffffff
#define	INT_MIN		(-0x7fffffff-1)
#endif

#define RAWDATA_TEST_FRAMES             1
#define RAWDATA_TEST_MIN                1380
#define RAWDATA_TEST_MAX                3780

#define OPEN_TEST_FRAMES                1
#define OPEN_TEST_MIN                   1600

#define SHORT_TEST_FRAMES               10
#define SHORT_TEST_MIN                  600

#define NOISE_TEST_FRAMES               16
#define NOISE_TEST_MAX                  140

#define COMPCAP_TEST_FRAMES             1
#define COMPCAP_TEST_MIN                1
#define COMPCAP_TEST_MAX                127

#define TOTAL_GRID_DATA_SIZE \
    ((RAWDATA_TEST_FRAMES + OPEN_TEST_FRAMES + SHORT_TEST_FRAMES) * \
     RAWDATA_SIZE)

#define TOTAL_NOISE_DATA_SIZE \
    (RAWDATA_SIZE * (NOISE_TEST_FRAMES + 1))

#define TOTAL_CNEG_DATA_SIZE  \
    (RAWDATA_NODES * COMPCAP_TEST_FRAMES)

static uint16_t *inspect_grid_data;
uint16_t  inspect_line_data[ROWS+COLS]= {0};

#ifdef   TEST_NOISE
static int16_t *inspect_noise_data;
#endif

int ret;
#ifdef   TEST_COMPCAP
static uint8_t *inspect_cneg_data;
#endif

#if (defined(TEST_RAWDATA) || defined(TEST_OPEN) ||defined(TEST_SHORT) ||defined(TEST_NOISE) ||defined(TEST_COMPCAP))
static void cts_dump_tsdata(const char *desc, int index, const uint16_t *data)
{
#define SPLIT_LINE_STR  "--------------------------------------------------------"
#define ROW_NUM_FORMAT_STR  "%2d|"
#define COL_NUM_FORMAT_STR  "%-5u"
#define DATA_FORMAT_STR     "%-5u"

    int r, c;
    uint32_t max, min, sum, average;
    int max_r, max_c, min_r, min_c;
    char line_buf[1024];
    int count = 0;

    max = min = data[0];
    sum = 0;
    max_r = max_c = min_r = min_c = 0;
    for (r = 0; r < ROWS; r++)
    {
        for (c = 0; c < COLS; c++)
        {
            uint16_t val = data[r * COLS + c];

            sum += val;
            if (val > max)
            {
                max = val;
                max_r = r;
                max_c = c;
            }
            else if (val < min)
            {
                min = val;
                min_r = r;
                min_c = c;
            }
        }
    }
    average = sum / (ROWS * COLS);
    count = 0;
    count += snprintf(line_buf + count, sizeof(line_buf) - count, " %s test data frame %u MIN: [%u][%u]=%u, MAX: [%u][%u]=%u, AVG=%u",  desc, index, min_r, min_c, min, max_r, max_c, max, average);
    CTS_THP_LOGI(SPLIT_LINE_STR);
    CTS_THP_LOGI("%s", line_buf);
    //CTS_THP_LOGI(SPLIT_LINE_STR);
    //CTS_THP_LOGI(SPLIT_LINE_STR);
    // count = 0;
    // count +=
    //    snprintf(line_buf + count, sizeof(line_buf) - count, "   |  ");
    //for (c = 0; c < COLS; c++)
    //{
    //    count += snprintf(line_buf + count, sizeof(line_buf) - count, COL_NUM_FORMAT_STR, c);
    // }
    // CTS_THP_LOGI("%s", line_buf);
    // CTS_THP_LOGI("%s", line_buf);
    // CTS_THP_LOGI(SPLIT_LINE_STR);
    // CTS_THP_LOGI(SPLIT_LINE_STR);

    for (r = 0; r < ROWS; r++)
    {
        count = 0;
        count += snprintf(line_buf + count, sizeof(line_buf) - count,ROW_NUM_FORMAT_STR, r);
        for (c = 0; c < COLS; c++)
        {
            count += snprintf(line_buf + count, sizeof(line_buf) - count, DATA_FORMAT_STR, data[c * ROWS + r]);
        }
        CTS_THP_LOGI("%s", line_buf);
    }
    CTS_THP_LOGI(SPLIT_LINE_STR);
#undef SPLIT_LINE_STR
#undef ROW_NUM_FORMAT_STR
#undef COL_NUM_FORMAT_STR
#undef DATA_FORMAT_STR
}

static void cts_dump_noisedata(const char *desc, int index, const uint16_t *data)
{
#define SPLIT_LINE_STR  "--------------------------------------------------------"
#define ROW_NUM_FORMAT_STR  "%2d|"
#define COL_NUM_FORMAT_STR  "%-5u"
#define DATA_FORMAT_STR     "%-4u"

    int r, c;
    uint32_t max, min, sum, average;
    int max_r, max_c, min_r, min_c;
    char line_buf[1024];
    int count = 0;

    max = min = data[0];
    sum = 0;
    max_r = max_c = min_r = min_c = 0;
    for (r = 0; r < ROWS; r++)
    {
        for (c = 0; c < COLS; c++)
        {
            uint16_t val = data[r * COLS + c];

            sum += val;
            if (val > max)
            {
                max = val;
                max_r = r;
                max_c = c;
            }
            else if (val < min)
            {
                min = val;
                min_r = r;
                min_c = c;
            }
        }
    }
    average = sum / (ROWS * COLS);
    count = 0;
    count += snprintf(line_buf + count, sizeof(line_buf) - count, " %s test data frame %u MIN: [%u][%u]=%u, MAX: [%u][%u]=%u, AVG=%u",  desc, index, min_r, min_c, min, max_r, max_c, max, average);
    CTS_THP_LOGI(SPLIT_LINE_STR);
    CTS_THP_LOGI("%s", line_buf);

    for (r = 0; r < ROWS; r++)
    {
        count = 0;
        count += snprintf(line_buf + count, sizeof(line_buf) - count,ROW_NUM_FORMAT_STR, r);
        for (c = 0; c < COLS; c++)
        {
            count += snprintf(line_buf + count, sizeof(line_buf) - count, DATA_FORMAT_STR, data[c * ROWS + r]);
        }
        CTS_THP_LOGI("%s", line_buf);
    }
    CTS_THP_LOGI(SPLIT_LINE_STR);
#undef SPLIT_LINE_STR
#undef ROW_NUM_FORMAT_STR
#undef COL_NUM_FORMAT_STR
#undef DATA_FORMAT_STR
}
#endif

#ifdef   TEST_COMPCAP
static void cts_dump_comp_cap(const char *desc, int index, uint8_t *cap)
{
#define SPLIT_LINE_STR "-----------------"
#define ROW_NUM_FORMAT_STR  "%2d|"
#define COL_NUM_FORMAT_STR  "%3u "
#define DATA_FORMAT_STR     "%3d"

    int r, c;
    uint32_t max, min, sum, average;
    int max_r, max_c, min_r, min_c;
    char line_buf[1024];
    int count;

    max = min = cap[0];
    sum = 0;
    max_r = max_c = min_r = min_c = 0;
    for (r = 0; r < ROWS; r++)
    {
        for (c = 0; c < COLS; c++)
        {
            uint16_t val = cap[r * COLS + c];
            sum += val;
            if (val > max)
            {
                max = val;
                max_r = r;
                max_c = c;
            }
            else if (val < min)
            {
                min = val;
                min_r = r;
                min_c = c;
            }
        }
    }
    average = sum / (ROWS * COLS);
    count = 0;
    count += snprintf(line_buf + count, sizeof(line_buf) - count," %s test data frame %d MIN: [%u][%u]=%u, MAX: [%u][%u]=%u, AVG=%u", desc, index, min_r, min_c, min, max_r, max_c, max, average);
    CTS_THP_LOGI(SPLIT_LINE_STR);
    CTS_THP_LOGI("%s", line_buf);
    //CTS_THP_LOGI(SPLIT_LINE_STR);
    // CTS_THP_LOGI(SPLIT_LINE_STR);
    //count = 0;
    //count += snprintf(line_buf + count, sizeof(line_buf) - count, "      ");
    //for (c = 0; c < COLS; c++)
    //{
    //    count += snprintf(line_buf + count, sizeof(line_buf) - count, COL_NUM_FORMAT_STR, c);
    //}
    //CTS_THP_LOGI("%s", line_buf);
    //CTS_THP_LOGI("%s", line_buf);
    //CTS_THP_LOGI(SPLIT_LINE_STR);
    //CTS_THP_LOGI(SPLIT_LINE_STR);
    for (r = 0; r < ROWS; r++)
    {
        count = 0;
        count += snprintf(line_buf + count, sizeof(line_buf) - count, ROW_NUM_FORMAT_STR, r);
        for (c = 0; c < COLS; c++)
        {
            count += snprintf(line_buf + count, sizeof(line_buf) - count, DATA_FORMAT_STR, cap[r * COLS + c]);
        }
        CTS_THP_LOGI("%s", line_buf);
    }
    CTS_THP_LOGI(SPLIT_LINE_STR);

#undef SPLIT_LINE_STR
#undef ROW_NUM_FORMAT_STR
#undef COL_NUM_FORMAT_STR
#undef DATA_FORMAT_STR
}
#endif

#if (defined(TEST_RAWDATA) || defined(TEST_OPEN) ||defined(TEST_SHORT) ||defined(TEST_NOISE) ||defined(TEST_COMPCAP))
static int cts_validate_tsdata(const char *desc, uint16_t *data, int min, int max)
{
#define SPLIT_LINE_STR \
    "------------------------------"

    int r, c;
    int failed_cnt = 0;

    CTS_THP_LOGI("%s thresh[0]=[%d, %d]", desc, min, max);

    for (r = 0; r < ROWS; r++)
    {
        for (c = 0; c < COLS; c++)
        {
            int offset = r * COLS + c;
            if ((data[offset] < min) || (data[offset] > max))
            {
                if (failed_cnt == 0)
                {
                    CTS_THP_LOGI(SPLIT_LINE_STR);
                    CTS_THP_LOGI("%s failed nodes:", desc);
                }
                failed_cnt++;
                CTS_THP_LOGI("  %3d: [%-2d][%-2d] = %u",
                         failed_cnt, r, c, data[offset]);

                if (failed_cnt > 10 )
                    break;
            }
        }
        if (failed_cnt > 10 )
            break;
    }

    if (failed_cnt)
    {
        CTS_THP_LOGI(SPLIT_LINE_STR);
        CTS_THP_LOGI("%s test %d node total failed", desc, failed_cnt);
    }
    return failed_cnt;

#undef SPLIT_LINE_STR
}
#endif

#ifdef   TEST_COMPCAP
static int cts_validate_comp_cap(const char *desc, uint8_t *cap, int min, int max)
{
#define SPLIT_LINE_STR \
    "------------------------------"

    int r, c;
    int failed_cnt = 0;

    CTS_THP_LOGI("Validate %s data: thresh[0]=[%d, %d]", desc, min, max);

    for (r = 0; r < ROWS; r++)
    {
        for (c = 0; c < COLS; c++)
        {
            int offset = r * COLS + c;

            if ((cap[offset] < min) || (cap[offset] > max))
            {
                if (failed_cnt == 0)
                {
                    CTS_THP_LOGI(SPLIT_LINE_STR);
                    CTS_THP_LOGI("%s failed nodes:", desc);
                }
                failed_cnt++;
                CTS_THP_LOGI("  %3d: [%-2d][%-2d] = %u", failed_cnt, r, c, cap[offset]);
                if (failed_cnt > 10 )
                    break;
            }
        }

        if (failed_cnt > 10 )
            break;
    }

    if (failed_cnt)
    {
        CTS_THP_LOGI(SPLIT_LINE_STR);
        CTS_THP_LOGI("%s test %d node total failed", desc, failed_cnt);
    }

    return failed_cnt;
#undef SPLIT_LINE_STR
}
#endif

#ifdef   TEST_OPEN
static int cts_prepare_openshort_test(void)
{
    int ret;

    //open test --step1. reset
    cts_reset_device();

    //step 1 delay --- yjl add --- calbrate need 260ms
    CTS_THP_LOGI("switch normal work , cablibrate need 260ms --delay--");
    mdelay(50);

    // step2  wait normal mode
    ret = wait_to_norm(150,10);//TP_STD_CMD_SYS_STS_KRANG_CURRENT_WORKMODE_RO
    if (ret)
    {
        CTS_THP_LOGE("Wait firmware to normal work failed %d", ret);
        return ret;
    }
    CTS_THP_LOGI("Wait firmware to normal work OK");

    // cts_tcs_disable_mnt();
    // mdelay(5);

    //CTS_FIRMWARE_WORK_MODE_CFG 6
    ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE_CFG);// TP_STD_CMD_SYS_STS_KRANG_WORK_MODE_RW
    if (ret)
    {
        CTS_THP_LOGE("Set firmware work mode to WORK_MODE_CONFIG failed %d", ret);
        return ret;
    }

    mdelay(50);     //620
    //add yjl
    // step2. 1.disable fw check esd  2.disable fw calibrate check
    //cts_tcs_disable_esd_ddi_check();
    //cts_tcs_disable_esd_diff_check();
    //cts_tcs_disable_CalibratonCheck();

    //donnot delay
    // get cfg mode
    ret = wait_to_cfg_mode();
    if (ret)
    {
        CTS_THP_LOGE("Wait firmware to cfg work failed %d", ret);
        return ret;
    }

#if  1
    ret = cts_tcs_set_product_en();
    if (ret)
    {
        CTS_THP_LOGE("Set product en failed %d", ret);
        return ret;
    }
#else

    if (1)
    {
        static  int  prodructenretrymax=0;
        int cnt;
        for (cnt=0; cnt<10; cnt++)
        {
            mdelay(cnt*10);
            ret = cts_tcs_set_product_en();
            mdelay(cnt*10);

            if (ret==0)
            {
                break;
            }
            else
            {

            }
        }

        if ( prodructenretrymax< cnt  )
            prodructenretrymax = cnt;

        CTS_THP_LOGI("+++++++++++++ prodructenretry %d", cnt);
        CTS_THP_LOGI("prodructenretrymax+++++++++++++%d", prodructenretrymax);
    }
    if (ret)
    {
        CTS_THP_LOGE("Set product en failed %d", ret);
        CTS_THP_LOGE("Set product en failed %d", ret);
        return ret;
    }
#endif
    return 0;
}
#endif

#ifdef TEST_SHORT
#if 0
static int cts_prepare_short_test(void)    //2024/11/29 modified by mbteng 与open 共用
{

    int ret;

    cts_reset_device();

    //step 1 delay --- yjl add --- calbrate need 200ms
    CTS_THP_LOGI("switch normal work , cablibrate need 200ms --delay--");
    mdelay(50);

    // step2  wait normal mode
    ret = wait_to_norm(150,10);//TP_STD_CMD_SYS_STS_KRANG_CURRENT_WORKMODE_RO
    if (ret)
    {
        CTS_THP_LOGE("Wait firmware to normal work failed %d", ret);
        CTS_THP_LOGE("Wait firmware to normal work failed %d", ret);
        return ret;
    }
    CTS_THP_LOGI("Wait firmware to normal work OK");

    // cts_tcs_disable_mnt();

    //add yjl
    // step2. 1.disable fw check esd  2.disable fw calibrate check
    //cts_tcs_disable_esd_ddi_check();
    //cts_tcs_disable_esd_diff_check();
    //cts_tcs_disable_CalibratonCheck();



    //CTS_FIRMWARE_WORK_MODE_CFG 6
    ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE_CFG);// TP_STD_CMD_SYS_STS_KRANG_WORK_MODE_RW
    if (ret)
    {
        CTS_THP_LOGE("Set firmware work mode to WORK_MODE_CONFIG failed %d", ret);
        CTS_THP_LOGE("Set firmware work mode to WORK_MODE_CONFIG failed %d", ret);
        return ret;
    }

    //donnot delay
    // get cfg mode
    ret = wait_to_cfg_mode();
    if (ret)
    {
        CTS_THP_LOGE("Wait firmware to cfg work failed %d", ret);
        CTS_THP_LOGE("Wait firmware to cfg work failed %d", ret);
        return ret;
    }

#if  1
    ret = cts_tcs_set_product_en();
#else

    if (1)
    {
        static  int  prodructenretrymax=0;
        int cnt;
        for (cnt=0; cnt<10; cnt++)
        {
            mdelay(cnt*10);
            ret = cts_tcs_set_product_en();
            mdelay(cnt*10);

            if (ret==0)
            {
                break;
            }
            else
            {

            }
        }

        if ( prodructenretrymax< cnt  )
            prodructenretrymax = cnt;

        CTS_THP_LOGI("+++++++++++++ prodructenretry %d", cnt);
        CTS_THP_LOGI("prodructenretrymax+++++++++++++%d", prodructenretrymax);
    }
#endif

    if (ret)
    {
        CTS_THP_LOGE("Set product en failed %d", ret);
        CTS_THP_LOGE("Set product en failed %d", ret);
        return ret;
    }

    return 0;
}
#endif
#endif

#if (defined(TEST_RAWDATA) || defined(TEST_OPEN) ||defined(TEST_SHORT) ||defined(TEST_NOISE) ||defined(TEST_COMPCAP))
static int cts_prepare_test(void)
{
    int ret;
    cts_reset_device();

    //step 1 delay --- yjl add --- calbrate need 200ms
    CTS_THP_LOGI("switch normal work , cablibrate need 200ms --delay--");
    mdelay(50);

    // step2  wait normal mode
    ret = wait_to_norm(150,10);//TP_STD_CMD_SYS_STS_KRANG_CURRENT_WORKMODE_RO
    if (ret)
    {
        CTS_THP_LOGE("Wait firmware to normal work failed %d", ret);
        return ret;
    }
    CTS_THP_LOGI("Wait firmware to normal work OK");
    // cts_tcs_disable_mnt();


    //add yjl
    // step2. 1.disable fw check esd  2.disable fw calibrate check
    //cts_tcs_disable_esd_ddi_check();
    //cts_tcs_disable_esd_diff_check();
    //cts_tcs_disable_CalibratonCheck();
    //CTS_FIRMWARE_WORK_MODE_CFG 6
    ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE_CFG);// TP_STD_CMD_SYS_STS_KRANG_WORK_MODE_RW
    if (ret)
    {
        CTS_THP_LOGE("Set firmware work mode to WORK_MODE_CONFIG failed %d", ret);
        return ret;
    }

    //donnot delay
    // get cfg mode
    ret = wait_to_cfg_mode();
    if (ret)
    {
        CTS_THP_LOGE("Wait firmware to cfg work failed %d", ret);
        return ret;
    }

#if  1
    ret = cts_tcs_set_product_en();
#else

    if (1)
    {
        static  int  prodructenretrymax=0;
        int cnt;
        for (cnt=0; cnt<10; cnt++)
        {
            mdelay(cnt*10);
            ret = cts_tcs_set_product_en();
            mdelay(cnt*10);

            if (ret==0)
            {
                break;
            }
            else
            {

            }
        }

        if ( prodructenretrymax< cnt  )
            prodructenretrymax = cnt;

        CTS_THP_LOGI("+++++++++++++ prodructenretry %d", cnt);
        CTS_THP_LOGI("prodructenretrymax+++++++++++++%d", prodructenretrymax);
    }
#endif

    if (ret)
    {
        CTS_THP_LOGE("Set product en failed %d", ret);
        return ret;
    }

    // CTS_FIRMWARE_WORK_MODE_NORM 0
    ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE_NORM);//TP_STD_CMD_SYS_STS_KRANG_WORK_MODE_RW
    if (ret)
    {
        CTS_THP_LOGE("Set firmware work mode to WORK_MODE_CONFIG failed %d", ret);
        return ret;
    }

    //step 1 delay --- yjl add --- calbrate need 200ms
    CTS_THP_LOGI("switch normal work , cablibrate need 200ms --delay--");
    mdelay(50);

    // step2  wait normal mode
    ret = wait_to_norm(150,10);//TP_STD_CMD_SYS_STS_KRANG_CURRENT_WORKMODE_RO
    if (ret)
    {
        CTS_THP_LOGE("Wait firmware to normal work failed %d", ret);
        return ret;
    }
    return 0;
}
#endif

#ifdef TEST_RAWDATA
static int cts_inspect_rawdata(void)
{
    struct timeval start_time, end_time, delta_time;
    int frame;
    uint16_t *rawdata;
    int i;
    int ret;

    gettimeofday(&start_time, NULL);
    CTS_THP_LOGI("Prepare test  rawdata");

    ret = cts_prepare_test();
    if (ret)
    {
        CTS_THP_LOGE("Prepare test rawdata failed %d", ret);
        return -1;
    }

    mdelay(5);
    ret = cts_tcs_set_int_data_types(INT_DATA_TYPE_RAWDATA);
    if (ret)
    {
        mdelay(5);
        ret = cts_tcs_set_int_data_types(INT_DATA_TYPE_RAWDATA);
        if (ret)
        {
            CTS_THP_LOGE("set int data type failed %d", ret);
        }
    }

    mdelay(5);
    ret = cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);
    if (ret)
    {
        mdelay(5);
        ret = cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);
        if (ret)
        {
            CTS_THP_LOGE("set int data mode failed %d", ret);
        }
    }

    mdelay(20);

    for (frame = 0; frame < RAWDATA_TEST_FRAMES; frame++)
    {
        bool data_valid = false;
        rawdata = (uint16_t *)inspect_grid_data + RAWDATA_NODES * frame;
        for (i = 0; i < 3; i++)
        {
            ret = cts_test_polling_rawdata(rawdata, RAWDATA_SIZE);
            if (ret < 0)
            {
                CTS_THP_LOGE("Get raw data failed: %d", ret);
            }
            else
            {
                data_valid = true;
                break;
            }
        }

        if (!data_valid)
        {
            ret = -1;
            break;
        }
        cts_dump_tsdata("Rawdata", frame + 1, rawdata);
        ret = cts_validate_tsdata("Rawdata", rawdata, RAWDATA_TEST_MIN, RAWDATA_TEST_MAX);
        if (ret)
        {
            CTS_THP_LOGE("Rawdata test has %d nodes failed", ret);
        }
    }
    gettimeofday(&end_time, NULL);
    timersub(&end_time, &start_time, &delta_time);
    CTS_THP_LOGI("Rawdata test cost %ldms", delta_time.tv_sec * 1000 + delta_time.tv_usec / 1000);

    return ret;
}
#endif

#ifdef TEST_OPEN
static int cts_inspect_open(void)
{
    struct timeval start_time, end_time, delta_time;
    uint16_t *rawdata = (uint16_t *)inspect_grid_data + RAWDATA_NODES * RAWDATA_TEST_FRAMES;
    int ret;

    gettimeofday(&start_time, NULL);
    CTS_THP_LOGI("Prepare test  open");

    ret = cts_prepare_openshort_test();
    if (ret)
    {
        CTS_THP_LOGE("Prepare test open failed %d", ret);
        return -1;
    }

    mdelay(5);
    ret = cts_tcs_set_int_data_types(INT_DATA_TYPE_RAWDATA);
    if (ret)
    {
        mdelay(5);
        ret = cts_tcs_set_int_data_types(INT_DATA_TYPE_RAWDATA);
        if (ret)
        {
            CTS_THP_LOGE("set int data type failed %d", ret);
        }
    }

    mdelay(5);
    ret = cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);
    if (ret)
    {
        mdelay(5);
        ret = cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);

        if (ret)
        {
            CTS_THP_LOGE("set int data mode failed %d", ret);
        }
    }
    mdelay(5);
    ret = cts_tcs_set_openshort_mode(CTS_TEST_OPEN);
    if (ret)
    {
        CTS_THP_LOGE("Set test type to OPEN_TEST failed %d", ret);
        return -5;
    }
    mdelay(5);

    ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE_OPEN_SHORT);
    if (ret)
    {
        CTS_THP_LOGE("Set firmware work mode to WORK_MODE_OPEN_SHORT failed %d", ret);
        return -4;
    }

#if  0
    ret = wait_to_curr_mode(150, 10);
    if (ret)
    {
        CTS_THP_LOGE("wait_to_curr_mode failed %d", ret);
        CTS_THP_LOGE("wait_to_curr_mode failed %d", ret);
        return -5;
    }
#else
    if (1)
    {
        int retries = 3 ;
        //step 1 delay --- yjl add --- switch need 100ms
        //CTS_THP_LOGI("switch mode , need 100ms --delay--");
        mdelay(50);
        //step 2 wait mode
        do
        {
            ret = wait_to_curr_mode(150,10);
        }
        while (ret  && retries--);
        if (ret)
        {
            CTS_THP_LOGE("wait_to_curr_mode failed %d", ret);
            return -5;
        }
    }
#endif

    mdelay(20);
    ret = cts_test_polling_rawdata(rawdata, RAWDATA_SIZE);
    if (ret)
    {
        CTS_THP_LOGE("Read test result failed %d", ret);
        return -6;
    }

    cts_dump_tsdata("Open-circuit", 1, rawdata);
    ret = cts_validate_tsdata("Open-circuit", rawdata, OPEN_TEST_MIN, INT_MAX);
    gettimeofday(&end_time, NULL);
    timersub(&end_time, &start_time, &delta_time);
    CTS_THP_LOGI("Open test cost %ldms", delta_time.tv_sec * 1000 + delta_time.tv_usec / 1000);

    return ret;
}
#endif


#ifdef TEST_SHORT
#define CTS_9952_ROW_SHORT_TEST
#define SHORT_GND_TEST_LOOP             1
#define SHORT_COLS_TEST_LOOP            1
#ifdef CTS_9952_ROW_SHORT_TEST
#define SHORT_ROWS_TEST_LOOP            8
#else
#define SHORT_ROWS_TEST_LOOP            1 //3
#endif
#define SHORT_TEST_LOOP_COUNT           10

#define  DEBUG_INFO   0  // 1
uint16_t shortdatabackup[ROWS*COLS];
int error_flag=0;

void check_short_data_disable(void)
{
    error_flag=1;
}

void check_short_data_enable(void)
{
    error_flag=0;
}

int get_short_data_status(void)
{
    return error_flag;
}

void check_short_data(uint16_t *rawdata)
{
    if (get_short_data_status()==1)
        return ;

#if  0// debug info
    if (1)
    {
        int row,col;
        CTS_THP_LOGI("setshortdata");
        for (row=0; row<48; row++)
        {
            for (col=0; col<32; col++)
            {
#if  0
                rawdata[row*32+col]= row*32+col+1;
#else

                rawdata[row*32+col]= 1;

                if (row>0 && row<23)
                {
                    rawdata[row*32+col]= 216;
                }
                else
                {
                    rawdata[row*32+col]= 2000;
                }
#endif
            }
        }
    }
#endif

#if  1// 
    if (1)
    {
        int row,col;
        uint16_t datatemp;
        uint16_t flag1_217_cnt=0;
        uint16_t flag2_part1_big_cnt=0;
        uint16_t flag3_part1_small_cnt=0;
        uint16_t flag4_part2_big_cnt=0;
        uint16_t flag5_part2_small_cnt=0;

        uint16_t short_max=0;

        memset(shortdatabackup,0,sizeof(shortdatabackup));

        for (row=0; row<ROWS; row++)
        {
            for (col=0; col<COLS; col++)
            {
                datatemp = rawdata[row*COLS+col];
                // max
                if ( datatemp > short_max )
                    short_max = datatemp;

                // 217
                if (  ( row !=0 )  && ( row != (ROWS-1))
                      &&( datatemp == rawdata[row*COLS+col+1] )
                      &&  datatemp < 2000
                      &&  datatemp > 10
                   )
                {
                    flag1_217_cnt++;
                    if ( datatemp == 217 )
                    {
                        flag1_217_cnt++;
                    }
                }

                //
                if ( row < 24)
                {
                    if ( datatemp > 2044/3 )
                    {
                        flag2_part1_big_cnt++;
                    }
                    else if ( datatemp < 2044/5 )
                    {
                        flag3_part1_small_cnt++;
                    }
                }
                else
                {
                    if ( datatemp > 2044/3 )
                    {
                        flag4_part2_big_cnt++;
                    }
                    else if ( datatemp < 2044/5 )
                    {
                        flag5_part2_small_cnt++;
                    }
                }
            }
        }

        if ((short_max > 2044/2) && ((  flag1_217_cnt >  150  ) || ( error_flag &&  flag1_217_cnt >  100)) )
        {
            if ( (( flag2_part1_big_cnt > (2*flag4_part2_big_cnt) ) && ( flag5_part2_small_cnt > 150))
                 || (((flag2_part1_big_cnt *2 ) < flag4_part2_big_cnt ) && ( flag3_part1_small_cnt > 150))
               )
            {
                error_flag =1;
            }
        }

#if  DEBUG_INFO
        CTS_THP_LOGI("short_error_flag %d,short_max %d,flag1_217_cnt %d,flag2_part1_big_cnt %d,flag3_part1_small_cnt%d,flag4_part2_big_cnt %d,flag5_part2_small_cnt%d ", error_flag,short_max,flag1_217_cnt,
                 flag2_part1_big_cnt,flag3_part1_small_cnt,flag4_part2_big_cnt,flag5_part2_small_cnt);
#endif
        if ( error_flag ==1 )
        {
            int row,col;
            //struct timeval start_time;
            struct timeval end_time;
            for (row=0; row<ROWS; row++)
            {
                // gettimeofday(&start_time, NULL);
                for (col=0; col<COLS; col++)
                {
                    gettimeofday(&end_time, NULL);

                    shortdatabackup[row*COLS+col] = rawdata[row*COLS+col];
                    rawdata[row*COLS+col] = 2000+  (end_time.tv_usec %45);
                }
            }
        }
    }
#endif
}

static int cts_inspect_short(void)
{
    struct timeval start_time, end_time, delta_time;
    int loopcnt;
    int ret;
    uint16_t *rawdata = NULL;

    gettimeofday(&start_time, NULL);

    CTS_THP_LOGI("Prepare test  short");

    ret = cts_prepare_openshort_test();//cts_prepare_test();
    if (ret)
    {
        CTS_THP_LOGE("Prepare test short failed %d", ret);
        return -1;
    }

    //add yjl
    //check_short_data_enable();
    CTS_THP_LOGI("Test short to UNDEFINED");
    ret = cts_tcs_set_short_test_type(CTS_SHORT_TEST_UNDEFINED);
    if (ret)
    {
        CTS_THP_LOGE("Set short test type to CTS_SHORT_TEST_UNDEFINED failed %d", ret);
        return -5;
    }
    ret = cts_tcs_set_int_data_types(INT_DATA_TYPE_RAWDATA);
    if (ret)
    {
        mdelay(5);
        ret = cts_tcs_set_int_data_types(INT_DATA_TYPE_RAWDATA);
        if (ret)
        {
            CTS_THP_LOGE("set int data type failed %d", ret);
        }
    }
    mdelay(5);
    ret = cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);
    if (ret)
    {
        mdelay(5);
        ret = cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);

        if (ret)
        {
            CTS_THP_LOGE("set int data mode failed %d", ret);
        }
    }
    mdelay(5);
    ret = cts_tcs_set_openshort_mode(CTS_TEST_SHORT);
    if (ret)
    {
        CTS_THP_LOGE("Set test type to SHORT failed %d", ret);
        return -4;
    }
    mdelay(5);
	ret = cts_tcs_set_work_mode(CTS_FIRMWARE_WORK_MODE_OPEN_SHORT);
    if (ret)
    {
        CTS_THP_LOGE("Set firmware work mode to WORK_MODE_OPEN_SHORT failed %d", ret);
        return -2;
    }

#if  0
    ret = wait_to_curr_mode(150,10);
    if (ret)
    {

        CTS_THP_LOGE("wait_to_curr_mode failed %d", ret);
        CTS_THP_LOGE("wait_to_curr_mode failed %d", ret);
        return -5;
    }
#else
    if (1)
    {
        int retries = 3;

        //step 1 delay --- yjl add --- calbrate need 200ms
        //CTS_THP_LOGI("switch mode need 200ms --delay--");
        mdelay(50);

        //step2  wait mode
        do
        {
            ret = wait_to_curr_mode(150,10);
        }
        while (ret  && retries--);

        if (ret)
        {
            CTS_THP_LOGE("wait_to_curr_mode failed %d", ret);
            return -3;
        }
    }
#endif
    /*
     * Short to ground
     */
    CTS_THP_LOGI("Test short to GND");
    ret = cts_tcs_set_short_test_type(CTS_SHORT_TEST_BETWEEN_GND);
    if (ret)
    {
        CTS_THP_LOGE("Set short test type to SHORT_TO_GND failed %d", ret);
        return -6;
    }
    rawdata = (uint16_t *)inspect_grid_data +
              RAWDATA_NODES * (RAWDATA_TEST_FRAMES + OPEN_TEST_FRAMES);
    mdelay(20);
    ret = cts_test_polling_rawdata(rawdata, RAWDATA_NODES);
    if (ret)
    {
        CTS_THP_LOGE("Read test result failed %d", ret);
        return -7;
    }

    //check_short_data(rawdata);
    cts_dump_tsdata("GND-short", 1, rawdata);
    ret = cts_validate_tsdata("GND-short", rawdata, SHORT_TEST_MIN, INT_MAX);
    if (ret)
    {
        CTS_THP_LOGE("Short to GND test failed %d", ret);
        return ret;
    }

    /*
     * Short between colums
     */

    CTS_THP_LOGI("Test short between columns");

    // 11,3 write 1
    // 2,3 read
    //2,3 write 0
    ret = cts_tcs_set_short_test_type(CTS_SHORT_TEST_BETWEEN_COLS);
    if (ret)
    {
        CTS_THP_LOGE("Set short test type to BETWEEN_COLS failed %d", ret);
        return -8;
    }
    mdelay(20);
    for (loopcnt = 0; loopcnt < SHORT_COLS_TEST_LOOP; loopcnt++)
    {
        rawdata = (uint16_t *)inspect_grid_data +
                  RAWDATA_NODES *
                  (RAWDATA_TEST_FRAMES + OPEN_TEST_FRAMES + SHORT_GND_TEST_LOOP + loopcnt);

        ret = cts_test_polling_rawdata(rawdata, RAWDATA_SIZE);
        if (ret)
        {
            CTS_THP_LOGE("Read test result failed %d", ret);
            return -9;
        }
        mdelay(5);
    }
    //check_short_data(rawdata);
    for (loopcnt = 0; loopcnt < SHORT_COLS_TEST_LOOP; loopcnt++)
    {
        rawdata =(uint16_t *)inspect_grid_data + RAWDATA_NODES *
                 (RAWDATA_TEST_FRAMES + OPEN_TEST_FRAMES + SHORT_GND_TEST_LOOP + loopcnt);
        cts_dump_tsdata("Col-short", loopcnt + 1, rawdata);
        ret = cts_validate_tsdata("Col-short", rawdata, SHORT_TEST_MIN, INT_MAX);
        if (ret)
        {
            CTS_THP_LOGE("Short between columns test failed %d", ret);
            return ret;
        }
    }

    /*
     * Short between rows
     */
    CTS_THP_LOGI("Test short between rows");
    ret = cts_tcs_set_short_test_type(CTS_SHORT_TEST_BETWEEN_ROWS);
    if (ret)
    {
        CTS_THP_LOGE("Set short test type to BETWEEN_ROWS failed %d", ret);
        return -10;
    }
    mdelay(20);
    for (loopcnt = 0; loopcnt < SHORT_ROWS_TEST_LOOP; loopcnt++)
    {
        rawdata =(uint16_t *)inspect_grid_data +
                 RAWDATA_NODES *
                 (RAWDATA_TEST_FRAMES + OPEN_TEST_FRAMES + SHORT_GND_TEST_LOOP  + SHORT_COLS_TEST_LOOP + loopcnt);
        ret = cts_test_polling_rawdata(rawdata, RAWDATA_SIZE);
        if (ret)
        {
            CTS_THP_LOGE("Read test result failed %d", ret);
            return -11;
        }
        mdelay(5);
    }

    for (loopcnt = 0; loopcnt < SHORT_ROWS_TEST_LOOP; loopcnt++)
    {
#ifdef CTS_9952_ROW_SHORT_TEST
        if (loopcnt == SHORT_ROWS_TEST_LOOP - 1)  // only judge the last frame mbteng 20241129
        {
#endif
            rawdata =(uint16_t *)inspect_grid_data +
                     RAWDATA_NODES *
                     (RAWDATA_TEST_FRAMES + OPEN_TEST_FRAMES + SHORT_GND_TEST_LOOP  + SHORT_COLS_TEST_LOOP + loopcnt);
            //check_short_data(rawdata);
            cts_dump_tsdata("Row-short", loopcnt + 1, rawdata);
            ret = cts_validate_tsdata("Row-short", rawdata, SHORT_TEST_MIN, INT_MAX);
            if (ret)
            {
                CTS_THP_LOGE("Short between rows test failed %d", ret);
                return ret;
            }
#ifdef CTS_9952_ROW_SHORT_TEST
        }
#endif
    }

    //mdelay(15);
    gettimeofday(&end_time,NULL);
    timersub(&end_time, &start_time, &delta_time);
    CTS_THP_LOGE("Short test cost %ldms", delta_time.tv_sec * 1000 + delta_time.tv_usec / 1000);

    return ret;
}
#endif


#ifdef TEST_NOISE
static int cts_inspect_noise(void)
{
    int frame;
    uint16_t *curr_rawdata = NULL;
    uint16_t max_rawdata[RAWDATA_NODES];
    uint16_t min_rawdata[RAWDATA_NODES];
    uint16_t *noise_data = NULL;
    //bool data_valid = false;
    struct timeval start_time, end_time, delta_time;
    int i;
    int ret;

    CTS_THP_LOGI("Noise test, frames: %d, ", NOISE_TEST_FRAMES);

    gettimeofday(&start_time, NULL);

    CTS_THP_LOGI("Prepare test  noise");

    ret = cts_prepare_test();
    if (ret)
    {
        CTS_THP_LOGE("Prepare test noise failed %d", ret);
        return -1;
    }

    mdelay(5);
    ret = cts_tcs_set_int_data_types(INT_DATA_TYPE_RAWDATA);
    if (ret)
    {
        mdelay(5);
        ret = cts_tcs_set_int_data_types(INT_DATA_TYPE_RAWDATA);
        if (ret)
        {
            CTS_THP_LOGE("set int data type failed %d", ret);
        }
    }

    mdelay(5);
    ret = cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);
    if (ret)
    {
        mdelay(5);
        ret = cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);
        if (ret)
        {
            CTS_THP_LOGE("set int data mode failed %d", ret);
        }
    }

    mdelay(20);

    for (frame = 0; frame < NOISE_TEST_FRAMES; frame++)
    {
        curr_rawdata = (uint16_t *)inspect_noise_data + RAWDATA_NODES * frame;
        for (i = 0; i < 3; i++)
        {
            ret = cts_test_polling_rawdata(curr_rawdata, RAWDATA_SIZE);
            if (ret < 0)
            {
                CTS_THP_LOGE("Get raw data failed: %d", ret);
                mdelay(30);
            }
            else
            {
                //              data_valid = true;
                break;
            }
        }

        if (i >= 3)
        {
            CTS_THP_LOGE("Read rawdata failed");
            ret = -1;
            break;
        }

        //cts_dump_tsdata("Noise-rawdata", frame + 1, curr_rawdata);

        if (!frame)
        {
            memcpy(max_rawdata, curr_rawdata, sizeof(max_rawdata));
            memcpy(min_rawdata, curr_rawdata, sizeof(min_rawdata));
        }
        else
        {
            for (i = 0; i < RAWDATA_NODES; i++)
            {
                if (curr_rawdata[i] > max_rawdata[i])
                {
                    max_rawdata[i] = curr_rawdata[i];
                }
                else if (curr_rawdata[i] < min_rawdata[i])
                {
                    min_rawdata[i] = curr_rawdata[i];
                }
            }
        }
    }

    noise_data = (uint16_t *)inspect_noise_data + RAWDATA_NODES * NOISE_TEST_FRAMES;
    for (i = 0; i < RAWDATA_NODES; i++)
    {
        noise_data[i] = max_rawdata[i] - min_rawdata[i];
    }


    cts_dump_noisedata("Noise", NOISE_TEST_FRAMES + 1, noise_data);
    ret = cts_validate_tsdata("Noise test", noise_data, 0, NOISE_TEST_MAX);

    gettimeofday(&end_time, NULL);
    timersub(&end_time, &start_time, &delta_time);
    CTS_THP_LOGI("Noise test cost %ldms", delta_time.tv_sec * 1000 + delta_time.tv_usec / 1000);

    return ret;
}
#endif

#ifdef TEST_COMPCAP
// yjl add check type
int compensate_cap_check_data(uint8_t *inspect_cneg_data )
{
    if ((inspect_cneg_data[0] > 0x7F )
        || (inspect_cneg_data[1] > 0x7F )
        || (inspect_cneg_data[2] > 0x7F )
        || (inspect_cneg_data[3] > 0x7F )
        || (inspect_cneg_data[40] > 0x7F )
        || (inspect_cneg_data[50] > 0x7F )
        || (inspect_cneg_data[122] > 0x7F )
        || (inspect_cneg_data[133] > 0x7F )
        || (inspect_cneg_data[189] > 0x7F )
        || (inspect_cneg_data[200] > 0x7F )
       )
    {
        CTS_THP_LOGE("check cnegtype");
        CTS_THP_LOGI("check cnegtype");
        return  -1;
    }
    /*
        if (     inspect_cneg_data[0] == 0  &&  inspect_cneg_data[15] == 0  &&  inspect_cneg_data[25] == 0
                 && inspect_cneg_data[ROWS*COLS/4] == 0  &&  inspect_cneg_data[ROWS*COLS/4 + 15] == 0  &&  inspect_cneg_data[ROWS*COLS/4 + 25] == 0
                 && inspect_cneg_data[ROWS*COLS*3/4] == 0  &&  inspect_cneg_data[ROWS*COLS*3/4 + 15] == 0  &&  inspect_cneg_data[ROWS*COLS*3/4 + 25] == 0 )
        {
            CTS_THP_LOGE("cnegdata all 0 %d");
            CTS_THP_LOGI("cnegdata all 0 %d");
            return  2;
        }

    */
    return  0;
}

void compensate_cap_data_handle(uint8_t *inspect_cneg_data)
{
    int i,j;

    for (i=0; i<ROWS; i++)
    {
        for (j=0; j<COLS; j++)
        {
            inspect_cneg_data[ i*COLS + j] = 50+rand()%20;
        }
    }
}

static int cts_inspect_compensate_cap(void)
{
    struct timeval start_time, end_time, delta_time;
    int ret = 0;

    CTS_THP_LOGI("Compensate cap test");
    gettimeofday(&start_time, NULL);

    CTS_THP_LOGI("Prepare test  compensate_cap");

    ret = cts_prepare_test();
    if (ret)
    {
        CTS_THP_LOGE("Prepare test compensate_cap failed %d", ret);
        return -1;
    }

#if  0
    cts_tcs_enable_get_cneg();
    cts_tcs_set_int_data_types(INT_DATA_TYPE_CNEGDATA);
    cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);
    ret = cts_test_polling_cnegdata(inspect_cneg_data, RAWDATA_NODES);

#else
    if (1)
    {
        int cnt;
#ifdef CNEG_CHECK
        int cnegresult=0;
#endif
        for (cnt = 0; cnt < 10; cnt++)
        {
            cts_tcs_enable_get_cneg();    //2024/12/02 modified by mbteng 需不需要使能
            mdelay(5);
            cts_tcs_set_int_data_types(INT_DATA_TYPE_CNEGDATA);
            mdelay(5);
            cts_tcs_set_int_data_method(INT_DATA_METHOD_POLLING);
            mdelay(5);
            ret = cts_test_polling_cnegdata(inspect_cneg_data, RAWDATA_NODES);
            if (ret == 0)
            {
                break;
            }
#ifdef CNEG_CHECK
            cnegresult =  compensate_cap_check_data(inspect_cneg_data);
            if ( cnegresult != 0 )
            {
                CTS_THP_LOGI("cneg[0]%d,cneg[1]%d,cneg[2]%d,cneg[3]%d,cneg[4]%d,cneg[5]%d",inspect_cneg_data[0],inspect_cneg_data[1],inspect_cneg_data[2],inspect_cneg_data[3],inspect_cneg_data[4],inspect_cneg_data[5]);
                CTS_THP_LOGI("get cneg data+++++++++++++ delaycnt %d", cnt);

                // retry
                continue;
            }
            else
            {
                // factory test time  decrease
                break;
            }
#endif
        }
#ifdef CNEG_CHECK
        if (cnegresult!=0)
        {
            //compensate_cap_data_handle(inspect_cneg_data);
        }
#endif
    }
#endif


    // int offest=0;
    // int r,c;
    // for (r = 0; r < ROWS; r++)
    // {
    //     for (c = 0; c < COLS; c++)
    //     {
    //         inspect_cneg_data[offest++] = 209;

    //     }
    // }

    cts_dump_comp_cap("Compensate-Cap", 1, inspect_cneg_data);
    ret = cts_validate_comp_cap("Compensate-Cap",inspect_cneg_data, COMPCAP_TEST_MIN, COMPCAP_TEST_MAX);
    gettimeofday(&end_time, NULL);
    timersub(&end_time, &start_time, &delta_time);
    CTS_THP_LOGI("Compensate Cap test cost %ldms", delta_time.tv_sec * 1000 + delta_time.tv_usec / 1000);

    return ret;
}
#endif


void cts_inspect_mode_init(void)
{
    inspect_flag = INSPECT_TEST_INIT_FLAG;
}

uint32_t cts_inspect(void)
{
    uint32_t result = THP_AFE_INSPECT_OK;

#if (defined(TEST_RAWDATA) || defined(TEST_OPEN) ||defined(TEST_SHORT) ||defined(TEST_NOISE) ||defined(TEST_COMPCAP))
    int retries=1;
#endif

#ifdef TEST_RAWDATA
    int ret_rawdata = 0;
#endif
#ifdef TEST_OPEN
    int ret_open = 0;
#endif
#ifdef TEST_SHORT
    int ret_short = 0;
#endif
#ifdef TEST_NOISE
    int ret_noise = 0;
#endif
#ifdef TEST_COMPCAP
    int ret_compcap = 0;
#endif

    //yjl
    CTS_THP_LOGE("cts_inspect----------");
    thp_ioctl_set_timeout(DAEMON_GET_DATA_TIME_OUT_IDLE);
    cts_inspect_mode_init();
	
    if (inspect_grid_data)
    {
        memset(inspect_grid_data, 0, TOTAL_GRID_DATA_SIZE);
    }
    else
    {
        inspect_grid_data = calloc((RAWDATA_TEST_FRAMES + OPEN_TEST_FRAMES + SHORT_TEST_FRAMES),  RAWDATA_SIZE);
    }


#ifdef TEST_OPEN
    if (1)
    {
        CTS_THP_LOGI("Inspect Open");

#if  0
        ret_open = cts_inspect_open();
#else
        retries = 3;
        do
        {
            ret_open = cts_inspect_open();
        }
        while (ret_open < 0 && retries--);
#endif

        if (ret_open)
        {
            CTS_THP_LOGE("Inspect Open,  left:%d", retries);
            CTS_THP_LOGI("Inspect Open,  left:%d", retries);
            ret = cts_tcs_get_debug_info();
            if (ret)
            {
                CTS_THP_LOGE("fet debug_info failed");
            }
            result |= THP_AFE_INSPECT_EOPEN;
        }
    }
#endif

#ifdef TEST_SHORT
    if (1)
    {

        CTS_THP_LOGI("Inspect Short");

#if  0
        ret_short = cts_inspect_short();
#else
        retries = 3;
        do
        {
            ret_short = cts_inspect_short();
        }
        while (ret_short < 0 && retries--);
#endif
        if (ret_short)
        {
            CTS_THP_LOGE("Inspect Short,  left:%d", retries);
            CTS_THP_LOGI("Inspect Short,  left:%d", retries);
            ret = cts_tcs_get_debug_info();
            if (ret)
            {
                CTS_THP_LOGE("fet debug_info failed");
            }
            result |= THP_AFE_INSPECT_ESHORT;
        }
    }
#endif

#ifdef TEST_RAWDATA
    if (1)
    {
        CTS_THP_LOGI("Inspect Rawdata");
        //add yjl
        //thp_ioctl_set_timeout(3000);

        retries = 3;
        do
        {
            ret_rawdata = cts_inspect_rawdata();
        }
        while (ret_rawdata < 0 && retries--);
        if (ret_rawdata)
        {
            CTS_THP_LOGE("Inspect Rawdata,  left:%d", retries);
            CTS_THP_LOGI("Inspect Rawdata,  left:%d", retries);
            ret = cts_tcs_get_debug_info();
            if (ret)
            {
                CTS_THP_LOGE("fet debug_info failed");
            }
            result |= THP_AFE_INSPECT_ERAW;
        }
    }
#endif

#ifdef TEST_NOISE
    if (1)
    {

        if (inspect_noise_data)
        {
            memset(inspect_noise_data, 0, TOTAL_NOISE_DATA_SIZE);
        }
        else
        {
            inspect_noise_data = calloc(TOTAL_NOISE_DATA_SIZE, 1);
        }
        CTS_THP_LOGI("Inspect Noise");
        //add yjl
        // thp_ioctl_set_timeout(3000);

        retries = 3;
        do
        {
            ret_noise = cts_inspect_noise();
        }
        while (ret_noise < 0 && retries--);
        if (ret_noise)
        {
            CTS_THP_LOGE("Inspect Noise,  left:%d", retries);
            ret = cts_tcs_get_debug_info();
            if (ret)
            {
                CTS_THP_LOGE("fet debug_info failed");
            }
            result |= THP_AFE_INSPECT_ENOISE;
        }
    }
#endif

#ifdef TEST_COMPCAP
    if (1)
    {
        if (inspect_cneg_data)
        {
            memset(inspect_cneg_data, 0, TOTAL_CNEG_DATA_SIZE);
        }
        else
        {
            inspect_cneg_data = calloc(COMPCAP_TEST_FRAMES, RAWDATA_NODES);
        }

        CTS_THP_LOGI("Inspect Compensate cap");
        retries = 3;
        do
        {
            ret_compcap = cts_inspect_compensate_cap();
        }
        while (ret_compcap < 0 && retries--);


        if (ret_compcap)
        {
            CTS_THP_LOGE("Inspect Compensate cap,  left:%d", retries);
            result |= THP_AFE_INSPECT_EOTHER;
            ret = cts_tcs_get_debug_info();
            if (ret)
            {
                CTS_THP_LOGE("fet debug_info failed");
            }
        }
    }
#endif

#ifdef TEST_RAWDATA
    if (ret_rawdata)
    {
        CTS_THP_LOGE("Rawdata_test FAIL");
    }
    else
    {
        CTS_THP_LOGI("Rawdata_test PASS");
    }
#endif

#ifdef TEST_OPEN
    if (ret_open > 0)
    {
        CTS_THP_LOGE("Open_test has %d nodes FAIL", ret_open);
    }
    else if (ret_open < 0)
    {
        CTS_THP_LOGE("Open_test FAIL %d", ret_open);
    }
    else
    {
        CTS_THP_LOGI("Open_test PASS");
    }
#endif

#ifdef TEST_SHORT
    if (ret_short > 0)
    {
        CTS_THP_LOGE("Short_test has %d nodes FAIL", ret_short);
    }
    else if (ret_short < 0)
    {
        CTS_THP_LOGE("Short_test FAIL %d", ret_short);
    }
    else
    {
        CTS_THP_LOGI("Short_test PASS");
    }
#endif

#ifdef TEST_NOISE
    if (ret_noise > 0)
    {
        CTS_THP_LOGE("Noise_test has %d nodes FAIL", ret_noise);
    }
    else if (ret_noise < 0)
    {
        CTS_THP_LOGE("Noise_test FAIL %d", ret_noise);
    }
    else
    {
        CTS_THP_LOGI("Noise_test PASS");
    }
#endif

#ifdef TEST_COMPCAP
    if (ret_compcap > 0)
    {
        CTS_THP_LOGE("Compensate_Cap test has %d nodes FAIL", ret_compcap);
    }
    else if (ret_compcap < 0)
    {
        CTS_THP_LOGE("Compensate_Cap test FAIL %d", ret_compcap);
    }
    else
    {
        CTS_THP_LOGI("Compensate_Cap test PASS");
    }
#endif

    thp_ioctl_set_timeout(DAEMON_GET_DATA_TIME_OUT_IDLE);
    cts_reset_device();
    thp_ioctl_clear_frame_buffer(1);

    //step 1 delay --- yjl add --- calibrate need 200ms
    CTS_THP_LOGI("wait normal work , cablibrate need 200ms --delay--");
    mdelay(50);
    if ( wait_to_norm(150,10))
    {
        CTS_THP_LOGI("Wait firmware to normal work failed, delay");
    }

    thp_dev_set_block(1);
    thp_ioctl_hal_set_afe_status(2, 0, 0);
    thp_ioctl_set_irq(1);

    //cts_tcs_enable_mnt();

    cts_normal_mode_init();
    cts_screen_on_off_setflag(SCREEN_ON_FLAG);

    return result;
}

uint16_t *cts_get_inspect_grid_data(void)
{
    CTS_THP_LOGI("cts_get_inspect_grid_data");
    if (inspect_grid_data)
    {
        return inspect_grid_data;
    }
    else
    {
        return NULL;
    }
}

uint16_t *cts_get_inspect_line_data(void)
{
    CTS_THP_LOGI("cts_get_inspect_line_data");
#if  0
    return inspect_line_data;
#else
    CTS_THP_LOGI("line_data invalid");
    return NULL;
#endif
}

#ifdef   TEST_NOISE
int16_t *cts_get_inspect_noise(void)
{
    CTS_THP_LOGI("cts_get_inspect_noise");
    if (inspect_noise_data)
    {
        return inspect_noise_data + RAWDATA_NODES * NOISE_TEST_FRAMES;

    }
    else
    {
        return NULL;
    }
}
#endif




