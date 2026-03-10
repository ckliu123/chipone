#ifndef CTS_UTILS_H
#define CTS_UTILS_H

#include "cts_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/*                                  Time                                     */
/*****************************************************************************/
long cts_tmdiff2ms(TIME_T start, TIME_T end);
long cts_elapsedms(TIME_T start);
void cts_mdelay(uint32_t ms);


/*****************************************************************************/
/*                                  CRC                                      */
/*****************************************************************************/
uint16_t cts_crc16(const uint8_t *buf, size_t len);
uint32_t cts_crc32(const uint8_t *buf, size_t len);


/*****************************************************************************/
/*                                  PACK                                     */
/*****************************************************************************/
static inline uint8_t *cts_put_unaligned_le16(uint8_t *p, uint16_t v)
{
	p[0] = (v >> 0) & 0xFF;
	p[1] = (v >> 8) & 0xFF;
	return p;
}

static inline uint8_t *cts_put_unaligned_be16(uint8_t *p, uint16_t v)
{
	p[0] = (v >> 8) & 0xFF;
	p[1] = (v >> 0) & 0xFF;
	return p;
}

static inline uint8_t *cts_put_unaligned_le24(uint8_t *p, uint32_t v)
{
	p[0] = (v >>  0) & 0xFF;
	p[1] = (v >>  8) & 0xFF;
	p[2] = (v >> 16) & 0xFF;
	return p;
}

static inline uint8_t *cts_put_unaligned_be24(uint8_t *p, uint32_t v)
{
	p[0] = (v >> 16) & 0xFF;
	p[1] = (v >>  8) & 0xFF;
	p[2] = (v >>  0) & 0xFF;
	return p;
}

static inline uint8_t *cts_put_unaligned_le32(uint8_t *p, uint32_t v)
{
	p[0] = (v >>  0) & 0xFF;
	p[1] = (v >>  8) & 0xFF;
	p[2] = (v >> 16) & 0xFF;
	p[3] = (v >> 24) & 0xFF;
	return 0;
}

static inline uint8_t *cts_put_unaligned_be32(uint8_t *p, uint32_t v)
{
	p[0] = (v >> 24) & 0xFF;
	p[1] = (v >> 16) & 0xFF;
	p[2] = (v >>  8) & 0xFF;
	p[3] = (v >>  0) & 0xFF;
	return p;
}

static inline uint16_t cts_get_unaligned_le16(uint8_t *p)
{
	return (p[0] | (p[1] << 8));
}

static inline uint16_t cts_get_unaligned_be16(uint8_t *p)
{
	return (p[1] | (p[0] << 8));
}

static inline uint32_t cts_get_unaligned_le24(uint8_t *p)
{
	return (p[0] | (p[1] << 8) | (p[2] << 16));
}

static inline uint32_t cts_get_unaligned_be24(uint8_t *p)
{
	return (p[2] | (p[1] << 8) | (p[0] << 16));
}

static inline uint32_t cts_get_unaligned_le32(uint8_t *p)
{
	return (p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}

static inline uint32_t cts_get_unaligned_be32(uint8_t *p)
{
	return (p[3] | (p[2] << 8) | (p[1] << 16) | (p[0] << 24));
}


/*****************************************************************************/
/*                                  Flip                                     */
/*****************************************************************************/

/******************************************************************************
*                                                                             *
*                       1 +----+ 2             2 +----+ 1                     *
*                         |    |                 |    |                       *
*                         |    |       =>        |    |                       *
*                         |    |                 |    |                       *
*                       3 +----+ 4             4 +----+ 3                     *
*                                                                             *
******************************************************************************/
int cts_flipx(uint16_t *data, size_t nrow, size_t ncol);

/******************************************************************************
*                                                                             *
*                       1 +----+ 2             3 +----+ 4                     *
*                         |    |                 |    |                       *
*                         |    |       =>        |    |                       *
*                         |    |                 |    |                       *
*                       3 +----+ 4             1 +----+ 2                     *
*                                                                             *
******************************************************************************/
int cts_flipy(uint16_t *data, size_t nrow, size_t ncol);

/******************************************************************************
*                                                                             *
*                       1 +----+ 2             4 +----+ 3                     *
*                         |    |                 |    |                       *
*                         |    |       =>        |    |                       *
*                         |    |                 |    |                       *
*                       3 +----+ 4             2 +----+ 1                     *
*                                                                             *
******************************************************************************/
int cts_flipxy(uint16_t *data, size_t nrow, size_t ncol);

/******************************************************************************
*                                                                             *
*                       1 +----+ 2             2 +----+ 4                     *
*                         |    |                 |    |                       *
*                         |    |       =>        |    |                       *
*                         |    |                 |    |                       *
*                       3 +----+ 4             1 +----+ 3                     *
*                                                                             *
******************************************************************************/
int cts_conterclockwise_90(uint16_t *data, size_t nrow, size_t ncol);

/******************************************************************************
*                                                                             *
*                       1 +----+ 2             1 +----+ 3                     *
*                         |    |                 |    |                       *
*                         |    |       =>        |    |                       *
*                         |    |                 |    |                       *
*                       3 +----+ 4             2 +----+ 4                     *
*                                                                             *
******************************************************************************/
int cts_exchange_xy1(uint16_t *data, size_t nrow, size_t ncol);

/*****************************************************************************/
/*                                DUMP                                       */
/*****************************************************************************/
int cts_dump_spi_tx(const uint8_t *buf, size_t len);
int cts_dump_spi_rx(const uint8_t *buf, size_t len);
int cts_dump_rawdata(const uint16_t *data, size_t nrow, size_t ncol);
int cts_dump_diffdata(const int16_t *data, size_t nrow, size_t ncol);
int	cts_dump_pendata(const int16_t *data_f1, const int16_t *data_f2, size_t nrow, size_t ncol);

#ifdef __cplusplus
}
#endif

#endif /* CTS_UTILS_H */

