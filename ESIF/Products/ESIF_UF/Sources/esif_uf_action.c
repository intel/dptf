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
#include "esif_sdk_iface_esif.h"	/* ESIF Services Interface */
#include "esif_uf_actmgr.h"		/* Action Manager */
#include "esif_uf_service.h"	/* ESIF Service */
#include "esif_uf_primitive.h"
#include "esif_uf_action.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#ifndef ESIF_FEAT_OPT_ACTION_SYSFS
#define EsifActSysfsExit()
#define EsifActSysfsInit()
#endif
#define ACTION_DEBUG ESIF_DEBUG

typedef eEsifError (ESIF_CALLCONV *GetIfaceFuncPtr)(EsifActInterfacePtr);

/*
 * Need to move to header POC.  Also don't forget to free returned
 * memory JDH
 */
extern char *esif_str_replace(char *orig, char *rep, char *with);

static eEsifError ActionCreate(
	EsifActPtr actionPtr,
	GetIfaceFuncPtr ifaceFuncPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifActTypePtr action_type_ptr = NULL;
	EsifData p1   = {ESIF_DATA_STRING, "action_iD", sizeof("action_id")};
	EsifData p2   = {ESIF_DATA_STRING, "domain_qualifier", sizeof("domain_qaulifier")};
	EsifData p3   = {ESIF_DATA_UINT32, "kernel_abi_type", sizeof("Kernel_abi_type")};
	EsifData p4   = {ESIF_DATA_UINT8, "mode", sizeof("mode")};

	char name[ESIF_NAME_LEN] = {0};
	ESIF_DATA(data_name, ESIF_DATA_STRING, name, ESIF_NAME_LEN);

	char desc[ESIF_DESC_LEN] = {0};
	ESIF_DATA(data_desc, ESIF_DATA_STRING, desc, ESIF_DESC_LEN);

	char version[ESIF_DESC_LEN] = {0};
	ESIF_DATA(data_version, ESIF_DATA_STRING, version, ESIF_DESC_LEN);

	UInt32 action_type_id = 0;
	EsifData action_type  = {ESIF_DATA_UINT32, &action_type_id, sizeof(action_type_id), 0};

	esif_guid_t guid   = {0};
	EsifData data_guid = {ESIF_DATA_GUID, &guid, sizeof(guid), 0};

	EsifString act_type_ptr = NULL;
	EsifInterface act_service_iface;

	ESIF_ASSERT(actionPtr != NULL);
	ESIF_ASSERT(ifaceFuncPtr != NULL);

	/* Assign the EsifInterface Functions */
	act_service_iface.fIfaceType              = eIfaceTypeEsifService;
	act_service_iface.fIfaceVersion           = 1;
	act_service_iface.fIfaceSize              = (UInt16)sizeof(EsifInterface);

	act_service_iface.fGetConfigFuncPtr       = EsifSvcConfigGet;
	act_service_iface.fSetConfigFuncPtr       = EsifSvcConfigSet;
	act_service_iface.fPrimitiveFuncPtr       = EsifSvcPrimitiveExec;
	act_service_iface.fWriteLogFuncPtr        = EsifSvcWriteLog;
	act_service_iface.fRegisterEventFuncPtr   = EsifSvcEventRegister;
	act_service_iface.fUnregisterEventFuncPtr = EsifSvcEventUnregister;

	/* GetApplicationInterface Handleshake send ESIF receive APP Interface */
	rc = ifaceFuncPtr(&actionPtr->fInterface);
	if (ESIF_OK != rc) {
		goto exit;
	}

	/* Check EsifAppInterface */
	if (actionPtr->fInterface.fIfaceType != eIfaceTypeAction ||
		actionPtr->fInterface.fIfaceSize != (UInt16)sizeof(EsifActInterface) ||
		actionPtr->fInterface.fIfaceVersion != 1 ||

		/* Functions Pointers */
		actionPtr->fInterface.fActCreateFuncPtr == NULL ||
		actionPtr->fInterface.fActDestroyFuncPtr == NULL ||
		actionPtr->fInterface.fActGetAboutFuncPtr == NULL ||
		actionPtr->fInterface.fActGetDescriptionFuncPtr == NULL ||
		actionPtr->fInterface.fActGetFuncPtr == NULL ||
		actionPtr->fInterface.fActGetIDFuncPtr == NULL ||
		actionPtr->fInterface.fActGetGuidFuncPtr == NULL ||
		actionPtr->fInterface.fActGetNameFuncPtr == NULL ||
		actionPtr->fInterface.fActGetStateFuncPtr == NULL ||
		actionPtr->fInterface.fActGetStatusFuncPtr == NULL ||
		actionPtr->fInterface.fActGetVersionFuncPtr == NULL ||
		actionPtr->fInterface.fActSetFuncPtr == NULL ||
		actionPtr->fInterface.fActSetStateFuncPtr == NULL) {
		ESIF_TRACE_ERROR("The required function pointer in EsifActInterface is NULL\n");
		goto exit;
	}

	/* Callback for application information */
	rc = actionPtr->fInterface.fActGetNameFuncPtr(&data_name);
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = actionPtr->fInterface.fActGetDescriptionFuncPtr(&data_desc);
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = actionPtr->fInterface.fActGetVersionFuncPtr(&data_version);
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = actionPtr->fInterface.fActGetIDFuncPtr(&action_type);
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = actionPtr->fInterface.fActGetGuidFuncPtr(&data_guid);
	if (ESIF_OK != rc) {
		goto exit;
	}

	act_type_ptr = "plugin";

	ESIF_TRACE_DEBUG("\n\n"
		"Action Name   : %s\n"
		"Action Desc   : %s\n"
		"Action Type   : %s\n"
		"Action Version: %s\n\n",
		(EsifString)data_name.buf_ptr,
		(EsifString)data_desc.buf_ptr,
		(EsifString)act_type_ptr,
		(EsifString)data_version.buf_ptr);

	/* Create The Application */
	CMD_OUT("create action\n");
	rc = actionPtr->fInterface.fActCreateFuncPtr(
		&act_service_iface,
		NULL,
		&actionPtr->fHandle,
		eEsifActStateEnabled,
		&p1,
		&p2,
		&p3,
		&p4,
		NULL);

	if (ESIF_OK != rc) {
		goto exit;
	}

	/* Append New Action To Linked List */
	action_type_ptr = (EsifActTypePtr)esif_ccb_malloc(sizeof(EsifActType));

	if (NULL == action_type_ptr) {
		ESIF_TRACE_ERROR("Fail to allocate EsifActType\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	action_type_ptr->fHandle = actionPtr->fHandle;
	action_type_ptr->fType   = *(UInt8 *)action_type.buf_ptr;

	esif_ccb_strcpy(action_type_ptr->fName,
					(EsifString)data_name.buf_ptr, ESIF_NAME_LEN);
	esif_ccb_strcpy(action_type_ptr->fDesc,
					(EsifString)data_desc.buf_ptr, ESIF_DESC_LEN);
	esif_ccb_strcpy(action_type_ptr->fOsType, 
		            ESIF_ATTR_OS, ESIF_NAME_LEN);
	esif_ccb_strcpy(action_type_ptr->fVersion, 
		            (EsifString)data_version.buf_ptr, ESIF_NAME_LEN);

	action_type_ptr->fActGetFuncPtr = actionPtr->fInterface.fActGetFuncPtr;
	action_type_ptr->fActSetFuncPtr = actionPtr->fInterface.fActSetFuncPtr;
	action_type_ptr->fGetFuncPtr = NULL;
	action_type_ptr->fSetFuncPtr = NULL;

	esif_ccb_memcpy(action_type_ptr->fGuid, data_guid.buf_ptr, ESIF_GUID_LEN);
	action_type_ptr->fIsKernel   = ESIF_ACTION_IS_NOT_KERNEL_ACTION;
	action_type_ptr->fIsPlugin   = ESIF_ACTION_IS_PLUGIN;

	/* Register Action */
	if (NULL != g_actMgr.AddActType) {
		rc = g_actMgr.AddActType(&g_actMgr, action_type_ptr);
	} else {
		ESIF_TRACE_ERROR("Fail to add action type since g_actMrg.AddActType is NULL\n");
		esif_ccb_free(action_type_ptr);
		rc = ESIF_E_NO_CREATE;
	}
exit:
	return rc;
}


eEsifError EsifActStart(EsifActPtr actionPtr)
{
	eEsifError rc = ESIF_OK;
	GetIfaceFuncPtr iface_func_ptr = NULL;
	EsifString iface_func_name     = "GetActionInterface";

	char libPath[ESIF_LIBPATH_LEN];

	ESIF_TRACE_DEBUG("Name=%s\n", actionPtr->fLibNamePtr);
	esif_build_path(libPath, sizeof(libPath), ESIF_PATHTYPE_DLL, actionPtr->fLibNamePtr, ESIF_LIB_EXT);
	actionPtr->fLibHandle = esif_ccb_library_load(libPath);

	if (NULL == actionPtr->fLibHandle || NULL == actionPtr->fLibHandle->handle) {
		rc = esif_ccb_library_error(actionPtr->fLibHandle);
		ESIF_TRACE_ERROR("esif_ccb_library_load() %s failed [%s (%d)]: %s\n", libPath, esif_rc_str(rc), rc, esif_ccb_library_errormsg(actionPtr->fLibHandle));
		goto exit;
	}
	ESIF_TRACE_DEBUG("esif_ccb_library_load() %s completed.\n", libPath);

	iface_func_ptr = (GetIfaceFuncPtr)esif_ccb_library_get_func(actionPtr->fLibHandle, (EsifString)iface_func_name);
	if (NULL == iface_func_ptr) {
		rc = esif_ccb_library_error(actionPtr->fLibHandle);
		ESIF_TRACE_ERROR("esif_ccb_library_get_func() %s failed [%s (%d)]: %s\n", libPath, esif_rc_str(rc), rc, esif_ccb_library_errormsg(actionPtr->fLibHandle));
		goto exit;
	}

	ESIF_TRACE_DEBUG("esif_ccb_library_get_func() %s completed.\n", iface_func_name);
	rc = ActionCreate(actionPtr, iface_func_ptr);
	if (ESIF_OK != rc) {
		ESIF_TRACE_DEBUG("ActionCreate failed.\n");
		goto exit;
	}
	ESIF_TRACE_DEBUG("ActionCreate completed.\n");

exit:
	if (ESIF_OK != rc) {
		esif_ccb_library_unload(actionPtr->fLibHandle);
		actionPtr->fLibHandle = NULL;
	}
	return rc;
}


eEsifError EsifActCallPluginGet(
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcActionPtr actionPtr,
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
		ESIF_TRACE_WARN("Participant For Participant ID %d NOT FOUND\n", upPtr->fInstance);
		goto exit;
	}

	if (NULL == actionPtr) {
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

	rc = EsifActionGetParams(actionPtr,
		params,
		sizeof(params)/sizeof(*params));
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = actGetFuncPtr(actionHandle,
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
	const void *actionHandle,
	EsifUpPtr upPtr,
	const EsifFpcActionPtr actionPtr,
	ActExecuteSetFunction actSetFuncPtr,
	const EsifDataPtr requestPtr
	)
{
	eEsifError rc    = ESIF_OK;
	EsifData params[NUMBER_OF_PARAMETERS_FOR_AN_ACTION] = {0};

	if (NULL == upPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("Participant For Participant ID %d NOT FOUND\n", upPtr->fInstance);
		goto exit;
	}

	if (NULL == actionPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("NULL action pointer received\n");
		goto exit;	
	}

	//if (NULL == actSetFuncPtr) {
	//	ESIF_TRACE_DEBUG("Plugin function pointer is NULL\n");
	//	rc = ESIF_E_PARAMETER_IS_NULL;
	//	goto exit;
	//}

	if (NULL == requestPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		ESIF_TRACE_WARN("NULL request pointer\n");
		goto exit;
	}

	rc = EsifActionGetParams(actionPtr,
		params,
		sizeof(params)/sizeof(*params));
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = actSetFuncPtr(actionHandle,
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


/* WARNING:  The allocated strings in replacedStrsPtr must be released by the caller */
eEsifError EsifActionGetParams(
	EsifFpcActionPtr actionPtr,
	EsifDataPtr paramsPtr,
	UInt8 numParams
	)
{
	eEsifError rc = ESIF_OK;
	UInt8 i;

	if ((NULL == actionPtr) || (NULL == paramsPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	for (i = 0; i < numParams; i++) {
		rc = EsifActionGetParamAsEsifData(actionPtr,
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
eEsifError EsifActionGetParamAsEsifData(
	EsifFpcActionPtr actionPtr,
	UInt8 paramNum,
	EsifDataPtr paramPtr
	)
{
	eEsifError rc = ESIF_OK;
	DataItemPtr dataItemPtr = NULL;
	EsifString paramStr = NULL;

	if ((NULL == actionPtr) || (NULL == paramPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	paramPtr->buf_len = 0;
	paramPtr->buf_ptr = NULL;
	paramPtr->type    = ESIF_DATA_UINT32;

	dataItemPtr = EsifActionGetParam(actionPtr, paramNum);
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


/* WARNING:  The allocated strings in replacedStrPtr must be released by the caller */
/* Return a NULL string if no token replacement takes place */
EsifString EsifActionCreateTokenReplacedParamString(
	const EsifString paramStr,
	const EsifUpPtr upPtr,
	const EsifFpcPrimitivePtr primitivePtr
	)
{
	EsifString replacedStr = NULL;
	char domainStr[8] = "";
	char idBuf[ESIF_NAME_LEN + 1 + sizeof(domainStr)];

	if ((NULL == paramStr) || (NULL == upPtr) || (NULL == primitivePtr)) {
		goto exit;
	}

	esif_primitive_domain_str(primitivePtr->tuple.domain, domainStr, sizeof(domainStr));
	esif_ccb_sprintf(sizeof(idBuf) - 1, idBuf, "%s.%s", upPtr->fMetadata.fName, domainStr);

	replacedStr = esif_str_replace(paramStr, "%nm%", idBuf);

	if (replacedStr != NULL) {
		ESIF_TRACE_DEBUG("\tEXPANDED data %s\n", replacedStr);
	}
exit:
	return replacedStr;
}


DataItemPtr EsifActionGetParam(
	const EsifFpcActionPtr actionPtr,
	const UInt8 paramNum
	)
{
	if (paramNum >= NUMBER_OF_PARAMETERS_FOR_AN_ACTION) {
		return NULL;
	}

	if (actionPtr->param_valid[paramNum] == 0) {
		return NULL;
	}

	return (DataItemPtr)(((UInt8 *)actionPtr) + actionPtr->param_offset[paramNum]);
}


eEsifError EsifActionCopyIntToBufBySize(
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



eEsifError EsifActStop (EsifActPtr actPtr)
{
	eEsifError rc = ESIF_OK;
	ESIF_ASSERT(actPtr != NULL);

	// TODO: Cleanup

	if (ESIF_OK == rc) {
		esif_ccb_free(actPtr->fLibNamePtr);
		esif_ccb_library_unload(actPtr->fLibHandle);
		memset(actPtr, 0, sizeof(*actPtr));
	}
	return rc;
}


eEsifError EsifActInit()
{
	EsifActConfigInit();
	EsifActConstInit();
	EsifActSystemInit();
	EsifActDelegateInit();
	EsifActSysfsInit();
	ESIF_TRACE_EXIT_INFO();
	return ESIF_OK;
}


void EsifActExit()
{
	EsifActConfigExit();
	EsifActConstExit();
	EsifActSystemExit();
	EsifActDelegateExit();
	EsifActSysfsExit();
	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
