#ifndef CTS_INSPECT_H
#define CTS_INSPECT_H

#include "cts_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __MUSL__
#define	INT_MAX					0x7fffffff
#define	INT_MIN					(-0x7fffffff-1)
#endif

enum cts_osc_trim_cali_info {
	OSC_TRIM_CALI_OK		= 0x1,
	OSC_TRIM_CALI_NG		= 0xA,
	OSC_TRIM_CALI_CNT_DEF	= 0x0,
	OSC_TRIM_CALI_CNT		= 0x1,
	OSC_TRIM_CALI_MAX		= 0x1,
};

/* match thp api */
uint32_t cts_inspect(void);
int cts_inspect_hsync(void);
int cts_inspect_hsync_only(void);
uint16_t *cts_get_inspect_grid_data(void);
uint16_t *cts_get_inspect_line_data(void);
int16_t *cts_get_inspect_noise(void);

#ifdef  CTS_FOR_FFT_MODE
extern int cts_tcs_set_fft_mode(uint8_t fft_mode);
extern int cts_test_polling_fftdata(uint16_t *buf, size_t size);
#endif

void cts_test_osc_trim(uint16_t real_sync, uint8_t trigger_ratio,
	uint8_t max_ratio, uint16_t osc_trim, uint16_t osc_trim_fine);
int cts_get_osc_trim(void);
void cts_rw_flash_prework(void);
void cts_set_cali_info(uint8_t osc_trim_cali_done, uint8_t osc_trim_cali_cnt);

#ifdef __cplusplus
}
#endif

#endif /* CTS_INSPECT_H */

