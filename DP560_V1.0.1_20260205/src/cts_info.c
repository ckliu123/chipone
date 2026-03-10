
#include "cts_hal.h"

#include <string.h>
#include <stdlib.h>

#include "cts_tcs.h"
#include "thp/thp_afe_hal.h"

#define AFE_VERSION						"1.0.1"

static THP_AFE_INFO_STRUCT g_thp_info;

extern uint16_t g_version;
extern uint32_t g_hwid;


THP_AFE_INFO_STRUCT *cts_get_info(void)
{
	char buf[32];
	
	THP_LOGI("cts_get_info +");

	MEMCPY(g_thp_info.vendor_name, "chipone", 32);
	
	SNPRINTF(buf, 32, "icnt%04x", (g_hwid & 0xFFFF));
	MEMCPY(g_thp_info.product_name, buf, 32);

	SNPRINTF(buf, 32, "%s_V0x%x", AFE_VERSION, g_version);
	MEMCPY(g_thp_info.version, buf, 32);

	THP_LOGI("VENDOR: %s, PRODUCT: %s, VERSION: %s", g_thp_info.vendor_name, g_thp_info.product_name, g_thp_info.version);
	
	return &g_thp_info;
}
