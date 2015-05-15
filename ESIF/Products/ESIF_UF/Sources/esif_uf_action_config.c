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
#include "esif_uf_cfgmgr.h"		/* Configuration */
#include "esif_primitive.h"
#include "esif_uf_eventmgr.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

static eEsifError ActionConfigSignalChangeEvents(
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifDataPtr requestPtr
	);

/*
 * Handle ESIF Action Request
 */

static eEsifError ESIF_CALLCONV ActionConfigGet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifData params[2] = {0};
	EsifString replacedStrs[2] = {0};
	EsifString replacedStr = NULL;
	UInt8 i;

	UNREFERENCED_PARAMETER(actionHandle);
	UNREFERENCED_PARAMETER(requestPtr);

	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);

	rc = EsifActionGetParams(actionPtr,
		params,
		sizeof(params)/sizeof(*params));
	if (ESIF_OK != rc) {
		goto exit;
	}

	for (i = 0; i < sizeof(replacedStrs) / sizeof(*replacedStrs); i++) {
		replacedStr = EsifActionCreateTokenReplacedParamString(params[i].buf_ptr, upPtr, primitivePtr);
		if (replacedStr != NULL) {
			params[i].buf_ptr = replacedStr;
			replacedStrs[i] = replacedStr;
		}
	}

	ESIF_ASSERT(NULL != params[0].buf_ptr);
	ESIF_ASSERT(ESIF_DATA_STRING == params[0].type);

	rc = EsifConfigGet(&params[0], &params[1], responsePtr);
exit:
	for (i = 0; i < sizeof(replacedStrs) / sizeof(*replacedStrs); i++) {
		esif_ccb_free(replacedStrs[i]);
	}
	return rc;
}


static eEsifError ESIF_CALLCONV ActionConfigSet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	esif_flags_t flags = ESIF_SERVICE_CONFIG_PERSIST;
	EsifData params[3] = {0};
	EsifString replacedStrs[2] = {0};
	EsifString replacedStr = NULL;
	UInt8 i;
	UInt8 nparams = 2;

	UNREFERENCED_PARAMETER(actionHandle);

	ESIF_ASSERT(NULL != requestPtr);
	ESIF_ASSERT(NULL != requestPtr->buf_ptr);

	/* 255 To delete temperature based key */
	if (ESIF_DATA_TEMPERATURE == requestPtr->type &&
		255 == *(UInt32 *)requestPtr->buf_ptr) {
		flags |= ESIF_SERVICE_CONFIG_DELETE;
	}

	/* Optional 3rd Parameter = Config Flags */
	if (actionPtr->param_valid[2]) {
		nparams++;
	}

	rc = EsifActionGetParams(actionPtr,
		params,
		nparams);
	if (ESIF_OK != rc) {
		goto exit;
	}

	/* Extract Optional EsifConfigSet flags */
	if (nparams > 2 && params[2].type == ESIF_DATA_UINT32) {
		flags = *(UInt32 *)(params[2].buf_ptr);
	}

	for (i = 0; i < sizeof(replacedStrs) / sizeof(*replacedStrs); i++) {
		replacedStr = EsifActionCreateTokenReplacedParamString(params[i].buf_ptr, upPtr, primitivePtr);
		if (replacedStr != NULL) {
			params[i].buf_ptr = replacedStr;
			replacedStrs[i] = replacedStr;
		}
	}

	ESIF_ASSERT(NULL != params[0].buf_ptr);
	ESIF_ASSERT(ESIF_DATA_STRING == params[0].type);

	rc = EsifConfigSet(&params[0], &params[1], flags, requestPtr);

	/*
	 * Ignore DataVault I/O Errors so DSP Overrides can be written to or cleared 
	 * from DataCache even if DV file system is read-only
	 */
	switch (rc) {
	case ESIF_E_IO_ERROR:
	case ESIF_E_IO_OPEN_FAILED:
	case ESIF_E_IO_DELETE_FAILED:
		rc = ESIF_OK;
		break;
	default:
		break;
	}

	if (ESIF_OK == rc) {
		ActionConfigSignalChangeEvents(upPtr, primitivePtr, requestPtr);
	}
exit:
	for (i = 0; i < sizeof(replacedStrs) / sizeof(*replacedStrs); i++) {
		esif_ccb_free(replacedStrs[i]);
	}
	return rc;
}


static eEsifError ActionConfigSignalChangeEvents(
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifDataPtr requestPtr
	)
{
	eEsifError rc    = ESIF_OK;
	eEsifEventType targetEvent = 0;
	char domainStr[8] = "";
	struct esif_data voidData = {ESIF_DATA_VOID, NULL, 0};

	ESIF_ASSERT(upPtr != NULL);
	ESIF_ASSERT(primitivePtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	switch (primitivePtr->tuple.id) {
	case SET_TRIP_POINT_ACTIVE:
	case SET_TRIP_POINT_CRITICAL:
	case SET_TRIP_POINT_HOT:
	case SET_TRIP_POINT_PASSIVE:
	case SET_TRIP_POINT_WARM:
		targetEvent = ESIF_EVENT_PARTICIPANT_SPEC_INFO_CHANGED;
		break;
	case SET_THERMAL_RELATIONSHIP_TABLE:
		targetEvent = ESIF_EVENT_APP_THERMAL_RELATIONSHIP_CHANGED;
		break;
	case SET_PASSIVE_RELATIONSHIP_TABLE:
		targetEvent = ESIF_EVENT_PASSIVE_TABLE_CHANGED;
		break;
	case SET_ACTIVE_RELATIONSHIP_TABLE:
		targetEvent = ESIF_EVENT_APP_ACTIVE_RELATIONSHIP_CHANGED;
		break;
	case SET_ADAPTIVE_PERFORMANCE_CONDITIONS_TABLE:
		targetEvent = ESIF_EVENT_ADAPTIVE_PERFORMANCE_CONDITIONS_CHANGED;
		break;
	case SET_ADAPTIVE_PERFORMANCE_ACTIONS_TABLE:
		targetEvent = ESIF_EVENT_ADAPTIVE_PERFORMANCE_ACTIONS_CHANGED;
		break;
	case SET_VIRTUAL_SENSOR_CALIB_TABLE:
		targetEvent = ESIF_EVENT_VIRTUAL_SENSOR_CALIB_TABLE_CHANGED;
		break;
	case SET_VIRTUAL_SENSOR_POLLING_TABLE:
		targetEvent = ESIF_EVENT_VIRTUAL_SENSOR_POLLING_TABLE_CHANGED;
		break;
	case SET_RAPL_POWER_CONTROL_CAPABILITIES:
		targetEvent = ESIF_EVENT_DOMAIN_POWER_CAPABILITY_CHANGED;
		break;
	case SET_PROC_PERF_PSTATE_DEPTH_LIMIT:
	case SET_PERF_PSTATE_DEPTH_LIMIT:
	case SET_PERF_SUPPORT_STATE:
		targetEvent = ESIF_EVENT_DOMAIN_PERF_CAPABILITY_CHANGED;
		break;
	case SET_TEMPERATURE:
		targetEvent = ESIF_EVENT_DOMAIN_TEMP_THRESHOLD_CROSSED;
		break;
	case SET_TEMPERATURE_THRESHOLD_HYSTERESIS:
		targetEvent = ESIF_EVENT_PARTICIPANT_SPEC_INFO_CHANGED;
		if (requestPtr->buf_ptr == NULL) {
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}
		esif_primitive_domain_str(primitivePtr->tuple.domain, domainStr, sizeof(domainStr));
		ESIF_TRACE_DEBUG("Hysteresis changed for domain:%s\n", domainStr);
		EsifUp_UpdateHysteresis(upPtr, primitivePtr->tuple.domain, *(esif_temp_t *) requestPtr->buf_ptr);
		break;
	case SET_PARTICIPANT_SAMPLE_PERIOD:
		targetEvent = 0;
		if (requestPtr->buf_ptr == NULL) {
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}
		esif_primitive_domain_str(primitivePtr->tuple.domain, domainStr, sizeof(domainStr));
		ESIF_TRACE_DEBUG("Participant sample period changed for domain:%s\n", domainStr);
		EsifUp_UpdatePolling(upPtr, primitivePtr->tuple.domain, *(UInt32 *) requestPtr->buf_ptr);
		break;
	case SET_DISPLAY_BRIGHTNESS_LEVELS:
	case SET_DISPLAY_CAPABILITY:
	case SET_DISPLAY_DEPTH_LIMIT:
		targetEvent = ESIF_EVENT_DOMAIN_DISPLAY_CAPABILITY_CHANGED;
		break;
	case SET_PDR_TABLE:
		targetEvent = ESIF_EVENT_OS_POWER_SOURCE_CHANGED;
		break;
	default:
		targetEvent = 0;
		break;
	}
	if (targetEvent > 0) {
		EsifEventMgr_SignalEvent(upPtr->fInstance, primitivePtr->tuple.domain, targetEvent, &voidData);
	}	
exit:
	return rc;
}


/*
 *******************************************************************************
 ** Register ACTION with ESIF
 *******************************************************************************
 */
static EsifActType g_config = {
	0,
	ESIF_ACTION_CONFIG,
	{PAD},
	"CONFIG",
	"Configuration Data Management",
	"ALL",
	"x1.0.0.1",
	{0},
	ESIF_ACTION_IS_NOT_KERNEL_ACTION,
	ESIF_ACTION_IS_NOT_PLUGIN,
	{PAD},
	ActionConfigGet,
	ActionConfigSet,
	NULL,
	NULL
};

enum esif_rc EsifActConfigInit()
{
	if (NULL != g_actMgr.AddActType) {
		g_actMgr.AddActType(&g_actMgr, &g_config);
	}
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}


void EsifActConfigExit()
{
	if (NULL != g_actMgr.RemoveActType) {
		g_actMgr.RemoveActType(&g_actMgr, 0);
	}
	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
