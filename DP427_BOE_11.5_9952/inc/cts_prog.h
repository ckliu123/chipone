#ifndef CTS_PROG_H
#define CTS_PROG_H

#include <stdint.h>
#include <string.h>
#include <errno.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    CTS_SPI_TARGET_SLAVE,
    CTS_SPI_TARGET_MASTER,
    CTS_SPI_TARGET_ALL,
    CTS_SPI_TARGET_INVALID_ID   = 0xFF,
} CTS_SPI_TARGET_ENUM;

#define MEMSET(dst, val, size)              memset(dst, val, size)
#define MEMCPY(dst, src, size)              memcpy(dst, src, size)
#define MALLOC(size)                        malloc(size)

int cts_enter_prog_mode(void);
int cts_enter_drw_mode(void);

int cts_prog_read_raw(uint32_t addr, uint8_t *rbuf, size_t rlen);
int cts_drw_read_raw(uint32_t addr, uint8_t *rbuf, size_t rlen);

int cts_prog_write_raw(uint32_t addr, uint8_t *wbuf, size_t wlen);
int cts_drw_write_raw(uint32_t addr, uint8_t *wbuf, size_t wlen);

int cts_prog_readsb(uint32_t addr, uint8_t *rval, size_t len);

int cts_prog_read_u8(uint32_t addr, uint8_t *rval);
int cts_drw_read_u8(uint32_t addr, uint8_t *rval);

int cts_prog_read_u16(uint32_t addr, uint16_t *rval);
int cts_drw_read_u16(uint32_t addr, uint16_t *rval);

int cts_prog_read_u32(uint32_t addr, uint32_t *rval);
int cts_drw_read_u32(uint32_t addr, uint32_t *rval);

int cts_prog_write_u8(uint32_t addr, uint8_t wval);
int cts_drw_write_u8(uint32_t addr, uint8_t wval);

int cts_prog_write_u16(uint32_t addr, uint16_t wval);
int cts_drw_write_u16(uint32_t addr, uint16_t wval);

int cts_prog_write_u32(uint32_t addr, uint32_t wval);
int cts_drw_write_u32(uint32_t addr, uint32_t wval);

int cts_prog_spi_switch(CTS_SPI_TARGET_ENUM tgt);
int cts_drw_spi_switch(CTS_SPI_TARGET_ENUM tgt);

CTS_SPI_TARGET_ENUM cts_prog_get_target_id(void);
CTS_SPI_TARGET_ENUM cts_drw_get_target_id(void);
































#ifdef __cplusplus
}
#endif

#endif /* CTS_PROG_H */

