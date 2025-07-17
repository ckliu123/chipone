#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "cts_log.h"
#include "cts_utils.h"
#include "cts_spi.h"
#include "cts_prog.h"
#include "cts_tcs.h"
#include "firmware_data.h"
#include "thp_ioctl.h"
#include "cts_core.h"

#define HWID_ICNL9952                0x00990520
#define HWID_MASK                    0x00FFFFF0
#define RAM_CRC_ADDR                 (0x37FF0)
#define RAM_CRC_LEN                  (0x10)

#define SRAM_LOAD_FLASH_STATUS_REGISTER_ADRR    (0x2E000)
#define SRAM_LOAD_FLASH_ID_ADRR                 (0x2E010)

#define SECTOR_LENGTH    (0x8000)
#define FLASH_LENGTH     (0x20000)
#define FLASH_WRITE_ADDR (0x30000)
#define FLASH_WRITE_ADDR111 (0x30010)

#define SYS_CHIP_VER                 (0x70000)
#define SYS_SW_RST_CTL               (0x7000A)
#define SYS_BOOT_MODE                (0x70010)
#define SYS_BOOT_CURRENT_MODE        (0x70011)
#define SYS_CLK_DIV_CFG              (0x70033)
#define CASC_STATE                   (0x73060)
#define SFCTL_BASE                   (0x74000)
#define SFCTL_FLASH_ERASE_ADDR       (0x74004)
#define SFCTL_FLASH_WRITE_ADDR       (0x74004)
#define SFCTL_SRAM_ADDR              (0x74008)
#define SFCTL_DATA_LENGTH            (0x7400C)
#define SFCTL_START_DEXC_ADDR        (0x74010)
#define SFCTL_CRC_RESULT             (0x7401C)
#define SFCTL_SW_CRC_START           (0x74020)
#define SW_FLASH_CRC_START           (0x74022)
#define SFCTL_SF_BUSY                (0x74024)

#define FLASH_MASK                         (0x01)
#define ERASE_FLASH                        (0x02)
#define SECTOR_E_FLASH                     (0x03)
#define FLASH_PAGE_PROGRAM                 (0x04)
#define READ_FLASH_STATUS_REGISTER         (0x05)
#define READ_FLASH_IDENTIFICATION          (0x06)

#define SPI2_TX_FORWARD                    0x78438
#define SPI2_TX_FORWARD_OFF                0x12
#define FIRMWARE_MARK_OFFSET               0x37ff0u
#ifdef __MUSL__
#define FIRMWARE_FILE                      "/chip_prod/etc/firmware/ts/icnl9952.bin"
#else
#define FIRMWARE_FILE                      "/odm/etc/firmware/ts/icnl9952.bin"
#endif
#define FW_BIN_VERSION_OFFSET        0x100
#define FW_BUILD_DATE_OFFSET         0xd0
#define FW_BUILD_DATE_LENGTH         12
#define FW_BUILD_TIME_OFFSET         0xdc
#define FW_BUILD_TIME_LENGTH         9

#define BOOT_MODE_DOWNLOAD           2
#define BOOT_MODE_SRAM               3

int SpiIsBusy(uint16_t RetryNum )
{
    uint16_t retries =3;
    uint8_t spiflag_busy;
    int ret = -1;

    CTS_THP_LOGE("spi busy --- polling wait");
    retries = RetryNum;
    do
    {
        ret = cts_drw_read_u8(SFCTL_SF_BUSY, &spiflag_busy);
        if (ret < 0)
        {
            CTS_THP_LOGE("read spi busy flag failed: ret=%d", ret);
        }
        CTS_THP_LOGE("retries: %d", retries);
    }
    while (retries-- && spiflag_busy);

    if (ret < 0)
    {
        CTS_THP_LOGE("read sfctl busy failed: ret=%d", ret);
        return -1;
    }
    else if (spiflag_busy)
    {
        CTS_THP_LOGE("spi always busy!");
        return -1;
    }

    return 0;
}

void cts_get_fwfile_version(uint16_t *getfileFwversion)
{
    uint8_t *fwbuf = NULL;
    uint16_t version;

#ifdef  FIRMWARE_UPDATE_SDCARD
    size_t len = 0;
    const char *fwpath = FIRMWARE_FILE;
    //const char *fwpath_other = FIRMWARE_FILE_OTHER;
    if (!fwpath)
    {
        CTS_THP_LOGE("load firmware from sd odm/etc/firmware/ts/...  fw is NULL");
    }

    CTS_THP_LOGI("load firmware from external sdcard, odm/etc/firmware/ts/...");
    if (cts_load_file(fwpath, &fwbuf, &len) < 0)
    {
        //CTS_THP_LOGE("load firmware from '%s' failed", fwpath);

        CTS_THP_LOGI("load firmware from internal so...");
        fwbuf = Firmware_Data;
    }
#else
    CTS_THP_LOGI("load firmware from internal so...");
    fwbuf = Firmware_Data;
#endif

    version = cts_get_unaligned_le16(fwbuf + FW_BIN_VERSION_OFFSET);
    *getfileFwversion =  version;
}

static int cts_send_fw(uint8_t *fwbuf, size_t len)
{
    int ret = -1;
    size_t pos = 0;

    while ((pos + SPI_BUF_MAX_SIZ) < len)
    {
        //CTS_THP_LOGI("write file bin addr = %d, length=4k byte", pos);
        ret = cts_drw_write_raw(pos, fwbuf + pos, SPI_BUF_MAX_SIZ);
        if (ret)
        {
            CTS_THP_LOGE("write sram faild: pos = %lld, ret = %d", pos, ret);
            return ret;
        }
        pos += SPI_BUF_MAX_SIZ;
    }

    CTS_THP_LOGI("write file bin addr = %d, <4k byte", pos);
    ret = cts_drw_write_raw(pos, fwbuf + pos, len - pos);
    if (ret)
    {
        CTS_THP_LOGE("write sram faild: pos = %lld, ret = %d", pos, ret);
        return ret;
    }

    CTS_THP_LOGI("%ld bytes sent", len);

    return 0;
}

#if 0
static int cts_check_sram_crc_function( uint32_t crc32, uint32_t len, uint32_t* ramcrc)
{
    int ret;
    int retries = 15;
    uint8_t sf_busy;
    uint32_t crc32_result;

    uint8_t wbuf[RAM_CRC_LEN]= {0x5A,0x55,0x33,0xCC};
    //crc32 big
    //wbuf small
    wbuf[8] = crc32 & 0xFF;
    wbuf[9] = crc32 & 0xFF00;
    wbuf[10] = crc32 & 0xFF0000;
    wbuf[11] = crc32 & 0xFF0000;

    wbuf[12] = len & 0xFF;
    wbuf[13] = len & 0xFF00;
    wbuf[14] = len & 0xFF0000;
    wbuf[15] = len & 0xFF0000;


    //37FF0 0x10   flag dummy  crc len
    cts_drw_write_raw(RAM_CRC_ADDR, wbuf, RAM_CRC_LEN);

    ret = cts_drw_write_u32(SFCTL_SRAM_ADDR, RAM_CRC_ADDR);
    if (ret < 0)
    {
        CTS_THP_LOGE("write sram addr failed: ret=%d", ret);
        return -1;
    }

    ret = cts_drw_write_u32(SFCTL_DATA_LENGTH, RAM_CRC_LEN);
    if (ret < 0)
    {
        CTS_THP_LOGE("write data length failed: ret=%d", ret);
        return -1;
    }

    ret = cts_drw_write_u8(SFCTL_SW_CRC_START, 1);
    if (ret < 0)
    {
        CTS_THP_LOGE("write start crc calc failed: ret=%d", ret);
        return -1;
    }


    mdelay(10);

    do
    {
        ret = cts_drw_read_u8(SFCTL_SF_BUSY, &sf_busy);
        if (ret < 0)
        {
            CTS_THP_LOGE("read sfctl busy failed: ret=%d", ret);
        }
        else if (sf_busy)
        {
        }
        CTS_THP_LOGE("retries: %d", retries);
    }
    while (retries-- && sf_busy);

    if (ret < 0)
    {
        CTS_THP_LOGE("read sfctl busy failed: ret=%d", ret);
        return -1;
    }
    else if (sf_busy)
    {
        CTS_THP_LOGE("sfctl always busy!");
        return -1;
    }

    ret = cts_drw_read_u32(SFCTL_CRC_RESULT, &crc32_result);
    if (ret < 0)
    {
        CTS_THP_LOGE("read crc result failed: ret=%d", ret);
        return -1;
    }

    *ramcrc =  crc32_result ;
    CTS_THP_LOGE("crc = %#010x, %#010x", crc32_result, *ramcrc );
    return 0;
}
#endif

static int cts_check_sram_crc32(uint32_t addr, size_t len, uint32_t crc32)
{
    int ret;
    int retries = 5;
    uint8_t sf_busy;
    uint32_t crc32_result;

    ret = cts_drw_write_u32(SFCTL_SRAM_ADDR, addr);
    if (ret < 0)
    {
        CTS_THP_LOGE("write sram addr failed: ret=%d", ret);
        return -1;
    }

    ret = cts_drw_write_u32(SFCTL_DATA_LENGTH, len);
    if (ret < 0)
    {
        CTS_THP_LOGE("write data length failed: ret=%d", ret);
        return -1;
    }

    ret = cts_drw_write_u8(SFCTL_SW_CRC_START, 1);
    if (ret < 0)
    {
        CTS_THP_LOGE("write start crc calc failed: ret=%d", ret);
        return -1;
    }
    mdelay(10);
    retries = 5;
    do
    {
        ret = cts_drw_read_u8(SFCTL_SF_BUSY, &sf_busy);
        if (ret < 0)
        {
            CTS_THP_LOGE("read sfctl busy failed: ret=%d", ret);
        }
        else if (sf_busy)
        {
        }
        CTS_THP_LOGE("retries: %d", retries);
    }
    while (retries-- && sf_busy);

    if (ret < 0)
    {
        CTS_THP_LOGE("read sfctl busy failed: ret=%d", ret);
        return -1;
    }
    else if (sf_busy)
    {
        CTS_THP_LOGE("sfctl always busy!");
        return -1;
    }

    ret = cts_drw_read_u32(SFCTL_CRC_RESULT, &crc32_result);
    if (ret < 0)
    {
        CTS_THP_LOGE("read crc result failed: ret=%d", ret);
        return -1;
    }

    if (crc32_result != crc32)
    {
        CTS_THP_LOGE("crc mismatch: expect %#010x, %#010x received", crc32, crc32_result);
        return -1;
    }

    CTS_THP_LOGI("crc expect %#010x, %#010x received", crc32, crc32_result);
    return 0;
}

#if 1
static int cts_mark_fw(size_t len, uint32_t crc32)
{
    uint32_t mark[4] = {0xcc33555a, 0, crc32, len};
    int ret = 0;

    ret = cts_drw_write_raw(FIRMWARE_MARK_OFFSET, (uint8_t *)mark, sizeof(mark));

    if (ret < 0)
    {
        CTS_THP_LOGE("write firmware mark failed");
        return ret;
    }

    return ret;
}
#endif

static int cts_do_upfw(const char *fwpath)
{
    int ret = -1;
    uint8_t *fwbuf = NULL;
    size_t len = 0;
    uint32_t crc32;
    uint16_t version;
    char build_date[FW_BUILD_DATE_LENGTH];
    char build_time[FW_BUILD_TIME_LENGTH];
    uint32_t hwid;
    uint32_t status;

#ifdef  FIRMWARE_UPDATE_SDCARD
    if (!fwpath)
    {
        CTS_THP_LOGE("Firmware is NULL");
        return -1;
    }

    if (cts_load_file(fwpath, &fwbuf, &len) < 0)
    {
        //CTS_THP_LOGE("load firmware from '%s' failed", fwpath);

        CTS_THP_LOGD("load firmware from internal so...");
        fwbuf = Firmware_Data;
        len = sizeof(Firmware_Data);
    }
	else
	{
        //CTS_THP_LOGD("load firmware from extern so /chip_prod/etc/firmware/ts/...");
	}
#else
    CTS_THP_LOGD("load firmware from internal so...");
    fwbuf = Firmware_Data;
    len = sizeof(Firmware_Data);
#endif

    CTS_THP_LOGE("load firmware ok, size = %d = %x", len,len);
    crc32 = cts_crc32(fwbuf, len);
    version = cts_get_unaligned_le16(fwbuf + FW_BIN_VERSION_OFFSET);

    memcpy(build_date, fwbuf + FW_BUILD_DATE_OFFSET, FW_BUILD_DATE_LENGTH);
    build_date[FW_BUILD_DATE_LENGTH - 1] = '\0';
    memcpy(build_time, fwbuf + FW_BUILD_TIME_OFFSET, FW_BUILD_TIME_LENGTH);
    build_time[FW_BUILD_TIME_LENGTH - 1] = '\0';

    CTS_THP_LOGE("Load new firmware: ver: %04x, build at %s %s, crc32: %#010x", version, build_date, build_time, crc32);

    // ret = cts_drw_spi_switch(CTS_SPI_TARGET_ALL);
    // if (ret < 0)
    // {
    //     CTS_THP_LOGE("Switch spi to ALL failed");
    //     return ret;
    // }

    CTS_THP_LOGI("send firmware bin to ram");

    if ((ret = cts_send_fw(fwbuf, len)) < 0)
    {
        CTS_THP_LOGE("send firmware failed: ret=%d", ret);
        ret = -1;
        goto err_free_fwbuf;
    }

    ret = cts_drw_spi_switch(CTS_SPI_TARGET_SLAVE);
    // check ram CRC function
    if (ret < 0)
    {
        CTS_THP_LOGE("Switch spi to slave failed");

        return ret;
    }

    ret = cts_drw_read_u32(SYS_CHIP_VER, &hwid);
    if (ret < 0)
    {
        CTS_THP_LOGE("after FW read slave failed");
    }
    CTS_THP_LOGD("after FW read slaver hwid: 0x%x", hwid);

    status = cts_drw_get_target_id();
    CTS_THP_LOGE("Crc Read_slaver casc_status :%d", status);

    // check ram CRC function
    ret = cts_check_sram_crc32(0, len, crc32);
    if (ret < 0)
    {
        CTS_THP_LOGE("slaver crc error!");
        ret = -1;
        goto err_free_fwbuf;
    }

    ret = cts_mark_fw(len, crc32);
    if (ret < 0)
    {
        CTS_THP_LOGE("mark slave fw failed: ret=%d", ret);
        goto err_free_fwbuf;
    }
    CTS_THP_LOGE("slaver crc ok!");

    ret = cts_drw_spi_switch(CTS_SPI_TARGET_MASTER);
    if (ret < 0)
    {
        CTS_THP_LOGE("Switch spi to master failed");
        return ret;
    }

    ret = cts_drw_read_u32(SYS_CHIP_VER, &hwid);
    if (ret < 0)
    {
        CTS_THP_LOGE("after FW read maste failed");
        return ret;
    }
    CTS_THP_LOGD("after FW read master hwid: 0x%x", hwid);

    status = cts_drw_get_target_id();
    CTS_THP_LOGE("Crc Read_master casc_status :%d", status);

    ret = cts_check_sram_crc32(0, len, crc32);
    if (ret < 0)
    {
        CTS_THP_LOGE("master crc error!");
        ret = -1;
        goto err_free_fwbuf;
    }
    CTS_THP_LOGE("mster crc ok!");

    ret = cts_mark_fw(len, crc32);
    if (ret < 0)
    {
        CTS_THP_LOGE("mark master fw failed: ret=%d", ret);
        goto err_free_fwbuf;
    }

#if 1
    ret = cts_drw_spi_switch(CTS_SPI_TARGET_ALL);
    if (ret < 0)
    {
        CTS_THP_LOGE("Switch spi to ALL failed");
        return ret;
    }
#endif
    ret = 0;

err_free_fwbuf:
    // free(fwbuf);
    return ret;
}

static int cts_upfw_single(CTS_SPI_TARGET_ENUM target)
{
    int ret = -1;

    ret = cts_do_upfw(FIRMWARE_FILE);

    if (ret < 0)
    {
        return ret;
    }
    return 0;
}

int cts_update_firmware(void)
{
    uint32_t hwid;
    uint32_t status;
    uint8_t boot_mode = 0;
    int retries = 8;
    int ret = -1;
    int ret1 = -1;

    CTS_THP_LOGE("cts_update_firmware +");
    CTS_THP_LOGE("Switch to Master");
    //mdelay(50);

    retries = 3;
    do
    {
        ret = cts_drw_spi_switch(CTS_SPI_TARGET_MASTER);
    }
    while (ret < 0 && retries--);
    if (ret < 0)
    {
        CTS_THP_LOGE("Switch spi to Master failed");
        return ret;
    }

    retries = 3;
    do
    {
        ret = cts_tcs_set_krang_stop();
    }
    while (ret < 0 && retries--);
    if (ret < 0)
    {
        CTS_THP_LOGE("Set krang stop failed");
        // return ret;
    }
    mdelay(50);
    retries = 3;
    do
    {
        ret = cts_enter_drw_mode();

        boot_mode = 0;
        ret1 = cts_drw_read_u8(SYS_BOOT_CURRENT_MODE, &boot_mode);
        if (ret1 < 0)
            CTS_THP_LOGE("master get mode failed");
        else
            CTS_THP_LOGE("master get mode :%d[0:idle 1:flash 2:download 3:sram]", boot_mode);
    }
    while ( ( !(ret == 0 && ret1 ==0 && boot_mode == BOOT_MODE_DOWNLOAD)) && retries--);
    if (ret < 0)
    {
        CTS_THP_LOGE("Enter drw mode failed");
        return ret;
    }
    CTS_THP_LOGE("step3 master  enter drw mode passed,send  CC 33 55 5A ");
    //step4 check  master mode & hw id

    retries = 8;
    do
    {
        status = cts_drw_get_target_id();
        CTS_THP_LOGE("Read_master casc_status=%d [0:slave  1:master]", status);

        ret = cts_drw_read_u32(SYS_CHIP_VER, &hwid);
        CTS_THP_LOGE("Read+1 hwid:0x%x,retries left %d ", hwid,retries);

        mdelay(5);
    }
    while ( (!(ret == 0 && ((hwid & HWID_MASK) == HWID_ICNL9952) && status == CTS_SPI_TARGET_MASTER)) && retries--);

    if (ret)
    {
        CTS_THP_LOGE("Master ret fail :%d", ret);
        return ret;
    }
    CTS_THP_LOGE("Read_master casc_status=%d [0:slave  1:master], Read_9952_hwid=0x%x ", status, hwid);

    //step5   send 82
    retries = 3;
    do
    {
        ret = cts_drw_spi_switch(CTS_SPI_TARGET_SLAVE);
    }
    while (ret < 0 && retries--);
    if (ret < 0)
    {
        CTS_THP_LOGE("Switch spi to Slave failed");
        return ret;
    }

    //step6  slave  enter drw mode,send  CC 33 55 5A
    retries = 3;
    do
    {
        ret = cts_enter_drw_mode();

        boot_mode = 0;
        ret1 = cts_drw_read_u8(SYS_BOOT_CURRENT_MODE, &boot_mode);
        if (ret1 < 0)
            CTS_THP_LOGE("slave get mode failed");
        else
            CTS_THP_LOGE("slave get mode :%d[0:idle 1:flash 2:9952 pro 3:sram]", boot_mode);
    }
    while ( ( !(ret == 0 && ret1 ==0 && boot_mode ==2)) && retries--);
    CTS_THP_LOGE("step6  slave  enter drw mode passed,send  CC 33 55 5A ");

    //step7  slave  spi tx forword. write reg 78438  data 12
#if 0 //9952 SPI 
    retries = 3;
    do
    {
        ret = cts_drw_write_u8(SPI2_TX_FORWARD, SPI2_TX_FORWARD_OFF);
    }
    while (ret < 0 && retries--);
    if (ret < 0)
    {
        CTS_THP_LOGE("Close SPI2_TX_FORWARD failed");
    }
#endif

    //step8 check  slave mode & hw id
    retries = 8;
    do
    {
        status = cts_drw_get_target_id();
        CTS_THP_LOGE("Read_slave casc_status=%d [0:slave  1:master]", status);

        ret = cts_drw_read_u32(SYS_CHIP_VER, &hwid);
        CTS_THP_LOGE("Read+2 hwid :0x%x, retries left %d ", hwid, retries);

        mdelay(5);
    }
    while ( (!(ret == 0 && ((hwid & HWID_MASK) == HWID_ICNL9952) && status == CTS_SPI_TARGET_SLAVE))&& retries--);

    if (ret)
    {
        CTS_THP_LOGE("Slave ret fail:%d", ret);
        return ret;
    }

    CTS_THP_LOGE("step8 check  slave mode & hw id passed");

    //step9  switch master .send 81
    retries = 3;
    do
    {
        ret = cts_drw_spi_switch(CTS_SPI_TARGET_ALL);
    }
    while (ret < 0 && retries--);
    if (ret < 0)
    {
        CTS_THP_LOGE("Switch spi to ALL failed");
        return ret;
    }

    retries = 3;
    do
    {
        ret = cts_upfw_single(CTS_SPI_TARGET_ALL);
    }
    while (ret < 0 && retries--);
    if (ret < 0)
    {
        CTS_THP_LOGE("send firmware  failed");
        return ret;
    }

    // hwid =0;
    // ret = cts_drw_read_u32(SYS_CHIP_VER, &hwid);
    // if (ret < 0)
    //     CTS_THP_LOGE("last read master HWID:0x%x failed!!", hwid);
    // else
    //     CTS_THP_LOGD("last read master HWID: 0x%x",hwid);

    CTS_THP_LOGE("change to mode:%d", BOOT_MODE_SRAM);
    retries = 3;
    do
    {
        ret = cts_drw_write_u8(SYS_BOOT_MODE, BOOT_MODE_SRAM);
    }
    while (ret < 0 && retries--);
    if (ret < 0)
    {
        CTS_THP_LOGE("change to mode :%d failed", ret);
        return ret;
    }

    mdelay(50);
    boot_mode = 0;
    // ret = cts_drw_read_u8(SYS_BOOT_MODE, &boot_mode);
    // if (ret < 0)
    //     CTS_THP_LOGE("get mode failed");
    // else
    //     CTS_THP_LOGE("get mode :%d", boot_mode);


    // CTS_THP_LOGI("flash->ram,Do not reset");

    // mdelay(10);
    // cts_reset_device();


    //mdelay(50);

    for (size_t i = 0; i < 3; i++)
    {
        ret = cts_tcs_read_hw_reg(SYS_BOOT_MODE, &boot_mode, 1);
        if ((ret != 0) || (boot_mode != 3))
        {
            CTS_THP_LOGE("read hw reg:0x%x failed!!", boot_mode);
        }
        else
        {
            CTS_THP_LOGD("boot mode_normal = %d",boot_mode);
            break;
        }
        mdelay(5);
    }

    cts_reset_device();
    mdelay(50);

    return ret;
}
