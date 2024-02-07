/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#pragma once

#include "esif.h"
#include "esif_sdk_data.h"
#include "esif_uf_ccb_thermalapi.h"



#define MAX_SYSTEM_CMD	256

// Execute System Command. Encapsulate to avoid Linux warnings when using gcc -O9
static ESIF_INLINE void esif_ccb_system(const char *cmd)
{
	if (esif_ccb_strlen(cmd, MAX_SYSTEM_CMD) < MAX_SYSTEM_CMD) {
		int rc = system(cmd);
		UNREFERENCED_PARAMETER(rc);
	}
}

// Reboot
static ESIF_INLINE void esif_ccb_reboot()
{
	esif_ccb_system("reboot");
}

/* Convert GUID format from
   AABBCCDD-EEFF-GGHH-IIJJ-KKLLMMNNOOPP to 
   DDCCBBAA-FFEE-HHGG-IIJJ-KKLLMMNNOOPP 
 */
static ESIF_INLINE void esif_guid_mangle(esif_guid_t *guid)
{
	u8 *ptr = (u8 *)guid;
	u8 b[ESIF_GUID_LEN] = {0};

	esif_ccb_memcpy(&b, ptr, ESIF_GUID_LEN);

	*(ptr + 0) = b[3];
	*(ptr + 1) = b[2];
	*(ptr + 2) = b[1];
	*(ptr + 3) = b[0];
	*(ptr + 4) = b[5];
	*(ptr + 5) = b[4];
	*(ptr + 6) = b[7];
	*(ptr + 7) = b[6];
}

// Enter S0 Shutdown
static ESIF_INLINE void esif_ccb_shutdown(
	UInt32 temperature,
	UInt32 tripPointTemperature,
	EsifString namePtr
	)
{
#if   defined(ESIF_ATTR_OS_CHROME)
	esif_ccb_system("shutdown -P now");
#else
	esif_ccb_system("shutdown -h now");
#endif
}

#define SUSPEND_DELAY_IN_MILLISECONDS 400

// Enter S4 Hibernation
static ESIF_INLINE void esif_ccb_hibernate(
	UInt32 temperature,
	UInt32 tripPointTemperature,
	EsifString namePtr
	)
{
#if   defined(ESIF_ATTR_OS_CHROME)
	/* NA */
#else
	esif_ccb_system("pm-hibernate");
#endif
}


// Enter S3 or CS
static ESIF_INLINE void esif_ccb_suspend()
{
#if   defined(ESIF_ATTR_OS_CHROME)
	esif_ccb_system("powerd_dbus_suspend");
#else
	esif_ccb_system("pm-suspend");
#endif
}

#define esif_ccb_disable_all_power_settings \
	ESIF_E_ACTION_NOT_IMPLEMENTED

#define esif_ccb_enable_power_setting(req_ptr) \
	ESIF_E_ACTION_NOT_IMPLEMENTED

#define esif_ccb_disable_power_setting(req_ptr) \
	ESIF_E_ACTION_NOT_IMPLEMENTED

#define esif_ccb_remove_power_setting(req_ptr) \
	ESIF_E_ACTION_NOT_IMPLEMENTED


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
