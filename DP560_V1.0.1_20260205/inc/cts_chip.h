#ifndef _CHIPONE_TOUCH_CHIP_H
#define _CHIPONE_TOUCH_CHIP_H
//////////////////////////////////////////////////////////////////////////////
/*************** Only one can be selected ***************/
#define IC_TYPE_ICNT92X8						(0)	/* 9268/9288 */
#define IC_TYPE_ICNT9268S						(1)
#define IC_TYPE_ICNT93XX						(0)	/* 9385/9388 */
/********************************************************/
#define __IC_TYPE_SUM (IC_TYPE_ICNT92X8 + IC_TYPE_ICNT9268S + IC_TYPE_ICNT93XX)
#if __IC_TYPE_SUM != 1
    #error "IC type config error"
#endif
#undef __IC_TYPE_SUM
#include "cts_tcs.h"

#if IC_TYPE_ICNT92X8
#define CTS_IC_HWID								(0x00009268)
#define TR_NUM_MAX								(126)
#define TR_ORDER_MAX							(84)

#elif IC_TYPE_ICNT9268S
#define FIRMWARE_MAX_LENGTH     				(192 * 1024)
#define CTS_IC_HWID								(0x00009368)
#define TR_NUM_MAX								(63)
#define TR_ORDER_MAX							(42)

#elif IC_TYPE_ICNT93XX
#define FIRMWARE_MAX_LENGTH     				(416 * 1024)
#define CTS_IC_HWID								(0x00009388)
#define TR_NUM_MAX								(50 + 85)
#define TR_ORDER_MAX							(85)

#endif

#define CTS_IC_HWID_MASK						(0x0000FFFF)

//////////////////////////////////////////////////////////////////////////////

#define MATH_PATH_SYS               			512

#define TCS_RETRY_NUM               			3
#define I2C_RETRY_NUM               			3
#define TransferSize                			65
#define SPI_FIFO_LENGTH             			62

#define TCS_SPI_WRITE							0xF0
#define TCS_SPI_READ							0xF1
#define DRW_SPI_WRITE							0x60
#define DRW_SPI_READ							0x61

#define I2C_NORMAL_ADDR							0x90
#define I2C_PROG_ADDR							0x60

#define READ_FLASH_BUSY_TIMEOUT     			1000

//////////////////////////////////////////////////////////////////////////////
#define REGDEF_BASE                         	(0x00070000)
#define REGDEF_CHIP_VER                     	(REGDEF_BASE + 0x0000)
#define REGDEF_RSTCFG                       	(REGDEF_BASE + 0x0008)
#define REGDEF_BOOT_MODE                    	(REGDEF_BASE + 0x0010)

#define SFCTL_BASE                          	(0x00074000)
#define SFCTL_CMD_SEL                       	(SFCTL_BASE + 0x0000)
#define SFCTL_RESET                         	(SFCTL_BASE + 0x0003)
#define SFCTL_FLASH_ADDR                    	(SFCTL_BASE + 0x0004)
#define SFCTL_SRAM_ADDR	                    	(SFCTL_BASE + 0x0008)
#define SFCTL_DATA_LENGTH                   	(SFCTL_BASE + 0x000C)
#define SFCTL_START_DEXC                    	(SFCTL_BASE + 0x0010)
#define SFCTL_RELEASE_FLASH                 	(SFCTL_BASE + 0x0014)
#define SFCTL_CLEAR_HW_STATE                	(SFCTL_BASE + 0x0018)
#define SFCTL_CRC_RESULT                    	(SFCTL_BASE + 0x001C)
#define SFCTL_SW_CRC_START                  	(SFCTL_BASE + 0x0020)
#define SFCTL_SF_BUSY                       	(SFCTL_BASE + 0x0024)
#define SFCTL_WATCHDOG_CRC_CFG              	(SFCTL_BASE + 0x0028)
#define SFCTL_WATCHDOG_CRC_TARGET           	(SFCTL_BASE + 0x002C)
#define SFCTL_MCHECK_CFG                    	(SFCTL_BASE + 0x0030)
#define SFCTL_SINGLE_BYTE_CFG               	(SFCTL_BASE + 0x003C)


#if IC_TYPE_ICNT92X8
#define FLASH_CMD_FAST_READ						(0x01)
#define FLASH_CMD_TEST_PAGE_PROGRAM				(0x03)
#define FLASH_CMD_ERASE_SECTOR					(0x04)
#define FLASH_CMD_ERASE_CHIP					(0x05)
#define FLASH_CMD_PAGE_PROGRAM					(0x06)
#define FLASH_CMD_AUTO_PAGE_PROGRAM				(0x07)
#else
#define FLASH_CMD_FAST_READ                 	(0x01)
#define FLASH_CMD_ERASE_SECTOR              	(0x02)
#define FLASH_CMD_ERASE_BLOCK               	(0x03)
#define FLASH_CMD_PAGE_PROGRAM              	(0x04)
#define FLASH_CMD_READ_STATUS               	(0x05)
#define FLASH_CMD_READ_IDENTIFICATION       	(0x06)
#define FLASH_CMD_COMM_BYTE                 	(0x07)
#define FLASH_CMD_READ_STATUSH              	(0x08)
#define FLASH_CMD_WRITE_STATUS              	(0x09)
#define FLASH_CMD_DEEP_POWER                	(0x0A)
#define FLASH_CMD_RELEASE_DP                	(0x0B)
#define FLASH_CMD_ERASE_CHIP                	(0x0C)
#endif

#define REGDEF_N11M_BASE                    	(0x00470000)
#define REGDEF_N11_RSTREG_ADDRESS           	(REGDEF_N11M_BASE + 0x000008)
#define REGDEF_N11M_MBIST_ADDRESS           	(REGDEF_N11M_BASE + 0x000C04)

#define REGDEF_G40_BASE                     	(0x00070000)
#define REGDEF_G40_MBIST_ADDRESS            	(REGDEF_G40_BASE + 0x000C14)
#define REGDEF_G40_INTREG_ADDRESS           	(REGDEF_G40_BASE + 0x003017)
#define REGDEF_G40_SP2_RXFIFO_DATA          	(REGDEF_G40_BASE + 0x009008)

#define SPI_BASE                            	(0x40038000)
#define SPI_CFG                             	(SPI_BASE + 0x0000)

//////////////////////////////////////////////////////////////////////////////
#define FLASH_CONFIGURE_ADDRESS                 (0x02F000)
#define FLASH_CONFIGURE_LENGTH                  (20)

#define SECTOR_SIZE_4KB	                        (4 * 1024)
#define SECTOR_SIZE_8KB	                        (8 * 1024)
#define SECTOR_SIZE_32KB                        (32 * 1024)

#define FLASH_BASE_ADDRESS                      (0x00000000)
#define FLASH_PAGE_PRGRAM_SIZE              	(0x100)

#if IC_TYPE_ICNT9268S
#define FLASH_PARTITION_TOTAL_SIZE              (0x40000)
#define FLASH_PARTITION_ENABLE_FALG             (0x0000C35A)
#define FLASH_PARTITION_FWCODE_BEGIN            (FLASH_BASE_ADDRESS + 0x00000000)
#define FLASH_PARTITION_FWCODE_SIZE             (0x30000)
#define FLASH_PARTITION_MAX_BLOCK_SIZE          (0x10000)
#define FLASH_PARTITION_FIRMWARE_SRAM_SIZE      (0x18000)       
#define FLASH_PARTITION_FIRMWARE_FLASH_SIZE     (0x2F000)
#define FLASH_OVERLAP_MAXLENGTH                 (0x28000)

#define FLASH_FLASH2RAM_START_ADDRESS           (FLASH_BASE_ADDRESS + 0x2FFE0)
#define FLASH_FLASH2RAM_CODE_OFFSET_ADDRESS     (FLASH_BASE_ADDRESS + 0x00000)

#define FLASH_FLASH2REG_INFO_START_ADDRESS      (FLASH_BASE_ADDRESS + 0x3BFB0)  
#define FLASH_IC_CONFIG_START_ADDRESS           (FLASH_BASE_ADDRESS + 0x3C000)
#define FLASH_FLASH2REG_INFO_LAST_ADDRESS       (FLASH_BASE_ADDRESS + 0x3C040) 
#define FLASH_FLASH2REG_START_ADDRESS           (FLASH_BASE_ADDRESS + 0x3CFE8)
#define FLASH_FLASH2REG_ADDRESS_LENGTH          (20 + 4)

#define FLASH_ID_INFO_ADDRESS                   (FLASH_BASE_ADDRESS + 0x3D000)
#define FLASH_ID_INFO_LENGTH                    (128)
#define PROJECT_ID_COUNT                        (108)

#define FLASH_HSYNCOSCTRIM_ADDRESS              (FLASH_BASE_ADDRESS + 0x31800)
#define FLASH_FT_TRIMCODE_ADDRESS               (FLASH_BASE_ADDRESS + 0x30FF8)
#define FLASH_NVR6BACKUP_ADDRESS                (FLASH_BASE_ADDRESS + 0x32800)

#define FLASH_OSCTRIMBACKUP_ADDRESS             (FLASH_BASE_ADDRESS + 0x31600)
#define FLASH_OSCTRIMBACKUP_LENGTH              (32)
#define FLASH_WAFERLOTID_ADDRESS                (FLASH_BASE_ADDRESS + 0x39000)

#elif IC_TYPE_ICNT93XX
#define FLASH_PARTITION_TOTAL_SIZE              (0x80000)
#define FLASH_PARTITION_ENABLE_FALG             (0x0000C35A)
#define FLASH_PARTITION_FWCODE_BEGIN            (FLASH_BASE_ADDRESS + 0x00000)
#define FLASH_PARTITION_FWCODE_SIZE             (0x68000)
#define FLASH_PARTITION_MAX_BLOCK_SIZE          (0x20000)
#define FLASH_PARTITION_FIRMWARE_SRAM_SIZE      (0x28000)       
#define FLASH_PARTITION_FIRMWARE_FLASH_SIZE     (0x67000)
#define FLASH_OVERLAP_MAXLENGTH                 (0x38000)

#define FLASH_FLASH2RAM_START_ADDRESS           (FLASH_BASE_ADDRESS + 0x67FE0)
#define FLASH_FLASH2RAM_CODE_OFFSET_ADDRESS     (FLASH_BASE_ADDRESS + 0x00000)

#define FLASH_FLASH2REG_INFO_START_ADDRESS      (FLASH_BASE_ADDRESS + 0x73F80)  
#define FLASH_IC_CONFIG_START_ADDRESS           (FLASH_BASE_ADDRESS + 0x74000)
#define FLASH_FLASH2REG_INFO_LAST_ADDRESS       (FLASH_BASE_ADDRESS + 0x74050) 
#define FLASH_FLASH2REG_START_ADDRESS           (FLASH_BASE_ADDRESS + 0x74FE8)
#define FLASH_FLASH2REG_ADDRESS_LENGTH          (24)

#define FLASH_ID_INFO_ADDRESS                   (FLASH_BASE_ADDRESS + 0x75000)
#define FLASH_ID_INFO_LENGTH                    (128)
#define PROJECT_ID_COUNT                        (108)
#define FLASH_WAFERLOTID_ADDRESS                (FLASH_BASE_ADDRESS + 0x71000)

#define FLASH_OSCTRIMBACKUP_ADDRESS             (FLASH_BASE_ADDRESS + 0x74C00)
#define FLASH_OSCTRIMBACKUP_LENGTH              (32)
#endif

//////////////////////////////////////////////////////////////////////////////

typedef int(*CHIPONE_SPI_INIT)(int spi_mode, int spi_scale);
typedef int(*CHIPONE_SPI_WRITE)(unsigned char *wbuf, unsigned int wsize, unsigned char *rbuf, unsigned int rsize);
typedef int(*CHIPONE_I2C_INIT)(int i2c_scale, int i2c_normal_address, int i2c_program_address);
typedef int(*CHIPONE_I2C_WRITE)(unsigned char addr, unsigned char *buf, unsigned int size);
typedef int(*CHIPONE_I2C_READ)(unsigned char addr, unsigned char *buf, unsigned int size);
typedef int(*CHIPONE_CLEAR_NCK_STATE)();
typedef int(*CHIPONE_SET_RESET_PIN)(int val);

extern CHIPONE_SPI_INIT spi_init;
extern CHIPONE_SPI_WRITE spi_write;
extern CHIPONE_I2C_INIT i2c_init;
extern CHIPONE_I2C_WRITE i2c_write;
extern CHIPONE_I2C_READ i2c_read;
extern CHIPONE_CLEAR_NCK_STATE clear_nck_state;
extern CHIPONE_SET_RESET_PIN set_reset_pin;

//////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)
typedef struct
{
	unsigned char log_gate;
	unsigned char sync_fw_paras;
	unsigned char fw_system_type;
	char download_firmware_path[MATH_PATH_SYS];
	char download_firmware_path2[MATH_PATH_SYS];
	unsigned char download_firmware_master;
	unsigned char download_firmware_slave;
	unsigned char download_mode;
	int m_TransMode;
	int m_Evdd;
	unsigned char m_useExtPower;
	int m_I2CScale;
	int m_SpiScale;
	int m_I2cAddr;
	int m_ProgI2cAddr;
	int m_SpiAddr;
	int m_ProgSpiAddr;
	int m_ProgI2cAddrSlave;
	int m_NormSpiMode;
	int m_ProgSpiMode;
	int m_SpiProgReadDummyBytes;
	int m_SpiCheckCrc;
	int m_SpiCrcNum;
	unsigned char m_UsbDataTransferMode;
	unsigned char wakeup_address;
	unsigned char m_drw_mode;//true:drw
	int m_data_ready_mode;// 0: polling, 1: interrupt
	int m_I2cNormalSlave1Addr;
	int m_I2cNormalSlave2Addr;
	int m_I2cProgSlave2Addr;
	char download_firmware_path3[MATH_PATH_SYS];
	unsigned char download_firmware_slave2;
	unsigned char i2c_single_port;
	unsigned char check_borad_fw;
	unsigned char m_SpiType;
	unsigned char m_FlashSpiType;
}SYSTEM_SETTINGS;

typedef struct
{
	unsigned char baseFlag;
	unsigned char classID;
	unsigned char cmdID;
	unsigned char isRead;
	unsigned char isWrite;
	unsigned char isData;
	int retLen;
} STRUCT_TCS_CMD;

#pragma pack(pop)

enum DOWNLOAD_FIRMWARE_TYPE
{
    DOWNLOAD_FIRMWARE_TYPE_SRAM = 0,
    DOWNLOAD_FIRMWARE_TYPE_FLASH,
};

enum FLASH_ERASE_MODE
{
    FLASH_ERASE_MODE_SECTOR_ERASE = 0x01,
    FLASH_ERASE_MODE_BLOCK_ERASE = 0x02,
    FLASH_ERASE_MODE_CHIP_ERASE = 0x04,
};

//////////////////////////////////////////////////////////////////////////////
int I2C_Init(int i2c_scale, int i2c_normal_address, int i2c_program_address);
int ClearNckState();
//int SetRstPin(int val);

int SPI_Init(int spi_mode, int spi_scale);
int SPI_Write(unsigned char* wbuf, unsigned int wsize, unsigned char* rbuf, unsigned int rsize);
int SpiWriteProg(unsigned int dataAddr, unsigned char *data, unsigned int len);
int SpiReadProg(unsigned int dataAddr, unsigned char *data, unsigned int len);
unsigned short CalcCRC16(unsigned char *buf, unsigned int len, unsigned int offset);
void ChangeSpiMode(int func_spi_mode);
unsigned int ChanageDieModeAddress(unsigned int dataAddr);
int ChipReset(int bProgMode);

int BootFromFlash(int bProgMode, int millisecondsTimeout);
int GotoNormalMode();
int IsProgMode();
int WriteTCS(STRUCT_TCS_CMD cmd, unsigned char *data, unsigned int len, int delay);
unsigned char GetClassID(unsigned char baseFlag, unsigned char onlyRead, unsigned char onlyWrite, unsigned char classID);
unsigned char CalcCheckSum(unsigned char *data, unsigned int len, unsigned int offset);
int I2cWriteTCS(STRUCT_TCS_CMD cmd, unsigned char * data, unsigned int len);
int SpiWriteTCS(STRUCT_TCS_CMD cmd, unsigned char *data, unsigned int len, int delay);
int IsSupportedMcu();
int ReadDataTCS(STRUCT_TCS_CMD cmd, unsigned char *buffer, unsigned int len, int delay);
int I2cReadDataTCS(STRUCT_TCS_CMD cmd, unsigned char * buffer, unsigned int len, int delay);
int SpiReadDataTCS(STRUCT_TCS_CMD cmd, unsigned char * buffer, unsigned int len, int delay);
int ReadXdataTCS(unsigned int offset, unsigned char * buffer, unsigned int len, int delay);
int ReadSubXdataTCS(unsigned int offset, unsigned char * buffer, unsigned int len, int delay);
int WriteXdataTCS(unsigned int offset, unsigned char * buffer, unsigned int len, int delay);
void GetTcsCmd(enum TcsCmdIndex index, STRUCT_TCS_CMD *cmd);
int DownloadFlashbinary(int len, unsigned char* buf, enum DOWNLOAD_FIRMWARE_TYPE mode, unsigned char isBoot, int delay);
int VerifyIsBigBinFile(int len, unsigned char * buf);
unsigned int GetSectorStartAddress(unsigned int u32Address, unsigned int SectorSize);
int ReadProg(unsigned int dataAddr, unsigned char * data, unsigned int len);
int WriteProg(unsigned int dataAddr, unsigned char * data, unsigned int len);
int FlashBusyWait();
int ReadFlashBuffer(unsigned int u32FlashAddr, unsigned char * buf, unsigned int length);
int EraseSecotorFlash(unsigned int u32FlashAddr, unsigned int len);
int EraseFlash_S(enum FLASH_ERASE_MODE mode, unsigned int u32FlashAddr, unsigned int len);
int FlashState(unsigned char *state);
int DownloadFirmware(int firmwareLength, unsigned char* firmwareBuffer, enum DOWNLOAD_FIRMWARE_TYPE mode, unsigned char isBoot, int delay);
int GetFlashId(unsigned int *id);
int EraseFlashCodeRegion();
unsigned int CalcCRC32(unsigned char* buf, unsigned int len, unsigned int offset);
int DownloadToRam(unsigned char* buf, int len);
int WriteRamProgMode(unsigned int addr, unsigned char* data, int len, int step);
unsigned int ChipCalculateCrc(unsigned int address, unsigned int len, unsigned char bFlash);
int WriteFlashBuffer(unsigned int u32FlashAddr, unsigned char * buffer, unsigned int length);
int CopyMemToFlashEx2(unsigned int flash_addr, unsigned int sram_addr, unsigned int len);
int CopyFlashToMemEx2(unsigned int flash_addr, unsigned int sram_addr, unsigned int len);
int WriteFlash2Ram(unsigned int codeLength, unsigned int codeCrcValue, unsigned int extraCodeLength, unsigned int extraCodeCrc);
int FillFlash2reg(unsigned int actRegLength, unsigned char* SectorBuffer);
int GotoProgModeSPI();
int GetFirmwareVersion(unsigned short *version);
int GetChipConnectStatus(unsigned char bProgmode, unsigned char bdelay);
int GetChipID(unsigned char *id);
int GotoProgMode(unsigned char bResetSFCTL, unsigned char bRestoreReg);
int ReadCmdTCS(STRUCT_TCS_CMD cmd, unsigned char * buffer, unsigned int readLen, int readOffset, int delay);
int GetChipIdVersion(unsigned short * version);

int GetTcsVersion(unsigned short* _tcsVersion);

int GetProjectID(char *id);
int SetProjectID(char *id);

int ReadWaferLotID(char* lotID, int len);
#endif
