#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <malloc.h>
// #include <Windows.h>
#include "cts_chip.h"
#include "cts_log.h"

CHIPONE_SPI_INIT spi_init = NULL;
CHIPONE_SPI_WRITE spi_write = NULL;
CHIPONE_I2C_INIT i2c_init = NULL;
CHIPONE_I2C_WRITE i2c_write = NULL;
CHIPONE_I2C_READ i2c_read = NULL;
CHIPONE_CLEAR_NCK_STATE clear_nck_state = NULL;
CHIPONE_SET_RESET_PIN set_reset_pin = NULL;

SYSTEM_SETTINGS system_settings;
extern void cts_mdelay(uint32_t ms);
extern int cts_reset_device();

int retry_num = 3;
const int SpiReadDummyBytes = 0;
unsigned char chipTypeId[2] = { 0x68, 0x93 };

/////////////////////////////////////////////////////////////////////////////
#if IC_TYPE_ICNT92X8
	// do nothing
#else
void ChangeSpiMode(int func_spi_mode)
{

}

int SPI_Init(int spi_mode, int spi_scale)
{
	if (spi_init == NULL)
		return 0;

	return spi_init(spi_mode, spi_scale);
}

int SPI_Write(unsigned char* wbuf, unsigned int wsize, unsigned char* rbuf, unsigned int rsize)
{
	// if (spi_write == NULL)
	// 	return 0;
	int ret = 0;
	ret = cts_spi_sync_raw(wbuf, wsize, rbuf, rsize);
	if (ret){
		THP_LOGE("err:%d",ret);
		return 0;
	}
	return 1;
}

int I2C_Init(int i2c_scale, int i2c_normal_address, int i2c_program_address)
{
	if (i2c_init == NULL)
		return 0;

	return i2c_init(i2c_scale, i2c_normal_address, i2c_program_address);
}

int I2C_Write(unsigned char addr, unsigned char *buf, unsigned int size)
{
	if (i2c_write == NULL)
		return 0;

	return i2c_write(addr, buf, size);
}

int I2C_Read(unsigned char addr, unsigned char *buf, unsigned int size)
{
	if (i2c_read == NULL)
		return 0;

	return i2c_read(addr, buf, size);
}

//清除I2C NAK状态
int ClearNckState()
{
	if (clear_nck_state == NULL)
		return 0;

	return clear_nck_state();
}

int I2cWriteProgMode(unsigned int uiOffset, unsigned char *buffer, unsigned int uiSize)
{
	if (system_settings.m_TransMode != 0)
	{
		return SpiWriteProg(uiOffset, buffer, uiSize);
	}
	else
	{
		//DEMO代码使用USB传输，所以拆包写入，可以根据需要自己实现一次写入
		unsigned char rptBuf[65];
		unsigned int src_addr = uiOffset;
		unsigned int src_index = 0;
		unsigned int src_len = uiSize;

		while (src_len != 0)
		{
			unsigned int write_len = src_len;
			if (write_len > 55)
				write_len = 55;

			//write address
			rptBuf[0] = (unsigned char)(src_addr >> 16);
			rptBuf[1] = (unsigned char)(src_addr >> 8);
			rptBuf[2] = (unsigned char)(src_addr);
			for (unsigned int i = 0; i < write_len; i++)
				rptBuf[3 + i] = buffer[src_index++];

			if (I2C_Write(I2C_PROG_ADDR, rptBuf, write_len + 3) == 0)
			{
				ClearNckState();
				return 0;
			}

			src_addr += write_len;
			src_len -= write_len;
		}

		return 1;
	}
}

int I2cReadProgMode(unsigned int uiOffset, unsigned char *buffer, unsigned int uiSize)
{
	if (system_settings.m_TransMode != 0)
	{
		return SpiReadProg(uiOffset, buffer, uiSize);
	}
	else
	{
		//DEMO代码使用USB传输，所以拆包读取，可以根据需要自己实现一次读取
		unsigned char rptBuf[65];
		unsigned int src_addr = uiOffset;
		unsigned int src_index = 0;
		unsigned int src_len = uiSize;

		while (src_len != 0)
		{
			unsigned int read_len = src_len;
			if (read_len > 57)
				read_len = 57;

			rptBuf[0] = (unsigned char)(src_addr >> 16);
			rptBuf[1] = (unsigned char)(src_addr >> 8);
			rptBuf[2] = (unsigned char)(src_addr);
			if (I2C_Write(I2C_PROG_ADDR, rptBuf, 3) == 0)
			{
				ClearNckState();
				return 0;
			}

			//read data
			if (I2C_Read(I2C_PROG_ADDR, rptBuf, read_len) == 0)
			{
				ClearNckState();
				return 0;
			}

			src_addr += read_len;
			for (unsigned int i = 0; i < read_len; i++)
				buffer[src_index++] = rptBuf[i];
			src_len -= read_len;
		}
		return 1;
	}
}

int GotoProgMode(unsigned char bResetSFCTL, unsigned char bRestoreReg)
{
	unsigned char ucTemp[4];

	if (0 == system_settings.m_TransMode)
	{
		ucTemp[0] = 0x5a;
		if (0 == WriteProg(0xcc3355, ucTemp, 1))
			return 0;

		if (0 == IsProgMode())
			return 0;		
	}
	else
	{
		if (0 == GotoProgModeSPI())
			return 0;
	}

    if (bResetSFCTL != 0)
    {

    }

	if (bRestoreReg)
	{
		
	}

	return 1;
}

int ChipReset(int bProgMode)
{
	int bRes = 1;
	unsigned char ucTemp[4];
	ucTemp[0] = 0xFE;

	if (bProgMode)
	{
		if (0 == WriteProg(REGDEF_RSTCFG, ucTemp, 1))
		{
			bRes = 0;
		}
	}
	else
	{
		if (0 == WriteXdataTCS(REGDEF_RSTCFG, ucTemp, 1, 0))
		{
			bRes = 0;
		}
	}

	return bRes;
}

int ChipResetAndSleep(int bProgMode, int millisecondsTimeout)
{
	int bRes = ChipReset(bProgMode);

	if (bRes != 0 && millisecondsTimeout > 0)
	{
		cts_mdelay(millisecondsTimeout);
	}

	return bRes;
}

int BootFromFlash(int bProgMode, int millisecondsTimeout)
{
	const int blockTimeout = 20;

	//cts_reset_device();
	if (bProgMode != 0)
	{
		GotoProgMode(1, 1);
	}
	int result = ChipResetAndSleep(bProgMode, millisecondsTimeout);
	if (result != 0)
	{
		int totalTimeout = 500;
		while (totalTimeout > 0)
		{
			unsigned char ucTemp[4];
			if (GetChipID(ucTemp) != 0)
			{
				THP_LOGI("read chip ID success [%x][%x]",ucTemp[0],ucTemp[1]);
				break;
			}
			totalTimeout -= blockTimeout;
		}
	}
	return result;
}

int GotoProgModeSPI()
{
	unsigned char ucTemp[4];
	unsigned char ucSpi[4];

	if (IsProgMode() != 0)
	{
		return 1;
	}

	thp_dev_reset(0);
	cts_mdelay(5);
	thp_dev_reset(1);
	cts_mdelay(30);

	ucTemp[0] = 0x5a;
	if (0 == WriteProg(0xcc3355, ucTemp, 1))
		return 0;

	if (IsProgMode() != 0)
	{
		return 1;
	}

	//Method1:wirte TCH prog mode
	ucTemp[0] = 0x02;
	WriteXdataTCS(REGDEF_BOOT_MODE, ucTemp, 1, 0);

	if (IsProgMode() != 0)
	{
		return 1;
	}

	//Method2: cc 33 55 5a
	ucTemp[0] = 0x5a;
	if (0 == WriteProg(0xcc3355, ucTemp, 1))
		return 0;

	if (IsProgMode() != 0)
	{
		return 1;
	}

	//Method3: write mcu tp reset .same method1 ,no rst pin
	if (0 == IsSupportedMcu())
	{
		return 0;
	}

	//////////////////////////
	//Method4: close PRGMSWITCH_DETECT_OFF
	if (0 == ReadXdataTCS(SPI_CFG, ucTemp, 4, 0))
	{
		return 0;
	}

	for (int i = 0; i < 4; i++)
	{
		ucSpi[i] = ucTemp[i];
	}

	ucTemp[1] = (unsigned char)(ucTemp[1] & 0x7F);
	if (0 == WriteXdataTCS(SPI_CFG, ucTemp, 4, 0))
	{
		return 0;
	}

	ucTemp[0] = 0x5a;
	if (0 == WriteProg(0xcc3355, ucTemp, 1))
		return 0;

	return IsProgMode();
}

int IsProgMode()
{
	unsigned char ucTemp[4] = { 0 };
	if ((system_settings.m_TransMode != 0) && system_settings.m_drw_mode)
	{
		if (ReadProg(REGDEF_BOOT_MODE, ucTemp, 4) != 0)
		{
			if ((ucTemp[1] & 0x07) == 0x02)
			{
				return 1;
				//THP_LOGE("[%s][%d]: value error %d\n", __FILE__, __LINE__,ucTemp[1]);
			}
		}
	}
	else
	{
		if (ReadProg(REGDEF_CHIP_VER, ucTemp, 4) != 0)
		{
			if (ucTemp[0] == chipTypeId[0] && ucTemp[1] == chipTypeId[1])
				return 1;
		}
	}

	return 0;
}

int GotoNormalMode()
{
	unsigned char ucTemp[4];

	ucTemp[0] = 0x03;
	if (WriteProg(REGDEF_BOOT_MODE, ucTemp, 1) == 0)
		return 0;

	return 1;
}

int SpiWriteProg(unsigned int dataAddr, unsigned char *data, unsigned int len)
{
	if (dataAddr == 0xCC3355)
	{
		unsigned char buf[4];
		buf[0] = 0xCC;
		buf[1] = 0x33;
		buf[2] = 0x55;
		buf[3] = 0x5A;

		return SPI_Write(buf, 4, NULL, 0);
	}

	unsigned int ioCount = 0;
	unsigned char *wBuf;
	unsigned char *rBuf;
	unsigned int pageLen = 0;
	unsigned short crcValue = 0;
	unsigned int offset = 0;
	int ioResult = 1;

	if (system_settings.m_drw_mode)
	{
		// CMD(1) + addr(3) + len(3) + head crc(2) + dummy(4) + data(len) + data crc(2) + dummy(1) + hack(1) + aack(1)
		ioCount = len + 18;
	}
	else
	{
		// CMD(1) + addr(3) + data(len)
		ioCount = len + 4;
	}

	// combo R/W data
	wBuf = (unsigned char *)malloc(ioCount);
	if (0 == wBuf)
		return 0;
	rBuf = (unsigned char *)malloc(ioCount);
	if (0 == rBuf)
	{
		free(wBuf);
		return 0;
	}
	for (unsigned int i = 0; i < ioCount; i++)
	{
		wBuf[i] = 0xFF;
		rBuf[i] = 0xFF;
	}
	offset = 0;

	// CMD(1)
	wBuf[offset++] = DRW_SPI_WRITE;

	// addr(3)
	wBuf[offset++] = (unsigned char)(dataAddr >> 16);
	wBuf[offset++] = (unsigned char)(dataAddr >> 8);
	wBuf[offset++] = (unsigned char)(dataAddr);

	if (system_settings.m_drw_mode)
	{
		// len(3)
		wBuf[offset++] = (unsigned char)(len >> 16);
		wBuf[offset++] = (unsigned char)(len >> 8);
		wBuf[offset++] = (unsigned char)(len);
        		
		// head crc(2)
		crcValue = CalcCRC16(wBuf, offset, 0);
		if ((DRW_SPI_WRITE & 0x02) == 0)
			crcValue = (unsigned short)~crcValue;
		wBuf[offset++] = (unsigned char)(crcValue >> 8);
		wBuf[offset++] = (unsigned char)(crcValue);
	
		// dummy(4)
		offset += 4;
	}

	// data(len)
	for (unsigned int i = 0; i < len; i++)
	{
		wBuf[offset + i] = data[i];
	}
	offset += len;

	if (system_settings.m_drw_mode)
	{
		// data crc(2)
		crcValue = CalcCRC16(data, len, 0);
		if ((DRW_SPI_WRITE & 0x02) == 0)
			crcValue = (unsigned short)~crcValue;
		wBuf[offset++] = (unsigned char)(crcValue >> 8);
		wBuf[offset++] = (unsigned char)(crcValue);
	}

	if (0 == SPI_Write(wBuf, offset, rBuf, ioCount - offset))
	{
		free(wBuf);
		free(rBuf);
		return 0;
	}

	// check ACK
	if (system_settings.m_drw_mode && (rBuf[ioCount - 2] == 0xE0 || rBuf[ioCount - 1] == 0xE0))
	{
		free(wBuf);
		free(rBuf);
		return 0;
	}

	free(wBuf);
	free(rBuf);
	return 1;
}

unsigned int gDataAddr_backup = 0xFFFFFF;
unsigned int ChanageDieModeAddress(unsigned int dataAddr)
{
	unsigned char data[2];

	if ((dataAddr & 0xF00000) == gDataAddr_backup)
		return (dataAddr & 0x0FFFFF);

	gDataAddr_backup = dataAddr & 0xF00000;

	if ((dataAddr >= 0x400000) && (dataAddr <= 0x4FFFFF))
	{
		data[0] = 0x82;
		data[1] = 0x82;
	}
	else if ((dataAddr >= 0x800000) && (dataAddr <= 0x8FFFFF))
	{
		data[0] = 0x84;
		data[1] = 0x84;
	}
	else if ((dataAddr >= 0x000000) && (dataAddr <= 0x0FFFFF))
	{
		data[0] = 0x81;
		data[1] = 0x81;
	}
	else
	{
		return dataAddr;
	}

	unsigned char wbuf[1];

	for (int i = 0; i < 2; i++)
	{
		if (0 == system_settings.m_TransMode)
		{
			I2C_Write(data[i], wbuf, 0);
		}
		else
		{
			wbuf[0] = data[i];
			SPI_Write(wbuf, 1, NULL, 0);
		}
	}

	return (dataAddr & 0x0FFFFF);
}

int SpiReadProg(unsigned int dataAddr, unsigned char *data, unsigned int len)
{
	unsigned int ioCount = 0;
	unsigned char *wBuf;
	unsigned char *rBuf;
	unsigned int pageLen = 0;
	unsigned short crcValue = 0;
	unsigned int offset = 0;
	unsigned int dataOffset = 0;
	int ioResult = 1;

	if (system_settings.m_drw_mode)
	{
		// CMD(1) + addr(3) + len(3) + head crc(2) + dummy(4) + data(len) + data crc(2) + dummy(1) + hack(1) + aack(1)
		ioCount = len + 18;
	}
	else
	{
		// CMD(1) + addr(3) + data(len)
		ioCount = len + 4 + system_settings.m_SpiProgReadDummyBytes;
	}

	// combo R/W data
	wBuf = (unsigned char *)malloc(ioCount);
	if (0 == wBuf)
		return 0;
	rBuf = (unsigned char *)malloc(ioCount);
	if (0 == rBuf)
	{
		free(wBuf);
		return 0;
	}
	for (unsigned int i = 0; i < ioCount; i++)
	{
		wBuf[i] = 0xFF;
		rBuf[i] = 0xFF;
	}
	offset = 0;

	// CMD(1)
	wBuf[offset++] = DRW_SPI_READ;

	// addr(3)
	wBuf[offset++] = (unsigned char)(dataAddr >> 16);
	wBuf[offset++] = (unsigned char)(dataAddr >> 8);
	wBuf[offset++] = (unsigned char)(dataAddr);

	if (system_settings.m_drw_mode)
	{
		// len(3)
		wBuf[offset++] = (unsigned char)(len >> 16);
		wBuf[offset++] = (unsigned char)(len >> 8);
		wBuf[offset++] = (unsigned char)(len);
        		
		// head crc(2)
		crcValue = CalcCRC16(wBuf, offset, 0);
		if ((DRW_SPI_READ & 0x02) == 0)
			crcValue = (unsigned short)~crcValue;
		wBuf[offset++] = (unsigned char)(crcValue >> 8);
		wBuf[offset++] = (unsigned char)(crcValue);
		
		// dummy(4)
		offset += 4;
	}
	else
	{
		offset += system_settings.m_SpiProgReadDummyBytes;
	}

	dataOffset = offset;

	if (0 == SPI_Write(wBuf, offset, rBuf, ioCount - offset))
	{
		free(wBuf);
		free(rBuf);
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	// check ACK
	if (system_settings.m_drw_mode)
	{
        if ((DRW_SPI_READ & 0x04) == 0)
        {
            // check data crc
            unsigned short crcValue = CalcCRC16(rBuf, len, offset);
            if ((DRW_SPI_READ & 0x02) == 0)
                crcValue = (unsigned short)~crcValue;

            // if (crcValue != (rBuf[ioCount - 5] * 256 + rBuf[ioCount - 4]))
            if (crcValue != (rBuf[len] * 256 + rBuf[len + 1]))

            {
                free(wBuf);
                free(rBuf);
				THP_LOGE("[%s][%d]: crc error,cal:%d vs crc %d", __FILE__, __LINE__,crcValue,(rBuf[len] * 256 + rBuf[len + 1]));
                return 0;
            }
        }

        // if ((rBuf[ioCount - 2] == 0xE0 || rBuf[ioCount - 1] == 0xE0))
        if ((rBuf[len + 3] == 0xE0 || rBuf[len + 4] == 0xE0))
        {
            free(wBuf);
            free(rBuf);
			THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }
	}

	for (unsigned int i = 0; i < len; i++)
	{
		// data[i] = rBuf[dataOffset + i];//already offset
		data[i] = rBuf[i];
		// THP_LOGI("%d ", data[i]);
	}
	free(wBuf);
	free(rBuf);
	return 1;
}

unsigned short Crc16Table[] = {
		   0x0000,0x8005,0x800F,0x000A,0x801B,0x001E,0x0014,0x8011,0x8033,0x0036,0x003C,0x8039,0x0028,0x802D,0x8027,0x0022,
		   0x8063,0x0066,0x006C,0x8069,0x0078,0x807D,0x8077,0x0072,0x0050,0x8055,0x805F,0x005A,0x804B,0x004E,0x0044,0x8041,
		   0x80C3,0x00C6,0x00CC,0x80C9,0x00D8,0x80DD,0x80D7,0x00D2,0x00F0,0x80F5,0x80FF,0x00FA,0x80EB,0x00EE,0x00E4,0x80E1,
		   0x00A0,0x80A5,0x80AF,0x00AA,0x80BB,0x00BE,0x00B4,0x80B1,0x8093,0x0096,0x009C,0x8099,0x0088,0x808D,0x8087,0x0082,
		   0x8183,0x0186,0x018C,0x8189,0x0198,0x819D,0x8197,0x0192,0x01B0,0x81B5,0x81BF,0x01BA,0x81AB,0x01AE,0x01A4,0x81A1,
		   0x01E0,0x81E5,0x81EF,0x01EA,0x81FB,0x01FE,0x01F4,0x81F1,0x81D3,0x01D6,0x01DC,0x81D9,0x01C8,0x81CD,0x81C7,0x01C2,
		   0x0140,0x8145,0x814F,0x014A,0x815B,0x015E,0x0154,0x8151,0x8173,0x0176,0x017C,0x8179,0x0168,0x816D,0x8167,0x0162,
		   0x8123,0x0126,0x012C,0x8129,0x0138,0x813D,0x8137,0x0132,0x0110,0x8115,0x811F,0x011A,0x810B,0x010E,0x0104,0x8101,
		   0x8303,0x0306,0x030C,0x8309,0x0318,0x831D,0x8317,0x0312,0x0330,0x8335,0x833F,0x033A,0x832B,0x032E,0x0324,0x8321,
		   0x0360,0x8365,0x836F,0x036A,0x837B,0x037E,0x0374,0x8371,0x8353,0x0356,0x035C,0x8359,0x0348,0x834D,0x8347,0x0342,
		   0x03C0,0x83C5,0x83CF,0x03CA,0x83DB,0x03DE,0x03D4,0x83D1,0x83F3,0x03F6,0x03FC,0x83F9,0x03E8,0x83ED,0x83E7,0x03E2,
		   0x83A3,0x03A6,0x03AC,0x83A9,0x03B8,0x83BD,0x83B7,0x03B2,0x0390,0x8395,0x839F,0x039A,0x838B,0x038E,0x0384,0x8381,
		   0x0280,0x8285,0x828F,0x028A,0x829B,0x029E,0x0294,0x8291,0x82B3,0x02B6,0x02BC,0x82B9,0x02A8,0x82AD,0x82A7,0x02A2,
		   0x82E3,0x02E6,0x02EC,0x82E9,0x02F8,0x82FD,0x82F7,0x02F2,0x02D0,0x82D5,0x82DF,0x02DA,0x82CB,0x02CE,0x02C4,0x82C1,
		   0x8243,0x0246,0x024C,0x8249,0x0258,0x825D,0x8257,0x0252,0x0270,0x8275,0x827F,0x027A,0x826B,0x026E,0x0264,0x8261,
		   0x0220,0x8225,0x822F,0x022A,0x823B,0x023E,0x0234,0x8231,0x8213,0x0216,0x021C,0x8219,0x0208,0x820D,0x8207,0x0202,
};
extern uint16_t cts_crc16(const uint8_t *buf, size_t len);
unsigned short CalcCRC16(unsigned char *buf, unsigned int len, unsigned int offset)
{
	// unsigned short u16CrcValue = 0;
	// unsigned int u16Length = len;
	// unsigned char u8CheckData = 0;
	// unsigned int i = 0;
	// while (u16Length != 0)
	// {
	// 	u8CheckData = buf[offset + i++];
	// 	u16CrcValue = (unsigned short)((u16CrcValue << 8) ^ Crc16Table[(u16CrcValue >> 8) ^ (u8CheckData)]);
	// 	u16Length--;
	// }

	// return u16CrcValue;
	return cts_crc16(buf,len);
}

void GetTcsCmd(enum TcsCmdIndex index, STRUCT_TCS_CMD *cmd)
{
	// baseFlage, classID, cmdID, isRead, isWrite, isData, retLen
	cmd->baseFlag = TcsCmdValue[index][0];
	cmd->classID = TcsCmdValue[index][1];
	cmd->cmdID = TcsCmdValue[index][2];
	cmd->isRead = TcsCmdValue[index][3];
	cmd->isWrite = TcsCmdValue[index][4];
	cmd->isData = TcsCmdValue[index][5];
	cmd->retLen = TcsCmdValue[index][6];
}

int WriteTCS(STRUCT_TCS_CMD cmd, unsigned char *data, unsigned int len, int delay)
{
	int res = 0;

	for (int retry = 0; retry < TCS_RETRY_NUM; retry++)
	{
		if (0 == system_settings.m_TransMode)
			res = I2cWriteTCS(cmd, data, len);
		else
			res = SpiWriteTCS(cmd, data, len, delay);

		if (res != 0)
			break;
	}

	return res;
}

unsigned char GetClassID(unsigned char baseFlag, unsigned char onlyRead, unsigned char onlyWrite, unsigned char classID)
{
	unsigned char bit7 = (unsigned char)(baseFlag ? 0x80 : 0x00);
	unsigned char bit6 = (unsigned char)(onlyRead ? 0x40 : 0x00);
	unsigned char bit5 = (unsigned char)(onlyWrite ? 0x20 : 0x00);
	unsigned char bit4_0 = (unsigned char)(classID & 0x1F);

	return (unsigned char)(bit7 | bit6 | bit5 | bit4_0);
}

unsigned char CalcCheckSum(unsigned char *data, unsigned int len, unsigned int offset)
{
	unsigned char ret = 0;

	for (unsigned int x = 0; x < len; x++)
	{
		ret += data[x + offset];
	}

	return (unsigned char)~ret;
}

int I2cWriteTCS(STRUCT_TCS_CMD cmd, unsigned char * data, unsigned int len)
{
	int dataCount = (int)len;
	int inBuf_len = 6 + dataCount + 2;
	unsigned char *inBuf = (unsigned char *)malloc(inBuf_len);// TCS header + data + crc
	int blockLen = 0;
	int offset = 0;
	unsigned char blockBuf[TransferSize];
	unsigned char cmdID[2];

	if (0 == inBuf)
		return 0;

	if (0 == cmd.isWrite)
	{
		free(inBuf);
		return 0;
	}

	// set header
	inBuf[0] = cmd.cmdID;// CMD_L
	inBuf[1] = GetClassID(cmd.baseFlag, 0, cmd.isWrite, cmd.classID);// CMD_H
	inBuf[2] = (unsigned char)(dataCount & 0xFF);//DataLenL
	inBuf[3] = (unsigned char)((dataCount & 0xFF00) >> 8);//DataLenH
	inBuf[4] = CalcCheckSum(inBuf, 4, 0);
	inBuf[5] = 1;

	// backup cmd id
	cmdID[0] = inBuf[0];
	cmdID[1] = inBuf[1];
	for (int i = 0; i < dataCount; i++)
	{
		inBuf[6 + i] = data[i];
	}

	// set crc
	inBuf[6 + dataCount] = CalcCheckSum(inBuf, (unsigned int)dataCount, 6);
	inBuf[6 + dataCount + 1] = 1;

	dataCount = inBuf_len;

	// send header + body
	while (dataCount > 0)
	{
		blockLen = dataCount > (TransferSize - 9) ? (TransferSize - 9) : dataCount;
		for (int i = 0; i < blockLen; i++)
		{
			blockBuf[i] = inBuf[offset + i];
		}

		for (int retry = I2C_RETRY_NUM; retry > 0; retry--)
		{
			if (I2C_Write(system_settings.m_I2cAddr, blockBuf, (unsigned int)blockLen) != 0)
			{
				break;
			}
			else if (retry == 1)
			{
				free(inBuf);
				return 0;
			}
		}

		offset += blockLen;
		dataCount -= blockLen;
	}
	// receive errorCode + cmd + crc
	for (int x = 0; x < TransferSize; x++)
		blockBuf[x] = 0xFF;

	for (int retry = I2C_RETRY_NUM; retry > 0; retry--)
	{
		if (I2C_Read(system_settings.m_I2cAddr, blockBuf, 5) != 0)
		{
			break;
		}
		else if (retry == 0)
		{
			free(inBuf);
			return 0;
		}
	}

	// check crc, cmd id, error code
	unsigned char checkSumValue = CalcCheckSum(blockBuf, 3, 0);
	if (checkSumValue != blockBuf[3] || 1 != blockBuf[4] || cmdID[0] != blockBuf[1] || cmdID[1] != blockBuf[2] || 0 != blockBuf[0])
	{
		free(inBuf);
		return 0;
	}

	free(inBuf);
	return 1;
}

int SpiWriteTCS(STRUCT_TCS_CMD cmd, unsigned char *data, unsigned int len, int delay)
{
	int dataCount = (int)len;
	int inBuf_len = dataCount + 7 + 2;
	unsigned char * inBuf = (unsigned char *)malloc(inBuf_len);// TCS header + data + crc
	unsigned int crcValue = 0;
	int offset = 0;
	unsigned char blockBuf[SPI_FIFO_LENGTH];
	unsigned char cmdID[2];

	if (0 == inBuf)
	{
		return 0;
	}

	if (0 == cmd.isWrite)
	{
		free(inBuf);
		return 0;
	}

	ChangeSpiMode(system_settings.m_NormSpiMode);

	// set header
	inBuf[offset++] = TCS_SPI_WRITE;
	inBuf[offset++] = cmd.cmdID;// CMD_L
	inBuf[offset++] = GetClassID(cmd.baseFlag, 0, cmd.isWrite, cmd.classID);// CMD_H
	inBuf[offset++] = (unsigned char)(dataCount & 0xFF);//DataLenL
	inBuf[offset++] = (unsigned char)((dataCount & 0xFF00) >> 8);//DataLenH

	// backup cmd id
	cmdID[0] = inBuf[1];
	cmdID[1] = inBuf[2];

	crcValue = CalcCRC16(inBuf, 5, 0);
	inBuf[offset++] = (unsigned char)(crcValue & 0xFF);//CheckL
	inBuf[offset++] = (unsigned char)((crcValue & 0xFF00) >> 8);//CheckH

	for (int i = 0; i < dataCount; i++)
	{
		inBuf[offset + i] = data[i];
	}
	offset += dataCount;

	// set crc
	crcValue = CalcCRC16(data, (unsigned int)dataCount, 0);
	inBuf[offset++] = (unsigned char)(crcValue & 0xFF);//CheckL
	inBuf[offset++] = (unsigned char)((crcValue & 0xFF00) >> 8);//CheckH

	dataCount = inBuf_len;

	// send header + body
	if (0 == SPI_Write(inBuf, dataCount, NULL, 0))
	{
		free(inBuf);
		return 0;
	}

	// receive errCode + cmdID + crc
	for (int x = 0; x < SPI_FIFO_LENGTH; x++)
		blockBuf[x] = 0xFF;

	if (0 == SPI_Write(blockBuf, 0, blockBuf, 5))
	{
		free(inBuf);
		return 0;
	}

	// check crc, cmd id, error code
	crcValue = CalcCRC16(blockBuf, 3, 0);
	if ((unsigned char)crcValue != blockBuf[3] || (unsigned char)(crcValue >> 8) != blockBuf[4] || cmdID[0] != blockBuf[1] || cmdID[1] != blockBuf[2] || 0 != blockBuf[0])
	{
		free(inBuf);
		return 0;
	}

	free(inBuf);
	return 1;
}

int IsSupportedMcu()
{
	unsigned char data[SPI_FIFO_LENGTH];
	STRUCT_TCS_CMD cmd = { 0 };
	GetTcsCmd(TP_STD_CMD_INFO_CHIP_FW_ID_RO, &cmd);

	if (0 == ReadDataTCS(cmd, data, 4, 0))
	{
		return 0;
	}

	if (data[0] == chipTypeId[0] && data[1] == chipTypeId[1])
		return 1;
	else
		return 0;
}

int ReadDataTCS(STRUCT_TCS_CMD cmd, unsigned char *buffer, unsigned int len, int delay)
{
	int res = 0;
	for (int retry = 0; retry < TCS_RETRY_NUM; retry++)
	{
		if (0 == system_settings.m_TransMode)
			res = I2cReadDataTCS(cmd, buffer, len, 0);
		else
			res = SpiReadDataTCS(cmd, buffer, len, delay);

		if (res != 0)
			break;
	}

	return res;
}

int I2cReadDataTCS(STRUCT_TCS_CMD cmd, unsigned char * buffer, unsigned int len, int delay)
{
	unsigned char inBuf[TransferSize];
	int blockLen = 0;

	int offset = 0;
	unsigned char * recBuf = (unsigned char *)malloc(len + 5);// data + errCode + cmd_L + cmd_H + crc_L + crc_H
	int recLen = len + 5;
	unsigned char cmdID[2];

	if (0 == recBuf)
	{
		return 0;
	}

	if (0 == cmd.isRead)
	{
		free(recBuf);
		return 0;
	}

	// set header
	inBuf[0] = cmd.cmdID;// CMD_L
	inBuf[1] = GetClassID(cmd.baseFlag, cmd.isRead, 0, cmd.classID);// CMD_H
	inBuf[2] = (unsigned char)(len & 0xFF);//DataLenL
	inBuf[3] = (unsigned char)((len & 0xFF00) >> 8);//DataLenH
	inBuf[4] = CalcCheckSum(inBuf, 4, 0);
	inBuf[5] = 1;
	// backup cmd ID
	cmdID[0] = inBuf[0];// CMD_L
	cmdID[1] = inBuf[1];// CMD_H

	// send header
	for (int retry = I2C_RETRY_NUM; retry > 0; retry--)
	{
		if (I2C_Write(system_settings.m_I2cAddr, inBuf, 6) != 0)
		{
			break;
		}
		else if (retry == 1)
		{
			free(recBuf);
			return 0;
		}
	}

	if (delay > 0)
	{
		cts_mdelay(delay);
	}

	// receive body + checkSum
	while (recLen > 0)
	{
		blockLen = recLen > (TransferSize - 9) ? (TransferSize - 9) : recLen;

		for (int retry = I2C_RETRY_NUM; retry > 0; retry--)
		{
			if (I2C_Read(system_settings.m_I2cAddr, inBuf, (unsigned int)blockLen) != 0)
			{
				break;
			}
			else if (retry == 1)
			{
				free(recBuf);
				return 0;
			}
		}

		for (int i = 0; i < blockLen; i++)
		{
			recBuf[offset + i] = inBuf[i];
		}

		recLen -= blockLen;
		offset += blockLen;
	}

	recLen = len + 5;
	unsigned char checkSumValue = CalcCheckSum(recBuf, (unsigned int)(recLen - 2), 0);
	if (1 != recBuf[recLen - 1] || checkSumValue != recBuf[recLen - 2] || cmdID[1] != recBuf[recLen - 3] || cmdID[0] != recBuf[recLen - 4] || 0 != recBuf[recLen - 5])
	{
		free(recBuf);
		return 0;
	}

	for (unsigned int i = 0; i < len; i++)
	{
		buffer[i] = recBuf[i];
	}

	free(recBuf);
	return 1;
}

int SpiReadDataTCS(STRUCT_TCS_CMD cmd, unsigned char * buffer, unsigned int len, int delay)
{
	unsigned char inBuf[TransferSize];
	unsigned int checkCrcValue = 0;
	int offset = 0;
	unsigned char * recBuf = (unsigned char *)malloc(len + 5 + SpiReadDummyBytes);
	int recLen = len + 5 + SpiReadDummyBytes;
	unsigned char cmdID[2];

	if (0 == recBuf)
	{
		return 0;
	}

	if (0 == cmd.isRead)
	{
		free(recBuf);
		return 0;
	}

	ChangeSpiMode(system_settings.m_NormSpiMode);

	// set header
	inBuf[offset++] = TCS_SPI_READ;
	inBuf[offset++] = cmd.cmdID;// CMD_L
	inBuf[offset++] = GetClassID(cmd.baseFlag, cmd.isRead, 0, cmd.classID);// CMD_H
	inBuf[offset++] = (unsigned char)(len & 0xFF);//DataLenL
	inBuf[offset++] = (unsigned char)((len & 0xFF00) >> 8);//DataLenH

	unsigned int crcValue = CalcCRC16(inBuf, (unsigned int)offset, 0);
	inBuf[offset++] = (unsigned char)(crcValue & 0xFF);//CheckL
	inBuf[offset++] = (unsigned char)((crcValue & 0xFF00) >> 8);//CheckH

	// backup cmd ID
	cmdID[0] = inBuf[1];// CMD_L
	cmdID[1] = inBuf[2];// CMD_H

	if (0 == SPI_Write(inBuf, 7, NULL, 0))
	{
		free(recBuf);
		return 0;
	}

	cts_mdelay(2);

	for (int i = 0; i < recLen; i++)
	{
		recBuf[i] = 0xFF;
	}

	if (0 == SPI_Write(recBuf, 0, recBuf, recLen))
	{
		free(recBuf);
		return 0;
	}

	// check crc, cmd id, error code
	checkCrcValue = CalcCRC16(recBuf, (unsigned int)(len + 3), 0);
	if ((unsigned char)checkCrcValue != recBuf[recLen - 2] || (unsigned char)(checkCrcValue >> 8) != recBuf[recLen - 1] || cmdID[0] != recBuf[recLen - 4] || cmdID[1] != recBuf[recLen - 3] || 0 != recBuf[recLen - 5])
	{
		free(recBuf);
		THP_LOGE("crc cal: [%2x][%2x] vs recv: [%2x][%2x]",checkCrcValue & 0xFF,(unsigned char)(checkCrcValue >> 8),recBuf[recLen - 2],recBuf[recLen - 1]);
		THP_LOGE("cmd cal: [%2x][%2x] vs recv: [%2x][%2x]",cmdID[0],cmdID[1],recBuf[recLen - 4],recBuf[recLen - 3]);
		THP_LOGE("err code: [%2x]",recBuf[recLen - 5]);
		return 0;
	}

	for (unsigned int i = 0; i < len; i++)
	{
		buffer[i] = recBuf[i];
	}

	free(recBuf);
	return 1;
}

int ReadXdataTCS(unsigned int offset, unsigned char * buffer, unsigned int len, int delay)
{
	unsigned int blockLen = 128;
	unsigned char blockBuf[128];
	unsigned int receiveLen = 0;
	while (receiveLen < len)
	{
		unsigned int curLen = blockLen < (len - receiveLen) ? blockLen : (len - receiveLen);
		if (0 == ReadSubXdataTCS(offset + receiveLen, blockBuf, curLen, delay))
		{
			return 0;
		}
		for (unsigned int i = 0; i < curLen; i++)
		{
			buffer[receiveLen + i] = blockBuf[i];
		}
		receiveLen += curLen;
	}

	return 1;
}

int ReadSubXdataTCS(unsigned int offset, unsigned char * buffer, unsigned int len, int delay)
{
	unsigned char para[5];
	// set offset and len
	para[0] = (unsigned char)offset;
	para[1] = (unsigned char)(offset >> 8);
	para[2] = (unsigned char)(offset >> 16);
	para[3] = (unsigned char)len;
	para[4] = (unsigned char)(len >> 8);

	STRUCT_TCS_CMD cmd = { 0 };
	GetTcsCmd(TP_STD_CMD_SYS_STS_OFFSET_AND_LEN_CFG_RW, &cmd);

	if (0 == WriteTCS(cmd, para, 5, 0))
	{
		return 0;
	}

	cts_mdelay(100);

	// Get analog data ready flag, wait 10 * 100 millisecond
	unsigned char tempBuf[1];
	int tryTimes = 10;
	while (tryTimes-- > 0)
	{
		cts_mdelay(1);
		GetTcsCmd(TP_STD_CMD_SYS_STS_REG_DAT_RDY_FLAG_RW, &cmd);

		if (0 == ReadDataTCS(cmd, tempBuf, 1, 0))
		{
			return 0;
		}
		if (tempBuf[0] != 0)
		{
			break;
		}
	}

	if (-1 == tryTimes)
	{
		// timeout
		return 0;
	}

	// Read analog die data
	GetTcsCmd(TP_STD_CMD_SYS_STS_REG_READ_START_RO, &cmd);

	if (0 == ReadDataTCS(cmd, buffer, len, delay))
	{
		return 0;
	}

	// Set analog data ready flag
	GetTcsCmd(TP_STD_CMD_SYS_STS_REG_DAT_RDY_FLAG_RW, &cmd);
	para[0] = 0;
	if (0 == WriteTCS(cmd, para, 1, 0))
	{
		return 0;
	}

	return 1;
}

int WriteXdataTCS(unsigned int offset, unsigned char * buffer, unsigned int len, int delay)
{
	unsigned char * para = (unsigned char *)malloc(len + 6);
	if (0 == para)
		return 0;

	// set len and offset
	para[0] = (unsigned char)len;
	para[1] = (unsigned char)(len >> 8);
	para[2] = (unsigned char)offset;
	para[3] = (unsigned char)(offset >> 8);
	para[4] = (unsigned char)(offset >> 16);
	para[5] = 0;
	for (unsigned int i = 0; i < len; i++)
	{
		para[6 + i] = buffer[i];
	}

	STRUCT_TCS_CMD cmd = { 0 };
	GetTcsCmd(TP_STD_CMD_SYS_STS_WR_REG_RAM_SEQUENCE_WO, &cmd);

	int result = WriteTCS(cmd, para, len + 6, delay);

	free(para);

	return result;
}

int ReadProg(unsigned int dataAddr, unsigned char* data, unsigned int len)
{
    if (0 == system_settings.m_TransMode)
    {
        return I2cReadProgMode(dataAddr, data, len);
    }
    else
    {
        ChangeSpiMode(system_settings.m_ProgSpiMode);
        dataAddr = ChanageDieModeAddress(dataAddr);
        return SpiReadProg(dataAddr, data, len);
    }
}

int WriteProg(unsigned int dataAddr, unsigned char* data, unsigned int len)
{
    if (0 == system_settings.m_TransMode)
    {
        return I2cWriteProgMode(dataAddr, data, len);
    }
    else
    {
        ChangeSpiMode(system_settings.m_ProgSpiMode);
        dataAddr = ChanageDieModeAddress(dataAddr);
        return SpiWriteProg(dataAddr, data, len);
    }
}

int WriteFlashBuffer(unsigned int u32FlashAddr, unsigned char* buffer, unsigned int length)
{
    unsigned int u32SectorAddress = GetSectorStartAddress(u32FlashAddr, SECTOR_SIZE_4KB);

    if (u32FlashAddr + length > u32SectorAddress + SECTOR_SIZE_4KB)
    {
        // Not support
        return 0;
    }

    unsigned char* sectorBuffer = (unsigned char*)malloc(SECTOR_SIZE_4KB);
    if (0 == sectorBuffer)
        return 0;

    if (!IsProgMode())
    {
        if (0 == GotoProgMode(1, 1))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            free(sectorBuffer);
            return 0;
        }
    }

    // length less than page size need read data back
    if (length < SECTOR_SIZE_4KB)
    {
        if (0 == ReadFlashBuffer(u32SectorAddress, sectorBuffer, SECTOR_SIZE_4KB))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            free(sectorBuffer);
            return 0;
        }
    }

    if (0 == EraseSecotorFlash(u32SectorAddress, SECTOR_SIZE_4KB))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        free(sectorBuffer);
        return 0;
    }

    // Copy data to sector buffer
    memcpy((sectorBuffer + u32FlashAddr - u32SectorAddress), buffer, length);

    unsigned int buf_crc_value = CalcCRC32(sectorBuffer, SECTOR_SIZE_4KB, 0);

    if (0 == WriteProg(0, sectorBuffer, SECTOR_SIZE_4KB))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        free(sectorBuffer);
        return 0;
    }

    unsigned int ram_crc_value = ChipCalculateCrc(0, SECTOR_SIZE_4KB, 0);
    if (buf_crc_value != ram_crc_value)
    {
        THP_LOGE("[%s][%d]: crc error\n", __FILE__, __LINE__);
        free(sectorBuffer);
        return 0;
    }

    if (0 == CopyMemToFlashEx2(u32SectorAddress, 0, SECTOR_SIZE_4KB))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        free(sectorBuffer);
        return 0;
    }

    unsigned int check_crc_value = ChipCalculateCrc(u32SectorAddress, SECTOR_SIZE_4KB, 1);
    if (buf_crc_value != check_crc_value)
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        free(sectorBuffer);
        return 0;
    }

    free(sectorBuffer);
    return 1;
}

int ReadFlashBuffer(unsigned int u32FlashAddr, unsigned char* buf, unsigned int length)
{
    if (0 == GotoProgMode(1, 1))
        return 0;

    if (0 == CopyFlashToMemEx2(u32FlashAddr, 0, length))
        return 0;

    if (0 == ReadProg(0, buf, (unsigned int)length))
        return 0;

    return 1;
}

unsigned int GetInformationAddress()
{
	return FLASH_CONFIGURE_ADDRESS;
}

int GetInformationLength()
{
	return FLASH_CONFIGURE_LENGTH;
}

int VerifyIsBigBinFile(int len, unsigned char * buf)
{
	unsigned int infoStartAddress = GetInformationAddress();
	int infoLength = GetInformationLength();

	unsigned short flagValue;
	unsigned short version;

	if ((unsigned int)len < infoStartAddress + infoLength)
	{
		// not enouth data
		return 0;
	}

	flagValue = (unsigned short)(buf[infoStartAddress] | (buf[infoStartAddress + 1] << 8));
	version = (unsigned short)(buf[infoStartAddress + 2] | (buf[infoStartAddress + 2 + 1] << 8));

	return version == 1 && flagValue == 0xAA55;
}

unsigned int GetSectorStartAddress(unsigned int u32Address, unsigned int SectorSize)
{
	return u32Address / SectorSize * SectorSize;
}

int CopyMemToFlashEx2(unsigned int flash_addr, unsigned int sram_addr, unsigned int len)
{
	unsigned char ucTemp[4];
    unsigned int u32FlashTempAddr = flash_addr;
    unsigned int u32SramTempAddr = sram_addr;
    unsigned int u32NotAlignLength;

    for (unsigned int i = 0; i < len;)
    {
        ucTemp[0] = (unsigned char)(u32FlashTempAddr);
        ucTemp[1] = (unsigned char)(u32FlashTempAddr >> 8);
        ucTemp[2] = (unsigned char)(u32FlashTempAddr >> 16);
        ucTemp[3] = (unsigned char)(u32FlashTempAddr >> 24);
        if (0 == WriteProg(SFCTL_FLASH_ADDR, ucTemp, 4))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        ucTemp[0] = (unsigned char)(u32SramTempAddr);
        ucTemp[1] = (unsigned char)(u32SramTempAddr >> 8);
        ucTemp[2] = (unsigned char)(u32SramTempAddr >> 16);
        ucTemp[3] = (unsigned char)(u32SramTempAddr >> 24);
        if (0 == WriteProg(SFCTL_SRAM_ADDR, ucTemp, 4))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        //  check u32FlashTempAddr page aglin
        if ((u32FlashTempAddr % FLASH_PAGE_PRGRAM_SIZE) > 0) // not aglin
        {
            u32NotAlignLength = (FLASH_PAGE_PRGRAM_SIZE - (u32FlashTempAddr % FLASH_PAGE_PRGRAM_SIZE));
            
            if (len <= u32NotAlignLength)
            {
                u32NotAlignLength = len;
            }        
        }
        else
        {
            if (i + FLASH_PAGE_PRGRAM_SIZE <= len)
            {
                u32NotAlignLength = FLASH_PAGE_PRGRAM_SIZE;
            }
            else
            {
                u32NotAlignLength = (len - i);
            }
        }
        u32FlashTempAddr += u32NotAlignLength; // change the flash and sram address
        u32SramTempAddr += u32NotAlignLength;
        i += u32NotAlignLength;

        ucTemp[0] = (unsigned char)(u32NotAlignLength);
        ucTemp[1] = (unsigned char)(u32NotAlignLength >> 8);
        ucTemp[2] = (unsigned char)(u32NotAlignLength >> 16);
        ucTemp[3] = (unsigned char)(u32NotAlignLength >> 24);
        if (0 == WriteProg(SFCTL_DATA_LENGTH, ucTemp, 4))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        ucTemp[0] = (unsigned char)FLASH_CMD_PAGE_PROGRAM;  //read
        if (0 == WriteProg(SFCTL_CMD_SEL, ucTemp, 1))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        ucTemp[0] = 1;
        if (0 == WriteProg(SFCTL_START_DEXC, ucTemp, 1))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        //// busy wait
        //if (0 == FlashBusyWait())
        //{
        //	THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        //	return 0;
        //}

        TIME_T start_time = GET_CURR_TIME();
        unsigned char state = 0xFF;
        while (1)
        {
            if (0 == FlashState(&state))
            {
                THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
                return 0;
            }

            if (state == 0)
                break;

            if (ELAPSED_MS(start_time) > 2000)
            {
                THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
                return 0;
            }
        }
    }

	return 1;
}

int CopyFlashToMemEx2(unsigned int flash_addr, unsigned int sram_addr, unsigned int len)
{
	unsigned char ucTemp[4];

	ucTemp[0] = (unsigned char)FLASH_CMD_FAST_READ;  //read
	if (0 == WriteProg(SFCTL_CMD_SEL, ucTemp, 1))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	ucTemp[0] = (unsigned char)(flash_addr);
	ucTemp[1] = (unsigned char)(flash_addr >> 8);
	ucTemp[2] = (unsigned char)(flash_addr >> 16);
	ucTemp[3] = (unsigned char)(flash_addr >> 24);
	if (0 == WriteProg(SFCTL_FLASH_ADDR, ucTemp, 4))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	ucTemp[0] = (unsigned char)(sram_addr);
	ucTemp[1] = (unsigned char)(sram_addr >> 8);
	ucTemp[2] = (unsigned char)(sram_addr >> 16);
	ucTemp[3] = (unsigned char)(sram_addr >> 24);
	if (0 == WriteProg(SFCTL_SRAM_ADDR, ucTemp, 4))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	ucTemp[0] = (unsigned char)(len);
	ucTemp[1] = (unsigned char)(len >> 8);
	ucTemp[2] = (unsigned char)(len >> 16);
	ucTemp[3] = (unsigned char)(len >> 24);
	if (0 == WriteProg(SFCTL_DATA_LENGTH, ucTemp, 4))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	ucTemp[0] = 1;
	if (0 == WriteProg(SFCTL_START_DEXC, ucTemp, 1))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	//// busy wait
	//if (0 == FlashBusyWait())
	//{
	//	THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
	//	return 0;
	//}

    TIME_T start_time = GET_CURR_TIME();
    unsigned char state = 0xFF;
    while (1)
    {
        if (0 == FlashState(&state))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        if (state == 0)
            break;

        if (ELAPSED_MS(start_time) > 2000)
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }
    }

	return 1;
}

int EraseFlashCodeRegion()
{
    if (0 == EraseFlash_S(FLASH_ERASE_MODE_BLOCK_ERASE, FLASH_PARTITION_FWCODE_BEGIN, FLASH_PARTITION_FWCODE_SIZE))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }

    return 1;
}

int EraseSecotorFlash(unsigned int u32FlashAddr, unsigned int len)
{
    if (0 == EraseFlash_S(FLASH_ERASE_MODE_SECTOR_ERASE, u32FlashAddr, len))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }

    return 1;
}

int EraseFlash_S(enum FLASH_ERASE_MODE mode, unsigned int u32FlashAddr, unsigned int len)
{
    unsigned char ucTemp[4] = {0, 0, 0, 0};
    unsigned char u8EraseSel = (unsigned char)FLASH_CMD_ERASE_SECTOR;
    unsigned int u32BlockSize = SECTOR_SIZE_4KB;

    if (FLASH_ERASE_MODE_SECTOR_ERASE == mode)
    {
        u8EraseSel = (uint8_t)FLASH_CMD_ERASE_SECTOR;
        u32BlockSize = SECTOR_SIZE_4KB;
    }
    else if (FLASH_ERASE_MODE_BLOCK_ERASE == mode)
    {
        u8EraseSel = (uint8_t)FLASH_CMD_ERASE_BLOCK;
        u32BlockSize = SECTOR_SIZE_32KB;
    }

    // Calculator aligned address and page count
    unsigned int u32AlignAddr = u32FlashAddr / u32BlockSize * u32BlockSize;
    int pageCount = (int)((len + u32FlashAddr - u32AlignAddr + u32BlockSize - 1) / u32BlockSize);

    if (FLASH_ERASE_MODE_CHIP_ERASE == mode)
    {
        u8EraseSel = (uint8_t)FLASH_CMD_ERASE_CHIP;
        u32AlignAddr = 0;
        pageCount = 1;
    }

    for (int x = pageCount - 1; x >= 0; x--)
    {
        // sel
        ucTemp[0] = u8EraseSel;
        if (0 == WriteProg(SFCTL_CMD_SEL, ucTemp, 1))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        // flash addr
        unsigned int u32ActAddr = u32AlignAddr + x * u32BlockSize;
        ucTemp[0] = (unsigned char)(u32ActAddr);
        ucTemp[1] = (unsigned char)(u32ActAddr >> 8);
        ucTemp[2] = (unsigned char)(u32ActAddr >> 16);
        ucTemp[3] = (unsigned char)(u32ActAddr >> 24);
        if (0 == WriteProg(SFCTL_FLASH_ADDR, ucTemp, 4))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        // start
        ucTemp[0] = 1;
        if (0 == WriteProg(SFCTL_START_DEXC, ucTemp, 1))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        // busy wait
        // clock_t start_time = clock();
		TIME_T start_time = GET_CURR_TIME();
        unsigned char state = 0xFF;
        while (1)
        {
            if (0 == FlashState(&state))
            {
                THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
                return 0;
            }

            if (state == 0)
                break;

            if (ELAPSED_MS(start_time) > 2000)
            {
                THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
                return 0;
            }
        }
    }

    return 1;
}

int GetFlashId(unsigned int* id)
{
    unsigned char ucTemp[4];
    
    ucTemp[0] = (uint8_t)FLASH_CMD_READ_IDENTIFICATION;
    if (0 == WriteProg(SFCTL_CMD_SEL, ucTemp, 1))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }

    ucTemp[0] = 1;
    if (0 == WriteProg(SFCTL_START_DEXC, ucTemp, 1))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }

    if (0 == FlashBusyWait())
    {
        return 0;
    }

    if (0 == ReadProg(SFCTL_MCHECK_CFG, ucTemp, 4))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }

    *id = (unsigned int)ucTemp[2] | (unsigned int)(ucTemp[1] << 8) | (unsigned int)(ucTemp[0] << 16);
    if ((*id & 0xffffff) == 0xffffff)
    {
        return 0;
    }

    return 1;
}

int FlashState(unsigned char *state)
{
	unsigned char ucTemp[4];

	ucTemp[0] = 0;
	ucTemp[1] = 0;

	if (0 == FlashBusyWait())
		return 0;

    ucTemp[0] = (uint8_t)FLASH_CMD_READ_STATUS;
    if (0 == WriteProg(SFCTL_CMD_SEL, ucTemp, 1))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }

    ucTemp[0] = 1;
    if (0 == WriteProg(SFCTL_START_DEXC, ucTemp, 1))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }

    if (0 == FlashBusyWait())
        return 0;

    if (0 == ReadProg(SFCTL_MCHECK_CFG, ucTemp, 1))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }

    *state = (unsigned int)(ucTemp[0] & 0x0F);

	return 1;
}

int FlashBusyWait()
{
    unsigned char ucTemp[4];
    TIME_T start_time = GET_CURR_TIME();

    while (1)
    {
        if (0 == ReadProg(SFCTL_SF_BUSY, ucTemp, 1))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        if ((ucTemp[0] & 0x1) == 0)
            break;

        if (ELAPSED_MS(start_time) > READ_FLASH_BUSY_TIMEOUT)
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }
    }

    return 1;
}

int DownloadFlashbinary(int len, unsigned char* buf, enum DOWNLOAD_FIRMWARE_TYPE mode, unsigned char isBoot, int delay)
{

    if (len <= FLASH_PARTITION_TOTAL_SIZE)
    {
        if (VerifyIsBigBinFile(len, buf) != 0)// done
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        unsigned int magicNumber = (unsigned int)(buf[0xC0] | buf[0xC1] << 8 | buf[0xC2] << 16 | buf[0xC3] << 24);
        if (1 == system_settings.m_TransMode)
        {
            if (magicNumber != 0x53504953)
            {
                THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
                return 0;
            }
        }
        else
        {
            if (magicNumber != 0x58583538)
            {
                THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
                return 0;
            }
        }

        return DownloadFirmware(len, buf, mode, isBoot, delay);
    }
    else
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }
}

unsigned int chipFwCrcValue = 0;
unsigned int chipFwLength = 0;
unsigned int chipFwCrcCheckValue = 0;
int DownloadFirmware(int firmwareLength, unsigned char *firmwareBuffer, enum DOWNLOAD_FIRMWARE_TYPE mode, unsigned char isBoot, int delay)
{
    // The actual size of the firmwareBuffer should be greater than 192k
	unsigned int codeStartAddress = FLASH_FLASH2RAM_CODE_OFFSET_ADDRESS;

    int fwLimitSize = FLASH_PARTITION_FIRMWARE_SRAM_SIZE;
    if (mode == DOWNLOAD_FIRMWARE_TYPE_FLASH)
    {
        // Flash support: 9268S --> 188k, 9385/9388 --> 412k 
        fwLimitSize = FLASH_PARTITION_FIRMWARE_FLASH_SIZE;
    }

    int align = 0x8;
    if (firmwareLength > FLASH_PARTITION_FIRMWARE_SRAM_SIZE
		&& firmwareLength <= FLASH_OVERLAP_MAXLENGTH)
    {
        // firmware size must be 8k bytes alignment when size is more than 96k
        align = SECTOR_SIZE_8KB;
    }

    // Firmware size and alignment
    if (firmwareLength % align != 0)
    {
        firmwareLength = ((firmwareLength - 1) / align + 1) * align;
    }
    if (firmwareLength == 0)
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }
    if (firmwareLength > fwLimitSize)
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }

    // program mode
	if (0 == GotoProgMode(1, 1))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

    int CalcCRCLength = firmwareLength;

    unsigned int code_crc_value = 0;
    unsigned int code_extra_length = 0;
    unsigned int code_extra_crc = 0;

	unsigned int flash_id = 0xFFFF;
	if (DOWNLOAD_FIRMWARE_TYPE_FLASH == mode)
	{
		if (0 == GetFlashId(&flash_id))
		{
			THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
			return 0;
		}	

        // erase flash
        if (0 == EraseFlashCodeRegion())
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        // Flash2Reg length must be: 9268S --> 96k, 9385/9388 --> 160k
        CalcCRCLength = FLASH_PARTITION_FIRMWARE_SRAM_SIZE;
        if (firmwareLength < CalcCRCLength)
        {
            firmwareLength = CalcCRCLength;
        }
        else if (firmwareLength > CalcCRCLength)
        {
            // Divide to 8k blocks when size between 96k ~ 160k
            int overlaySize = (FLASH_OVERLAP_MAXLENGTH < firmwareLength) ? FLASH_OVERLAP_MAXLENGTH : firmwareLength;
            int exceedSize = overlaySize - CalcCRCLength;
            int offsetPos = CalcCRCLength;

            while (exceedSize > 0)
            {
                unsigned int block_crc_value = CalcCRC32(firmwareBuffer, (unsigned int)(align - 4), (unsigned int)offsetPos);

                firmwareBuffer[offsetPos + align - 4] = (uint8_t)(block_crc_value >> 24);
                firmwareBuffer[offsetPos + align - 3] = (uint8_t)(block_crc_value >> 16);
                firmwareBuffer[offsetPos + align - 2] = (uint8_t)(block_crc_value >> 8);
                firmwareBuffer[offsetPos + align - 1] = (uint8_t)(block_crc_value);

                offsetPos += align;
                exceedSize -= align;
            }
        }

        code_crc_value = CalcCRC32(firmwareBuffer, (unsigned int)CalcCRCLength, 0);
        chipFwCrcValue = code_crc_value;
        chipFwLength = (unsigned int)firmwareLength;
        
        if (firmwareLength > CalcCRCLength)
        {
            code_extra_length = (unsigned int)(firmwareLength - CalcCRCLength);
            code_extra_crc = CalcCRC32(firmwareBuffer, code_extra_length, (unsigned int)CalcCRCLength);
        }

        int downloadSize = firmwareLength;
        int offsetFlash = 0;
        unsigned char* downloadBuffer = (unsigned char*)malloc(FLASH_PARTITION_MAX_BLOCK_SIZE);
        if (downloadBuffer == 0)
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

        while (downloadSize > 0)
        {
            int currentDownloadSize = downloadSize < FLASH_PARTITION_MAX_BLOCK_SIZE ? downloadSize : FLASH_PARTITION_MAX_BLOCK_SIZE;
            memset(downloadBuffer, 0, FLASH_PARTITION_MAX_BLOCK_SIZE);
            memcpy(downloadBuffer, (firmwareBuffer + offsetFlash), currentDownloadSize);

            if (0 == DownloadToRam(downloadBuffer, currentDownloadSize))
            {
                THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
                free(downloadBuffer);
                return 0;
            }

            // 5) copy sram to flash                        
            if (0 == CopyMemToFlashEx2((unsigned int)(codeStartAddress + offsetFlash), 0, (unsigned int)currentDownloadSize))
            {
                THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
                free(downloadBuffer);
                return 0;
            }

            offsetFlash += currentDownloadSize;
            downloadSize -= currentDownloadSize;
        }

        free(downloadBuffer);

		// Calc CRC of data in flash
        chipFwCrcCheckValue = ChipCalculateCrc(codeStartAddress, (unsigned int)CalcCRCLength, 1);

		if (code_crc_value != chipFwCrcCheckValue)
		{
            EraseFlashCodeRegion();
			THP_LOGE(" flash crc cal:%d,recv:%d\n", code_crc_value, chipFwCrcCheckValue);
			return 0;
		}

        //Write flag
        if (0 == WriteFlash2Ram((unsigned int)FLASH_PARTITION_FIRMWARE_SRAM_SIZE, chipFwCrcCheckValue, code_extra_length, code_extra_crc))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }
	}
	else if (DOWNLOAD_FIRMWARE_TYPE_SRAM == mode)
	{
        code_crc_value = CalcCRC32(firmwareBuffer, firmwareLength, 0);
        chipFwCrcValue = code_crc_value;
        chipFwLength = firmwareLength;
        if (0 == DownloadToRam(firmwareBuffer, firmwareLength))
        {
            THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
            return 0;
        }

		// Calc CRC of data in sram
        chipFwCrcCheckValue = ChipCalculateCrc(0, (unsigned int)firmwareLength, 0);
		if (code_crc_value != chipFwCrcCheckValue)
		{
			THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
			return 0;
		}
	}

	if (isBoot != 0)
	{
		if (DOWNLOAD_FIRMWARE_TYPE_FLASH == mode)
		{
			if (0 == BootFromFlash(1, delay))
			{
				THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
				return 0;
			}
		}
		else
		{
			//10. goto normal mode
			if (0 == GotoNormalMode())
			{
				THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
				return 0;
			}
			cts_mdelay(delay);
		}
	}
	else
	{
		// cts_mdelay(200);
		cts_mdelay(2);
	}
	All_LOG("update firmware success\n");
	return 1;
}

unsigned int CalcCRC32(unsigned char* buf, unsigned int len, unsigned int offset)
{
	unsigned int pos;
	unsigned int CRC_RESULT_F;
	unsigned char in_data_8b;

	CRC_RESULT_F = 0;
	for (pos = 0; pos < len; pos++)
	{
		in_data_8b = buf[pos + offset];

		unsigned int crc_reg_32b;
		unsigned char i;
		unsigned char xor_flag;

		crc_reg_32b = CRC_RESULT_F;

		for (i = 0; i < 32; i++)
		{
			if ((crc_reg_32b & 0x00000001) != 0)
			{
				crc_reg_32b ^= 0x04C11DB7;
				crc_reg_32b >>= 1;
				crc_reg_32b |= 0x80000000;
			}
			else
			{
				crc_reg_32b >>= 1;
			}
		}

		for (i = 0; i < 40; i++)
		{
			xor_flag = (unsigned char)(crc_reg_32b >> 31);
			crc_reg_32b = (unsigned int)((crc_reg_32b << 1) + (in_data_8b >> 7));
			in_data_8b = (unsigned char)(in_data_8b << 1);
			if (xor_flag != 0)
			{
				crc_reg_32b ^= 0x04C11DB7;
			}
		}

		CRC_RESULT_F = crc_reg_32b;
	}

	return CRC_RESULT_F;
}

int DownloadToRam(unsigned char* buf, int len)
{
	int ret = 0, retries = 0;
	
	//download firmware code
	do {
		THP_LOGI("send_fw_step: %d", send_fw_step[retries]);
		if (0 == WriteRamProgMode(0, buf, len, send_fw_step[retries])) {
			THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
			ret = 0;
		} else {
			ret = 1;
		}
	} while ((ret == 0) && (++retries < ARRAY_SIZE(send_fw_step)));

	return ret;
}

int WriteRamProgMode(unsigned int addr, unsigned char* data, int len, int step)
{
	unsigned int blockSize = (unsigned int)step;
	unsigned int blockLen;

	unsigned char* ioBuf = (unsigned char *)malloc(blockSize);
	unsigned int writeLen = (unsigned int)len;
	unsigned int writeOffset = 0;
	//unsigned int crcValue = 0;

	if (0 == ioBuf)
		return 0;

	while (writeLen > 0)
	{
		blockLen = (writeLen > blockSize) ? blockSize : writeLen;

		for (unsigned int i = 0; i < blockLen; i++)
		{
			ioBuf[i] = data[writeOffset + i];
		}

		//crcValue = CalcCRC32(ioBuf, blockLen, 0);

		for (int retry = 0; retry < 5; retry++)
		{
			if (WriteProg(addr + writeOffset, ioBuf, blockLen) != 0)
			{
				break;
			}

			if (retry == 4)
			{
				THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
				free(ioBuf);
				return 0;
			}
		}

		writeLen -= blockLen;
		writeOffset += blockLen;
	}

	free(ioBuf);
	return 1;
}

unsigned int ChipCalculateCrc(unsigned int address, unsigned int len, unsigned char bFlash)
{
	unsigned char ucTemp[4];

	ucTemp[0] = (unsigned char)(address);
	ucTemp[1] = (unsigned char)(address >> 8);
	ucTemp[2] = (unsigned char)(address >> 16);
	ucTemp[3] = (unsigned char)(address >> 24);
	if (bFlash != 0)
	{
		if (0 == WriteProg(SFCTL_FLASH_ADDR, ucTemp, 4))
		{
			THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
			return 0;
		}
	}
	else
	{
		if (0 == WriteProg(SFCTL_SRAM_ADDR, ucTemp, 4))
		{
			THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
			return 0;
		}
	}

	ucTemp[0] = (unsigned char)(len);
	ucTemp[1] = (unsigned char)(len >> 8);
	ucTemp[2] = (unsigned char)(len >> 16);
	ucTemp[3] = (unsigned char)(len >> 24);
	if (0 == WriteProg(SFCTL_DATA_LENGTH, ucTemp, 4))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	unsigned int iStart = (unsigned int)(bFlash ? 2 : 0);
	ucTemp[0] = 1;
	if (0 == WriteProg(SFCTL_SW_CRC_START + iStart, ucTemp, 1))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	if (0 == FlashBusyWait())
		return 0;

	if (0 == ReadProg(SFCTL_CRC_RESULT, ucTemp, 4))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	return (unsigned int)(ucTemp[0] | ucTemp[1] << 8 | ucTemp[2] << 16 | ucTemp[3] << 24);
}

int WriteFlash2Ram(unsigned int codeLength, unsigned int codeCrcValue, unsigned int extraCodeLength, unsigned int extraCodeCrc)
{
    unsigned int FLASH2RAM_START_ADDRESS_LEN = 20 + 12;
	unsigned char DO_CRC = 1;   // do crc keep 0xFFFFFFFF, or set 0x0000C35A

    unsigned char buf_temp[4];
	unsigned char *buf_info = (unsigned char *)malloc(FLASH2RAM_START_ADDRESS_LEN);
    if (0 == buf_info)
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }
	
	// set parameter
    unsigned int FLASH2RAM_EXTRA_DEN = (extraCodeLength != 0) ? 0x0000C35A : 0xFFFFFFFF;
    unsigned int FLASH2RAM_EXTRA_LENGTH = (extraCodeLength != 0) ? extraCodeLength : 0xFFFFFFFF;
    unsigned int FLASH2RAM_EXTRA_CRC = (extraCodeLength != 0) ? extraCodeCrc : 0xFFFFFFFF;
    unsigned int FLASH2RAM_QSPI_FLASH_MODE = (unsigned int)((system_settings.m_FlashSpiType == 1) ? 0x71756164 : 0x7473756E);
	unsigned int FLASH2RAM_INIT_EN = 0x0000C35A;
	unsigned int FLASH2RAM_CODE_LENGTH = codeLength;
	unsigned int FLASH2RAM_CRC_DEN = DO_CRC ? 0xFFFFFFFF : 0x0000C35A;
	unsigned int FLASH2RAM_CRC_TARGET = codeCrcValue;
    	
	// update data here
	////////////////////////////////////////
	unsigned int dataList[8] =
	{
        FLASH2RAM_EXTRA_DEN,
        FLASH2RAM_EXTRA_LENGTH,
        FLASH2RAM_EXTRA_CRC,
        FLASH2RAM_QSPI_FLASH_MODE,
        FLASH2RAM_INIT_EN,
		FLASH2RAM_CODE_LENGTH,
		FLASH2RAM_CRC_DEN,
		FLASH2RAM_CRC_TARGET
	};

	int offset = 0;
	for (int i = 0; i < 8; i++)
	{
		buf_temp[0] = (unsigned char)(dataList[i]);
		buf_temp[1] = (unsigned char)(dataList[i] >> 8);
		buf_temp[2] = (unsigned char)(dataList[i] >> 16);
		buf_temp[3] = (unsigned char)(dataList[i] >> 24);
		for (int j = 0; j < 4; j++)
		{
			buf_info[offset + j] = buf_temp[j];
		}
		offset += 4;
	}

	////////////////////////////////////////
	if (0 == WriteFlashBuffer(FLASH_FLASH2RAM_START_ADDRESS, buf_info, FLASH2RAM_START_ADDRESS_LEN))
	{
		free(buf_info);
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	free(buf_info);
	return 1;
}

int FillFlash2reg(unsigned int actRegLength, unsigned char* SectorBuffer)
{
	unsigned char DO_CRC = 1;   // do crc keep 0xFFFFFFFF, or set 0x0000C35A

    // Read flash2reg info table
    unsigned int flash2RegInfoLength = FLASH_FLASH2REG_INFO_LAST_ADDRESS - FLASH_FLASH2REG_INFO_START_ADDRESS;
    unsigned int flash2RegCPLength = FLASH_IC_CONFIG_START_ADDRESS - FLASH_FLASH2REG_INFO_START_ADDRESS;
    unsigned char* flash2RegInfoData = (unsigned char*)malloc(flash2RegInfoLength);
    if (NULL == flash2RegInfoData)
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }

    // Read CP info to buffer
    if (0 == ReadFlashBuffer(FLASH_FLASH2REG_INFO_START_ADDRESS, flash2RegInfoData, flash2RegCPLength))
    {
        THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
        return 0;
    }

    // Fill Register Info to buffer
    memcpy((flash2RegInfoData + flash2RegCPLength), SectorBuffer, actRegLength);
    // Fill 0xFF to others buffer
    memset((flash2RegInfoData + flash2RegCPLength + actRegLength), 0xFF, (flash2RegInfoLength - flash2RegCPLength - actRegLength));
    
    unsigned int flash2RegLength = FLASH_FLASH2REG_ADDRESS_LENGTH;  // >= 24 bytes
    unsigned char flash2RegData[FLASH_FLASH2REG_ADDRESS_LENGTH];

	// set parameter
    unsigned int FLASH2REG_ACTUAL_LENGTH = actRegLength;
	unsigned int FLASH2REG_INIT_EN_ADDR = 0x0000C35A;
	unsigned int FLASH2REG_LENGTH_ADDR = flash2RegInfoLength;
	unsigned int FLASH2REG_CRC_DEN_ADDR = DO_CRC == 1 ? 0xFFFFFFFF : 0x0000C35A;
	unsigned int FLASH2REG_CRC_TARGET_ADDR = CalcCRC32(flash2RegInfoData, flash2RegInfoLength, 0);
	unsigned int FLASH2REG_START_ADDR_ADDR = (FLASH_FLASH2REG_INFO_START_ADDRESS & 0xFFFF) | 0x030000;

	// update data here
	////////////////////////////////////////
	unsigned char buf_temp[4];
	unsigned int dataList[6] =
	{
		FLASH2REG_ACTUAL_LENGTH,
		FLASH2REG_INIT_EN_ADDR,
		FLASH2REG_LENGTH_ADDR,
		FLASH2REG_CRC_DEN_ADDR,
		FLASH2REG_CRC_TARGET_ADDR,
		FLASH2REG_START_ADDR_ADDR
	};

	int offset = 0;
	for (int i = 0; i < 6; i++)
	{
		buf_temp[0] = (unsigned char)(dataList[i]);
		buf_temp[1] = (unsigned char)(dataList[i] >> 8);
		buf_temp[2] = (unsigned char)(dataList[i] >> 16);
		buf_temp[3] = (unsigned char)(dataList[i] >> 24);
		for (int j = 0; j < 4; j++)
		{
			flash2RegData[offset + j] = buf_temp[j];
		}
		offset += 4;
	}

	// Write reg info to buffer
	for (unsigned int i = 0; i < FLASH_FLASH2REG_ADDRESS_LENGTH; i++)
	{
        SectorBuffer[i + SECTOR_SIZE_4KB - FLASH_FLASH2REG_ADDRESS_LENGTH] = flash2RegData[i];
	}

	free(flash2RegInfoData);
	
	return 1;
}

int GetFirmwareVersion(unsigned short *version)
{
	unsigned char data[4];

	STRUCT_TCS_CMD cmd = { 0 };
	GetTcsCmd(TP_STD_CMD_INFO_FW_VER_RO, &cmd);

	if (0 == ReadDataTCS(cmd, data, 4, 0))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	*version = (unsigned short)(data[0] | data[1] << 8);

	return 1;
}

int GetChipConnectStatus(unsigned char bProgmode, unsigned char bdelay)
{
	unsigned char ucTemp[4];
	int retry_num = 3;

	do
	{
		if (bdelay != 0)
			cts_mdelay(50);

		if (bProgmode != 0)
		{
			if (ReadProg(REGDEF_BASE, ucTemp, 4) != 0)
			{
				if (ucTemp[0] == chipTypeId[0] && ucTemp[1] == chipTypeId[1])
					THP_LOGE("read: [%2x][%2x] vs tgt: [%2x][%2x] %d\n",ucTemp[0],ucTemp[1],chipTypeId[0],chipTypeId[1]);

				return 1;
			}
		} 
		else
		{
			if (GetChipID(ucTemp) != 0)
			{
				THP_LOGI("read: [%2x][%2x]", ucTemp[0], ucTemp[1]);
				if (ucTemp[0] == chipTypeId[0] && ucTemp[1] == chipTypeId[1])
					THP_LOGE("read: [%2x][%2x] vs tgt: [%2x][%2x]", ucTemp[0], ucTemp[1], chipTypeId[0], chipTypeId[1]);

				return 1;
			}
		}

		retry_num--;
	} while (retry_num > 0);

	return 0;
}

int GetChipID(unsigned char *id)
{
	unsigned char data[4];

	STRUCT_TCS_CMD cmd = { 0 };
	GetTcsCmd(TP_STD_CMD_INFO_CHIP_FW_ID_RO, &cmd);

	if (0 == ReadDataTCS(cmd, data, 4, 0))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	id[0] = data[0];
	id[1] = data[1];

	return 1;
}

int ReadCmdTCS(STRUCT_TCS_CMD cmd, unsigned char * buffer, unsigned int readLen, int readOffset, int delay)
{
	unsigned char * buf = (unsigned char *) malloc(readLen);
	if (0 == buf)
		return 0;

	int res = 0;

	for (int retry = 0; retry < TCS_RETRY_NUM; retry++)
	{
		if (0 == system_settings.m_TransMode)
			res = I2cReadDataTCS(cmd, buf, readLen, delay);
		else
			res = SpiReadDataTCS(cmd, buf, readLen, delay);

		if (res != 0)
			break;
	}
	
	for (unsigned int i = 0; i < readLen; i++)
	{
		buffer[readOffset + i] = buf[i];
	}

	free(buf);
	return res;
}

int GetChipIdVersion(unsigned short * version)
{
	unsigned char data[2];
	STRUCT_TCS_CMD cmd = { 0 };

	GetTcsCmd(TP_STD_CMD_INFO_CHIP_FW_ID_RO, &cmd);
	if ( 0 == ReadDataTCS(cmd, data, 2, 0) )
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	*version = (unsigned short)(data[0] | data[1] << 8);

	return 1;
}

int GetTcsVersion(unsigned short *pu16TcsVersion)
{
	unsigned char body[2];
	STRUCT_TCS_CMD cmd = { 0 };

	*pu16TcsVersion = 0;	
	GetTcsCmd(TP_STD_CMD_INFO_TCS_VER_RO, &cmd);
	// get tcs version
	if (0 == ReadDataTCS(cmd, body, 2, 0))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	*pu16TcsVersion = (unsigned short)((body[1] << 8) | body[0]);

	return 1;
}

int GetProjectID(char *id)
{
	strcpy(id, "");

	if (0 == GotoProgMode(1, 1))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	unsigned char regReadBackBuf[128];
	if (ReadFlashBuffer(FLASH_ID_INFO_ADDRESS, regReadBackBuf, sizeof(regReadBackBuf)) == 0)
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	unsigned char dataType = regReadBackBuf[PROJECT_ID_COUNT + 4];
	unsigned char dataLen = regReadBackBuf[PROJECT_ID_COUNT + 4 + 1];
	char* prefix = "ProjectID=";

	if (dataType == 0xff && dataLen == 0xff)
	{
		for (int i = 4; i < sizeof(regReadBackBuf); i++)
		{
			if (regReadBackBuf[i] == ',')
			{
				dataLen = i + 1 - 4;
				dataType = 1;
				break;
			}
		}
	}
	if (dataType == 0xff && dataLen == 0xff)
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	if (dataLen <= strlen(prefix))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	if (dataLen > PROJECT_ID_COUNT)
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	dataLen -= (unsigned char)strlen(prefix);

	if (dataType == 1)
	{
		memcpy(id, regReadBackBuf + strlen(prefix) + 4, dataLen - 1);
		id[dataLen - 1] = 0;
		return 1;
	}
	else if (dataType == 2)
	{
		strcpy(id, "0x");
		unsigned char *src = regReadBackBuf + strlen(prefix) + 4;
		char *dst = id + 2;
		int len = dataLen - 1;
		char hex[16] = { '0','1','2','3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E','F' };
		for (int i = 0; i < len; i++)
		{
			dst[i * 2] = hex[(src[i] >> 4) & 0x0f];
			dst[i * 2 + 1] = hex[(src[i] >> 0) & 0x0f];
		}
		dst[len * 2] = 0;
		return 1;
	}
	else
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}
}

int SetProjectID(char *id)
{	
	char* prefix = "ProjectID=";

	if (0 == GotoProgMode(1, 1))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}
	if (0 == WriteFlashBuffer(FLASH_ID_INFO_ADDRESS + strlen(prefix) + 4, (unsigned char *)id, strlen(id))) {
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}
	return 1;
}

int ReadWaferLotID(char *lotID, int len)
{
	int ret = 1;
	unsigned char waferID[32];
    int actLen = (len >= 16) ? 16 : len;

	memset(lotID, 0, len);

	if (!IsProgMode())
	{
		GotoProgMode(1, 1);
	}

	if (0 == ReadFlashBuffer(FLASH_WAFERLOTID_ADDRESS, waferID, 32))
	{
		THP_LOGE("[%s][%d]: error\n", __FILE__, __LINE__);
		return 0;
	}

	for (int i = 0; i < actLen; i++)
	{
		if (waferID[i] + waferID[i + 16] != 0xFF)
		{
			ret = 0;
			break;
		}
	}

	if (ret)
	{		
		for (int i = 0; i < actLen; i++)
			sprintf((lotID + i * 2), "%02X", waferID[i]);
	}

	return ret;
}

#define SPI 1
#define I2C 0 
void init_system_settings(){
	MEMSET(&system_settings,0,sizeof(SYSTEM_SETTINGS));
	system_settings.m_drw_mode = 1;
	system_settings.m_TransMode = SPI;
	system_settings.m_SpiProgReadDummyBytes = 4;
}
#endif
