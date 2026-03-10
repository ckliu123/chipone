#ifndef CTS_TOOL_H
#define CTS_TOOL_H

#include "cts_hal.h"

#ifdef __cplusplus
extern "C" {
#endif
void cts_tool_start_thread(void);
int cts_tcs_read_spi_for_tool(uint8_t classID, uint8_t cmdID, uint8_t *buf, size_t len);
int cts_tcs_write_spi_for_tool(uint8_t classID, uint8_t cmdID, uint8_t *buf, size_t len);


void cts_tool_send_to_client(CTS_FRAME_STRUCT *cts_frame);

void cts_tool_save_frame_data(CTS_FRAME_STRUCT *cts_frame);

#ifdef __cplusplus
}
#endif

#endif /* CTS_TOOL_H */

