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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_CONJURE

#include "esif_uf.h"		/* Upper Framework */
#include "esif_pm.h"		/* Particpant Manager */
#include "esif_dsp.h"		/* Device Support Package */
#include "esif_uf_cnjmgr.h"	/* Conjure Manager */
#include "esif_uf_appmgr.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define CONJURE_DEBUG ESIF_DEBUG

/* Conjure Well Known Function */
typedef eEsifError (ESIF_CALLCONV *GetIfaceFuncPtr)(EsifConjureInterfacePtr);

/* Friends */
extern EsifCnjMgr g_cnjMgr;

static eEsifError ESIF_CALLCONV RegisterParticipant(const EsifParticipantIfacePtr piPtr)
{
	eEsifError rc    = ESIF_OK;
	char guid_str[ESIF_GUID_PRINT_SIZE];
	UInt8 newInstance = ESIF_INSTANCE_INVALID;

	UNREFERENCED_PARAMETER(guid_str);

	ESIF_ASSERT(piPtr != NULL);
	ESIF_TRACE_INFO(
		"\n"
		"=======================================================\n"
		"ESIF CREATE CONJURE PARTICIPANT:\n"
		"=======================================================\n"
		"Version:        %d\n"
		"Class:          %s\n"
		"Enumerator:     %s(%u)\n"
		"Flags:          0x%08x\n"
		"Name:           %s\n"
		"Description:    %s\n"
		"Driver Name:    %s\n"
		"Device Name:    %s\n"
		"Device Path:    %s\n"
		"Object ID:      %s\n\n",
		piPtr->version,
		esif_guid_print(&piPtr->class_guid, guid_str),
		esif_participant_enum_str(piPtr->enumerator),
		piPtr->enumerator,
		piPtr->flags,
		piPtr->name,
		piPtr->desc,
		piPtr->driver_name,
		piPtr->device_name,
		piPtr->device_path,
		piPtr->object_id);

	if (EsifUpPm_DoesAvailableParticipantExistByName(piPtr->name)) {
		ESIF_TRACE_WARN("Participant %s has already existed in upper framework\n", piPtr->name);
		rc     = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	rc = EsifUpPm_RegisterParticipant(eParticipantOriginUF, piPtr, &newInstance);
	if (ESIF_OK != rc) {
		ESIF_TRACE_ERROR("Fail to add participant %s in participant manager\n", piPtr->name);
		goto exit;
	}

	ESIF_TRACE_DEBUG("Create new UF participant: %s, instance = %d\n", piPtr->name, newInstance);

exit:
	if ((ESIF_OK != rc) && (ESIF_INSTANCE_INVALID != newInstance)) {
		ESIF_TRACE_WARN("Unregister participant in UP manager due to participant data creation failure\n");
		EsifUpPm_UnregisterParticipant(eParticipantOriginUF, newInstance);
	}

	return rc;
}


static eEsifError ESIF_CALLCONV UnRegisterParticipant(const EsifParticipantIfacePtr piPtr)
{
	eEsifError rc = ESIF_E_UNSPECIFIED;
	EsifUpPtr up_ptr = NULL;
	UInt8 instance = ESIF_INSTANCE_INVALID;

	ESIF_ASSERT(piPtr != NULL);

	up_ptr = EsifUpPm_GetAvailableParticipantByName(piPtr->name);
	if (NULL == up_ptr) {
		ESIF_TRACE_WARN("Unregister Participant Not Found: %s\n", piPtr->name);
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}
	ESIF_TRACE_DEBUG("Unregister Participant\n");
	instance = EsifUp_GetInstance(up_ptr);
	EsifUp_PutRef(up_ptr);
	rc = EsifUpPm_UnregisterParticipant(eParticipantOriginUF, instance);

exit:
	return rc;
}


/* Create A Conjure Library */
static eEsifError ConjureCreate(
	EsifCnjPtr conjurePtr,
	GetIfaceFuncPtr ifaceFuncPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifConjureServiceInterface conjure_service_iface;

	char name[ESIF_NAME_LEN] = "";
	ESIF_DATA(data_name, ESIF_DATA_STRING, name, ESIF_NAME_LEN);

	char desc[ESIF_DESC_LEN] = "";
	ESIF_DATA(data_desc, ESIF_DATA_STRING, desc, ESIF_DESC_LEN);

	char version[ESIF_DESC_LEN] = "";
	ESIF_DATA(data_version, ESIF_DATA_STRING, version, ESIF_DESC_LEN);

	ESIF_ASSERT(conjurePtr != NULL);
	ESIF_ASSERT(ifaceFuncPtr != NULL);

	/* Assign the EsifInterface Functions */
	conjure_service_iface.fIfaceType    = eIfaceTypeConjureService;
	conjure_service_iface.fIfaceVersion = 1;
	conjure_service_iface.fIfaceSize    = (UInt16)sizeof(EsifConjureServiceInterface);

	conjure_service_iface.fRegisterParticipantFuncPtr   = RegisterParticipant;
	conjure_service_iface.fUnRegisterParticipantFuncPtr = UnRegisterParticipant;

	/* GetConjureInterface Handleshake send ESIF receive APP Interface */
	rc = ifaceFuncPtr(&conjurePtr->fInterface);
	if (ESIF_OK != rc) {
		goto exit;
	}

	/* Check EsifAppInterface */
	if (conjurePtr->fInterface.fIfaceType != eIfaceTypeConjure ||
		conjurePtr->fInterface.fIfaceSize != (UInt16)sizeof(EsifConjureInterface) ||
		conjurePtr->fInterface.fIfaceVersion != 1 ||

		/* Functions Pointers */
		conjurePtr->fInterface.fConjureCreateFuncPtr == NULL ||
		conjurePtr->fInterface.fConjureDestroyFuncPtr == NULL ||
		conjurePtr->fInterface.fConjureGetAboutFuncPtr == NULL ||
		conjurePtr->fInterface.fConjureGetDescriptionFuncPtr == NULL ||
		conjurePtr->fInterface.fConjureGetGuidFuncPtr == NULL ||
		conjurePtr->fInterface.fConjureGetNameFuncPtr == NULL ||
		conjurePtr->fInterface.fConjureGetVersionFuncPtr == NULL) {
		ESIF_TRACE_ERROR("The required function pointer of EsifConjureInterface is NULL\n");
		goto exit;
	}

	/* Callback for application information */
	rc = conjurePtr->fInterface.fConjureGetNameFuncPtr(&data_name);
	if (ESIF_OK != rc) {
		goto exit;
	}
	rc = conjurePtr->fInterface.fConjureGetDescriptionFuncPtr(&data_desc);
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = conjurePtr->fInterface.fConjureGetVersionFuncPtr(&data_version);
	if (ESIF_OK != rc) {
		goto exit;
	}

	ESIF_TRACE_DEBUG("\n\n"
		"Conjure Lib Name   : %s\n"
		"Conjure Lib Desc   : %s\n"
		"Conjure Lib Type   : %s\n"
		"Conjure Lib Version: %s\n\n",
		(EsifString)data_name.buf_ptr,
		(EsifString)data_desc.buf_ptr,
		(EsifString)"plugin",
		(EsifString)data_version.buf_ptr);

	/* Create The Conjure */
	rc = conjurePtr->fInterface.fConjureCreateFuncPtr(
			&conjure_service_iface,
			NULL,
			&conjurePtr->fHandle);

	if (ESIF_OK != rc) {
		goto exit;
	}
exit:
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


/* Start Conjure Library */
eEsifError EsifConjureStart(EsifCnjPtr conjurePtr)
{
	eEsifError rc = ESIF_OK;
	GetIfaceFuncPtr iface_func_ptr = NULL;
	EsifString iface_func_name     = "GetConjureInterface";

	char libPath[ESIF_LIBPATH_LEN];

	ESIF_TRACE_DEBUG("Name=%s\n", conjurePtr->fLibNamePtr);
	esif_build_path(libPath, ESIF_LIBPATH_LEN, ESIF_PATHTYPE_DLL, conjurePtr->fLibNamePtr, ESIF_LIB_EXT);
	conjurePtr->fLibHandle = esif_ccb_library_load(libPath);

	if (NULL == conjurePtr->fLibHandle || NULL == conjurePtr->fLibHandle->handle) {
		rc = esif_ccb_library_error(conjurePtr->fLibHandle);
		ESIF_TRACE_ERROR("esif_ccb_library_load() %s failed [%s (%d)]: %s\n", libPath, esif_rc_str(rc), rc, esif_ccb_library_errormsg(conjurePtr->fLibHandle));
		goto exit;
	}
	ESIF_TRACE_DEBUG("esif_ccb_library_load() %s completed.\n", libPath);

	iface_func_ptr = (GetIfaceFuncPtr)esif_ccb_library_get_func(conjurePtr->fLibHandle, (EsifString)iface_func_name);
	if (NULL == iface_func_ptr) {
		rc = esif_ccb_library_error(conjurePtr->fLibHandle);
		ESIF_TRACE_ERROR("esif_ccb_library_get_func() %s failed [%s (%d)]: %s\n", iface_func_name, esif_rc_str(rc), rc, esif_ccb_library_errormsg(conjurePtr->fLibHandle));
		goto exit;
	}

	ESIF_TRACE_DEBUG("esif_ccb_library_get_func() %s completed.\n", iface_func_name);
	rc = ConjureCreate(conjurePtr, iface_func_ptr);
	if (ESIF_OK != rc) {
		ESIF_TRACE_DEBUG("ConjureCreate failed.\n");
		goto exit;
	}
	ESIF_TRACE_DEBUG("ConjureCreate completed.\n");

exit:
	if (ESIF_OK != rc) {
		esif_ccb_library_unload(conjurePtr->fLibHandle);
		conjurePtr->fLibHandle = NULL;
	}
	return rc;
}


eEsifError EsifConjureStop(EsifCnjPtr conjurePtr)
{
	eEsifError rc = ESIF_OK;
	ESIF_ASSERT(conjurePtr != NULL);

	rc = conjurePtr->fInterface.fConjureDestroyFuncPtr(conjurePtr->fHandle);
	if (ESIF_OK == rc) {
		esif_ccb_free(conjurePtr->fLibNamePtr);
		esif_ccb_library_unload(conjurePtr->fLibHandle);
		memset(conjurePtr, 0, sizeof(*conjurePtr));
	}
	return rc;
}


/* Find Conjure Instance From Name */
EsifCnjPtr esif_uf_conjure_get_instance_from_name(esif_string lib_name)
{
	UInt8 i = 0;
	EsifCnjPtr a_conjure_ptr = NULL;

	for (i = 0; i < ESIF_MAX_CONJURES; i++) {
		a_conjure_ptr = &g_cnjMgr.fEnrtries[i];

		if (NULL == a_conjure_ptr->fLibNamePtr) {
			continue;
		}

		if (!esif_ccb_stricmp(a_conjure_ptr->fLibNamePtr, lib_name)) {
			return a_conjure_ptr;
		}
	}
	return NULL;
}


eEsifError EsifCnjInit()
{
	return ESIF_OK;
}


void EsifCnjExit()
{
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

