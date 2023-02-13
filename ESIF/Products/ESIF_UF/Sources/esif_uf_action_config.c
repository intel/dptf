/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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


extern char *esif_str_replace(char *orig, char *rep, char *with);

static eEsifError ActionConfigSignalChangeEvents(
	EsifUpPtr upPtr,
	const EsifPrimitiveTuple tuple,
	const EsifDataPtr requestPtr,
	const esif_event_type_t signalEventId
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
	EsifData params[3] = { 0 };
	EsifString replacedStrs[sizeof(params) / sizeof(params[0])] = { 0 };
	EsifString replacedStr = NULL;
	EsifString primaryKey = NULL;
	UInt8 i;
	UInt8 nparams = 2;

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(requestPtr);

	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);

	// Optional 3rd Parameter = Subkey Name
	if (fpcActionPtr->param_valid[2]) {
		nparams++;
	}

	rc = EsifFpcAction_GetParams(fpcActionPtr,
		params,
		nparams);
	if (ESIF_OK != rc) {
		goto exit;
	}

	for (i = 0; i < nparams; i++) {
		if (params[i].type == ESIF_DATA_STRING) {
			replacedStr = EsifUp_CreateTokenReplacedParamString(upPtr, primitivePtr, params[i].buf_ptr);
			if (replacedStr != NULL) {
				if (i == 1 && EsifUp_IsPrimaryParticipant(upPtr)) {
					primaryKey = params[i].buf_ptr;
				}
				params[i].buf_ptr = replacedStr;
				replacedStrs[i] = replacedStr;
			}
		}
	}

	ESIF_ASSERT(NULL != params[0].buf_ptr);
	ESIF_ASSERT(ESIF_DATA_STRING == params[0].type);

	if (nparams == 2) {
		rc = EsifConfigGet(&params[0], &params[1], responsePtr);
	}
	else {
		rc = EsifConfigGetSubKey(&params[0], &params[1], responsePtr, &params[2]);
	}

	// Workaround for Backwards Compatibility with GDDV objects that use a Primary Participant not named IETM
	if (rc == ESIF_E_NOT_FOUND && primaryKey && esif_ccb_strstr(primaryKey, "%nm%") != NULL) {
		EsifString primaryParticipants[] = { ESIF_PARTICIPANT_DPTF_NAME, "DPTFZ", NULL };

		// If key not found for Primary Participant name, try other known Primary Participant names
		for (int j = 0; rc == ESIF_E_NOT_FOUND && primaryParticipants[j] != NULL; j++) {
			if (esif_ccb_stricmp(EsifUp_GetName(upPtr), primaryParticipants[j]) != 0) {
				char nameTag[ESIF_NAME_LEN] = { 0 };
				esif_ccb_sprintf(sizeof(nameTag), nameTag, "%s.D0", primaryParticipants[j]);
				
				char *keyname = esif_str_replace(primaryKey, "%nm%", nameTag);
				if (keyname != NULL) {
					char *buf_ptr = params[1].buf_ptr;
					params[1].buf_ptr = keyname;
					if (nparams == 2) {
						rc = EsifConfigGet(&params[0], &params[1], responsePtr);
					}
					else {
						rc = EsifConfigGetSubKey(&params[0], &params[1], responsePtr, &params[2]);
					}
					params[1].buf_ptr = buf_ptr;
					esif_ccb_free(keyname);
				}
			}
		}
	}

exit:
	for (i = 0; i < nparams; i++) {
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
	eEsifError rc = ESIF_E_PARAMETER_IS_NULL;
	esif_flags_t flags = ESIF_SERVICE_CONFIG_PERSIST;
	EsifData params[2] = {0};
	EsifString replacedStrs[2] = {0};
	EsifString replacedStr = NULL;
	UInt8 i;
	UInt8 nparams = 2;
	DataItemPtr optparam = NULL;
	esif_event_type_t signalEventId = ESIF_EVENT_NONE;

	UNREFERENCED_PARAMETER(actCtx);

	ESIF_ASSERT(NULL != requestPtr);

	/* First two parameters required: NAMESPACE, KEYNAME */
	if (!fpcActionPtr->param_valid[0] || !fpcActionPtr->param_valid[1]) {
		goto exit;
	}

	/* Allow NULL buf_ptr for variable-length types if buf_len = 0 */
	EsifData requestEmpty = {0};
	if (requestPtr && requestPtr->buf_ptr == NULL && requestPtr->buf_len == 0 && requestPtr->type != ESIF_DATA_VOID && esif_data_type_sizeof(requestPtr->type) == 0) {
		static char nonEmptyBuffer[1] = {0};
		requestEmpty.type = requestPtr->type;
		requestEmpty.buf_ptr = nonEmptyBuffer;
		requestPtr = &requestEmpty;
	}

	rc = EsifFpcAction_GetParams(fpcActionPtr,
		params,
		nparams);
	if (ESIF_OK != rc) {
		goto exit;
	}

	/* Optional 3rd Parameter = Config Flags */
	if (fpcActionPtr->param_valid[2] && (optparam = EsifFpcAction_GetParam(fpcActionPtr, 2)) != NULL && (size_t)optparam->data_length_in_bytes == sizeof(flags)) {
		flags = *(esif_flags_t *)optparam->data;
	}

	/* Optional 4th Parameter = Signal Event ID */
	if (fpcActionPtr->param_valid[3] && (optparam = EsifFpcAction_GetParam(fpcActionPtr, 3)) != NULL) {
		if ((optparam->data_type == ESIF_DSP_PARAMETER_TYPE_VARIANT) && (size_t)optparam->data_length_in_bytes == sizeof(esif_event_type_t)) {
			signalEventId = *(esif_event_type_t *)optparam->data;
		}
	}

	for (i = 0; i < sizeof(replacedStrs) / sizeof(*replacedStrs); i++) {
		replacedStr = EsifUp_CreateTokenReplacedParamString(upPtr, primitivePtr, params[i].buf_ptr);
		
		// Always save Primary Participant keys using IETM for GDDV Portability and OEM Updatability
		if (i == 1 && replacedStr != NULL && EsifUp_IsPrimaryParticipant(upPtr) && 
			(esif_ccb_stricmp(EsifUp_GetName(upPtr), ESIF_PARTICIPANT_DPTF_NAME) != 0) && 
			(esif_ccb_strstr(params[i].buf_ptr, "%nm%") != NULL)) {
			char nameTag[ESIF_NAME_LEN] = { 0 };
			esif_ccb_sprintf(sizeof(nameTag), nameTag, "%s.D0", ESIF_PARTICIPANT_DPTF_NAME);
			esif_ccb_free(replacedStr);
			replacedStr = esif_str_replace(params[i].buf_ptr, "%nm%", nameTag);
		}
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
		ActionConfigSignalChangeEvents(upPtr, primitivePtr->tuple, requestPtr, signalEventId);
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
	const EsifDataPtr requestPtr,
	const esif_event_type_t signalEventId
	)
{
	eEsifError rc    = ESIF_OK;
	char domainStr[8] = "";
	esif_temp_t hysteresis_val = 0;
	esif_temp_t *hysteresis_ptr = NULL;
	esif_time_t polling_val = 0;
	esif_time_t *polling_ptr = NULL;

	ESIF_ASSERT(upPtr != NULL);

	/*
	** Take custom actions for certain specific Primitives.
	** Target Signal Event ID is passed in via DSP CONFIG Action Parameter #4 instead of mapping here.
	*/
	switch (tuple.id) {
	case SET_TEMPERATURE_THRESHOLD_HYSTERESIS:
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
	default:
		break;
	}

	if (signalEventId != ESIF_EVENT_NONE) {
		EsifEventMgr_SignalEvent(EsifUp_GetInstance(upPtr), tuple.domain, signalEventId, NULL);
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
	const EsifDataPtr requestPtr,
	const esif_event_type_t signalEventId)
{
	eEsifError rc = ESIF_E_PARAMETER_IS_NULL;
	if (upPtr != NULL) {
		rc = ActionConfigSignalChangeEvents(upPtr, tuple, requestPtr, signalEventId);
	}
	return rc;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
