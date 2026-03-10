#ifndef CTS_UPFW_H
#define CTS_UPFW_H

#include "cts_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cts_firmware {
	uint32_t crc;
	size_t size;
	uint16_t fw_ver;
	int use_builtin;

	uint8_t *fw;
};

#pragma pack(push, 1)
typedef struct {
	uint32_t	f2r_en;
	uint32_t	len;
	uint32_t	crc_en;
	uint32_t	crc;
} f2r_data;
#pragma pack(pop)

int cts_update_firmware(uint16_t curr_ver);

#ifdef __cplusplus
}
#endif

#endif /* CTS_UPFW_H */

