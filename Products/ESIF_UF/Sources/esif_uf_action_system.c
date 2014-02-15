/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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
** Handle ESIF Action Request
*/
static eEsifError ActionSystemSet(
	const void *actionHandle,
	const EsifString devicePathPtr,
	const EsifDataPtr p1Ptr,
	const EsifDataPtr p2Ptr,
	const EsifDataPtr p3Ptr,
	const EsifDataPtr p4Ptr,
	const EsifDataPtr p5Ptr,
	const EsifDataPtr requestPtr
	)
{
	EsifString command = NULL;

	UNREFERENCED_PARAMETER(actionHandle);
	UNREFERENCED_PARAMETER(devicePathPtr);
	UNREFERENCED_PARAMETER(p2Ptr);
	UNREFERENCED_PARAMETER(p3Ptr);
	UNREFERENCED_PARAMETER(p4Ptr);
	UNREFERENCED_PARAMETER(p5Ptr);

	ESIF_ASSERT(NULL != p1Ptr);
	ESIF_ASSERT(NULL != p1Ptr->buf_ptr);
	ESIF_ASSERT(ESIF_DATA_STRING == p1Ptr->type);

	command = (EsifString)p1Ptr->buf_ptr;

	/* Well known/Special Commands magic is used to avoid accidental calls*/
	if (!strcmp("SYSTEM_SLEEP", command)) {
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
		esif_ccb_shutdown(temperature, tripPointTemperature);

	} else if (!strcmp("SYSTEM_HIBERNATE", command)) {
		esif_ccb_hibernate();

	} else if (!strcmp("SYSTEM_REBOOT", command)) {
		esif_ccb_reboot();

	} else if (!strcmp("SYSTEM_REMPOL", command)) {
		//esif_ccb_remove_policy(requestPtr);

	} else {
		system(command);
	}
	return ESIF_OK;
}


/*
 *******************************************************************************
 ** Register ACTION with ESIF
 *******************************************************************************
 */
extern EsifActMgr g_actMgr;

#define PAD 0
#define IS_KERNEL 0
#define IS_PLUGIN 0

static EsifActType g_system = {
	0,
	ESIF_ACTION_SYSTEM,
	{PAD},
	"SYSTEM",
	"System Call",
	"ALL",
	"x1.0.0.1",
	{0},
	IS_KERNEL,
	IS_PLUGIN,
	{PAD},
	NULL,
	ActionSystemSet
};

enum esif_rc EsifActSystemInit()
{
	if (NULL != g_actMgr.AddActType) {
		g_actMgr.AddActType(&g_actMgr, &g_system);
	}
	return ESIF_OK;
}


void EsifActSystemExit()
{
	if (NULL != g_actMgr.RemoveActType) {
		g_actMgr.RemoveActType(&g_actMgr, 0);
	}
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
