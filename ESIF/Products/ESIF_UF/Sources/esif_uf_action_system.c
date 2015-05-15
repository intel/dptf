/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
#include "esif_uf.h"			/* Upper Framework */
#include "esif_uf_actmgr.h"		/* Action Manager */
#include "esif_uf_ccb_system.h"	/* System Commands */

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/*
 * Handle ESIF Action "Get" Request
 */
static eEsifError ESIF_CALLCONV ActionSystemGet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError esifStatus = ESIF_E_ACTION_NOT_IMPLEMENTED;
	EsifData p1 = {0};
	EsifString command = NULL;

	UNREFERENCED_PARAMETER(actionHandle);
	UNREFERENCED_PARAMETER(upPtr);
	UNREFERENCED_PARAMETER(primitivePtr);
	UNREFERENCED_PARAMETER(actionPtr);
	UNREFERENCED_PARAMETER(requestPtr);

	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);

	esifStatus = EsifActionGetParamAsEsifData(actionPtr, 0, &p1);
	if (ESIF_OK != esifStatus) {
		goto exit;
	}
	ESIF_ASSERT(NULL != p1.buf_ptr);
	ESIF_ASSERT(ESIF_DATA_STRING == p1.type);

	command = (EsifString)p1.buf_ptr;

	if (!strcmp("SYSTEM_GET_CDPNAME0", command)) {
		esifStatus = system_get_ctdp_name(responsePtr, 0);

	} else if (!strcmp("SYSTEM_GET_CDPNAME1", command)) {
		esifStatus = system_get_ctdp_name(responsePtr, 1);

	} else if (!strcmp("SYSTEM_GET_CDPNAME2", command)) {
		esifStatus = system_get_ctdp_name(responsePtr, 2);
	} 
exit:
	return esifStatus;
}


/*
 * Handle ESIF Action "Set" Request
 */
static eEsifError ESIF_CALLCONV ActionSystemSet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const EsifDataPtr requestPtr
	)
{
	eEsifError esifStatus = ESIF_OK;
	EsifData p1 = {0};
	EsifString command = NULL;

	UNREFERENCED_PARAMETER(actionHandle);
	UNREFERENCED_PARAMETER(upPtr);
	UNREFERENCED_PARAMETER(primitivePtr);
	UNREFERENCED_PARAMETER(actionPtr);

	esifStatus = EsifActionGetParamAsEsifData(actionPtr, 0, &p1);
	if (ESIF_OK != esifStatus) {
		goto exit;
	}
	ESIF_ASSERT(NULL != p1.buf_ptr);
	ESIF_ASSERT(ESIF_DATA_STRING == p1.type);

	command = (EsifString)p1.buf_ptr;

	/* Well known/Special Commands magic is used to avoid accidental calls*/
	if (!strcmp("SYSTEM_SLEEP", command)) {
		ESIF_TRACE_INFO("SYSTEM_SLEEP command received - system suspend...\n");
		esif_ccb_suspend();

	} else if (!strcmp("SYSTEM_SHUTDOWN", command)) {
		UInt32 temperature = 0;
		UInt32 tripPointTemperature = 0;
		if (requestPtr && requestPtr->buf_ptr && ESIF_DATA_STRUCTURE == requestPtr->type) {
			/*
			** Thermal shutdown data was provided with request
			*/
			struct esif_data_complex_shutdown *shutdown_data =
				(struct esif_data_complex_shutdown *)requestPtr->buf_ptr;
			temperature = shutdown_data->temperature;
			tripPointTemperature = shutdown_data->tripPointTemperature;
		}
		ESIF_TRACE_INFO("SYSTEM_SHUTDOWN command received - temperature = %d, trip point = %d\n", temperature, tripPointTemperature);
		esif_ccb_shutdown(temperature, tripPointTemperature);

	} else if (!strcmp("SYSTEM_HIBERNATE", command)) {
		ESIF_TRACE_INFO("SYSTEM_HIBERNATE command received - system hibernate...\n");
		esif_ccb_hibernate();

	} else if (!strcmp("SYSTEM_REBOOT", command)) {
		ESIF_TRACE_INFO("SYSTEM_REBOOT command received - system reboot...\n");
		esif_ccb_reboot();

	} else if (!strcmp("SYSTEM_REM_PWRSETTING", command)) {
		esifStatus = esif_ccb_remove_power_setting(requestPtr);
		
	} else if (!strcmp("SYSTEM_CTDPCLR", command)) {
		esifStatus = system_clear_ctdp_names();

	} else if (!strcmp("SYSTEM_SET_CTDPNAME0", command)) {
		esifStatus = system_set_ctdp_name(requestPtr, 0);

	} else if (!strcmp("SYSTEM_SET_CTDPNAME1", command)) {
		esifStatus = system_set_ctdp_name(requestPtr, 1);

	} else if (!strcmp("SYSTEM_SET_CTDPNAME2", command)) {
		esifStatus = system_set_ctdp_name(requestPtr, 2);

	} else if (!strcmp("SYSTEM_ENA_PWRSETTING", command)) {
		esifStatus = esif_ccb_enable_power_setting(requestPtr);

	} else if (!strcmp("SYSTEM_DIS_PWRSETTING", command)) {
		esifStatus = esif_ccb_disable_power_setting(requestPtr);

	} else {
		system(command);
	}
exit:
	return esifStatus;
}


/*
 *******************************************************************************
 * Register ACTION with ESIF
 *******************************************************************************
 */
static EsifActType g_system = {
	0,
	ESIF_ACTION_SYSTEM,
	{PAD},
	"SYSTEM",
	"System Call",
	"ALL",
	"x1.0.0.1",
	{0},
	ESIF_ACTION_IS_NOT_KERNEL_ACTION,
	ESIF_ACTION_IS_NOT_PLUGIN,
	{PAD},
	ActionSystemGet,
	ActionSystemSet,
	NULL,
	NULL
};

enum esif_rc EsifActSystemInit()
{
	if (NULL != g_actMgr.AddActType) {
		g_actMgr.AddActType(&g_actMgr, &g_system);
	}
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}


void EsifActSystemExit()
{
	if (NULL != g_actMgr.RemoveActType) {
		g_actMgr.RemoveActType(&g_actMgr, 0);
	}
	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
