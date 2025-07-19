#ifndef CTS_TOOL_H
#define CTS_TOOL_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif
void cts_tool_start_thread(void);
void cts_tool_send_data_to_client(CTS_FRAME_STRUCT *cts_frame);
void cts_tool_save_frame_data(CTS_FRAME_STRUCT *cts_frame);

void cts_tool_start_gesture_data_thread();
void cts_tool_stop_gesture_data_thread();
#ifdef __cplusplus
}
#endif

#endif /* CTS_TOOL_H */

