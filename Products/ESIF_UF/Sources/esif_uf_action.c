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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_ACTION

#include "esif_uf.h"			/* Upper Framework */
#include "esif_uf_esif_iface.h"	/* ESIF Services Interface */
#include "esif_uf_actmgr.h"		/* Action Manager */
#include "esif_uf_service.h"	/* ESIF Service */

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define ACTION_DEBUG ESIF_DEBUG

extern EsifActMgr g_actMgr;

typedef eEsifError (*GetIfaceFuncPtr)(EsifActInterfacePtr);

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

	ESIF_TRACE_DEBUG("%s\n\n"
					 "Action Name   : %s\n"
					 "Action Desc   : %s\n"
					 "Action Type   : %s\n"
					 "Action Version: %s\n\n",
					 ESIF_FUNC,
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
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	action_type_ptr->fHandle = actionPtr->fHandle;
	action_type_ptr->fType   = *(UInt8 *)action_type.buf_ptr;

	esif_ccb_strcpy(action_type_ptr->fName,
					(EsifString)data_name.buf_ptr, ESIF_NAME_LEN);
	esif_ccb_strcpy(action_type_ptr->fDesc,
					(EsifString)data_desc.buf_ptr, ESIF_DESC_LEN);
	esif_ccb_strcpy(action_type_ptr->fOsType, ESIF_ATTR_OS, ESIF_NAME_LEN);

	action_type_ptr->fGetFuncPtr = actionPtr->fInterface.fActGetFuncPtr;
	action_type_ptr->fSetFuncPtr = actionPtr->fInterface.fActSetFuncPtr;
	esif_ccb_memcpy(action_type_ptr->fGuid, data_guid.buf_ptr, ESIF_GUID_LEN);
	action_type_ptr->fIsKernel   = ESIF_FALSE;
	action_type_ptr->fIsPlugin   = ESIF_TRUE;

	/* Register Action */
	if (NULL != g_actMgr.AddActType) {
		rc = g_actMgr.AddActType(&g_actMgr, action_type_ptr);
	} else {
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

	ESIF_TRACE_DEBUG("%s name=%s\n", ESIF_FUNC, actionPtr->fLibNamePtr);
	esif_build_path(libPath, sizeof(libPath), ESIF_PATHTYPE_DLL, actionPtr->fLibNamePtr, ESIF_LIB_EXT);
	actionPtr->fLibHandle = esif_ccb_library_load(libPath);

	if (NULL == actionPtr->fLibHandle) {
		rc = ESIF_E_UNSPECIFIED;
		ESIF_TRACE_DEBUG("%s esif_ccb_library_load() %s failed.\n", ESIF_FUNC, libPath);
		goto exit;
	}
	ESIF_TRACE_DEBUG("%s esif_ccb_library_load() %s completed.\n", ESIF_FUNC, libPath);

	iface_func_ptr = (GetIfaceFuncPtr)
		esif_ccb_library_get_func(actionPtr->fLibHandle, (EsifString)iface_func_name);

	if (NULL == iface_func_ptr) {
		rc = ESIF_E_UNSPECIFIED;
		ESIF_TRACE_DEBUG("%s esif_ccb_library_get_func() %s failed.\n",
						 ESIF_FUNC, iface_func_name);
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s esif_ccb_library_get_func() %s completed.\n",
					 ESIF_FUNC, iface_func_name);
	rc = ActionCreate(actionPtr, iface_func_ptr);
	ESIF_TRACE_DEBUG("%s ActionCreate completed.\n", ESIF_FUNC);
exit:
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
	return ESIF_OK;
}


void EsifActExit()
{
	EsifActConfigExit();
	EsifActConstExit();
	EsifActSystemExit();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
