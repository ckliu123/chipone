#ifndef CTS_UTILS_H
#define CTS_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/*                                  Time                                     */
/*****************************************************************************/
long tv2ms(struct timeval *tv);
long tvdiff2ms(struct timeval *start_tv, struct timeval *end_tv);
long elapsedms(struct timeval *start_tv);

long cts_tmdiff2ms(TIME_T start, TIME_T end);
long cts_elapsedms(TIME_T start);
long tv2ms(struct timeval *tv);



/*****************************************************************************/
/*                                  CRC                                      */
/*****************************************************************************/
uint16_t cts_crc16(const uint8_t *buf, size_t len);
uint32_t cts_crc32(const uint8_t *buf, size_t len);


/*****************************************************************************/
/*                                  File                                     */
/*****************************************************************************/
int cts_load_file(const char *filepath, uint8_t **buf, size_t *len);


/*****************************************************************************/
/*                                  PACK                                     */
/*****************************************************************************/
static inline uint8_t *cts_put_unaligned_le16(uint16_t v, uint8_t *p)
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

static inline uint8_t *cts_put_unaligned_le24(uint32_t v, uint8_t *p)
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

static inline uint8_t *cts_put_unaligned_le32(uint32_t v, uint8_t *p)
{
    p[0] = (v >>  0) & 0xFF;
    p[1] = (v >>  8) & 0xFF;
    p[2] = (v >> 16) & 0xFF;
    p[3] = (v >> 24) & 0xFF;
    return p;
}

static inline uint8_t *cts_put_unaligned_be32(uint8_t *p,uint32_t v)
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
void cts_flipx(uint16_t *data, size_t nrow, size_t ncol);

/******************************************************************************
*                                                                             *
*                       1 +----+ 2             3 +----+ 4                     *
*                         |    |                 |    |                       *
*                         |    |       =>        |    |                       *
*                         |    |                 |    |                       *
*                       3 +----+ 4             1 +----+ 2                     *
*                                                                             *
******************************************************************************/
void cts_flipy(uint16_t *data, size_t nrow, size_t ncol);

/******************************************************************************
*                                                                             *
*                       1 +----+ 2             4 +----+ 3                     *
*                         |    |                 |    |                       *
*                         |    |       =>        |    |                       *
*                         |    |                 |    |                       *
*                       3 +----+ 4             2 +----+ 1                     *
*                                                                             *
******************************************************************************/
void cts_flipxy(uint16_t *data, size_t nrow, size_t ncol);


/*****************************************************************************/
/*                                DUMP                                       */
/*****************************************************************************/
#define SPI_DATA_TYPE_SEND      0
#define SPI_DATA_TYPE_RECV      1

void cts_enable_dump_spi(void);
void cts_disable_dump_spi(void);
void cts_dump_spi(uint8_t spi_data_type, const uint8_t *buf, size_t len);
void cts_dump_spi_err(uint8_t spi_data_type, const uint8_t *buf, size_t len);
void dump_spi_full_data(const uint8_t *data, int len);
void dump_spi_full_data_16(const uint16_t *data, size_t rows, size_t cols);
void dump_spi_full_data_point(const uint16_t *data, size_t rows, size_t cols);
void cts_enable_dump_data(void);
void cts_disable_dump_data(void);
void cts_dump_rawdata(const uint16_t *data, size_t nrow, size_t ncol);
void cts_dump_diffdata(const int16_t *data, size_t nrow, size_t ncol);
void dump_frame_data(const uint16_t *data,
                     size_t nrow, size_t ncol, uint8_t data_type);
void dump_stylus_data(const uint16_t *data,
                      size_t nrow, size_t ncol, uint8_t pc);
/*****************************************************************************/
/*                                MISC                                       */
/*****************************************************************************/
void mdelay(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* CTS_UTILS_H */

