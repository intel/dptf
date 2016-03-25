/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

#ifdef ESIF_ATTR_OS_WINDOWS
#include "powrprof.h"
#include "win\dppe.h"
#endif

#ifdef ESIF_ATTR_OS_ANDROID
#include <cutils/android_reboot.h>
#include <cutils/properties.h>
#endif

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
#if defined(ESIF_ATTR_OS_WINDOWS)
	esif_ccb_system("shutdown /r /t 0");
#else
	esif_ccb_system("reboot");
#endif
}

static ESIF_INLINE void esif_guid_to_ms_guid(esif_guid_t *guid)
{
#ifdef ESIF_ATTR_OS_WINDOWS
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
#endif
}

// Enter S0 Shutdown
static ESIF_INLINE void esif_ccb_shutdown(
	UInt32 temperature,
	UInt32 tripPointTemperature
	)
{
#if defined(ESIF_ATTR_OS_WINDOWS)
	esif_ccb_report_thermal_event(
		ENVIRONMENTAL_EVENT_SHUTDOWN,
		temperature,
		tripPointTemperature
		);
	esif_ccb_system("shutdown /s /f /t 0");
#elif defined(ESIF_ATTR_OS_CHROME)
	esif_ccb_system("shutdown -P now");
#elif defined(ESIF_ATTR_OS_ANDROID)
	property_set(ANDROID_RB_PROPERTY, "shutdown");
#else
	esif_ccb_system("shutdown -h now");
#endif
}

#define SUSPEND_DELAY_IN_MILLISECONDS 400

// Enter S4 Hibernation
static ESIF_INLINE void esif_ccb_hibernate(
	UInt32 temperature,
	UInt32 tripPointTemperature
	)
{
#if defined(ESIF_ATTR_OS_WINDOWS)
	esif_ccb_report_thermal_event(
		ENVIRONMENTAL_EVENT_HIBERNATE,
		temperature,
		tripPointTemperature
		);
	/*
	** TODO: Remove this code later as this is only a temporary solution for
	** problems related to attempting a hibernate too soon after a resume in
	** Windows.
	*/
	esif_ccb_sleep_msec(SUSPEND_DELAY_IN_MILLISECONDS);
	SetSuspendState(1, 1, 0);
#elif defined(ESIF_ATTR_OS_CHROME)
	/* NA */
#else
	esif_ccb_system("pm-hibernate");
#endif
}


// Enter S3 or CS
static ESIF_INLINE void esif_ccb_suspend()
{
#if defined(ESIF_ATTR_OS_WINDOWS)
	/*
	** TODO: Remove this code later as this is only a temporary solution for
	** problems related to attempting a sleep too soon after a resume in
	** Windows.
	*/
	esif_ccb_sleep_msec(SUSPEND_DELAY_IN_MILLISECONDS);
	SetSuspendState(0, 1, 0);
#elif defined(ESIF_ATTR_OS_CHROME)
	esif_ccb_system("powerd_dbus_suspend");
#elif defined(ESIF_ATTR_OS_ANDROID)
	esif_ccb_system("input keyevent 26");
#else
	esif_ccb_system("pm-suspend");
#endif
}

#ifdef ESIF_ATTR_OS_WINDOWS
#define esif_ccb_disable_all_power_settings \
	esif_ccb_disable_all_power_settings_win

#define esif_ccb_enable_power_setting(req_ptr) \
	esif_ccb_enable_power_setting_win(req_ptr)

#define esif_ccb_disable_power_setting(req_ptr) \
	esif_ccb_disable_power_setting_win(req_ptr)

#define esif_ccb_remove_power_setting(req_ptr) \
	esif_ccb_remove_power_setting_win(req_ptr)

#define system_clear_ctdp_names() \
	system_clear_ctdp_names_win()

#define system_set_ctdp_name(request, instance) \
	system_set_ctdp_name_win(request, instance)

#define system_get_ctdp_name(response, instance) \
	system_get_ctdp_name_win(response, instance)

#else
#define esif_ccb_disable_all_power_settings \
	ESIF_E_ACTION_NOT_IMPLEMENTED

#define esif_ccb_enable_power_setting(req_ptr) \
	ESIF_E_ACTION_NOT_IMPLEMENTED

#define esif_ccb_disable_power_setting(req_ptr) \
	ESIF_E_ACTION_NOT_IMPLEMENTED

#define esif_ccb_remove_power_setting(req_ptr) \
	ESIF_E_ACTION_NOT_IMPLEMENTED

#define system_clear_ctdp_names() \
	ESIF_E_ACTION_NOT_IMPLEMENTED

#define system_set_ctdp_name(request, instance) \
	ESIF_E_ACTION_NOT_IMPLEMENTED

#define system_get_ctdp_name(response, instance) \
	ESIF_E_ACTION_NOT_IMPLEMENTED

#endif

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
