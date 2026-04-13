#ifndef CTS_INSPECT_H
#define CTS_INSPECT_H

#include "thp_afe_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* match thp api */
uint32_t cts_inspect(void);
uint16_t *cts_get_inspect_grid_data(void);
uint16_t *cts_get_inspect_line_data(void);
int16_t *cts_get_inspect_noise(void);

#ifdef __cplusplus
}
#endif

#endif /* CTS_INSPECT_H */

