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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_ACTION

#include "esif_uf.h"			/* Upper Framework */
#include "esif_uf_actmgr.h"		/* Action Manager */
#include "esif_uf_appmgr.h"		/* Application Manager */
#include "esif_uf_domain.h"
#include "esif_uf_primitive.h"
#include "esif_uf_cfgmgr.h"
#include "esif_lib_databank.h"
#include "esif_temp.h"

// !!!
// TODO: Once we move to the DOMAIN MGR some of the interfaces in this file might change!!!
// Currently we do not get any of the domain/participant information as part of the interface.
// !!!

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

static eEsifError EsifGetActionDelegateGtt0(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifGetActionDelegateGtt1(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifGetActionDelegateTemp(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifGetActionDelegateVirtualTemperature(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifGetActionDelegateCnfg(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

static eEsifError EsifSetActionDelegateSphb(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);
	
static eEsifError EsifSetActionDelegatePat0(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegatePat1(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateRset(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateAppc(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	const EsifFpcActionPtr fpcActionPtr);

static eEsifError EsifSetActionDelegateEvaluateParticipantCaps(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateSampleBehavior(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateVirtualTemperature(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateToSignalOSEvent(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	eEsifEventType eventType);

static eEsifError EsifSetActionDelegateToSignalForegroundAppChanged(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr);

static eEsifError EsifSetActionDelegateSsap(
	EsifUpPtr upPtr,
	EsifDataPtr requestPtr
	);


/*
** Handle ESIF Action Request
*/

static eEsifError ESIF_CALLCONV ActionDelegateGet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifData p1 = {0};
	UInt32 method;
	EsifUpDomainPtr domainPtr = NULL;

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(upPtr);

	ESIF_ASSERT(NULL != responsePtr);
	ESIF_ASSERT(NULL != responsePtr->buf_ptr);
	ESIF_ASSERT(NULL != primitivePtr);
	ESIF_ASSERT(NULL != fpcActionPtr);

	domainPtr = EsifUp_GetDomainById(upPtr, primitivePtr->tuple.domain);
	if (NULL == domainPtr) {
		ESIF_TRACE_ERROR("Unable to get domain\n");
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	rc = EsifFpcAction_GetParamAsEsifData(fpcActionPtr, 0, &p1);
	if ((ESIF_OK != rc) || (NULL == p1.buf_ptr)) {
		ESIF_TRACE_ERROR("Unable to get parameters\n");
		goto exit;
	}

	method = *((UInt32 *)p1.buf_ptr);

	switch (method) {
	/* Get Temperature Trip Points */
	case '0TTG':	/* GTT0 */
		rc = EsifGetActionDelegateGtt0(domainPtr, responsePtr);
		break;

	case '1TTG':	/* GTT1 */
		rc = EsifGetActionDelegateGtt1(domainPtr, responsePtr);
		break;

	case 'PMT_': /* _TMP */
		rc = EsifGetActionDelegateTemp(domainPtr, requestPtr, responsePtr);
		break;

	case 'GFNC': /* CNFG */
		rc = EsifGetActionDelegateCnfg(domainPtr, requestPtr, responsePtr);
		break;

	case 'PMTV': /* VTMP */
		rc = EsifGetActionDelegateVirtualTemperature(domainPtr, responsePtr);
		break;

	default:
		rc = ESIF_E_NOT_IMPLEMENTED;
		break;
	}
exit:
	return rc;
}

static eEsifError ESIF_CALLCONV ActionDelegateSet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr fpcActionPtr,
	EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifData p1 = {0};
	UInt32 method;
	EsifUpDomainPtr domainPtr = NULL;

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(upPtr);

	ESIF_ASSERT(NULL != requestPtr);
	ESIF_ASSERT(NULL != requestPtr->buf_ptr);
	ESIF_ASSERT(NULL != primitivePtr);
	ESIF_ASSERT(NULL != fpcActionPtr);
	
	domainPtr = EsifUp_GetDomainById(upPtr, primitivePtr->tuple.domain);
	if (NULL == domainPtr) {
		ESIF_TRACE_ERROR("Unable to get domain\n");
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	rc = EsifFpcAction_GetParamAsEsifData(fpcActionPtr, 0, &p1);
	if ((ESIF_OK != rc) || (NULL == p1.buf_ptr)) {
		ESIF_TRACE_ERROR("Unable to get parameters\n");
		goto exit;
	}

	method = *((UInt32 *)p1.buf_ptr);

	switch (method) {
	
	/* Set Temperature Trip Points */
	case '0TAP':	/* PAT0 */
		ESIF_TRACE_INFO("PAT0 received\n");
		rc = EsifSetActionDelegatePat0(domainPtr, requestPtr);
		break;

	case '1TAP':	/* PAT1 */
		ESIF_TRACE_INFO("PAT1 received\n");
		rc = EsifSetActionDelegatePat1(domainPtr, requestPtr);
		break;

	case 'BSPS':	/* SPSB: Set Participant Sample Behavior */
		ESIF_TRACE_INFO("Set Sample Behavior received\n");
		rc = EsifSetActionDelegateSampleBehavior(domainPtr, requestPtr);
		break;

	case 'PMTV':	/* VTMP: Virtual Temperature */
		ESIF_TRACE_INFO("Set Virtual Temperature received\n");
		rc = EsifSetActionDelegateVirtualTemperature(domainPtr, requestPtr);
		break;

	case 'BHPS':	/* SPHB: Set Participant Hysteresis Behavior */
		ESIF_TRACE_INFO("Set Participant Hysteresis Behavior received\n");
		rc = EsifSetActionDelegateSphb(domainPtr, requestPtr);
		break;

	case 'CSPS':    /* SPSC: Set Platform State Of Charge */
		ESIF_TRACE_INFO("Set OS Battery Percentage received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_BATTERY_PERCENT_CHANGED);
		break;

	case 'SPPS':    /* SPPS: Set Platform Power Source */
		ESIF_TRACE_INFO("Set Platform Power Source received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_POWER_SOURCE_CHANGED);
		break;

	case 'OPDS':    /* SDPO: Set Display Orientation */
		ESIF_TRACE_INFO("Set Display Orientation received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_DISPLAY_ORIENTATION_CHANGED);
		break;

	case 'OVDS':    /* SDVO: Set Device Orientation */
		ESIF_TRACE_INFO("Set Device Orientation received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_DEVICE_ORIENTATION_CHANGED);
		break;

	case 'COMS':    /* SMOC: Set Motion Changed */
		ESIF_TRACE_INFO("Set Motion Changed received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_MOTION_CHANGED);
		break;

	case 'MKDS':    /* SDKM: Set Dock Mode */
		ESIF_TRACE_INFO("Set Dock Mode received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_DOCK_MODE_CHANGED);
		break;

	case 'MLCS':    /* SCLM: Set Cooling Mode */
		ESIF_TRACE_INFO("Set Cooling Mode received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_SYSTEM_COOLING_POLICY_CHANGED);
		break;

	case 'TSLS':    /* SLST: Set Lid State */
		ESIF_TRACE_INFO("Set Lid State received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_LID_STATE_CHANGED);
		break;

	case 'TFPS':    /* SPFT: Set Platform Type */
		ESIF_TRACE_INFO("Set Platform Type received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED);
		break;

	case 'AGFS':    /* SFGA: Set Foreground Application */
		ESIF_TRACE_INFO("Set Foreground Application received\n");
		rc = EsifSetActionDelegateToSignalForegroundAppChanged(domainPtr, requestPtr);
		break;

	case 'NOMS':    /* SMON: Set Mobile Notification */
		ESIF_TRACE_INFO("Set Mobile Notification request received\n");
		rc = EsifSetActionDelegateToSignalOSEvent(domainPtr, requestPtr, ESIF_EVENT_OS_MOBILE_NOTIFICATION);
		break;

	case 'TESR':    /* RSET: Reset Override */
		ESIF_TRACE_INFO("Reset Override request received\n");
		rc = EsifSetActionDelegateRset(domainPtr, requestPtr);
		break;

	case 'LAVE':    /* EVAL: Re-evaluate participant capabilities */
		ESIF_TRACE_INFO("Re-evaluate participant capabilities request received\n");
		rc = EsifSetActionDelegateEvaluateParticipantCaps(domainPtr, requestPtr);
		break;

	case 'CPPA':    /* APPC: Application Control  */
		ESIF_TRACE_INFO("Application Control\n");
		rc = EsifSetActionDelegateAppc(domainPtr, requestPtr, fpcActionPtr);
		break;

	case 'PASS':	/* SSAP: Specific Action Primitive execution */
		rc = EsifSetActionDelegateSsap(upPtr, requestPtr);
		break;

	default:
		rc = ESIF_E_NOT_IMPLEMENTED;
		break;
	}
exit:
	return rc;
}

static eEsifError EsifGetActionDelegateTemp(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	EsifPrimitiveTuple tempTuple = {GET_TEMPERATURE_SUR, 0, 255};

	ESIF_ASSERT(NULL != domainPtr);
	ESIF_ASSERT(NULL != responsePtr);

	tempTuple.domain = domainPtr->domain;
	return EsifUp_ExecutePrimitive(domainPtr->upPtr, &tempTuple, requestPtr, responsePtr);
}

static eEsifError EsifSetActionDelegateSphb(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	u32 tempHysteresis = ESIF_DOMAIN_TEMP_INVALID;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	tempHysteresis = *(u32 *)requestPtr->buf_ptr;

	rc = EsifUpDomain_SetTempHysteresis(domainPtr, tempHysteresis);

	ESIF_TRACE_DEBUG("Set Hysteresis = %d\n", esif_temp_abs_to_rel(tempHysteresis));

exit:
	return rc;
}

static eEsifError EsifSetActionDelegatePat0(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple auxTuple = { SET_TEMPERATURE_THRESHOLDS_SUR, 0, (UInt8)ESIF_DOMAIN_AUX0 };
	u32 tempThreshold = ESIF_DOMAIN_TEMP_INVALID;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	tempThreshold = *(u32 *)requestPtr->buf_ptr;

	ESIF_TRACE_DEBUG("Setting AUX0 = %d\n", tempThreshold);

	EsifUpDomain_SetTempThresh(domainPtr, ESIF_DOMAIN_AUX0, tempThreshold);

	auxTuple.domain = domainPtr->domain;
	rc = EsifUp_ExecutePrimitive(domainPtr->upPtr, &auxTuple, requestPtr, NULL);
	if (rc == ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP)
		rc = ESIF_OK;
exit:
	return rc;
}

static eEsifError EsifSetActionDelegatePat1(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	u32 tempThreshold = ESIF_DOMAIN_TEMP_INVALID;
	EsifPrimitiveTuple auxTuple = {SET_TEMPERATURE_THRESHOLDS_SUR, 0, (UInt8)ESIF_DOMAIN_AUX1};

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	
	tempThreshold = *(u32 *) requestPtr->buf_ptr;

	ESIF_TRACE_DEBUG("Setting AUX1 = %d\n", tempThreshold);
	
	EsifUpDomain_SetTempThresh(domainPtr, ESIF_DOMAIN_AUX1, tempThreshold);

	auxTuple.domain = domainPtr->domain;
	rc = EsifUp_ExecutePrimitive(domainPtr->upPtr, &auxTuple, requestPtr, NULL);
	if (rc == ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP)
		rc = ESIF_OK;
exit:
	return rc;
}

static eEsifError EsifSetActionDelegateRset(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
	EsifPrimitiveTupleParameter parameters = { 0 };
	EsifPrimitiveTuple tuple = { 0 };
	Bool signal_event = ESIF_FALSE;
	char domain_str[8] = { 0 };
	int j = 0;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);
	
	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	if (requestPtr->data_len != sizeof(parameters)) {
		rc = ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS;
		goto exit;
	}
	
	// Convert BINARY Parameters to Primitive Tuple
	esif_ccb_memcpy(&parameters, requestPtr->buf_ptr, sizeof(parameters));
	
	ESIF_TRACE_DEBUG("CONFIG RESET: { %s (%hd), %s, %hd }\n",
		esif_primitive_str(parameters.id.integer.value),
		(u16)parameters.id.integer.value,
		esif_primitive_domain_str((u16)parameters.domain.integer.value, domain_str, sizeof(domain_str)),
		(u16)parameters.instance.integer.value
		);

	// Look up Primitive Tuple in the DSP and verify it is a valid SET primtive
	EsifDspPtr dspPtr = EsifUp_GetDsp(domainPtr->upPtr);
	if (dspPtr == NULL) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}
	tuple.id = (u16) parameters.id.integer.value;
	tuple.domain = (u16) parameters.domain.integer.value;
	tuple.instance = (u16) parameters.instance.integer.value;
	EsifFpcPrimitivePtr primitivePtr = dspPtr->get_primitive(dspPtr, &tuple);
	if (primitivePtr == NULL) {
		rc = ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP;
		goto exit;
	}
	if (primitivePtr->operation != ESIF_PRIMITIVE_OP_SET) {
		rc = ESIF_E_INVALID_REQUEST_TYPE;
		goto exit;
	}

	// Find first CONFIG Action and Delete its Key from its DataVault
	for (j = 0; j < (int)primitivePtr->num_actions; j++) {
		EsifFpcActionPtr fpcActionPtr = dspPtr->get_action(dspPtr, primitivePtr, (u8)j);
		DataItemPtr paramDataVault = EsifFpcAction_GetParam(fpcActionPtr, (const UInt8)0);
		DataItemPtr paramKeyName = EsifFpcAction_GetParam(fpcActionPtr, (const UInt8)1);
		EsifString expandedKeyName = NULL;
		if (fpcActionPtr->type != ESIF_ACTION_CONFIG) {
			continue;
		}
		if (paramDataVault == NULL || paramKeyName == NULL || paramDataVault->data_type != ESIF_DSP_PARAMETER_TYPE_STRING || paramKeyName->data_type != ESIF_DSP_PARAMETER_TYPE_STRING) {
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			goto exit;
		}
		
		// Replace "%nm%" tokens in the key name or make a copy of the key name for static keys
		expandedKeyName = EsifUp_CreateTokenReplacedParamString(domainPtr->upPtr, primitivePtr, (StringPtr)paramKeyName->data);
		if (expandedKeyName == NULL) {
			expandedKeyName = esif_ccb_strdup((StringPtr)paramKeyName->data);
			if (expandedKeyName == NULL) {
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}
		}

		// Valid SET CONFIG Primitive found with valid DV/Key Name; Delete the associated Key from the DataVault
		EsifDataPtr data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, (StringPtr)paramDataVault->data, 0, ESIFAUTOLEN);
		EsifDataPtr data_key    = EsifData_CreateAs(ESIF_DATA_STRING, expandedKeyName, 0, ESIFAUTOLEN);

		// Do not signal an Event if Key does not exist in DataVault
		if (DataBank_KeyExists(g_DataBankMgr, (StringPtr)paramDataVault->data, expandedKeyName) == ESIF_FALSE) {
			rc = ESIF_OK;
		}
		else if (data_nspace == NULL || data_key == NULL) {
			rc = ESIF_E_NO_MEMORY;
		}
		else {
			// Delete Existing Key from DataVault
			rc = EsifConfigDelete(data_nspace, data_key);
			if (rc == ESIF_OK) {
				signal_event = ESIF_TRUE;
			}

			ESIF_TRACE_DEBUG("CONFIG RESET: config delete @%s %s [rc=%s (%d)]\n",
				(StringPtr)data_nspace->buf_ptr,
				(StringPtr)data_key->buf_ptr,
				esif_rc_str(rc),
				rc
				);
		}

		// Signal any Event(s) associated with this SET Primitive
		if (signal_event) {
			EsifActConfigSignalChangeEvents(domainPtr->upPtr, tuple, NULL);
		}

		EsifData_Destroy(data_nspace);
		EsifData_Destroy(data_key);
		esif_ccb_free(expandedKeyName);
		break;
	}
	if (j >= (int)primitivePtr->num_actions) {
		rc = ESIF_E_UNSUPPORTED_ACTION_TYPE;
	}

exit:
	return rc;
}

static eEsifError EsifSetActionDelegateAppc(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	const EsifFpcActionPtr fpcActionPtr)
{
	eEsifError rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
	EsifData p2 = { 0 };
	EsifString appName = NULL;
	UInt32 opcode = 0;

	UNREFERENCED_PARAMETER(domainPtr);

	ESIF_ASSERT(NULL != domainPtr);
	ESIF_ASSERT(NULL != requestPtr);
	ESIF_ASSERT(NULL != requestPtr);

	// Action Parameter 2 is App Control Opcode ('STRT','STOP', etc)
	// Primitive requestPtr is App Name ("dptf", ...)
	rc = EsifFpcAction_GetParamAsEsifData(fpcActionPtr, 1, &p2);
	if ((ESIF_OK != rc) || (NULL == p2.buf_ptr)) {
		rc = ESIF_E_INVALID_ARGUMENT_COUNT;
		goto exit;
	}
	if ((p2.type == ESIF_DATA_UINT32) && (p2.data_len == sizeof(UInt32))) {
		opcode = *(UInt32 *)p2.buf_ptr;
	}

	if ((requestPtr->type == ESIF_DATA_STRING) && (NULL != requestPtr->buf_ptr)) {
		appName = (EsifString)requestPtr->buf_ptr;

		switch (opcode) {
		case 'TRTS': // STRT: Start App
			rc = EsifAppMgr_AppStart(appName);
			break;
		case 'POTS': // STOP: Stop App
			rc = EsifAppMgr_AppStop(appName);
			break;
		default:
			rc = ESIF_E_INVALID_REQUEST_TYPE;
			break;
		}
	}

exit:
	return rc;
}

static eEsifError EsifSetActionDelegateEvaluateParticipantCaps(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifDspPtr dspPtr = NULL;
	EsifUpPtr upPtr = NULL;
	eEsifError iterRc = ESIF_OK;
	EsifFpcDomainIterator dspDomainiterator = { 0 };
	UInt8 currentDomainIndex = 0;
	UInt16 targetDomain = 0;
	EsifFpcDomainPtr fpcDomainPtr = NULL;
	
	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(domainPtr->upPtr != NULL);

	UNREFERENCED_PARAMETER(requestPtr);
	
	targetDomain = domainPtr->domain;

	upPtr = domainPtr->upPtr; 
	dspPtr = EsifUp_GetDsp(upPtr);

	if (NULL == dspPtr) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	if ((NULL == dspPtr->init_fpc_iterator) ||
		(NULL == dspPtr->get_next_fpc_domain)) {
		rc = ESIF_E_PRIMITIVE_ACTION_FAILURE;
		goto exit;
	}

	iterRc = dspPtr->init_fpc_iterator(dspPtr, &dspDomainiterator);
	if (ESIF_OK != iterRc) {
		goto exit;
	}

	iterRc = dspPtr->get_next_fpc_domain(dspPtr, &dspDomainiterator, &fpcDomainPtr);
	while (ESIF_OK == iterRc) {
		if (NULL == fpcDomainPtr) {
			iterRc = dspPtr->get_next_fpc_domain(dspPtr, &dspDomainiterator, &fpcDomainPtr);
			currentDomainIndex++;
			continue;
		}
		
		if ((UInt16) fpcDomainPtr->descriptor.domain == targetDomain) {
			/* Reset capabilities on the domain */
			EsifUpDomain_EnableCaps(domainPtr, fpcDomainPtr->capability_for_domain.capability_flags, fpcDomainPtr->capability_for_domain.capability_mask);
		}
		
		iterRc = dspPtr->get_next_fpc_domain(dspPtr, &dspDomainiterator, &fpcDomainPtr);
		currentDomainIndex++;
	}

	/* Perform capability detection */
	rc = EsifUpDomain_DspReadyInit(domainPtr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	/*
	 * Remove and re-add participant from app to re-establish capabilities and
	 * start polling temperature if necessary
	 */
	rc = EsifAppMgr_DestroyParticipantInAllApps(upPtr);
	if (ESIF_OK == rc) {
		rc = EsifAppMgr_CreateParticipantInAllApps(upPtr);
	}

exit:
	return rc;
}

static eEsifError EsifGetActionDelegateGtt0(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(responsePtr != NULL);

	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	*(u32 *) responsePtr->buf_ptr = (u32) domainPtr->tempAux0;
exit:
	return rc;
}

static eEsifError EsifGetActionDelegateGtt1(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(responsePtr != NULL);

	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	*(u32 *) responsePtr->buf_ptr = (u32) domainPtr->tempAux1;
exit:
	return rc;
}

static eEsifError EsifGetActionDelegateVirtualTemperature(
	const EsifUpDomainPtr domainPtr,
	EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(responsePtr != NULL);

	if (responsePtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	*(u32 *)responsePtr->buf_ptr = (u32)domainPtr->virtTemp;
exit:
	return rc;
}

static eEsifError EsifGetActionDelegateCnfg(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	extern int g_shell_enabled; // ESIF Shell Enabled Flag
	extern Bool g_ws_restricted;// Web Server Restricted Mode Flag
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple dcfgTuple = { GET_CONFIG_ACCESS_CONTROL_SUR, 0, 255 };
	EsifPrimitiveTuple gddvTuple = { GET_CONFIG_DATAVAULT_SUR, 0, 255 };
	EsifData dcfgData = { ESIF_DATA_UINT32, NULL, ESIF_DATA_ALLOCATE, 0 };
	EsifData gddvData = { ESIF_DATA_AUTO, NULL, ESIF_DATA_ALLOCATE, 0 };

	ESIF_ASSERT(NULL != domainPtr);
	ESIF_ASSERT(NULL != requestPtr);

	UNREFERENCED_PARAMETER(responsePtr); // Not currently used by DPTF

	dcfgTuple.domain = domainPtr->domain;
	gddvTuple.domain = domainPtr->domain;

	// Execute DCFG to read Access Control List Bitmask from BIOS, if it exists
	rc = EsifUp_ExecutePrimitive(domainPtr->upPtr, &dcfgTuple, requestPtr, &dcfgData);
	if (rc == ESIF_OK && dcfgData.buf_ptr != NULL && dcfgData.buf_len >= sizeof(UInt32)) {
		DCfgOptions newmask = { .asU32 = *(UInt32 *)dcfgData.buf_ptr };
		DCfg_Set(newmask);

		ESIF_TRACE_INFO("DCFG Loaded: 0x%08X\n", newmask.asU32);

		// Disable ESIF Shell if Access Control forbids it
		if (DCfg_Get().opt.ShellAccessControl) {
			g_shell_enabled = 0;
		}

		// Stop Web Server (if Started) if Restricted or Generic Access Control forbids it
		if (EsifWebIsStarted() && 
			((!g_ws_restricted && DCfg_Get().opt.GenericUIAccessControl)|| 
			 (g_ws_restricted && DCfg_Get().opt.RestrictedUIAccessControl)) ) {
			EsifWebStop();
		}
	}

	// Execute GDDV to read DataVault from BIOS, if it exists
	rc = EsifUp_ExecutePrimitive(domainPtr->upPtr, &gddvTuple, requestPtr, &gddvData);
	if (rc != ESIF_OK) {
		// Always Return OK if no ESIF_LF or GDDV object in BIOS
		if (rc == ESIF_E_NO_LOWER_FRAMEWORK || rc == ESIF_E_ACPI_OBJECT_NOT_FOUND) {
			rc = ESIF_OK;
		}
	}
	else {
		char *dv_name = "__merge"; // Temporary DV Name
		DataVaultPtr DB = DataBank_GetNameSpace(g_DataBankMgr, dv_name);
		if (DB != NULL) {
			DataBank_CloseNameSpace(g_DataBankMgr, dv_name);
		}
		DB = DataBank_OpenNameSpace(g_DataBankMgr, dv_name);

		// Load Datavault into temporary namespace. DV may or may not be preceded by a variant
		if (DB) {
			u32 skipbytes = 0;
			void *buffer = NULL;

			//
			// This is in place to resolve a static code analysis issue.
			// This should never happen if EsifUp_ExecutePrimitive is successful above.
			//
			if (NULL == gddvData.buf_ptr) {
				DataBank_CloseNameSpace(g_DataBankMgr, dv_name);
				ESIF_TRACE_DEBUG("No data returned for BIOS datavault.\n");
				goto exit;
			}

			skipbytes = (memcmp(gddvData.buf_ptr, "\xE5\x1F", 2) == 0 ? 0 : sizeof(union esif_data_variant));
			buffer = esif_ccb_malloc(gddvData.data_len);
			if (NULL == buffer) {
				DataBank_CloseNameSpace(g_DataBankMgr, dv_name);
				ESIF_TRACE_DEBUG("Unable to allocate memory\n");
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}

			esif_ccb_memcpy(buffer, (u8*)gddvData.buf_ptr + skipbytes, gddvData.data_len - skipbytes);
			IOStream_SetMemory(DB->stream, buffer, gddvData.data_len - skipbytes);

			if ((rc = DataVault_ReadVault(DB)) != ESIF_OK) {
				DataBank_CloseNameSpace(g_DataBankMgr, dv_name);
				ESIF_TRACE_DEBUG("Unable to Open DataVault: %s\n", esif_rc_str(rc));
				rc = ESIF_OK;
			}
			else {
				EsifDataPtr data_nspace = NULL;
				EsifDataPtr data_key = NULL;
				EsifDataPtr data_targetdv = NULL;
				esif_flags_t options = 0; // NOPERSIST
				esif_string keyspec = "*"; // Merge All Keys
				esif_string targetdv = g_DataVaultDefault;

				DB->flags |= (ESIF_SERVICE_CONFIG_READONLY);

				// Merge Contents into Default DataVault
				data_nspace = EsifData_CreateAs(ESIF_DATA_STRING, dv_name, 0, ESIFAUTOLEN);
				data_targetdv = EsifData_CreateAs(ESIF_DATA_STRING, targetdv, 0, ESIFAUTOLEN);
				data_key = EsifData_CreateAs(ESIF_DATA_STRING, keyspec, 0, ESIFAUTOLEN);
				if (data_nspace == NULL || data_key == NULL || data_targetdv == NULL) {
					rc = ESIF_E_NO_MEMORY;
				}
				else {
					rc = EsifConfigCopy(data_nspace, data_targetdv, data_key, options, ESIF_FALSE, NULL);
				}

				ESIF_TRACE_INFO("GDDV Loaded: %d bytes, %d keys => %s.dv [%s]\n",
					(int)IOStream_GetSize(DB->stream),
					DataCache_GetCount(DB->cache),
					targetdv,
					esif_rc_str(rc));

				EsifData_Destroy(data_nspace);
				EsifData_Destroy(data_key);
				EsifData_Destroy(data_targetdv);
				DataBank_CloseNameSpace(g_DataBankMgr, dv_name);
			}
			esif_ccb_free(buffer);
		}
	}
exit:
	esif_ccb_free(dcfgData.buf_ptr);
	esif_ccb_free(gddvData.buf_ptr);
	return rc;
}

static eEsifError EsifSetActionDelegateSampleBehavior(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	u32 samplePeriod = 0;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	samplePeriod = *(u32 *)requestPtr->buf_ptr;

	ESIF_TRACE_DEBUG("Setting Sample Period = %d\n", samplePeriod);

	EsifUpDomain_SetTempPollPeriod(domainPtr, samplePeriod);

exit:
	return rc;
}

static eEsifError EsifSetActionDelegateVirtualTemperature(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	u32 virtTemp = ESIF_DOMAIN_TEMP_INVALID;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	virtTemp = *(u32 *)requestPtr->buf_ptr;

	ESIF_TRACE_DEBUG("Setting Virtual Temp = %d\n", virtTemp);

	EsifUpDomain_SetVirtualTemperature(domainPtr, virtTemp);

exit:
	return rc;
}

static eEsifError EsifSetActionDelegateToSignalOSEvent(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr,
	eEsifEventType eventType)
{
	eEsifError rc = ESIF_OK;
	u32 updatedValue = 0;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	updatedValue = *(u32 *)requestPtr->buf_ptr;
	EsifUpDomain_SignalOSEvent(domainPtr, updatedValue, eventType);

exit:
	return rc;
}

static eEsifError EsifSetActionDelegateToSignalForegroundAppChanged(
	const EsifUpDomainPtr domainPtr,
	const EsifDataPtr requestPtr)
{
	eEsifError rc = ESIF_OK;
	EsifString appName = NULL;

	ESIF_ASSERT(domainPtr != NULL);
	ESIF_ASSERT(requestPtr != NULL);

	if (requestPtr->buf_ptr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	appName = (EsifString)requestPtr->buf_ptr;

	ESIF_TRACE_DEBUG("Setting Foreground App = %s\n", appName);

	EsifUpDomain_SignalForegroundAppChanged(domainPtr, appName);

exit:
	return rc;
}

/* Non-public function used by EsifSetActionDelegateSsap */
eEsifError EsifUp_ExecuteSpecificActionPrimitive(
	EsifUpPtr self,
	EsifPrimitiveTuplePtr tuplePtr,
	const EsifPrimitiveActionSelectorPtr selectorPtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
);


static eEsifError EsifSetActionDelegateSsap(
	EsifUpPtr upPtr,
	EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifSpecificActionRequestPtr sarPtr = NULL;

	ESIF_ASSERT(requestPtr != NULL);

	if ((NULL == requestPtr->buf_ptr) || (requestPtr->buf_len < sizeof(*sarPtr))) {
		rc = ESIF_E_REQUEST_DATA_OUT_OF_BOUNDS;
		goto exit;
	}

	sarPtr = (EsifSpecificActionRequestPtr)requestPtr->buf_ptr;

	rc = EsifUp_ExecuteSpecificActionPrimitive(upPtr, &sarPtr->tuple, &sarPtr->selector, sarPtr->req_ptr, sarPtr->rsp_ptr);
	if (ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP == rc) {
		rc = ESIF_E_PRIMITIVE_SUR_NOT_FOUND_IN_DSP;
	}
exit:
	return rc;
}


/*
 *******************************************************************************
 ** Register ACTION with ESIF
 *******************************************************************************
 */
static EsifActIfaceStatic g_delegate = {
	eIfaceTypeAction,
	ESIF_ACT_IFACE_VER_STATIC,
	sizeof(g_delegate),
	ESIF_ACTION_DELEGATE,
	ESIF_ACTION_FLAGS_DEFAULT,
	"DELEGATE",
	"Delegate Action",
	ESIF_ACTION_VERSION_DEFAULT,
	NULL,
	NULL,
	ActionDelegateGet,
	ActionDelegateSet
};

eEsifError EsifActDelegateInit()
{
	EsifActMgr_RegisterAction((EsifActIfacePtr)&g_delegate);
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}

void EsifActDelegateExit()
{
	EsifActMgr_UnregisterAction((EsifActIfacePtr)&g_delegate);
	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
