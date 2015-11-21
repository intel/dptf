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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_ACTION

#include "esif_uf.h"			/* Upper Framework */
#include "esif_sdk_iface_upe.h"
#include "esif_sdk_iface_esif.h"	/* ESIF Services Interface */
#include "esif_uf_actmgr.h"		/* Action Manager */
#include "esif_uf_primitive.h"
#include "esif_uf_action.h"
#include "esif_uf_service.h"
#include "esif_uf_eventmgr.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/*
 * PUBLIC FRIEND FUNCTIONS
 */
eEsifError EsifAct_CreateAction(
	EsifActIfacePtr actIfacePtr,
	UInt8 upInstance,
	EsifActPtr *actPtr
	);

void EsifAct_DestroyAction(EsifActPtr actPtr);

eEsifError EsifActIface_GetType(
	EsifActIfacePtr self,
	enum esif_action_type *typePtr
	);
	
void EsifAct_MarkAsPlugin(EsifActPtr self);


/*
 * PRIVATE FUNCTION PROTOTYPES
 */
UInt16 EsifActIface_Sizeof(EsifActIfaceVer fIfaceVersion);
static UInt16 EsifActIface_GetVersion(EsifActIfacePtr self);
static EsifString EsifActIface_GetDesc(EsifActIfacePtr self);
static EsifString EsifActIface_GetName(EsifActIfacePtr self);
static eEsifError EsifAct_CallIfaceDestroy(EsifActPtr self);
static eEsifError EsifAct_CallIfaceCreate(EsifActPtr self);
static eEsifError EsifAct_RegisterEvents(EsifActPtr self);
static void EsifAct_UnregisterEvents(EsifActPtr self);

static eEsifError EsifActIface_RegisterEvents(
	EsifActIfacePtr self,
	esif_handle_t esifInstHandle
	);

static void EsifActIface_UnregisterEvents(
	EsifActIfacePtr self,
	esif_handle_t esifInstHandle
	);
	
static eEsifError ESIF_CALLCONV EsifActWriteLogHandler(
	const EsifDataPtr message,
	const eLogType logType
	);

static eEsifError ESIF_CALLCONV EsifAct_EventCallback(
	void *contextPtr,
	UInt8 upInstance,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);

static eEsifError EsifActIface_SendIfaceEvent(
	EsifActIfacePtr ifacePtr,
	esif_context_t actCtx,
	enum esif_event_type eventType,
	UInt16 domainId,
	EsifDataPtr eventDataPtr
	);
	
static eEsifError ESIF_CALLCONV EsifActIface_ReceiveEventV1(
	const esif_handle_t esifInstHandle,
	enum esif_event_type eventType,
	UInt16 domain,
	const EsifDataPtr dataPtr
	);


/*
 * FUNCTION DEFINITIONS
 */
eEsifError EsifAct_CreateAction(
	EsifActIfacePtr actIfacePtr,
	UInt8 upInstance,
	EsifActPtr *actPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifActPtr newActPtr = NULL;
	enum esif_action_type actionType;
	UInt16 intfcSize = 0;

	if ((NULL == actIfacePtr) || (NULL == actPtr)){
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	intfcSize = EsifActIface_Sizeof(actIfacePtr->hdr.fIfaceVersion);
	if (intfcSize != actIfacePtr->hdr.fIfaceSize) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	newActPtr = esif_ccb_malloc(sizeof(*newActPtr));
	if (NULL == newActPtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	esif_ccb_memcpy(&newActPtr->iface, actIfacePtr, intfcSize);

	rc = EsifActIface_GetType(actIfacePtr, &actionType);
	if (rc != ESIF_OK) {
		goto exit;
	}
	newActPtr->type = actionType;
	newActPtr->upInstace = upInstance;

	newActPtr->refCount = 1;
	newActPtr->markedForDelete = ESIF_FALSE;

	esif_ccb_lock_init(&newActPtr->objLock);
	esif_ccb_event_init(&newActPtr->deleteEvent);

	rc = EsifAct_CallIfaceCreate(newActPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	rc = EsifAct_RegisterEvents(newActPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	ESIF_TRACE_DEBUG("\n"
		"Created new action:"
		"  Name   : %s\n"
		"  Desc   : %s\n"
		"  Type   : %d\n"
		"  Version: %u\n",
		EsifAct_GetName(newActPtr),
		EsifAct_GetDesc(newActPtr),
		EsifAct_GetType(newActPtr),
		EsifAct_GetVersion(newActPtr));

	*actPtr = newActPtr;
exit:
	if (rc != ESIF_OK) {
		EsifAct_DestroyAction(newActPtr);
	}
	return rc;
}


void EsifAct_DestroyAction(
	EsifActPtr self
	)
{
	if (NULL == self) {
		goto exit;
	}

	self->markedForDelete = ESIF_TRUE;
	EsifAct_PutRef(self);

	ESIF_TRACE_INFO("Destroy action %d : waiting for delete event...\n", self->type);
	esif_ccb_event_wait(&self->deleteEvent);

	EsifAct_UnregisterEvents(self);

	EsifAct_CallIfaceDestroy(self);

	esif_ccb_event_uninit(&self->deleteEvent);
	esif_ccb_lock_uninit(&self->objLock);

	esif_ccb_free(self);
exit:
	return;
}


static eEsifError EsifAct_CallIfaceCreate(
	EsifActPtr self
	)
{
	eEsifError rc = ESIF_OK;
	ActionHandle actHandle = {0}; 

	ESIF_ASSERT(self != NULL);
		
	switch (self->iface.hdr.fIfaceVersion) {
	case ESIF_ACT_IFACE_VER_STATIC:
		if (self->iface.ifaceStatic.createFuncPtr != NULL) {
			rc = self->iface.ifaceStatic.createFuncPtr(&self->iface,
				NULL,
				(esif_handle_t)(size_t)self->type,
				&self->actCtx);
			self->createCalled = ESIF_TRUE;
		}
		break;
	case ESIF_ACT_IFACE_VER_V1:
		if (self->iface.actIfaceV1.createFuncPtr != NULL) {

			self->iface.actIfaceV1.traceLevel = g_traceLevel;
			self->iface.actIfaceV1.writeLogFuncPtr = EsifActWriteLogHandler;
			self->iface.actIfaceV1.sendEventFuncPtr = EsifActIface_ReceiveEventV1;

			actHandle.s.type = self->type;
			actHandle.s.upInstance = self->upInstace;

			rc = self->iface.actIfaceV1.createFuncPtr(&self->iface,
				NULL,
				(esif_handle_t)(size_t)actHandle.asDword,
				&self->actCtx);		
			self->createCalled = ESIF_TRUE;
		}
		break;
	default:
		break;
	}
exit:
	if (ESIF_OK != rc) {
		ESIF_TRACE_ERROR("Error creating action. Error Code : %x", rc);
		goto exit;
	}	
	return rc;
}


static eEsifError EsifAct_CallIfaceDestroy(EsifActPtr self)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	if (self->createCalled) {
		switch (self->iface.hdr.fIfaceVersion) {
		case ESIF_ACT_IFACE_VER_STATIC:
			if (self->iface.ifaceStatic.destroyFuncPtr != NULL) {
				rc = self->iface.ifaceStatic.destroyFuncPtr(self->actCtx);
			}
			break;
		case ESIF_ACT_IFACE_VER_V1:
			if (self->iface.actIfaceV1.destroyFuncPtr != NULL) {
				rc = self->iface.actIfaceV1.destroyFuncPtr(self->actCtx);		
			}
			break;
		default:
			break;
		}
		if (ESIF_OK != rc) {
			ESIF_TRACE_ERROR("Error destroying action. Error Code : %x", rc);
		}	
	}
	return rc;
}


/*
 * Takes an additional reference on an action object.  (The function is
 * called for you by the Action Manager when one of the functions are
 * called which returns a pointer to an action.)  After using the
 * action, EsifAct_PutRef must be called to release the reference.
 */
eEsifError EsifAct_GetRef(EsifActPtr self)
{
	eEsifError rc = ESIF_OK;

	if (self == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_write_lock(&self->objLock);

	if (self->markedForDelete == ESIF_TRUE) {
		esif_ccb_write_unlock(&self->objLock);
		ESIF_TRACE_DEBUG("Action marked for delete\n");
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	self->refCount++;
	ESIF_TRACE_DEBUG("ref = %d\n", self->refCount);
	esif_ccb_write_unlock(&self->objLock);
exit:
	return rc;
}

/*
 * Releases a reference on an action object.  This function should be
 * called when done using an action pointer obtained through any of the
 * Action Manager interfaces.
 */
void EsifAct_PutRef(EsifActPtr self)
{
	UInt8 needRelease = ESIF_FALSE;

	if (self != NULL) {
		esif_ccb_write_lock(&self->objLock);

		self->refCount--;

		ESIF_TRACE_DEBUG("ref = %d\n", self->refCount);

		if ((self->refCount == 0) && (self->markedForDelete)) {
			needRelease = ESIF_TRUE;
		}

		esif_ccb_write_unlock(&self->objLock);

		if (needRelease == ESIF_TRUE) {
			ESIF_TRACE_DEBUG("Signal delete event\n");
			esif_ccb_event_set(&self->deleteEvent);
		}
	}
}


static eEsifError EsifAct_RegisterEvents(
	EsifActPtr self
	)
{
	ESIF_ASSERT(self != NULL);

	EsifActIface_RegisterEvents(&self->iface, (esif_handle_t)(size_t)self->type);

	return ESIF_OK;
}


static eEsifError EsifActIface_RegisterEvents(
	EsifActIfacePtr self,
	esif_handle_t esifInstHandle
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	switch (self->hdr.fIfaceVersion) {
	case ESIF_ACT_IFACE_VER_V1:
		if (NULL == self->actIfaceV1.rcvEventFuncPtr) {
			rc = ESIF_E_NOT_SUPPORTED;
			goto exit;
		}
		EsifEventMgr_RegisterEventByType(ESIF_EVENT_LOG_VERBOSITY_CHANGED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAct_EventCallback, (void *)(size_t)esifInstHandle);
		break;
	case ESIF_ACT_IFACE_VER_STATIC:
	default:
		break;
	}
exit:
	return rc;
}


static void EsifAct_UnregisterEvents(
	EsifActPtr self
	)
{
	ESIF_ASSERT(self != NULL);

	EsifActIface_UnregisterEvents(&self->iface, (esif_handle_t)(size_t)self->type);

	return;
}


static void EsifActIface_UnregisterEvents(
	EsifActIfacePtr self,
	esif_handle_t esifInstHandle
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	switch (self->hdr.fIfaceVersion) {
	case ESIF_ACT_IFACE_VER_V1:
		if (NULL == self->actIfaceV1.rcvEventFuncPtr) {
			rc = ESIF_E_NOT_SUPPORTED;
			goto exit;
		}
		EsifEventMgr_UnregisterEventByType(ESIF_EVENT_LOG_VERBOSITY_CHANGED, EVENT_MGR_MATCH_ANY, EVENT_MGR_DOMAIN_D0, EsifAct_EventCallback, (void *)(size_t)esifInstHandle);
		break;
	case ESIF_ACT_IFACE_VER_STATIC:
	default:
		break;
	}
exit:
	return;
}


static eEsifError ESIF_CALLCONV EsifAct_EventCallback(
	void *contextPtr,
	UInt8 upInstance,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifActPtr actPtr = NULL;

	UNREFERENCED_PARAMETER(upInstance);

	if (NULL == fpcEventPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	actPtr = EsifActMgr_GetAction((UInt8)(size_t)contextPtr, upInstance);
	if (NULL == actPtr) {
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}
	rc = EsifActIface_SendIfaceEvent(&actPtr->iface, 
		actPtr->actCtx,
		fpcEventPtr->esif_event,
		domainId,
		eventDataPtr);
exit:
	EsifAct_PutRef(actPtr);
	return rc;
}


static eEsifError EsifActIface_SendIfaceEvent(
	EsifActIfacePtr self,
	esif_context_t actCtx,
	enum esif_event_type eventType,
	UInt16 domainId,
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);

	switch (self->hdr.fIfaceVersion) {
	case ESIF_ACT_IFACE_VER_V1:
		if (NULL == self->actIfaceV1.rcvEventFuncPtr) {
			rc = ESIF_E_NOT_SUPPORTED;
			goto exit;
		}
		self->actIfaceV1.rcvEventFuncPtr(actCtx,
			eventType,
			domainId,
			eventDataPtr
			);
		break;
	case ESIF_ACT_IFACE_VER_STATIC:
	default:
		break;
	}
exit:
return rc;
}


static eEsifError ESIF_CALLCONV EsifActIface_ReceiveEventV1(
	const esif_handle_t esifInstHandle,
	enum esif_event_type eventType,
	UInt16 domain,
	const EsifDataPtr dataPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifActPtr actPtr = NULL;
	ActionHandle actHandle = {0}; 

	actHandle.asDword = (UInt32)(size_t)esifInstHandle;

	actPtr = EsifActMgr_GetAction(actHandle.s.type, ACT_MGR_NO_UPINSTANCE);
	if (NULL == actPtr) {
		rc = ESIF_E_NOT_FOUND;
		goto exit;
	}

	rc = EsifEventMgr_SignalEvent((UInt8)actHandle.s.upInstance, domain, eventType, dataPtr);
exit:
	EsifAct_PutRef(actPtr);
	return rc;
}


EsifActIfacePtr EsifAct_GetIface(EsifActPtr self)
{
	return (self != NULL) ? &self->iface : NULL;
}


EsifActIfaceVer EsifAct_GetIfaceVersion(
	EsifActPtr self
	)
{
	return (self != NULL) ? self->iface.hdr.fIfaceVersion : ESIF_ACT_IFACE_VER_INVALID;
}


/*
 * If you pass in a NULL pointer, it will return 0.
 */
enum esif_action_type EsifAct_GetType(
	EsifActPtr self
	)
{
	eEsifError rc = ESIF_OK;
	enum esif_action_type type = 0;

	if (NULL == self) {
		goto exit;
	}

	rc = EsifActIface_GetType(&self->iface, &type);
	if (rc != ESIF_OK) {
		type = 0;
	}
exit:
	return type;
}


eEsifError EsifActIface_GetType(
	EsifActIfacePtr self,
	enum esif_action_type *typePtr
	)
{
	eEsifError rc = ESIF_OK;

	if ((NULL == self) || (NULL == typePtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	switch (self->hdr.fIfaceVersion) {
	case ESIF_ACT_IFACE_VER_STATIC:
		*typePtr = self->ifaceStatic.type;
		break;
	case ESIF_ACT_IFACE_VER_V1:
		*typePtr = self->actIfaceV1.type;
		break;
	default:
		rc = ESIF_E_NOT_SUPPORTED;
		break;
	}
exit:
	return rc;
}


Bool EsifAct_IsPlugin(EsifActPtr self)
{
	return (self != NULL) ? self->isPlugin : ESIF_FALSE;
}


esif_context_t EsifAct_GetActCtx(EsifActPtr self)
{
	return (self != NULL) ? self->actCtx : NULL;
}


EsifString EsifAct_GetName(EsifActPtr self) {
	return (self != NULL) ? EsifActIface_GetName(&self->iface) : "UNK";
}

static EsifString EsifActIface_GetName(
	EsifActIfacePtr self
	)
{
	EsifString name = "UNK";

	ESIF_ASSERT(self != NULL);

	switch (self->hdr.fIfaceVersion) {
	case ESIF_ACT_IFACE_VER_STATIC:
		name = self->ifaceStatic.name;
		break;
	case ESIF_ACT_IFACE_VER_V1:
		name = self->actIfaceV1.name;
		break;
	default:
		break;

	}
	return name;
}


EsifString EsifAct_GetDesc(EsifActPtr self) {
	return (self != NULL) ? EsifActIface_GetDesc(&self->iface) : "UNK";
}

static EsifString EsifActIface_GetDesc(
	EsifActIfacePtr self
	)
{
	EsifString desc = "UNK";

	ESIF_ASSERT(self != NULL);

	switch (self->hdr.fIfaceVersion) {
	case ESIF_ACT_IFACE_VER_STATIC:
		desc = self->ifaceStatic.desc;
		break;
	case ESIF_ACT_IFACE_VER_V1:
		desc = self->actIfaceV1.desc;
		break;
	default:
		break;

	}
	return desc;
}


UInt16 EsifAct_GetVersion(EsifActPtr self) {
	return (self != NULL) ? EsifActIface_GetVersion(&self->iface) : ESIF_ACTION_VERSION_INVALID;
}

static UInt16 EsifActIface_GetVersion(
	EsifActIfacePtr self
	)
{
	UInt16 actVersion = ESIF_ACTION_VERSION_INVALID;

	ESIF_ASSERT(self != NULL);

	switch (self->hdr.fIfaceVersion) {
	case ESIF_ACT_IFACE_VER_STATIC:
		actVersion = self->ifaceStatic.actVersion;
		break;
	case ESIF_ACT_IFACE_VER_V1:
		actVersion = self->actIfaceV1.actVersion;
		break;
	default:
		break;
	}
	return actVersion;
}


UInt16 EsifActIface_Sizeof(
	EsifActIfaceVer fIfaceVersion
	)
{
	UInt16 size = 0xFFFF;

	switch (fIfaceVersion) {
	case ESIF_ACT_IFACE_VER_STATIC:
		size = sizeof(EsifActIfaceStatic);
		break;
	case ESIF_ACT_IFACE_VER_V1:
		size = sizeof(EsifActIfaceUpeV1);
		break;
	default:
		break;

	}
return size;
}


eEsifError EsifActCallPluginGet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcActionPtr fpcActionPtr,
	ActExecuteGetFunction actGetFuncPtr,
	const EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifData params[NUMBER_OF_PARAMETERS_FOR_AN_ACTION] = {0};

	/*  Participant Check */
	if (NULL == upPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("Participant For Participant ID %d NOT FOUND\n", EsifUp_GetInstance(upPtr));
		goto exit;
	}

	if (NULL == fpcActionPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("NULL action pointer received\n");
		goto exit;	
	}

	if (NULL == actGetFuncPtr) {
		ESIF_TRACE_DEBUG("Plugin function pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (NULL == requestPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("NULL request pointer\n");
		goto exit;
	}

	rc = EsifFpcAction_GetParams(fpcActionPtr,
		params,
		sizeof(params)/sizeof(*params));
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = actGetFuncPtr(actCtx,
		upPtr->fMetadata.fDevicePath,
		&params[0],
		&params[1],
		&params[2],
		&params[3],
		&params[4],
		requestPtr,
		responsePtr);
exit:
	return rc;
}


eEsifError EsifActCallPluginSet(
	esif_context_t actCtx,
	EsifUpPtr upPtr,
	const EsifFpcActionPtr fpcActionPtr,
	ActExecuteSetFunction actSetFuncPtr,
	const EsifDataPtr requestPtr
	)
{
	eEsifError rc    = ESIF_OK;
	EsifData params[NUMBER_OF_PARAMETERS_FOR_AN_ACTION] = {0};

	if (NULL == upPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("Participant For Participant ID %d NOT FOUND\n", EsifUp_GetInstance(upPtr));
		goto exit;
	}

	if (NULL == fpcActionPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("NULL action pointer received\n");
		goto exit;	
	}

	if (NULL == requestPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("NULL request pointer\n");
		goto exit;
	}

	rc = EsifFpcAction_GetParams(fpcActionPtr,
		params,
		sizeof(params)/sizeof(*params));
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = actSetFuncPtr(actCtx,
		upPtr->fMetadata.fDevicePath,
		&params[0],
		&params[1],
		&params[2],
		&params[3],
		&params[4],
		requestPtr);
exit:
	return rc;
}


void EsifAct_MarkAsPlugin(
	EsifActPtr self
	)
{
	if (self != NULL){
		self->isPlugin = ESIF_TRUE;
	}
}

static eEsifError ESIF_CALLCONV EsifActWriteLogHandler(
	const EsifDataPtr message,	/* Message For Log */
	const eLogType logType		/* Log Type e.g. crticial, debug, info,e tc */
	)
{
	eEsifError rc = ESIF_OK;
	size_t msgLen = 0;

	if ((NULL == message) || (NULL == message->buf_ptr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	msgLen = esif_ccb_strlen(message->buf_ptr, message->buf_len);
	if (msgLen > 0) {
		ESIF_TRACE_IFACTIVE(ESIF_TRACE_ID, logType,  message->buf_ptr);
	}

exit:
	return rc;
}


/* WARNING:  The allocated strings in replacedStrsPtr must be released by the caller */
eEsifError EsifFpcAction_GetParams(
	EsifFpcActionPtr fpcActionPtr,
	EsifDataPtr paramsPtr,
	UInt8 numParams
	)
{
	eEsifError rc = ESIF_OK;
	UInt8 i;

	if ((NULL == fpcActionPtr) || (NULL == paramsPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	for (i = 0; i < numParams; i++) {
		rc = EsifFpcAction_GetParamAsEsifData(fpcActionPtr,
			i,
			&paramsPtr[i]);
		if (ESIF_OK != rc) {
			break;
		}
	}
exit:
	return rc;
}


/* WARNING:  The allocated strings in replacedStrPtr must be released by the caller */
eEsifError EsifFpcAction_GetParamAsEsifData(
	EsifFpcActionPtr fpcActionPtr,
	UInt8 paramNum,
	EsifDataPtr paramPtr
	)
{
	eEsifError rc = ESIF_OK;
	DataItemPtr dataItemPtr = NULL;
	EsifString paramStr = NULL;

	if ((NULL == fpcActionPtr) || (NULL == paramPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	paramPtr->buf_len = 0;
	paramPtr->buf_ptr = NULL;
	paramPtr->type    = ESIF_DATA_UINT32;

	dataItemPtr = EsifFpcAction_GetParam(fpcActionPtr, paramNum);
	if (NULL == dataItemPtr) {
		goto exit;
	}

	switch (dataItemPtr->data_type) {
	case DATA_ITEM_TYPE_STRING:
	{
		paramStr = (EsifString) &dataItemPtr->data;

		paramPtr->buf_ptr  = paramStr;
		paramPtr->buf_len  = (u32)esif_ccb_strlen(paramStr, MAXPARAMLEN);
		paramPtr->data_len = (u32)esif_ccb_strlen(paramStr, MAXPARAMLEN);
		paramPtr->type     = ESIF_DATA_STRING;
		break;
	}

	case DATA_ITEM_TYPE_UINT32:
		paramPtr->buf_ptr  = (u32 *)&dataItemPtr->data;
		paramPtr->buf_len  = dataItemPtr->data_length_in_bytes;
		paramPtr->data_len = dataItemPtr->data_length_in_bytes;
		paramPtr->type     = ESIF_DATA_UINT32;
		break;

	default:
		break;
	}
exit:
	return rc;
}


DataItemPtr EsifFpcAction_GetParam(
	const EsifFpcActionPtr fpcActionPtr,
	const UInt8 paramNum
	)
{
	if (paramNum >= NUMBER_OF_PARAMETERS_FOR_AN_ACTION) {
		return NULL;
	}

	if (fpcActionPtr->param_valid[paramNum] == 0) {
		return NULL;
	}

	return (DataItemPtr)(((UInt8 *)fpcActionPtr) + fpcActionPtr->param_offset[paramNum]);
}


eEsifError EsifCopyIntToBufBySize(
	size_t typeSize,
	void *dstPtr,
	u64 val
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(dstPtr != NULL);

	switch(typeSize) {
	case sizeof(u8):
		*((u8 *)dstPtr) = (u8)val;
		break;
	case sizeof(u16):
		*((u16 *)dstPtr) = (u16)val;
		break;
	case sizeof(u32):
		*((u32 *)dstPtr) = (u32)val;
		break;
	case sizeof(u64):
		*((u64 *)dstPtr) = (u64)val;
		break;
	default:
		rc =  ESIF_E_UNSUPPORTED_RESULT_DATA_TYPE;
		break;
	}
	return rc;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
