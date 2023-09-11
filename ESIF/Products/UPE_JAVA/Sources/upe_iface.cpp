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

/*
 * TODO: The following must be declared prior to SDK include files or defined
 * in the project pre-processor macros.  The appropriate OS-type should be
 * selected.
 */
// #define ESIF_ATTR_USER
// #define ESIF_ATTR_OS_WINDOWS
// #define ESIF_ATTR_OS_LINUX

#include "upe.h"
#include "esif_sdk_iface_upe.h"
#include "esif_sdk_iface_participant.h"
#include "esif_sdk_action_type.h"
#include "esif_ccb_string.h"
#include "esif_ccb_memory.h"
#include "jhs_binder_service.h"


using namespace jhs;

extern "C" {
/*
 * GLOBAL DECLARATIONS
 */
ActSendEventFunction g_sendEventFuncPtr = NULL;
ActExecutePrimitiveFunction g_execPrimFuncPtr = NULL;
ActEventRegistrationFunction g_eventRegistrationFuncPtr = NULL;

ActWriteLogFunction g_esifLogFuncPtr = NULL;
eLogType g_upeTraceLevel = ACTION_UPE_TRACE_LEVEL_DEFAULT;

/*
 * PRIVATE FUNCTION PROTOTYPES
 */
static eEsifError ESIF_CALLCONV ActionCreate(
	const EsifActIfacePtr actIfacePtr,
	esif_context_t *actCtxPtr
	);

static eEsifError ESIF_CALLCONV ActionDestroy(
	esif_context_t actCtx
	);

static eEsifError ESIF_CALLCONV ActionGet(
	esif_context_t actCtx,
	const esif_handle_t participantId,
	const esif_string devicePathPtr,
	const EsifDataPtr p1Ptr,
	const EsifDataPtr p2Ptr,
	const EsifDataPtr p3Ptr,
	const EsifDataPtr p4Ptr,
	const EsifDataPtr p5Ptr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

static eEsifError ESIF_CALLCONV ActionSet(
	esif_context_t actCtx,
	const esif_handle_t participantId,
	const esif_string devicePathPtr,
	const EsifDataPtr p1Ptr,
	const EsifDataPtr p2Ptr,
	const EsifDataPtr p3Ptr,
	const EsifDataPtr p4Ptr,
	const EsifDataPtr p5Ptr,
	const EsifDataPtr requestPtr
	);

static eEsifError ESIF_CALLCONV ActionReceiveEvent(
	esif_context_t actCtx,
	const esif_handle_t participantId,
	enum esif_event_type type,
	UInt16 domain,
	const EsifDataPtr dataPtr
	);

static eEsifError ActionGetPpss(
	EsifDataPtr responsePtr
	);

static sp<IJhsService> gJhs = NULL;

static void getJhsService(void) {
 	sp<IServiceManager> sm = defaultServiceManager();
	if (sm != NULL) {
		sp<IBinder> binder = sm->getService(String16(JHS_SERVICE_NAME));
		gJhs = interface_cast<IJhsService>(binder);
	}
}

/*
 * FUNCTION DEFINITIONS
 */

/*
 * TODO:  This function must be exported; it is the "known function" that ESIF
 * uses to obtain the UPE interface.
 */
ESIF_EXPORT eEsifError  GetActionInterface(
	EsifActIfacePtr actIfacePtr /* For UPE's, this can be cast to a EsifUpeIfacePtr */
	)
{
	eEsifError rc = ESIF_OK;
	EsifActIfaceUpeV1Ptr upeIfacePtr = NULL;

	UPE_TRACE_INFO("UPE_JAVA: Entry");

	if (NULL == actIfacePtr) {
		UPE_TRACE_ERROR("UPE_JAVA: Action interface pointer is NULL");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	upeIfacePtr = (EsifActIfaceUpeV1Ptr)actIfacePtr;

	if ((upeIfacePtr->hdr.fIfaceType != eIfaceTypeAction) ||
		(upeIfacePtr->hdr.fIfaceSize < sizeof(*upeIfacePtr))) {
		UPE_TRACE_ERROR("UPE_JAVA: Interface not supported: Type = %d, size = %d; Req %d:%d",
			upeIfacePtr->hdr.fIfaceType,
			upeIfacePtr->hdr.fIfaceSize,
			eIfaceTypeAction,
			(UInt32)sizeof(*upeIfacePtr));
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	upeIfacePtr->hdr.fIfaceVersion = ACTION_UPE_IFACE_VERSION;
	upeIfacePtr->hdr.fIfaceSize    = (UInt16)sizeof(*upeIfacePtr);

	/* TODO: Specify the "Action Type" of the UPE */
	upeIfacePtr->type = ACTION_UPE_ACTION_TYPE;

	upeIfacePtr->actVersion = ACTION_UPE_VERSION;

	esif_ccb_strcpy(upeIfacePtr->name, ACTION_UPE_NAME, sizeof(upeIfacePtr->name));
	esif_ccb_strcpy(upeIfacePtr->desc, ACTION_UPE_DESC, sizeof(upeIfacePtr->desc));

	upeIfacePtr->createFuncPtr  = ActionCreate;
	upeIfacePtr->destroyFuncPtr = ActionDestroy;

	upeIfacePtr->getFuncPtr     = ActionGet;
	upeIfacePtr->setFuncPtr     = ActionSet;

	upeIfacePtr->rcvEventFuncPtr = ActionReceiveEvent;

	UPE_TRACE_INFO("UPE_JAVA: \n"
		"UPE INTERFACE\n"
		"  Version: %d\n"
		"  Size: %d\n"
		"  Type: %d\n"
		"  Name: %s\n"
		"  Desc: %s\n"
		"  Action version: %d\n",
		upeIfacePtr->hdr.fIfaceVersion,
		upeIfacePtr->hdr.fIfaceSize,
		upeIfacePtr->type,
		upeIfacePtr->name,
		upeIfacePtr->desc,
		upeIfacePtr->actVersion);
exit:
	UPE_TRACE_INFO("UPE_JAVA: Exit status = %s(%d)", esif_rc_str(rc), rc);
	return rc;
}


/*
 * Creates the action.  Called after the interface has been retrieved and allows
 * the action to receive interface functions from ESIF and return an
 * action-specific context.
 */
static eEsifError ESIF_CALLCONV ActionCreate(
	/* For UPE's, this can be cast to a EsifUpeIfacePtr */
	const EsifActIfacePtr actIfacePtr,

	/*
	 * To be returned by the action during this function and will be passed back
	 * in calls to the action
	 */
	esif_context_t *actCtxPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifActIfaceUpeV1Ptr upeIfacePtr = NULL;

	UPE_TRACE_INFO("UPE_JAVA: Entry");
	getJhsService();
	if (gJhs == NULL) {
		UPE_TRACE_ERROR("UPE_JAVA: Cannot locate JHS\n");
	}

	if (NULL == actIfacePtr) {
		UPE_TRACE_ERROR("UPE_JAVA: Action interface pointer is NULL");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	upeIfacePtr = (EsifActIfaceUpeV1Ptr)actIfacePtr;

	if ((upeIfacePtr->hdr.fIfaceType != eIfaceTypeAction) ||
		(upeIfacePtr->hdr.fIfaceSize < sizeof(*upeIfacePtr)) ||
		(upeIfacePtr->hdr.fIfaceVersion != ACTION_UPE_IFACE_VERSION)) {
		UPE_TRACE_ERROR("UPE_JAVA: Interface not supported: Type = %d, size = %d, ver = %d; Req %d:%d:%d",
			upeIfacePtr->hdr.fIfaceType,
			upeIfacePtr->hdr.fIfaceSize,
			upeIfacePtr->hdr.fIfaceVersion,
			eIfaceTypeAction,
			(UInt32)sizeof(*upeIfacePtr),
			ACTION_UPE_IFACE_VERSION);
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	if (NULL == actCtxPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/*
	 * Save a copy of the function pointer for logging trace messages through
	 * ESIF and the current trace level
	 */
	UPE_TRACE_INFO("UPE_JAVA: Changing trace level to %d\n", upeIfacePtr->traceLevel);

	g_upeTraceLevel = upeIfacePtr->traceLevel;

	UPE_TRACE_INFO("UPE_JAVA: Changed trace level to %d\n", g_upeTraceLevel);

	g_esifLogFuncPtr = upeIfacePtr->writeLogFuncPtr;

	/*
	 * Save a copy of the function pointers to access ESIF
	 */
	if ((NULL == upeIfacePtr->sendEventFuncPtr) ||
		(NULL == upeIfacePtr->execPrimitiveFuncPtr)) {
		UPE_TRACE_ERROR("UPE_JAVA: ESIF access pointer is NULL");
		rc = ESIF_E_CALLBACK_IS_NULL;
		goto exit;
	}

	g_sendEventFuncPtr = upeIfacePtr->sendEventFuncPtr;
	g_execPrimFuncPtr = upeIfacePtr->execPrimitiveFuncPtr;
	g_eventRegistrationFuncPtr = upeIfacePtr->eventRegistrationFuncPtr;


	/* TODO:  Return the context required, if any */
	*actCtxPtr = 0;

	ActionRegisterEvent(ACTION_UPE_MATCH_ANY, ACTION_UPE_DOMAIN_D0, ESIF_EVENT_LOG_VERBOSITY_CHANGED);

	UPE_TRACE_INFO("UPE_JAVA: \n"
		"CREATED UPE ACTION\n"
		"  UPE Ctx: %p\n",
		(void *)(size_t)*actCtxPtr);

exit:
	UPE_TRACE_INFO("UPE_JAVA: Exit status = %s(%d)", esif_rc_str(rc), rc);
	return rc;
}


/*
 * Called to destroy a given instance of the action; as specified by the
 * action context that was received when calling the ActCreateFunction function.
 */
static eEsifError ESIF_CALLCONV ActionDestroy(
	esif_context_t actCtx
	)
{
	eEsifError rc = ESIF_OK;

	UNREFERENCED_PARAMETER(actCtx);

	UPE_TRACE_INFO("UPE_JAVA: Entry");

/*
 * TODO: Perform any needed cleanup
 * NOTE:  All threads should be in a known state prior to this point so that
 * there are no race conditions during access to the global items as they are
 * NULL'ed.
 */
	ActionUnregisterEvent(ACTION_UPE_MATCH_ANY, ACTION_UPE_DOMAIN_D0, ESIF_EVENT_LOG_VERBOSITY_CHANGED);

	g_esifLogFuncPtr = NULL;
	g_sendEventFuncPtr = NULL;
	g_execPrimFuncPtr = NULL;

	UPE_TRACE_INFO("UPE_JAVA: Exit status = %s(%d)", esif_rc_str(rc), rc);
	return rc;
}


/*
 * Used to implement the "GET" functionality allowing ESIF to retrieve
 * information from the action.
 */
static eEsifError ESIF_CALLCONV ActionGet(
	esif_context_t actCtx,
	const esif_handle_t participantId,
	const esif_string devicePathPtr,
	const EsifDataPtr p1Ptr,
	const EsifDataPtr p2Ptr,
	const EsifDataPtr p3Ptr,
	const EsifDataPtr p4Ptr,
	const EsifDataPtr p5Ptr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	ActionParameters action = {0};
	JhsReplyHeader replyHeader = {0};
	JhsParticipantHandle handle = { 0 };
	handle.mParticipantId = participantId; // Convert esif_handle_t to JHS Participant Handle

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(p4Ptr);
	UNREFERENCED_PARAMETER(p5Ptr);
	UNREFERENCED_PARAMETER(requestPtr);

	if (gJhs == NULL) {
		// Try again and see if JHS is now up
		UPE_TRACE_ERROR("UPE_JAVA: Retry finding JHS\n");
		getJhsService();
		if (gJhs == NULL) {
			UPE_TRACE_ERROR("UPE_JAVA: Cannot locate JHS, abort ActionGet\n");
			rc = ESIF_E_ACTION_NOT_IMPLEMENTED;
			goto exit;
		}
	}

	if (devicePathPtr) {
		// To do: add code that uses devicePathPtr
	}

	if ((NULL == responsePtr) || (NULL == responsePtr->buf_ptr)  ||
	    (NULL == p1Ptr) || (NULL == p1Ptr->buf_ptr)) {
		UPE_TRACE_ERROR("UPE_JAVA: Required input parameter is NULL");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Check the return buffer size up front; additional check for non-intger types must be added as required */
	if (responsePtr->buf_len < (u32) esif_data_type_sizeof(responsePtr->type)) {
		UPE_TRACE_ERROR("UPE_JAVA: Response buffer size/type mismatch: Len = %d, Type = %s",
			responsePtr->buf_len,
			esif_data_type_str(responsePtr->type));
		rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		goto exit;
	}

	/*
	 * P1 will contain the main GET function as a 32-bit character constant
	 * (ESIF_DATA_STRING type where the string is always 4bytes long)
	 */
	if (p1Ptr->buf_len < sizeof(UInt32)) {
		UPE_TRACE_ERROR("UPE_JAVA: P1 buffer too small: %d", p1Ptr->buf_len);
		rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		goto exit;
	}

	UPE_TRACE_DEBUG("UPE_JAVA: Executing primitive for Participant ID: " ESIF_HANDLE_FMT, esif_ccb_handle2llu(participantId));

	action.mActionType = *((UInt32*) p1Ptr->buf_ptr);
	action.mDomainId = *((UInt32 *) p2Ptr->buf_ptr);
	action.mInstanceId = *((UInt32 *) p3Ptr->buf_ptr);
	action.mReplyDataType = responsePtr->type;
	action.mReplyBufferSize = responsePtr->buf_len;
	UPE_TRACE_DEBUG("UPE_JAVA: Execute Java Get Action: mActionType: 0x%x, mDomainId = %d, mInstanceId = %d, mReplyBufferSize = %d",
		action.mActionType, action.mDomainId, action.mInstanceId, action.mReplyBufferSize);

	rc = (eEsifError) gJhs->getValue(handle, action, &replyHeader, (JhsReplyPayload) responsePtr->buf_ptr);
	responsePtr->data_len = replyHeader.mActualDataSize;

	UPE_TRACE_DEBUG("UPE_JAVA: Response length: %d", responsePtr->data_len);
exit:
	UPE_TRACE_DEBUG("UPE_JAVA: Exit status = %s(%d)", esif_rc_str(rc), rc);
	return rc;
}


/*
 * Used to implement the "SET" functionality allowing ESIF to provide
 * information to the action.
 */
static eEsifError ESIF_CALLCONV ActionSet(
	esif_context_t actCtx,
	const esif_handle_t participantId,
	const esif_string devicePathPtr,
	const EsifDataPtr p1Ptr,
	const EsifDataPtr p2Ptr,
	const EsifDataPtr p3Ptr,
	const EsifDataPtr p4Ptr,
	const EsifDataPtr p5Ptr,
	const EsifDataPtr requestPtr
	)
{
	eEsifError rc = ESIF_OK;
	ActionParameters action = {0};
	int32_t value = 0;
	JhsParticipantHandle handle = { 0 };
	handle.mParticipantId = participantId; // Convert esif_handle_t to JHS Participant Handle

	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(p4Ptr);
	UNREFERENCED_PARAMETER(p5Ptr);

	if (gJhs == NULL) {
		// Try again and see if JHS is now up
		UPE_TRACE_ERROR("UPE_JAVA: Retry finding JHS\n");
		getJhsService();
		if (gJhs == NULL) {
			UPE_TRACE_ERROR("UPE_JAVA: Cannot locate JHS, abort ActionSet\n");
			rc = ESIF_E_ACTION_NOT_IMPLEMENTED;
			goto exit;
		}
	}

	if (devicePathPtr) {
		// To do: add code that uses devicePathPtr
	}

	if ((NULL == requestPtr) || (NULL == requestPtr->buf_ptr) ||
	    (NULL == p1Ptr) || (NULL == p1Ptr->buf_ptr)) {
		UPE_TRACE_ERROR("UPE_JAVA: Required input parameter is NULL");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/*
	 * Check the request buffer size against the specified type.  This allows
	 * for simplification of parameter validation of integer data types.  (Only
	 * the type needs to be verified later for integer types.)  Additional
	 * checks are required for binary types.
	 */
	if (requestPtr->buf_len < (u32) esif_data_type_sizeof(requestPtr->type)) {
		UPE_TRACE_ERROR("UPE_JAVA: Request buffer size/type mismatch: Len = %d, Type = %s",
			requestPtr->buf_len,
			esif_data_type_str(requestPtr->type));
		rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		goto exit;
	}

	/*
	 * P1 will contain the main GET opcode as a 32-bit character constant
	 * (ESIF_DATA_STRING type where the string is always 4 bytes long.)
	 */
	if (p1Ptr->buf_len < sizeof(UInt32)) {
		UPE_TRACE_ERROR("UPE_JAVA: P1 buffer too small: %d", p1Ptr->buf_len);
		rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		goto exit;
	}

	UPE_TRACE_DEBUG("UPE_JAVA: Executing primitive: Participant ID: " ESIF_HANDLE_FMT, esif_ccb_handle2llu(participantId));

	action.mActionType = *((UInt32*) p1Ptr->buf_ptr);
	if (p2Ptr->buf_ptr) {
		action.mDomainId = *((UInt32 *) p2Ptr->buf_ptr);
	} else {
		action.mDomainId = 0;
	}
	if (p3Ptr->buf_ptr) {
		action.mInstanceId = *((UInt32 *) p3Ptr->buf_ptr);
	} else {
		action.mInstanceId = ESIF_INSTANCE_INVALID;
	}
	action.mReplyDataType = 0; // Do not care
	action.mReplyBufferSize = 0; // Do not care
	value = *((Int32 *) requestPtr->buf_ptr);
	UPE_TRACE_DEBUG("UPE_JAVA: Execute Java Set Action: mActionType: 0x%x, mDomainId = %d, mInstanceId = %d, value = %d",
		action.mActionType, action.mDomainId, action.mInstanceId, value);

	rc = (eEsifError) gJhs->setValue(handle, action, value);

exit:
	UPE_TRACE_DEBUG("UPE_JAVA: Exit status = %s(%d)", esif_rc_str(rc), rc);
	return rc;
}


/*
 * Used to received events from ESIF.  The event types that are received are
 * dependent upon the UPE type.  (ESIF events are asynchronous and there is no
 * guarantee of what time in the past a given even occured.  For example, an S3
 * entry event may not be received until after reseume from S3.)
 */
static eEsifError ESIF_CALLCONV ActionReceiveEvent(
	esif_context_t actCtx,			/* Context provided by the UPE during action creation */
	const esif_handle_t participantId,
	enum esif_event_type type,		/* Event type */
	UInt16 domain, 					/* Domain for the event */
	const EsifDataPtr dataPtr	/* Event data if required (may be NULL) */
	)
{
	eEsifError rc = ESIF_OK;

	UNREFERENCED_PARAMETER(participantId);
	UNREFERENCED_PARAMETER(actCtx);
	UNREFERENCED_PARAMETER(domain); /* Ignored in initial implementation - Always D0 */

	/*
	 * Check the request buffer size against the specified type.  This allows
	 * for simplification of parameter validation of integer data types.  (Only
	 * the type needs to be verified later for integer types.)  Additional
	 * checks are required for binary types.
	 */
	if (dataPtr != NULL) {
		if (dataPtr->buf_len < (u32) esif_data_type_sizeof(dataPtr->type)) {
			UPE_TRACE_ERROR("UPE_JAVA: Data buffer size/type mismatch: Len = %d, Type = %s",
				dataPtr->buf_len,
				esif_data_type_str(dataPtr->type));
			rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
			goto exit;
		}
	}

	/*
	 * TODO: Implement code to receive events as needed.  The code provided here
	 * is for testing only an must be replaced.
	 */
	UPE_TRACE_DEBUG("UPE_JAVA: Received %s event", esif_event_type_str(type));

	switch(type) {
	case ESIF_EVENT_LOG_VERBOSITY_CHANGED:
		if ((NULL == dataPtr) || (NULL == dataPtr->buf_ptr)) {
			UPE_TRACE_ERROR("UPE_JAVA: Required input parameter is NULL");
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}
		if (dataPtr->buf_len < sizeof(eLogType)) {
			UPE_TRACE_ERROR("UPE_JAVA: Data buffer too small: Len = %d",
				dataPtr->buf_len);
			rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
			goto exit;
		}
		g_upeTraceLevel = *((eLogType *)dataPtr->buf_ptr);
		UPE_TRACE_INFO("UPE_JAVA: Trace level changed to %d\n", g_upeTraceLevel);
		break;

	default:
		rc = ESIF_E_NOT_SUPPORTED;
		break;
	}
exit:
	UPE_TRACE_DEBUG("UPE_JAVA: Exit status = %s(%d)", esif_rc_str(rc), rc);
	return rc;
}


eEsifError ActionSendEvent(
	const esif_handle_t participantId,
	enum esif_event_type eventType,
	const EsifDataPtr dataPtr
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == g_sendEventFuncPtr) {
		UPE_TRACE_WARN("UPE_JAVA: Send event pointer not available");
		rc = ESIF_E_IFACE_DISABLED;
		goto exit;
	}

	UPE_TRACE_DEBUG("UPE_JAVA: Sending %s event to ESIF", esif_event_type_str(eventType));

	rc = (*g_sendEventFuncPtr)(participantId, eventType, ACTION_UPE_DOMAIN_D0, dataPtr);
exit:
	UPE_TRACE_DEBUG("UPE_JAVA: Exit status = %s(%d)", esif_rc_str(rc), rc);
	return rc;
}


eEsifError ActionExecutePrimitive(
	const esif_handle_t participantId,
	const UInt16 primitiveId,
	const UInt8 instance,					/* Primitive instance */
	const EsifDataPtr requestPtr,			/* Input data to the primitive */
	EsifDataPtr responsePtr					/* Output data returned by primitivie execution */
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == g_execPrimFuncPtr) {
		UPE_TRACE_ERROR("UPE_JAVA: Primitive execution function pointer not initialized");
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	UPE_TRACE_DEBUG("UPE_JAVA: Executing primitive: PartID " ESIF_HANDLE_FMT ", PrimID %d, Instance", esif_ccb_handle2llu(participantId), primitiveId, instance);

	rc = (*g_execPrimFuncPtr)(participantId, primitiveId, ACTION_UPE_DOMAIN_D0, instance, requestPtr, responsePtr);
exit:
	UPE_TRACE_DEBUG("UPE_JAVA: Exit status = %s(%d)", esif_rc_str(rc), rc);
	return rc;
}


eEsifError ActionRegisterEvent(
	const esif_handle_t participantHandle,
	const UInt16 domain,
	const enum esif_event_type eventType
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == g_eventRegistrationFuncPtr) {
		UPE_TRACE_ERROR("Event registration function pointer not initialized");
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}
	rc = (*g_eventRegistrationFuncPtr)(participantHandle, domain, ACTION_UPE_ACTION_TYPE, eventType, UPE_REGISTER_EVENT);
exit:
	return rc;
}


eEsifError ActionUnregisterEvent(
	const esif_handle_t participantHandle,
	const UInt16 domain,
	const enum esif_event_type eventType
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == g_eventRegistrationFuncPtr) {
		UPE_TRACE_ERROR("Trying to unregister but event registration function pointer not initialized");
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	rc = (*g_eventRegistrationFuncPtr)(participantHandle, domain, ACTION_UPE_ACTION_TYPE, eventType, UPE_UNREGISTER_EVENT);
exit:
	return rc;
}

} // extern "C"
