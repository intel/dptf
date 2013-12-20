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

// #define ESIF_TRACE_DEBUG_DISABLED

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
typedef eEsifError (*GetIfaceFuncPtr)(EsifConjureInterfacePtr);

/* Friends */
extern EsifCnjMgr g_cnjMgr;

static eEsifError RegisterParticipant (const EsifParticipantIfacePtr piPtr)
{
	eEsifError rc    = ESIF_OK;
	EsifUpPtr up_ptr = NULL;
	UInt8 temp = 0;
	char guid_str[ESIF_GUID_PRINT_SIZE];
	EsifString dsp_code = "";

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

	if(EsifUpManagerDoesAvailableParticipantExistByName(piPtr->name)) {
		rc = ESIF_E_UNSPECIFIED;
		up_ptr = NULL;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Create Participant\n");
	up_ptr = EsifUpManagerCreateParticipant(eParticipantOriginUF, &temp, piPtr);
	ESIF_TRACE_DEBUG("Create Participant up_ptr %p\n", up_ptr);

	/* Assign DSP Now */
	if (NULL == up_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	dsp_code = esif_uf_dm_select_dsp(eParticipantOriginUF, piPtr);
	if (NULL == dsp_code) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}
	ESIF_TRACE_DEBUG("dsp_code %s\n", dsp_code);

	up_ptr->fDspPtr = esif_uf_dm_select_dsp_by_code(dsp_code);
	if (NULL == up_ptr->fDspPtr) {
		ESIF_TRACE_DEBUG("Missed DSP Lookup %s\n", dsp_code);
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}
	/* Report the participant to the apps */
	EsifAppMgrCreateCreateParticipantInAllApps(up_ptr);

	ESIF_TRACE_DEBUG("Hit DSP Lookup %s\n", up_ptr->fDspPtr->code_ptr);
	up_ptr = NULL;	/* Indicate the participant should not be destroyed. */

exit:

	/*
	 * If up_ptr is non-NULL at this point, indicates a failure and the
	 * participant must be disabled in the manager.  If everything is
	 * successful, up_ptr will be set to NULL above.
	 */
	if (up_ptr != NULL) {
		EsifUpManagerUnregisterParticipant(eParticipantOriginUF, up_ptr);
	}
	return rc;
}


static eEsifError UnRegisterParticipant (const EsifParticipantIfacePtr pi)
{
	UNREFERENCED_PARAMETER(pi);
	ESIF_TRACE_DEBUG("%s\n", ESIF_FUNC);

	return ESIF_E_NOT_IMPLEMENTED;
}


/* Create A Conjure Library */
static eEsifError ConjureCreate (
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

	ESIF_TRACE_DEBUG("%s\n\n"
					 "Conjure Lib Name   : %s\n"
					 "Conjure Lib Desc   : %s\n"
					 "Conjure Lib Type   : %s\n"
					 "Conjure Lib Version: %s\n\n",
					 ESIF_FUNC,
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
	return ESIF_OK;
}


/* Start Conjure Library */
eEsifError EsifConjureStart (EsifCnjPtr conjurePtr)
{
	eEsifError rc = ESIF_OK;
	GetIfaceFuncPtr iface_func_ptr = NULL;
	EsifString iface_func_name     = "GetConjureInterface";

	char libPath[ESIF_LIBPATH_LEN];
	esif_lib_t lib_handle = 0;

	ESIF_TRACE_DEBUG("%s name=%s\n", ESIF_FUNC, conjurePtr->fLibNamePtr);
	// esif_ccb_sprintf(128, libPath, "./%s.%s", conjurePtr->fLibNamePtr, ESIF_LIB_EXT);
	esif_ccb_sprintf(ESIF_LIBPATH_LEN, libPath, "%s.%s",
					 esif_build_path(libPath, ESIF_LIBPATH_LEN, ESIF_DIR_PRG, conjurePtr->fLibNamePtr), ESIF_LIB_EXT);
	lib_handle = esif_ccb_library_load(libPath);

	if (0 == lib_handle) {
		rc = ESIF_E_UNSPECIFIED;
		ESIF_TRACE_DEBUG("%s esif_ccb_library_load() %s failed.\n", ESIF_FUNC, libPath);
		goto exit;
	}
	ESIF_TRACE_DEBUG("%s esif_ccb_library_load() %s completed.\n", ESIF_FUNC, libPath);

	iface_func_ptr = (GetIfaceFuncPtr)esif_ccb_library_get_func(lib_handle, (EsifString)iface_func_name);
	if (NULL == iface_func_ptr) {
		rc = ESIF_E_UNSPECIFIED;
		ESIF_TRACE_DEBUG("%s esif_ccb_library_get_func() %s failed.\n", ESIF_FUNC, iface_func_name);
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s esif_ccb_library_get_func() %s completed.\n", ESIF_FUNC, iface_func_name);
	rc = ConjureCreate(conjurePtr, iface_func_ptr);
	ESIF_TRACE_DEBUG("%s ConjureCreate completed.\n", ESIF_FUNC);
exit:
	return rc;
}


eEsifError EsifConjureStop (EsifCnjPtr conjurePtr)
{
	eEsifError rc = ESIF_OK;
	ESIF_ASSERT(conjurePtr != NULL);

	rc = conjurePtr->fInterface.fConjureDestroyFuncPtr(conjurePtr->fHandle);
	if (ESIF_OK == rc) {
		memset(conjurePtr, 0, sizeof(*conjurePtr));
	}
	return rc;
}


/* Find Conjure Instance From Name */
EsifCnjPtr esif_uf_conjure_get_instance_from_name (esif_string lib_name)
{
	UInt8 i = 0;
	EsifCnjPtr a_conjure_ptr = NULL;

	for (i = 0; i < ESIF_MAX_CONJURES; i++) {
		a_conjure_ptr = &g_cnjMgr.fEnrtries[i];

		if (NULL == a_conjure_ptr->fLibNamePtr) {
			continue;
		}

		if (!strcmp(a_conjure_ptr->fLibNamePtr, lib_name)) {
			return a_conjure_ptr;
		}
	}
	return NULL;
}


eEsifError EsifCnjInit ()
{
	return ESIF_OK;
}


void EsifCnjExit ()
{
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

