/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_ACTION

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
	const EsifPrimitiveTuple tuple,
	const EsifDataPtr requestPtr
	);

/*
 * Handle ESIF Action Request
 */

static eEsifError ESIF_CALLCONV ActionConfigGet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifData params[2] = {0};
	EsifString replacedStrs[2] = {0};
	EsifString replacedStr = NULL;
	UInt8 i;

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(requestPtr);

	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);

	rc = EsifFpcAction_GetParams(fpcActionPtr,
		params,
		sizeof(params)/sizeof(*params));
	if (ESIF_OK != rc) {
		goto exit;
	}

	for (i = 0; i < sizeof(replacedStrs) / sizeof(*replacedStrs); i++) {
		replacedStr = EsifUp_CreateTokenReplacedParamString(upPtr, primitivePtr, params[i].buf_ptr);
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
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
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

	UNREFERENCED_PARAMETER(actCtx);

	ESIF_ASSERT(NULL != requestPtr);
	ESIF_ASSERT(NULL != requestPtr->buf_ptr);

	/* Optional 3rd Parameter = Config Flags */
	if (fpcActionPtr->param_valid[2]) {
		nparams++;
	}

	rc = EsifFpcAction_GetParams(fpcActionPtr,
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
		replacedStr = EsifUp_CreateTokenReplacedParamString(upPtr, primitivePtr, params[i].buf_ptr);
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
		ActionConfigSignalChangeEvents(upPtr, primitivePtr->tuple, requestPtr);
	}
exit:
	for (i = 0; i < sizeof(replacedStrs) / sizeof(*replacedStrs); i++) {
		esif_ccb_free(replacedStrs[i]);
	}
	return rc;
}


static eEsifError ActionConfigSignalChangeEvents(
	EsifUpPtr upPtr,
	const EsifPrimitiveTuple tuple,
	const EsifDataPtr requestPtr
	)
{
	eEsifError rc    = ESIF_OK;
	eEsifEventType targetEvent = 0;
	char domainStr[8] = "";
	esif_temp_t hysteresis_val = 0;
	esif_temp_t *hysteresis_ptr = NULL;
	esif_time_t polling_val = 0;
	esif_time_t *polling_ptr = NULL;

	ESIF_ASSERT(upPtr != NULL);

	switch (tuple.id) {
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
		targetEvent = ESIF_EVENT_ADAPTIVE_PERFORMANCE_CONDITIONS_TABLE_CHANGED;
		break;
	case SET_ADAPTIVE_PERFORMANCE_ACTIONS_TABLE:
		targetEvent = ESIF_EVENT_ADAPTIVE_PERFORMANCE_ACTIONS_TABLE_CHANGED;
		break;
	case SET_ADAPTIVE_PERFORMANCE_PARTICIPANT_CONDITION_TABLE:
		targetEvent = ESIF_EVENT_ADAPTIVE_PERFORMANCE_PARTICIPANT_CONDITION_TABLE_CHANGED;
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
	case SET_PERF_SUPPORT_STATES:
		targetEvent = ESIF_EVENT_DOMAIN_PERF_CAPABILITY_CHANGED;
		break;
	case SET_TEMPERATURE:
		targetEvent = ESIF_EVENT_DOMAIN_TEMP_THRESHOLD_CROSSED;
		break;
	case SET_TEMPERATURE_THRESHOLD_HYSTERESIS:
		targetEvent = ESIF_EVENT_PARTICIPANT_SPEC_INFO_CHANGED;
		if (requestPtr != NULL) {
			if (requestPtr->buf_ptr == NULL) {
				rc = ESIF_E_PARAMETER_IS_NULL;
				goto exit;
			}
			hysteresis_val = *(esif_temp_t *)requestPtr->buf_ptr;
			hysteresis_ptr = &hysteresis_val;

			esif_primitive_domain_str(tuple.domain, domainStr, sizeof(domainStr));
			ESIF_TRACE_DEBUG("Hysteresis changed for domain:%s\n", domainStr);
			EsifUp_UpdateHysteresis(upPtr, tuple.domain, hysteresis_ptr);
		}
		break;
	case SET_PARTICIPANT_SAMPLE_PERIOD:
		targetEvent = 0;
		if (requestPtr != NULL) {
			if (requestPtr->buf_ptr == NULL) {
				rc = ESIF_E_PARAMETER_IS_NULL;
				goto exit;
			}
			polling_val = *(esif_time_t *)requestPtr->buf_ptr;
			polling_ptr = &polling_val;

			esif_primitive_domain_str(tuple.domain, domainStr, sizeof(domainStr));
			ESIF_TRACE_DEBUG("Participant sample period changed for domain:%s\n", domainStr);
			EsifUp_UpdatePolling(upPtr, tuple.domain, polling_ptr);
		}
		break;
	case SET_DISPLAY_BRIGHTNESS_LEVELS:
	case SET_DISPLAY_CAPABILITY:
	case SET_DISPLAY_DEPTH_LIMIT:
		targetEvent = ESIF_EVENT_DOMAIN_DISPLAY_CAPABILITY_CHANGED;
		break;
	case SET_POWER_BOSS_CONDITIONS_TABLE:
		targetEvent = ESIF_EVENT_POWER_BOSS_CONDITIONS_TABLE_CHANGED;
		break;
	case SET_POWER_BOSS_ACTIONS_TABLE:
		targetEvent = ESIF_EVENT_POWER_BOSS_ACTIONS_TABLE_CHANGED;
		break;
	case SET_POWER_BOSS_MATH_TABLE:
		targetEvent = ESIF_EVENT_POWER_BOSS_MATH_TABLE_CHANGED;
		break;
	case SET_EMERGENCY_CALL_MODE_TABLE:
		targetEvent = ESIF_EVENT_EMERGENCY_CALL_MODE_TABLE_CHANGED;
		break;
	case SET_PID_ALGORITHM_TABLE:
		targetEvent = ESIF_EVENT_PID_ALGORITHM_TABLE_CHANGED;
		break;
	case SET_ACTIVE_CONTROL_POINT_RELATIONSHIP_TABLE:
		targetEvent = ESIF_EVENT_ACTIVE_CONTROL_POINT_RELATIONSHIP_TABLE_CHANGED;
		break;
	case SET_POWER_SHARING_ALGORITHM_TABLE:
		targetEvent = ESIF_EVENT_POWER_SHARING_ALGORITHM_TABLE_CHANGED;
		break;
	case SET_SUPPORTED_POLICIES:
		targetEvent = ESIF_EVENT_SUPPORTED_POLICIES_CHANGED;
		break;
	case SET_ACUR:
		targetEvent = ESIF_EVENT_PLATFORM_POWER_SOURCE_CHANGED;
		break;
	case SET_AVOL:
		targetEvent = ESIF_EVENT_PLATFORM_POWER_SOURCE_CHANGED;
		break;
	case SET_ADAPTER_POWER_RATING:
		targetEvent = ESIF_EVENT_PLATFORM_POWER_SOURCE_CHANGED;
		break;
	case SET_AP01:
		targetEvent = ESIF_EVENT_PLATFORM_POWER_SOURCE_CHANGED;
		break;
	case SET_AP02:
		targetEvent = ESIF_EVENT_PLATFORM_POWER_SOURCE_CHANGED;
		break;
	case SET_AP10:
		targetEvent = ESIF_EVENT_PLATFORM_POWER_SOURCE_CHANGED;
		break;
	case SET_PLATFORM_BATTERY_STEADY_STATE:
		targetEvent = ESIF_EVENT_PLATFORM_BATTERY_STEADY_STATE_CHANGED;
		break;
	case SET_PLATFORM_MAX_BATTERY_POWER:
		targetEvent = ESIF_EVENT_MAX_BATTERY_POWER_CHANGED;
		break;
	case SET_PLATFORM_REST_OF_POWER:
		targetEvent = ESIF_EVENT_PLATFORM_REST_OF_POWER_CHANGED;
		break;
	case SET_OEM_VARS:
		targetEvent = ESIF_EVENT_OEM_VARS_CHANGED;
		break;
	default:
		targetEvent = 0;
		break;
	}
	if (targetEvent > 0) {
		EsifEventMgr_SignalEvent(EsifUp_GetInstance(upPtr), tuple.domain, targetEvent, NULL);
	}	
exit:
	return rc;
}


/*
 *******************************************************************************
 ** Register ACTION with ESIF
 *******************************************************************************
 */
static EsifActIfaceStatic g_config = {
	eIfaceTypeAction,
	ESIF_ACT_IFACE_VER_STATIC,
	sizeof(g_config),
	ESIF_ACTION_CONFIG,
	ESIF_ACTION_FLAGS_DEFAULT,
	"CONFIG",
	"Configuration Data Management",
	ESIF_ACTION_VERSION_DEFAULT,
	NULL,
	NULL,
	ActionConfigGet,
	ActionConfigSet
};

eEsifError EsifActConfigInit()
{
	EsifActMgr_RegisterAction((EsifActIfacePtr)&g_config);
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}


void EsifActConfigExit()
{
	EsifActMgr_UnregisterAction((EsifActIfacePtr)&g_config);
	ESIF_TRACE_EXIT_INFO();
}

eEsifError EsifActConfigSignalChangeEvents(
	EsifUpPtr upPtr,
	const EsifPrimitiveTuple tuple,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_E_PARAMETER_IS_NULL;
	if (upPtr != NULL) {
		rc = ActionConfigSignalChangeEvents(upPtr, tuple, requestPtr);
	}
	return rc;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
