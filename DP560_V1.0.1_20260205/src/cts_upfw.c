#include "cts_hal.h"

uint16_t g_version = 0xdead;
int dump_spi = 0;

#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "cts_utils.h"
#include "cts_spi.h"
#include "cts_drw.h"
#include "cts_upfw.h"
#include "cts_fw.h"
#include "cts_core.h"
#include "cts_chip.h"

#define HWID_ICNT9268					0x01009268

#define SYS_CHIP_VER					(0x870000)
#define SYS_BOOT_MODE					(0x70010)

#define EFCTL_VOL_REG1					(0x71034)
#define EFCTL_VOL_REG2					(0x74084)

#define FLASH_FIRMWARE_START			0x0C0000u
#define SRAM_FIRMWARE_START				0x000000u

#define FIRMWARE_MARK_OFFSET			(0x2F800)
#define FLASH_SECTOR_ERASE_START		(FIRMWARE_MARK_OFFSET + FLASH_FIRMWARE_START)

#define FIRMWARE_FILE					"/chip_prod/etc/firmware/ts/icnt9288.bin"
//#define FIRMWARE_FILE					"/data/icnt9288.bin"
#define FW_BIN_VERSION_OFFSET			0xcc
#define FW_BUILD_DATE_OFFSET			0xd0
#define FW_BUILD_DATE_LENGTH			12
#define FW_BUILD_TIME_OFFSET			0xdc
#define FW_BUILD_TIME_LENGTH			9

#define BOOT_MODE_MASK					0x07
#define BOOT_MODE_SRAM					3
#define BOOT_MODE_PROGRAM				2

extern char *project_id;
//char *project_id_00 = "P382B71300";
//char *project_id_03 = "P382B71303";

struct cts_firmware firmware_info;

static int cts_load_file(const char *filepath, uint8_t **buf, size_t *len)
{
	FILE *filep;
	struct stat st;
	uint8_t *pbuf = NULL;
	size_t fitlen;
	THP_LOGI("Enter, load file '%s'", filepath);

	if (stat(filepath, &st) < 0) {
		THP_LOGE("stat '%s' failed: %s", filepath, strerror(errno));
		return -1;
	}

	fitlen = st.st_size;
	while ((fitlen % 4)) {
		fitlen++;
	}

	filep = fopen(filepath, "r");
	if (!filep) {
		THP_LOGE("open '%s' failed: %s", filepath, strerror(errno));
		return -1;
	}

	pbuf = (uint8_t *)MALLOC(fitlen);
	if (!pbuf) {
		THP_LOGE("malloc %d bytes for '%s' failed: %s", st.st_size,
		     filepath, strerror(errno));
		fclose(filep);
		return -1;
	}

	if (fread(pbuf, st.st_size, 1, filep) != 1) {
		THP_LOGE("read '%s' failed: %s", filepath, strerror(errno));
		LFREE(pbuf);
		fclose(filep);
		return -1;
	}

	fclose(filep);

	while ((st.st_size < fitlen)) {
		pbuf[st.st_size++] = 0xFF;
	}

	*buf = pbuf;
	*len = fitlen;
	return 0;
}

static int cts_send_fw(uint8_t *fwbuf, size_t len, uint32_t crc32, int step)
{
	int ret = -1;
	off_t offset = 0;

	dump_spi = 0;
	while ((offset + step) < len) {
		ret = cts_drw_write_raw(offset, fwbuf + offset, step);
		if (ret) {
			THP_LOGE("write sram faild: pos = %lld, ret = %d", offset, ret);
			return ret;
		}
		offset += step;
	}

	ret = cts_drw_write_raw(offset, fwbuf + offset, len - offset);
	if (ret) {
		THP_LOGE("write sram faild: offset = %lld, ret = %d", offset, ret);
		return ret;
	}
	dump_spi=1;

	THP_LOGD("%ld bytes sent", len);

	return 0;
}

static int cts_check_sram_crc32(uint32_t addr, size_t len, uint32_t crc32)
{
	int ret;
	int retries = 5;
	uint8_t sf_busy;
	uint32_t crc32_result;

	ret = cts_drw_write_u32(SFCTL_SRAM_ADDR, addr);
	if (ret < 0) {
		THP_LOGE("write sram addr failed: ret=%d", ret);
		return -1;
	}

	ret = cts_drw_write_u32(SFCTL_DATA_LENGTH, len);
	if (ret < 0) {
		THP_LOGE("write data length failed: ret=%d", ret);
		return -1;
	}

	ret = cts_drw_write_u8(SFCTL_SW_CRC_START, 1);
	if (ret < 0) {
		THP_LOGE("write start crc calc failed: ret=%d", ret);
		return -1;
	}

	cts_mdelay(10);

	do {
		ret = cts_drw_read_u8(SFCTL_SF_BUSY, &sf_busy);
		if (ret < 0) {
			THP_LOGE("read sfctl busy failed: ret=%d", ret);
		} else if (sf_busy) {
		}
		THP_LOGE("retries: %d", retries);
	} while (retries-- && sf_busy);

	if (ret < 0) {
		THP_LOGE("read sfctl busy failed: ret=%d", ret);
		return -1;
	} else if (sf_busy) {
		THP_LOGE("sfctl always busy!");
		return -1;
	}

	ret = cts_drw_read_u32(SFCTL_CRC_RESULT, &crc32_result);
	if (ret < 0) {
		THP_LOGE("read crc result failed: ret=%d", ret);
		return -1;
	}

	if (crc32_result != crc32) {
		THP_LOGE("crc mismatch: expect %#010x, %#010x received",
				crc32, crc32_result);
		return -1;
	}

	return 0;
}

static int cts_request_fw(const char *fwpath, uint16_t curr_ver)
{
	int ret = -1;
	size_t len = 0;
	uint32_t crc32;
	uint16_t version;
	char build_date[FW_BUILD_DATE_LENGTH];
	char build_time[FW_BUILD_TIME_LENGTH];

	firmware_info.use_builtin = 1;

	if (!cts_load_file(fwpath, &firmware_info.fw, &len)) {
		THP_LOGI("load firmware '%s' ok, %d@%p", fwpath, len, firmware_info.fw);
		firmware_info.use_builtin = 0;
	} else {
		if (!strcmp(project_id, PROJECT_ID_1)) {
			firmware_info.fw = firmware_pid1;
			len = sizeof(firmware_pid1);
		} else if (!strcmp(project_id, PROJECT_ID_2)) {
			firmware_info.fw = firmware_pid2;
			len = sizeof(firmware_pid2);
		} else {
			THP_LOGE("Invalid project_id: %s, PROJECT_ID_1: %s, PROJECT_ID_2: %s",
				project_id, PROJECT_ID_1, PROJECT_ID_2);
			return -1;
		}
		THP_LOGI("load builtin firmware ok, %d@%p", len, firmware_info.fw);
		firmware_info.use_builtin = 1;
	}

	ret = 0;
	crc32 = cts_crc32(firmware_info.fw, len);
	version = cts_get_unaligned_le16(firmware_info.fw + FW_BIN_VERSION_OFFSET);
	
	firmware_info.crc = crc32;
	firmware_info.size = len;
	firmware_info.fw_ver = version;
	g_version = version;

	MEMCPY(build_date, firmware_info.fw + FW_BUILD_DATE_OFFSET, FW_BUILD_DATE_LENGTH);
	build_date[FW_BUILD_DATE_LENGTH - 1] = '\0';
	MEMCPY(build_time, firmware_info.fw + FW_BUILD_TIME_OFFSET, FW_BUILD_TIME_LENGTH);
	build_time[FW_BUILD_TIME_LENGTH - 1] = '\0';

	THP_LOGI("Load new firmware: ver: 0x%04x, build at %s %s, crc32: 0x%#010x, len: %d",
		version, build_date, build_time, crc32, len);

#ifdef CTS_FOR_RELEASE_VER
	THP_LOGI("curr_ver: 0x%04x, get_ver: 0x%04x", curr_ver, version);
	if (curr_ver == version) {
		ret = 1;
		if (!firmware_info.use_builtin) {
			LFREE(firmware_info.fw);
		}
	}
#endif

	return ret;
}

static int cts_do_upfw(uint16_t curr_ver)
{
	int ret = -1;
	uint8_t index = 2;

	do {
		THP_LOGI("curr_ver: 0x%#04x, new_ver: 0x%#04x, is_use_builtin: %d, send_fw_step: %d",
			curr_ver, firmware_info.fw_ver, firmware_info.use_builtin, send_fw_step[index]);

		if ((ret = cts_send_fw(firmware_info.fw, firmware_info.size, firmware_info.crc, send_fw_step[index])) < 0) {
			THP_LOGE("send firmware failed: ret=%d", ret);
			ret = -1;
			continue;
		}

		ret = cts_check_sram_crc32(0, firmware_info.size, firmware_info.crc);
		if (ret < 0) {
			THP_LOGE("crc error!");
			ret = -1;
		} else {
			ret = 0;
		}
	} while (ret && (++index < ARRAY_SIZE(send_fw_step)));

	if (!firmware_info.use_builtin) {
		LFREE(firmware_info.fw);
	}

	return ret;
}

#if IC_TYPE_ICNT92X8
static int cts_upfw_single(uint16_t curr_ver)
{
	int ret = -1;

	ret = cts_do_upfw(curr_ver);
	if (ret < 0) {
		THP_LOGE("Update slave firmware failed");
		return ret;
	}
	return 0;
}

static int cts_upfw_prework(uint8_t cmd)
{
	int ret, retries = 0;
	uint8_t status;
	
	ret = cts_drw_write_u8(SFCTL_CMD_SEL, cmd);
	if (ret < 0) {
		THP_LOGE("send cmd failed: %d", ret);
		return ret;
	}

	ret = cts_drw_write_u8(SFCTL_START_DEXC, 1);
	if (ret < 0) {
		THP_LOGE("start data transsion failed: %d", ret);
		return ret;
	}

	do {
		ret = cts_drw_read_u8(SFCTL_SF_BUSY, &status);
		if (ret < 0) {
			THP_LOGE("read sfctl busy failed: ret=%d", ret);
		} else if (status == 0)
			break;
		cts_mdelay(30);
	} while (status && ++retries < 10);
	if (status) {
		THP_LOGE("read sfctl busy failed");
		return -1;
	}

	return ret;	
}

static int cts_upfw_sram_to_flash(void)
{
	int ret = -1;

	/* firmware start addr @ flash */
	ret = cts_drw_write_u32(SFCTL_FLASH_ADDR, FLASH_FIRMWARE_START);
	if (ret) {
		THP_LOGE("Write firmware start addr at flash failed");
		return ret;
	}

	/* firmware start addr @ sram */
	ret = cts_drw_write_u32(SFCTL_SRAM_ADDR, SRAM_FIRMWARE_START);
	if (ret) {
		THP_LOGE("Write firmware start addr at sram failed");
		return ret;
	}
	
	/* write firmware lenth */
	ret = cts_drw_write_u32(SFCTL_DATA_LENGTH, firmware_info.size);
	if (ret) {
		THP_LOGE("Write firmware start addr at sram failed");
		return ret;
	}

	ret = cts_upfw_prework(FLASH_CMD_AUTO_PAGE_PROGRAM);
	if (ret) {
		THP_LOGE("upfw to flshprework failed");
		return ret;
	}

	ret = cts_drw_write_u8(SFCTL_RESET, 1);
	if (ret < 0) {
		THP_LOGE("reset flash ip failed: %d", ret);
		return ret;
	}

	return ret;
}


static int cts_flash_to_sram_param(void)
{
	int ret = -1;
	int retries = 0;
	uint8_t status;
	f2r_data data;

	ret = cts_drw_write_u8(SFCTL_CMD_SEL, FLASH_CMD_ERASE_SECTOR);
	if (ret < 0) {
		THP_LOGE("Sector erase failed");
		return ret;
	}

	ret = cts_drw_write_u32(SFCTL_FLASH_ADDR, FLASH_SECTOR_ERASE_START);
	if (ret < 0) {
		THP_LOGE("Send flash start addr failed");
		return ret;
	}

	ret = cts_drw_write_u8(SFCTL_START_DEXC, 1);
	if (ret < 0) {
		THP_LOGE("start data transsion failed");
		return ret;
	}

	do {
		cts_drw_read_u8(SFCTL_SF_BUSY, &status);
		if (status == 0)
			break;
		cts_mdelay(10);
	} while (status && retries++ < 10);
	if (status) {
		THP_LOGE("read sfctl busy failed");
		return -1;
	}

	data.f2r_en = 0x0000C35A;
	data.len = firmware_info.size;
	data.crc_en = 0xFFFFFFFF;
	data.crc = firmware_info.crc;
	ret = cts_drw_write_raw(SRAM_FIRMWARE_START, (uint8_t *)&data, 16);
	if (ret) {
		THP_LOGE("Send data from flash to sram failed");
		return ret;
	}

	ret = cts_drw_write_u32(SFCTL_FLASH_ADDR, FLASH_SECTOR_ERASE_START);
	if (ret < 0) {
		THP_LOGE("Send flash start addr failed");
		return ret;
	}

	ret = cts_drw_write_u32(SFCTL_SRAM_ADDR, SRAM_FIRMWARE_START);
	if (ret < 0) {
		THP_LOGE("Send sram start addr failed");
		return ret;
	}

	ret = cts_drw_write_u32(SFCTL_DATA_LENGTH, 16);
	if (ret < 0) {
		THP_LOGE("Write data len failed");
		return ret;
	}

	ret = cts_drw_write_u8(SFCTL_CMD_SEL, FLASH_CMD_AUTO_PAGE_PROGRAM);
	if (ret < 0) {
		THP_LOGE("Write auto page program failed");
		return ret;
	}

	ret = cts_drw_write_u8(SFCTL_START_DEXC, 1);
	if (ret < 0) {
		THP_LOGE("start data transsion failed: %d", ret);
		return ret;
	}

	do {
		cts_drw_read_u8(SFCTL_SF_BUSY, &status);
		if (status == 0)
			break;
	} while (status && retries++ < 5);
	if (status) {
		THP_LOGE("read sfctl busy failed");
		return -1;
	}

	return ret;
}

static int cts_set_nvr_to_default(void)
{
	int ret = -1;
	int retries;
	uint32_t flash_addr = 0;

	for (retries = 0; retries < 3; retries++) {
		ret = cts_drw_write_u32(SFCTL_FLASH_ADDR, flash_addr);
		if (ret) {
			THP_LOGE("write 0x%x to flash addr 0x%x failed", flash_addr, SFCTL_FLASH_ADDR);
			continue;
		}
		ret = cts_nvr_lock();
		if (ret) {
			THP_LOGE("lock nvr area failed");
			continue;
		}
		break;
	}
	if (retries >= 3) {
		THP_LOGE("set nvr area failed, retries: %d", retries);
		return -1;
	}
	return 0;
}

int cts_update_firmware(uint16_t curr_ver)
{
	int ret = -1;
	int retries = 3;
	uint32_t hwid;
	uint32_t bootmode;

	ret = cts_request_fw(FIRMWARE_FILE, curr_ver);
	if (ret == 1) {
		THP_LOGI("No need update firmware");
		goto to_return;
	} else if (ret < 0) {
		THP_LOGE("Request firmware failed");
		goto to_return;
	}

	ret = cts_set_spi_speed(SPI_SPEED_PROG);
	if (ret < 0) {
		THP_LOGW("Set spi speed failed");
	}

	ret = cts_enter_drw_mode();
	if (ret < 0) {
		THP_LOGE("Enter prog mode failed");
		return ret;
	}

	do {
		ret = cts_drw_read_u32(SYS_CHIP_VER, &hwid);
		if ((ret == 0) && (hwid == HWID_ICNT9268)) {
			break;
		}
	} while (ret && retries-- > 0);

	if (ret) {
		THP_LOGE("Read hwid failed");
		return ret;
	}


	/***********************20221210**********************/
	ret = cts_drw_read_u32(SYS_BOOT_MODE, &bootmode);
	if (ret) {
		THP_LOGE("Read bootmode failed");
		return ret;
	}
	THP_LOGI("bootmode: 0x%08x", bootmode);
	if (((bootmode >> 8) & BOOT_MODE_MASK) != BOOT_MODE_PROGRAM) {
		THP_LOGE("bootmode 0x%x != 0x%x", (bootmode >> 8), BOOT_MODE_PROGRAM);
		return -1;
	}

	ret = cts_drw_write_u8(SFCTL_RESET, 1);
	if (ret < 0) {
		THP_LOGE("reset flash ip failed: %d", ret);
		return ret;
	}

	ret = cts_drw_read_u32(SYS_BOOT_MODE, &bootmode);
	if (ret) {
		THP_LOGE("Read bootmode failed");
		return ret;
	}
	if (((bootmode >> 8) & BOOT_MODE_MASK) != BOOT_MODE_PROGRAM) {
		THP_LOGE("bootmode 0x%x != 0x%x", (bootmode >> 8), BOOT_MODE_PROGRAM);
		return -1;
	}

	/**********According AE Request Change Erase Flash Flow 20230428**********/
	ret = cts_drw_write_u8(SFCTL_RESET, 1);
	if (ret < 0) {
		THP_LOGE("reset flash ip failed: %d", ret);
		return ret;
	}
	
	ret = cts_drw_write_u8(EFCTL_VOL_REG1, 0);
	if (ret < 0) {
		THP_LOGE("reset flash vol reg1(0x%x) failed: %d", EFCTL_VOL_REG1, ret);
		return ret;
	}
	ret = cts_drw_write_u8(EFCTL_VOL_REG2, 0);
	if (ret < 0) {
		THP_LOGE("reset flash vol reg2(0x%x) failed: %d", EFCTL_VOL_REG2, ret);
		return ret;
	}
	cts_mdelay(10);
	/*************************************************************************/

	ret = cts_set_nvr_to_default();
	if (ret) {
		THP_LOGE("set nvr to default failed");
		return ret;
	}
	
	ret = cts_upfw_prework(FLASH_CMD_ERASE_CHIP);
	if (ret) {
		THP_LOGE("upfw to sram prework failed");
		return ret;
	}
	/*****************************************************/

	
	/***************update firmware to sram***************/
	ret = cts_upfw_single(curr_ver);
	if (ret < 0) {
		THP_LOGE("Update master firmware failed");
		return ret;
	}
	/*****************************************************/

	/***********************20221210**********************/
	/**********update firmware from sram to flash*********/
	ret = cts_upfw_sram_to_flash();
	if (ret) {
		THP_LOGE("Update firmware from sram to flash failed");
		return ret;
	}
	
	ret = cts_flash_to_sram_param();
	if (ret) {
		THP_LOGE("Flash to sram param failed");
		return ret;
	}

	ret = cts_drw_write_u8(REGDEF_RSTCFG, 0xFE);
	if (ret) {
		THP_LOGE("Reset chip failed");
		return ret;
	}
	/*****************************************************/

	ret = cts_drw_write_u8(SYS_BOOT_MODE, BOOT_MODE_SRAM);
	if (ret < 0) {
		THP_LOGE("Change to normal mode failed: %d", ret);
		return ret;
	}

	cts_mdelay(40);
	
to_return:
	return ret;
}
#else
extern void init_system_settings();
int cts_update_firmware(uint16_t curr_ver)
{
	int ret = -1;
	enum DOWNLOAD_FIRMWARE_TYPE mode = DOWNLOAD_FIRMWARE_TYPE_FLASH;
	int retrycnt = 3;
	uint8_t firmware_buff[FIRMWARE_MAX_LENGTH];

	ret = cts_request_fw(FIRMWARE_FILE, curr_ver);
	if (ret == 1) {
		THP_LOGI("No need update firmware");
		return 0;
	} else if (ret < 0) {
		THP_LOGE("Request firmware failed");
		goto to_return;
	}

	MEMSET(firmware_buff, 0, sizeof(firmware_buff));
	MEMCPY(firmware_buff, firmware_info.fw, firmware_info.size);

	ret = cts_set_spi_speed(SPI_SPEED_PROG);
	if (ret < 0) {
		THP_LOGW("Set spi speed failed");
	}

	ret = cts_enter_drw_mode();
	if (ret < 0) {
		THP_LOGE("Enter prog mode failed");
		goto to_return;
		//return ret;
	}
	cts_mdelay(10);
	init_system_settings();	
	if (DownloadFlashbinary(firmware_info.size, firmware_buff, mode, 1, 100) != 0) {
		do {
			if (GetChipConnectStatus(0, 1) != 0)
			{
				break;
			}
			else
			{
				THP_LOGE("%s[%d]: GetChipConnectStatus failed\n", __FILE__, __LINE__);
			}
			cts_mdelay(10);
			retrycnt--;
		} while (retrycnt > 0);

		if (0 == retrycnt)
		{
			THP_LOGE("%s[%d]: Firmware download failed\n", __FILE__, __LINE__);
			ret = -1;
			//goto to_return;
		}
	} else{
		THP_LOGE("%s[%d]: Download firmware failed", __FILE__, __LINE__);
		ret = -1;
	}
	
to_return:
	if (!firmware_info.use_builtin) {
		LFREE(firmware_info.fw);
	}
	return ret;
}
#endif