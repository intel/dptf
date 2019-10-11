/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_EVENT

#include "esif_uf.h"		/* Upper Framework */
#include "esif_sdk_message.h"
#include "esif_uf_fpc.h"
#include "esif_uf_domain.h"
#include "esif_participant.h"
#include "esif_uf_eventmgr.h"
#include "esif_pm.h"
#include "esif_ccb_atomic.h"


atomic_t g_policyLoggingRefCount = ATOMIC_INIT(0);

typedef struct EsifEventMsgFrame_s {
	EsifMsgHdr header;
	EsifEventMsg payload;
} EsifEventMsgFrame, *EsifEventMsgFramePtr;

static eEsifError ESIF_CALLCONV MotionSensorEventCallback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr)
{
	UInt32 *evtDataPtr = NULL;
	esif_event_type_t evtType = fpcEventPtr->esif_event;
	EsifEventMsgFrame eventMsgFrame = { 0 };
	eEsifError rc = ESIF_OK;

	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(participantId);
	UNREFERENCED_PARAMETER(domainId);

	if (NULL == eventDataPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (eventDataPtr->data_len < sizeof(UInt32)) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	evtDataPtr = (UInt32*) eventDataPtr->buf_ptr;
	if (NULL == evtDataPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	EsifMsgHdr_Init(&eventMsgFrame.header, ESIFMSG_CLASS_EVENT, sizeof(eventMsgFrame.payload));
	eventMsgFrame.payload.revision = ESIF_EVENT_REVISION;
	eventMsgFrame.payload.v1.type = evtType;

	// Sanity checks
	switch (evtType) {
	case ESIF_EVENT_DISPLAY_ORIENTATION_CHANGED:
		eventMsgFrame.payload.v1.data.dispOrientationData = *evtDataPtr;
		break;

	case ESIF_EVENT_DEVICE_ORIENTATION_CHANGED:
		eventMsgFrame.payload.v1.data.platOrientationData = *evtDataPtr;
		break;

	case ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED:
		eventMsgFrame.payload.v1.data.platTypeData = *evtDataPtr;
		break;

	case ESIF_EVENT_MOTION_CHANGED:
		eventMsgFrame.payload.v1.data.motionStateData = *evtDataPtr;
		break;

	default:
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s event recevied. Event data = %u\n",
			esif_event_type_str(evtType),
			*evtDataPtr);
	rc = esif_ws_broadcast_data_buffer((u8 *)&eventMsgFrame, sizeof(eventMsgFrame));

exit:
	return rc;
}

static eEsifError ESIF_CALLCONV PolicyLoggingCallback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr)
{
	EsifPolicyLogDataPtr policyLogDataPtr = NULL;
	EsifEventMsgFrame eventMsgFrame = { 0 };
	eEsifError rc = ESIF_OK;

	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(participantId);
	UNREFERENCED_PARAMETER(domainId);

	// Sanity checks
	if (ESIF_EVENT_DPTF_POLICY_LOADED_UNLOADED != fpcEventPtr->esif_event) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	if (NULL == eventDataPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (eventDataPtr->data_len < sizeof(*policyLogDataPtr)) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	policyLogDataPtr = (EsifPolicyLogDataPtr)eventDataPtr->buf_ptr;
	if (NULL == policyLogDataPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	ESIF_TRACE_DEBUG("ESIF_EVENT_DPTF_POLICY_LOADED_UNLOADED event recevied\n");

	EsifMsgHdr_Init(&eventMsgFrame.header, ESIFMSG_CLASS_EVENT, sizeof(eventMsgFrame.payload));
	eventMsgFrame.payload.revision = ESIF_EVENT_REVISION;
	eventMsgFrame.payload.v1.type = ESIF_EVENT_DPTF_POLICY_LOADED_UNLOADED;
	eventMsgFrame.payload.v1.data.policyLogData= *policyLogDataPtr;

	rc = esif_ws_broadcast_data_buffer((u8 *)&eventMsgFrame, sizeof(eventMsgFrame));

exit:
	return rc;
}

static eEsifError ESIF_CALLCONV ControlActionCallback(
	esif_context_t context,
	esif_handle_t participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr)
{
	EsifCapabilityDataPtr capabilityDataPtr = NULL;
	EsifEventMsgFrame eventMsgFrame = { 0 };
	eEsifError rc = ESIF_OK;

	UNREFERENCED_PARAMETER(context);
	// Sanity checks
	if (ESIF_EVENT_DPTF_PARTICIPANT_CONTROL_ACTION != fpcEventPtr->esif_event) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	if (NULL == eventDataPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (eventDataPtr->data_len < sizeof(*capabilityDataPtr)) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	capabilityDataPtr = (EsifCapabilityDataPtr)eventDataPtr->buf_ptr;
	if (NULL == capabilityDataPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	ESIF_TRACE_DEBUG("ESIF_EVENT_DPTF_PARTICIPANT_CONTROL_ACTION: Participant ID: " ESIF_HANDLE_FMT ", domain ID: 0x%x\n",
		esif_ccb_handle2llu(participantId), domainId);

	EsifMsgHdr_Init(&eventMsgFrame.header, ESIFMSG_CLASS_EVENT, sizeof(eventMsgFrame.payload));
	eventMsgFrame.payload.revision = ESIF_EVENT_REVISION;
	eventMsgFrame.payload.v1.type = ESIF_EVENT_DPTF_PARTICIPANT_CONTROL_ACTION;
	eventMsgFrame.payload.v1.data.capabilityData = *capabilityDataPtr;

	rc = esif_ws_broadcast_data_buffer((u8 *) &eventMsgFrame, sizeof(eventMsgFrame));

exit:
	return rc;
}

static void EnableControlActionReportingForDomain(EsifUpDomainPtr domainPtr)
{
	UInt32 capMask = EsifUp_GetDomainCapabilityMask(domainPtr);
	EsifData capData = { ESIF_DATA_UINT32, &capMask, sizeof(capMask), sizeof(capMask) };

	EsifEventMgr_SignalEvent(domainPtr->participantId,
		domainPtr->domain,
		ESIF_EVENT_DPTF_PARTICIPANT_ACTIVITY_LOGGING_ENABLED,
		&capData
	);
}

static Bool ShouldEnableControlActionReportingForDomain(EsifUpDomainPtr domainPtr)
{
	Bool rc = ESIF_FALSE;

	ESIF_ASSERT(domainPtr != NULL);

	switch (domainPtr->domainType) {
	case ESIF_DOMAIN_TYPE_PROCESSOR:
	case ESIF_DOMAIN_TYPE_GRAPHICS:
	case ESIF_DOMAIN_TYPE_FAN:
	case ESIF_DOMAIN_TYPE_WIRELESS:
	case ESIF_DOMAIN_TYPE_MULTIFUNCTION:
	case ESIF_DOMAIN_TYPE_DISPLAY:
	case ESIF_DOMAIN_TYPE_BATTERYCHARGER:
	case ESIF_DOMAIN_TYPE_WWAN:
	case ESIF_DOMAIN_TYPE_POWER:
	case ESIF_DOMAIN_TYPE_CHIPSET:
		rc = ESIF_TRUE;
		break;
	default:
		break;
	}

	return rc;
}

static void EnableControlActionReporting(EsifUpPtr upPtr)
{
	UpDomainIterator udIter = { 0 };
	EsifUpDomainPtr domainPtr = NULL;
	eEsifError iterRc = ESIF_OK;

	ESIF_ASSERT(upPtr != NULL);
	
	iterRc = EsifUpDomain_InitIterator(&udIter, upPtr);
	if (ESIF_OK != iterRc) goto exit;

	iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
	while (ESIF_OK == iterRc) {
		// Found a valid domain
		if (ShouldEnableControlActionReportingForDomain(domainPtr)) {
			EnableControlActionReportingForDomain(domainPtr);
		}

		iterRc = EsifUpDomain_GetNextUd(&udIter, &domainPtr);
	}

	if (ESIF_E_ITERATION_DONE != iterRc) {
		EsifUp_PutRef(upPtr);
	}

exit:
	return;
}

/**
 * PUBLIC
 */

eEsifError EsifEventBroadcast_ControlActionEnableParticipant(EsifUpPtr upPtr)
{
	eEsifError rc = ESIF_OK;

	if (NULL == upPtr) {
		ESIF_TRACE_ERROR("Invalid Upper Participant");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Register for the DPTF Participant Control Action Callback
	// All participants are going to share a single callback
	EsifEventMgr_RegisterEventByType(ESIF_EVENT_DPTF_PARTICIPANT_CONTROL_ACTION,
		EVENT_MGR_MATCH_ANY,
		EVENT_MGR_MATCH_ANY_DOMAIN,
		ControlActionCallback,
		0
	);
		
	EnableControlActionReporting(upPtr);
exit:
	return rc;
}

void EsifEventBroadcast_PolicyLoggingEnable(Bool flag)
{
	atomic_t localCount = ATOMIC_INIT(0);
	if (ESIF_TRUE == flag) {

		atomic_inc(&g_policyLoggingRefCount);

		EsifEventMgr_RegisterEventByType(ESIF_EVENT_DPTF_POLICY_LOADED_UNLOADED, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, PolicyLoggingCallback, 0);
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_DPTF_POLICY_ACTIVITY_LOGGING_ENABLED, 0);
	}
	else
	{
		localCount = atomic_dec(&g_policyLoggingRefCount);
		if (localCount <= 0) {
			EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_DPTF_POLICY_ACTIVITY_LOGGING_DISABLED, 0);
		}
		EsifEventMgr_UnregisterEventByType(ESIF_EVENT_DPTF_POLICY_LOADED_UNLOADED, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, PolicyLoggingCallback, 0);
	}
}

void EsifEventBroadcast_MotionSensorEnable(Bool flag)
{
	if (ESIF_TRUE == flag) {
		EsifEventMgr_RegisterEventByType(ESIF_EVENT_DISPLAY_ORIENTATION_CHANGED, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, MotionSensorEventCallback, 0);
		EsifEventMgr_RegisterEventByType(ESIF_EVENT_DEVICE_ORIENTATION_CHANGED, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, MotionSensorEventCallback, 0);
		EsifEventMgr_RegisterEventByType(ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, MotionSensorEventCallback, 0);
		EsifEventMgr_RegisterEventByType(ESIF_EVENT_MOTION_CHANGED, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, MotionSensorEventCallback, 0);
	}
	else
	{
		EsifEventMgr_UnregisterEventByType(ESIF_EVENT_DISPLAY_ORIENTATION_CHANGED, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, MotionSensorEventCallback, 0);
		EsifEventMgr_UnregisterEventByType(ESIF_EVENT_DEVICE_ORIENTATION_CHANGED, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, MotionSensorEventCallback, 0);
		EsifEventMgr_UnregisterEventByType(ESIF_EVENT_OS_PLATFORM_TYPE_CHANGED, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, MotionSensorEventCallback, 0);
		EsifEventMgr_UnregisterEventByType(ESIF_EVENT_MOTION_CHANGED, EVENT_MGR_MATCH_ANY, EVENT_MGR_MATCH_ANY_DOMAIN, MotionSensorEventCallback, 0);
	}
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
