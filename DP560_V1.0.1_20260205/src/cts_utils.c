#include "cts_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "cts_utils.h"
#include "cts_core.h"

long cts_tmdiff2ms(TIME_T start, TIME_T end)
{
	return TM2MS(end) - TM2MS(start);
}

long cts_elapsedms(TIME_T start)
{
	TIME_T end = GET_CURR_TIME();

	return TMDIFF2MS(start, end);
}

void cts_mdelay(uint32_t ms) {
	MSLEEP(ms);
}

uint16_t cts_crc16(const uint8_t *buf, size_t len)
{
	static const uint16_t crc16_table[] = {
		0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
		0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
		0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
		0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
		0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
		0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
		0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
		0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
		0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
		0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
		0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
		0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
		0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
		0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
		0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
		0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
		0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
		0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
		0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
		0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
		0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
		0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
		0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
		0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
		0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
		0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
		0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
		0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
		0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
		0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
		0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
		0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202,
	};

	uint16_t crc16 = 0; /* Initial value set to 0 */

	while (len) { /* Traverse for all data */
		crc16 = (crc16 << 8) ^ crc16_table[((crc16 >> 8) ^ *buf) & 0xFF];
		buf++;
		len--;
	}

	return crc16;
}

uint32_t cts_crc32(const uint8_t *buf, size_t len)
{
	static const uint32_t crc32_table[] = {
		0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9,
		0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
		0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
		0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
		0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9,
		0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
		0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
		0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
		0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
		0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
		0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
		0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
		0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49,
		0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
		0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
		0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
		0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE,
		0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
		0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
		0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
		0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
		0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
		0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
		0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
		0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
		0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
		0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
		0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
		0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
		0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
		0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686,
		0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
		0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
		0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
		0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F,
		0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
		0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
		0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
		0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
		0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
		0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7,
		0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
		0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F,
		0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
		0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
		0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
		0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F,
		0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
		0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
		0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
		0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
		0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
		0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
		0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
		0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088,
		0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
		0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
		0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
		0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
		0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
		0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0,
		0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
		0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
		0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4,
	};

	uint32_t crc32 = 0;

	while (len) {
		crc32 = (crc32 << 8) ^ crc32_table[((crc32 >> 24) ^ *buf) & 0xFF];
		buf++;
		len--;
	}

	return crc32;
}

int cts_flipx(uint16_t *data, size_t nrow, size_t ncol)
{
	size_t row, col;
	int line, pos_l, pos_r;
	uint16_t tmpval;

	if (!data) {
		return -1;
	}

	for (row = 0; row < nrow; row++) {
		line = row * ncol;

		for (col = 0; col < ncol / 2; col++) {
			pos_l = line + col;
			pos_r = line + (ncol - 1 - col);
			tmpval = data[pos_l];
			data[pos_l] = data[pos_r];
			data[pos_r] = tmpval;
		}
	}

	return 0;
}

int cts_flipy(uint16_t *data, size_t nrow, size_t ncol)
{
	size_t row, col;
	void *tmpbuf, *buf_t, *buf_b;
	size_t line_size;
	int line_t, line_b;
	uint16_t tmpval;
	int pos_t, pos_b;

	if (!data) {
		return -1;
	}

	line_size = ncol * sizeof(uint16_t);
	tmpbuf = MALLOC(line_size);

	if (tmpbuf) {
		for (row = 0; row < nrow / 2; row++) {
			buf_t = data + row * ncol;
			buf_b = data + (nrow - 1 - row) * ncol;
			MEMCPY(tmpbuf, buf_t, line_size);
			MEMCPY(buf_t, buf_b, line_size);
			MEMCPY(buf_b, tmpbuf, line_size);
		}
		LFREE(tmpbuf);
	} else {
		for (row = 0; row < nrow / 2; row++) {
			line_t = row * ncol;
			line_b = (nrow - 1 - row) * ncol;
			for (col = 0; col < ncol; col++) {
				pos_t = line_t + col;
				pos_b = line_b + col;
				tmpval = data[pos_t];
				data[pos_t] = data[pos_b];
				data[pos_b] = tmpval;
			}
		}
	}

	return 0;
}

int cts_flipxy(uint16_t *data, size_t nrow, size_t ncol)
{
	int i, total, half;
	int pos_a, pos_b;
	uint16_t tmpval;

	if (!data) {
		return -1;
	}

	total = nrow * ncol;
	half = total / 2;
	for (i = 0; i < half; i++) {
		pos_a = i;
		pos_b = total - 1 - i;
		tmpval = data[pos_a];
		data[pos_a] = data[pos_b];
		data[pos_b] = tmpval;
	}

	return 0;
}

int cts_conterclockwise_90(uint16_t *data, size_t nrow, size_t ncol)
{
	int i, j;
	uint16_t tmp_data[nrow*ncol];

	MEMCPY(tmp_data, data, sizeof(tmp_data));
	for (i = 0; i < ncol; i++) {
		for (j = 0; j < nrow; j++)
			data[i*nrow+j] = tmp_data[j*ncol+(ncol-1-i)];
	}
	
	return 0;
}

int cts_exchange_xy1(uint16_t *data, size_t nrow, size_t ncol)
{
	int i, j;
	uint16_t tmp_data[nrow*ncol];

	MEMCPY(tmp_data, data, sizeof(tmp_data));
	for (i = 0; i < nrow; i++) {
		for (j = 0; j < ncol; j++)
			data[j + i * ncol] = tmp_data[i + j * nrow];
	}

	return 0;
}

#define CTS_DUMP_BUF_SIZ		(1024 * 4)

#define _BUF_PTR				(str_buf + offset)
#define _BUF_SIZ				(sizeof(str_buf) - offset)

#define STR_DEFINE() \
	char str_buf[CTS_DUMP_BUF_SIZ]; \
	off_t offset = 0;

#define	STR_APPEND(fmt, ...) \
	do { \
		offset += SNPRINTF(_BUF_PTR, _BUF_SIZ, fmt, __VA_ARGS__); \
	} while (0)

#define STR_FULL() ((_BUF_SIZ > 0) && (_BUF_SIZ < 16))

#define STR_RESET() \
	do { \
		offset = 0; \
	} while (0)

#define STR_DUMP() THP_LOGD(str_buf)

extern int dump_spi;

static int cts_dump_spi(const char *tag, const uint8_t *buf, size_t len)
{
#define SPI_DUMP_STEP		32
	STR_DEFINE();
	int i;
/*
	if (!dump_spi) {
		return 0;
	}
*/
	if (!buf || !len) {
		return -1;
	}

	if (len <= SPI_DUMP_STEP) {
		STR_APPEND("%s[%4ld]={", tag, len);
		for (i = 0; i < len; i++) {
			STR_APPEND("%02x,", buf[i]);
		}
		STR_APPEND("%s", "}");
		STR_DUMP();
	} else {
		STR_APPEND("%s[%4ld]={", tag, len);
		STR_DUMP();
		STR_RESET();

		for (i = 0; i < len; i++) {
			if (i == 0) {
				STR_APPEND("[%04d-%04d] ", i + 1, i + SPI_DUMP_STEP);
			} else if (i % SPI_DUMP_STEP == 0) {
				STR_DUMP();
				STR_RESET();
				STR_APPEND("[%04d-%04d] ", i + 1, i + SPI_DUMP_STEP);
			}
			STR_APPEND("%02x,", buf[i]);
		}

		STR_DUMP();
		STR_RESET();
		STR_APPEND("%s", "}");
		STR_DUMP();
	}

	return 0;
}

int cts_dump_spi_tx(const uint8_t *buf, size_t len)
{
	return cts_dump_spi(">>", buf, len);
}

int cts_dump_spi_rx(const uint8_t *buf, size_t len)
{
	return cts_dump_spi("<<", buf, len);
}

#ifndef RELEASE
static void dump_frame_head(const char *tag, size_t ncol)
{
	STR_DEFINE();
	int i;

	STR_APPEND("%-6s", tag ? tag : "DATA");
	for (i = 0; i < ncol; i++) {
		STR_APPEND("[%3d] ", i);
	}
	STR_DUMP();
}

static void dump_frame_data(const uint16_t *data,
		size_t nrow, size_t ncol, uint8_t data_type)
{
	STR_DEFINE();
	int row, col;
	void *pos;

	for (row = 0; row < nrow; row++) {
		STR_RESET();
		STR_APPEND("[%4d] ", row);
		for (col = 0; col < ncol; col++) {
			pos = (void *)(data + row * ncol + col);
			STR_APPEND("%5d ", data_type ? *(int16_t *)pos : *(uint16_t *)pos);
		}
		STR_DUMP();
	}
}

static void dump_frame_self_data(const uint16_t *data,
		size_t nrow, size_t ncol, uint8_t data_type)
{
	STR_DEFINE();
	int row, col, line;
	void *pos;

	line = 1;
#if 0
	/* col data */
	for (row = 0; row < line; row++) {
		STR_RESET();
		STR_APPEND("[%4d] ", row);
		for (col = 0; col < ncol; col++) {
			pos = (void *)(data + (nrow * (ncol / 2)) + row * ncol + col);
			STR_APPEND("%5d ", data_type ? *(int16_t *)pos : *(uint16_t *)pos);
		}
		STR_DUMP();
	}

	/* row data */
	dump_frame_head("selfdata", nrow);
	for (row = 0; row < line; row++) {
		STR_RESET();
		STR_APPEND("[%4d] ", row);
		for (col = 0; col < nrow; col++) {
			pos = (void *)(data + (nrow * (ncol / 2)) + ncol + row * ncol + col);
			STR_APPEND("%5d ", data_type ? *(int16_t *)pos : *(uint16_t *)pos);
		}
		STR_DUMP();
	}
#else
	/* row data */
	for (row = 0; row < line; row++) {
		STR_RESET();
		STR_APPEND("[%4d] ", row);
		for (col = 0; col < ncol + nrow; col++) {
			pos = (void *)(data + col);
			STR_APPEND("%5d ", data_type ? *(int16_t *)pos : *(uint16_t *)pos);
		}
		STR_DUMP();
}


#endif
}

static void dump_frame_pen_data(const int16_t *data,
		size_t nrow, size_t ncol)
{
	STR_DEFINE();
	int row, col, pendata;
	int count;
	void *pos;

	THP_LOGI("Dump stylus_I_data");
	STR_RESET();
	for (count = 0; count < (ncol + nrow); count++) {
		pos = (void *)(data + count);
		STR_APPEND("%5d ", *(int16_t *)pos);
	}
	STR_DUMP();
	THP_LOGI("Dump stylus_Q_data");
	STR_RESET();
	for (count = 0; count < (ncol + nrow); count++) {
		pos = (void *)(data + ncol + nrow + count);
		STR_APPEND("%5d ", *(int16_t *)pos);
	}
	STR_DUMP();

#if 1
	STR_RESET();
	STR_APPEND("%-6s", "COL");
	for (col = 0; col < ncol; col++) {
		pendata = sqrt(data[col] * data[col] + data[ncol + nrow + col] * data[ncol + nrow + col]);
		STR_APPEND("%5d ", pendata);
	}
	STR_DUMP();

	STR_RESET();
	STR_APPEND("%-6s", "ROW");
	for (row = 0; row < nrow; row++) {
		pendata = sqrt(data[ncol + row] * data[ncol + row] + data[ncol * 2 + nrow + row] * data[ncol * 2 + nrow + row]);
		STR_APPEND("%5d ", pendata);
	}
	STR_DUMP();
#endif
}

static void dump_frame_stat(const char *tag, const uint16_t *data,
			    size_t nrow, size_t ncol, uint8_t data_type)
{
	STR_DEFINE();
	uint16_t *start, *end;
	int32_t min_val, max_val, cur_val;
	size_t min_pos = 0, max_pos = 0;
	int64_t sum = 0;

	min_val = data_type ? *(int16_t *)data : *data;
	max_val = data_type ? *(int16_t *)data : *data;

	for (start = (uint16_t *)data, end = (uint16_t *)(data + nrow * ncol); start < end; start++) {
		cur_val = data_type ? *(int16_t *) start : *start;
		sum += cur_val;

		if (min_val > cur_val) {
			min_val = cur_val;
			min_pos = start - data;
		}
		if (max_val < cur_val) {
			max_val = cur_val;
			max_pos = start - data;
		}
	}

	STR_APPEND("%-5s ", tag);
	STR_APPEND("min:%4d(%ld,%ld), ", min_val, (min_pos / ncol),
		   (min_pos % ncol));
	STR_APPEND("max:%4d(%ld,%ld), ", max_val, (max_pos / ncol),
		   (max_pos % ncol));
	STR_APPEND("average:%5ld", sum / (nrow * ncol));
	STR_DUMP();
}
#endif

#define DATA_TYPE_U16	0
#define DATA_TYPE_S16	1

int cts_dump_rawdata(const uint16_t *data, size_t nrow, size_t ncol)
{
	if (!data) {
		return -1;
	}
#ifndef RELEASE
	if (nrow == ROWS && ncol == COLS) {
		dump_frame_head("rawdata", ncol / 2);
		dump_frame_data(data, nrow, ncol / 2, DATA_TYPE_U16);
		dump_frame_stat("rawdata", data, nrow, ncol/2, DATA_TYPE_U16);
	} else {
		dump_frame_head("rawdata", ncol);
		dump_frame_data(data, nrow, ncol, DATA_TYPE_U16);
		dump_frame_stat("rawdata", data, nrow, ncol, DATA_TYPE_U16);
	}

	dump_frame_head("selfdata", nrow + ncol);
	dump_frame_self_data(data + nrow * ncol, nrow, ncol, DATA_TYPE_S16);
#endif

	return 0;
}

int cts_dump_diffdata(const int16_t *data, size_t nrow, size_t ncol)
{
	if (!data) {
		return -1;
	}

#ifndef RELEASE
	dump_frame_head("diffdata", ncol);
	dump_frame_data((uint16_t *)data, nrow, ncol, DATA_TYPE_S16);
	dump_frame_stat("diffdata", (uint16_t *) data, nrow, ncol, DATA_TYPE_S16);
#endif

	return 0;
}

int cts_dump_pendata(const int16_t *data_f1, const int16_t *data_f2, size_t nrow, size_t ncol)
{
	if (!data_f1 || !data_f2) {
		return -1;
	}

#ifndef RELEASE
	dump_frame_pen_data(data_f1, nrow, ncol);
	dump_frame_pen_data(data_f2, nrow, ncol);
#endif

	return 0;
}

