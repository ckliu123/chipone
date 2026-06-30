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
#ifdef M2
// #warning m2
#include "firmware_data_m2.h"
#elif defined(DPT)
// #warning dpt
#include "firmware_data_csot.h"
#else
// #include "firmware_data_m2.h"
#endif
#include "thp_ioctl.h"
#include "cts_core.h"

#define HWID_ICNL9953                0x00990530
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
#define FIRMWARE_FILE                      "/chip_prod/etc/firmware/ts/icnl9953.bin"
#else
#define FIRMWARE_FILE                      "/odm/etc/firmware/ts/icnl9953.bin"
#endif


#define FW_BIN_VERSION_OFFSET        0x100
#define FW_BUILD_DATE_OFFSET         0xd0
#define FW_BUILD_DATE_LENGTH         12
#define FW_BUILD_TIME_OFFSET         0xdc
#define FW_BUILD_TIME_LENGTH         9

#define BOOT_MODE_DOWNLOAD           2
#define BOOT_MODE_SRAM               3



uint8_t cts_read_slave_boot_status(void)
{
    uint8_t  wbuf[16], rbuf[16];
    int ret = 0;

    memset(wbuf, 0, sizeof(wbuf));
    memset(rbuf, 0, sizeof(rbuf));

    //F0 01 21 04 00 14 24 01 11 00 87 45 96
    wbuf[0] = 0xF0;
    wbuf[1] = 0x01;
    wbuf[2] = 0x21;
    wbuf[3] = 0x04;
    wbuf[4] = 0x00;
    wbuf[5] = 0x14;
    wbuf[6] = 0x24;
    wbuf[7] = 0x01;
    wbuf[8] = 0x11;
    wbuf[9] = 0x00;
    wbuf[10] = 0x87;
    wbuf[11] = 0x45;
    wbuf[12] = 0x96;
    ret = cts_spi_sync_raw(wbuf, 13, rbuf, 0);

    //CTS_THP_LOGE("rbuf[0]=0x%02x, rbuf[1]=0x%02x, rbuf[2]=0x%02x, rbuf[3]=0x%02x, rbuf[4]=0x%02x, rbuf[5]=0x%02x, rbuf[6]=0x%02x, rbuf[7]=0x%02x, rbuf[8]=0x%02x, rbuf[9]=0x%02x, rbuf[10]=0x%02x, rbuf[11]=0x%02x, rbuf[12]=0x%02x", rbuf[0], rbuf[1], rbuf[2], rbuf[3], rbuf[4], rbuf[5], rbuf[6], rbuf[7], rbuf[8], rbuf[9], rbuf[10], rbuf[11], rbuf[12]);
    if (ret < 0)
    {
        CTS_THP_LOGE("cts_read_slave_boot_status 1 failed");
        return 0xFF;
    }
    mdelay(10);

    //FF FF FF FF FF
    wbuf[0] = 0xFF;
    wbuf[1] = 0xFF;
    wbuf[2] = 0xFF;
    wbuf[3] = 0xFF;
    wbuf[4] = 0xFF;
    ret = cts_spi_sync_raw(wbuf, 0, rbuf, 5);
    if (ret < 0)
    {
        CTS_THP_LOGE("cts_read_slave_boot_status 2 failed");
        return 0xFF;
    }


    //CTS_THP_LOGE("rbuf[0]=0x%02x, rbuf[1]=0x%02x, rbuf[2]=0x%02x, rbuf[3]=0x%02x, rbuf[4]=0x%02x, rbuf[5]=0x%02x", rbuf[0], rbuf[1], rbuf[2], rbuf[3], rbuf[4], rbuf[5]);
    mdelay(10);

    // F1 02 41 01 00 EF 81
    wbuf[0] = 0xF1;
    wbuf[1] = 0x02;
    wbuf[2] = 0x41;
    wbuf[3] = 0x01;
    wbuf[4] = 0x00;
    wbuf[5] = 0xEF;
    wbuf[6] = 0x81;
    ret = cts_spi_sync_raw(wbuf, 7, rbuf, 0);

    //CTS_THP_LOGE("rbuf[0]=0x%02x, rbuf[1]=0x%02x, rbuf[2]=0x%02x, rbuf[3]=0x%02x, rbuf[4]=0x%02x, rbuf[5]=0x%02x, rbuf[6]=0x%02x", rbuf[0], rbuf[1], rbuf[2], rbuf[3], rbuf[4], rbuf[5], rbuf[6]);
    if (ret < 0)
    {
        CTS_THP_LOGE("cts_read_slave_boot_status 3 failed");
        return 0xFF;
    }
    mdelay(10);

    //FF FF FF FF FF FF
    wbuf[0] = 0xFF;
    wbuf[1] = 0xFF;
    wbuf[2] = 0xFF;
    wbuf[3] = 0xFF;
    wbuf[4] = 0xFF;
    wbuf[5] = 0xFF;
    ret = cts_spi_sync_raw(wbuf, 0, rbuf, 6);
    if (ret < 0)
    {
        CTS_THP_LOGE("cts_read_slave_boot_status 4 failed");
        return 0xFF;
    }

    //CTS_THP_LOGE("rbuf[0]=0x%02x, rbuf[1]=0x%02x, rbuf[2]=0x%02x, rbuf[3]=0x%02x, rbuf[4]=0x%02x, rbuf[5]=0x%02x", rbuf[0], rbuf[1], rbuf[2], rbuf[3], rbuf[4], rbuf[5]);

    return rbuf[0];
}


bool  CopyMasterToSlave1ByRCP1(uint32_t master_addr, uint32_t slave_addr, uint32_t len_bytes)
{
    uint32_t start = 0;
    uint32_t SIZE_32KB = 32 * 1024;
    uint32_t writeLen = len_bytes;
    uint32_t writeOffsetMaster = master_addr;
    uint32_t writeOffsetSlave = slave_addr;

    uint32_t KR_DMA1_LEN = 0x7B010;
    uint32_t KR_DMA1_DST = 0x7B014;
    uint32_t KR_DMA1_SRC = 0x7B018;
    uint32_t KR_DMA1_CTL = 0x7B01C;

    uint8_t DMA1_CASC = 0x80; // M -> S1 ??
    uint8_t ucTemp[8];

    uint32_t blockLen = 0;
    uint32_t len_words = 0;

    int ret = 0;

    if ((master_addr % 4 != 0) || (slave_addr % 4 != 0) || (len_bytes % 4 != 0))
    {
        printf("CopyMasterToSlave1ByRCP1: invalid addr or len_bytes. Must be 4-byte aligned.\n");
        return false;
    }

    while (writeLen > 0)
    {
        blockLen = (writeLen > SIZE_32KB) ? SIZE_32KB : writeLen;
        len_words = blockLen / 4;

        // ���� DMA ���??���
        ucTemp[0] = (uint8_t)(len_words);
        ucTemp[1] = (uint8_t)(len_words >> 8);
        ucTemp[2] = DMA1_CASC;
        ucTemp[3] = 0x00;

        // if (WriteProg(KR_DMA1_LEN, ucTemp, 4) != 0)
        // {
        //     printf("DRW write failed at KR_DMA1_LEN\n");
        //     return FALSE;
        // }



        ret = cts_drw_write_raw(KR_DMA1_LEN, ucTemp, 4);
        if (ret < 0)
        {
            printf("DRW write failed at KR_DMA1_LEN\n");
            return false;
        }



        // ����?���?�����?��?��
        ucTemp[0] = (uint8_t)(writeOffsetSlave);
        ucTemp[1] = (uint8_t)(writeOffsetSlave >> 8);
        ucTemp[2] = (uint8_t)(writeOffsetSlave >> 16);

        // if (WriteProg(KR_DMA1_DST, ucTemp, 3) != 0)
        // {
        //     printf("DRW write failed at KR_DMA1_DST\n");
        //     return FALSE;
        // }


        ret = cts_drw_write_raw(KR_DMA1_DST, ucTemp, 3);
        if (ret < 0)
        {
            printf("DRW write failed at KR_DMA1_LEN\n");
            return false;
        }



        // ����?��?�����?��?��
        ucTemp[0] = (uint8_t)(writeOffsetMaster);
        ucTemp[1] = (uint8_t)(writeOffsetMaster >> 8);
        ucTemp[2] = (uint8_t)(writeOffsetMaster >> 16);

        // if (WriteProg(KR_DMA1_SRC, ucTemp, 3) != 0)
        // {
        //     printf("DRW write failed at KR_DMA1_SRC\n");
        //     return FALSE;
        // }

        ret = cts_drw_write_raw(KR_DMA1_SRC, ucTemp, 3);
        if (ret < 0)
        {
            printf("DRW write failed at KR_DMA1_LEN\n");
            return false;
        }




        // ���� DMA
        ucTemp[0] = 0x01;
        // if (WriteProg(KR_DMA1_CTL, ucTemp, 1) != 0)
        // {
        //     printf("DRW write failed at KR_DMA1_CTL\n");
        //     return FALSE;
        // }



        ret = cts_drw_write_raw(KR_DMA1_CTL, ucTemp, 1);
        if (ret < 0)
        {
            printf("DRW write failed at KR_DMA1_LEN\n");
            return false;
        }


        // �?� DMA ��?�?
        start = 0;
        while (1)
        {
            mdelay(1);

            ucTemp[0] = 0xFF;
            // if (ReadProg(KR_DMA1_CTL, ucTemp, 1) != 0)
            // {
            //     printf("DRW read failed at KR_DMA1_CTL\n");
            //     return FALSE;
            // }



            ret = cts_drw_read_raw(KR_DMA1_CTL, ucTemp, 1);
            if (ret < 0)
            {
                printf("DRW write failed at KR_DMA1_LEN\n");
                return false;
            }



            if (ucTemp[0] == 0)
                break;

            if (start++ > 50)
            {
                printf("CopyMasterToSlave1ByRCP1 timeout\n");
                return false;
            }
        }

        // ����?����
        writeLen -= blockLen;
        writeOffsetSlave += blockLen;
        writeOffsetMaster += blockLen;
    }

    return true;
}






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


    ret = 0;

err_free_fwbuf:
    // free(fwbuf);
    return ret;
}

static int cts_upfw_single(void)
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
    while ( (!(ret == 0 && ((hwid & HWID_MASK) == HWID_ICNL9953) && status == CTS_SPI_TARGET_MASTER)) && retries--);

    if (ret)
    {
        CTS_THP_LOGE("Master ret fail :%d", ret);
        return ret;
    }
    CTS_THP_LOGE("Read_master casc_status=%d [0:slave  1:master], Read_9953_hwid=0x%x ", status, hwid);


    retries = 3;
    do
    {
        ret = cts_upfw_single();
    }
    while (ret < 0 && retries--);
    if (ret < 0)
    {
        CTS_THP_LOGE("send firmware  failed");
        return ret;
    }




	CTS_THP_LOGE("updata master pass");

     if(CopyMasterToSlave1ByRCP1(0X37ff0, 0X37ff0, 16)==false)
	{
        printf("Slave1: DownloadCrcLen failed!");
		return -1;
	}

        



    CTS_THP_LOGE("updata slave crc info pass");
    

    

    
    cts_reset_device();

    //mdelay(200);

    //mdelay(50);

    CTS_THP_LOGE("tp rst");
    

     for (size_t i = 0; i < 3; i++)
    {
        ret = cts_tcs_read_hw_reg(SYS_BOOT_CURRENT_MODE, &boot_mode, 1);
        if ((ret != 0) || (boot_mode != 3))
        {
            CTS_THP_LOGE("read master boot state:0x%x failed!!", boot_mode);
        }
        else
        {
            CTS_THP_LOGD("boot master mode_normal = %d",boot_mode);
            break;
        }
        mdelay(5);
    }


    if (ret != 0)
        return ret;


    for (size_t i = 0; i < 3; i++)
    {
        boot_mode = cts_read_slave_boot_status();
        if (boot_mode != 3)
        {

            ret = -1;
            CTS_THP_LOGE("read slave boot state:0x%x failed!!", boot_mode);
        }
        else
        {
            CTS_THP_LOGD("boot slave mode_normal = %d",boot_mode);
            break;
        }
        mdelay(5);
    }
    




    return ret;
}
