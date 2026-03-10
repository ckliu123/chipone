#ifndef CTS_DRW_H
#define CTS_DRW_H

#include "cts_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

int cts_enter_drw_mode(void);

int cts_drw_read_raw(uint32_t addr, uint8_t *rbuf, size_t rlen);
int cts_drw_write_raw(uint32_t addr, uint8_t *wbuf, size_t wlen);

int cts_drw_read_u8(uint32_t addr, uint8_t *rval);
int cts_drw_read_u16(uint32_t addr, uint16_t *rval);
int cts_drw_read_u32(uint32_t addr, uint32_t *rval);
int cts_drw_write_u8(uint32_t addr, uint8_t wval);
int cts_drw_write_u16(uint32_t addr, uint16_t wval);
int cts_drw_write_u32(uint32_t addr, uint32_t wval);

#ifdef __cplusplus
}
#endif

#endif /* CTS_DRW_H */

