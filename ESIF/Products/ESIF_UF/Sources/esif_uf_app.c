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

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_APP

#include "esif_uf.h"				/* Upper Framework */
#include "esif_uf_app.h"
#include "esif_uf_appmgr.h"			/* Application Manager */
#include "esif_uf_cfgmgr.h"			/* Configuration Manager */
#include "esif_uf_log.h"			/* Logging */
#include "esif_sdk_iface_esif.h"		/* ESIF Service Interface */
#include "esif_uf_service.h"		/* ESIF Service */
#include "esif_uf_fpc.h"
#include "esif_dsp.h"
#include "esif_pm.h"
#include "esif_uf_eventmgr.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

typedef enum EventCategory_e {
	EVENT_CATEGORY_NONE = 0,
	EVENT_CATEGORY_APP,
	EVENT_CATEGORY_PARTICIPANT,
	EVENT_CATEGORY_DOMAIN,
	EVENT_CATEGORY_MAX
} EventCategory, *EventCategoryPtr;


static esif_string g_qualifiers[] = {
	"D0",
	"D1",
	"D2",
	"D3",
	"D4",
	"D5",
	"D6",
	"D7",
	"D8",
	"D9"
};

static eEsifError ESIF_CALLCONV EsifApp_EventCallback(
	void *contextPtr,
	UInt8 participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	);

static AppDomainDataMapPtr EsifApp_GetDomainDataMapByHandle(
	const AppParticipantDataMapPtr upMapPtr,
	const void *domainHandle
	);

static AppParticipantDataMapPtr EsifApp_GetParticipantDataMapFromInstance(
	const EsifAppPtr appPtr,
	const u8 participantId
	);

static ESIF_INLINE EventCategory EsifApp_CategorizeEvent(
	const void *appHandle,
	const void *upHandle,
	const void *domainHandle
	);

static eEsifError EsifApp_RegisterParticipantsWithApp(
	EsifAppPtr self
	);

static eEsifError EsifApp_DestroyParticipants(EsifAppPtr self);

/* Data For Interface Marshaling */
static AppDataPtr CreateAppData(esif_string pathBuf)
{
	AppDataPtr app_data_ptr = NULL;
	char policyPath[ESIF_PATH_LEN] = { 0 };

	if (NULL == pathBuf) {
		ESIF_TRACE_ERROR("Path buffer is NULL\n");
		goto exit;
	}

	/* Build path(s) for DPTF: "HomeDir" or "HomeDir|[#]PolicyDir" */
	esif_build_path(pathBuf, ESIF_PATH_LEN, ESIF_PATHTYPE_DPTF, NULL, NULL);
	esif_build_path(policyPath, sizeof(policyPath), ESIF_PATHTYPE_DLL, NULL, NULL);
	if (esif_ccb_strcmp(pathBuf, policyPath) != 0) {
		char policyDummyPath[ESIF_PATH_LEN] = { 0 }; /* empty if path starts with "#" */
		esif_build_path(policyDummyPath, sizeof(policyDummyPath), ESIF_PATHTYPE_DLL, "", NULL);
		esif_ccb_sprintf_concat(ESIF_PATH_LEN, pathBuf, "|%s%s", (policyDummyPath[0] ? "" : "#"), policyPath);
	}
	ESIF_TRACE_DEBUG("pathBuf=%s\n\n", (esif_string)pathBuf);

	app_data_ptr = (AppDataPtr)esif_ccb_malloc(sizeof(AppData));
	if (NULL == app_data_ptr) {
		ESIF_TRACE_ERROR("Fail to allocate AppData\n");
		goto exit;
	}

	app_data_ptr->fPathHome.buf_ptr  = (void *)pathBuf;
	app_data_ptr->fPathHome.buf_len  = ESIF_PATH_LEN;
	app_data_ptr->fPathHome.data_len = (UInt32)esif_ccb_strlen(pathBuf, ESIF_PATH_LEN);
	app_data_ptr->fPathHome.type     = ESIF_DATA_STRING;
	app_data_ptr->fLogLevel          = (eLogType) g_traceLevel;

exit:

	return app_data_ptr;
}


/*
    IMPLEMENT EsifAppInterface
 */
typedef eEsifError (ESIF_CALLCONV *GetIfaceFuncPtr)(AppInterfacePtr);

static eEsifError AppCreate(
	EsifAppPtr appPtr,
	GetIfaceFuncPtr ifaceFuncPtr
	)
{
	eEsifError rc = ESIF_OK;
	AppDataPtr app_data_ptr = NULL;
	char path_buf[ESIF_PATH_LEN];

	char name[ESIF_NAME_LEN];
	ESIF_DATA(data_name, ESIF_DATA_STRING, name, ESIF_NAME_LEN);

	char desc[ESIF_DESC_LEN];
	ESIF_DATA(data_desc, ESIF_DATA_STRING, desc, ESIF_DESC_LEN);

	char version[ESIF_DESC_LEN];
	ESIF_DATA(data_version, ESIF_DATA_STRING, version, ESIF_DESC_LEN);

	#define BANNER_LEN 1024
	char banner[BANNER_LEN];
	ESIF_DATA(data_banner, ESIF_DATA_STRING, banner, BANNER_LEN);

	esif_string app_type_ptr = NULL;
	EsifInterface app_service_iface;

	ESIF_ASSERT(appPtr != NULL);
	ESIF_ASSERT(ifaceFuncPtr != NULL);

	/* Assign the EsifInterface Functions */
	app_service_iface.fIfaceType              = eIfaceTypeEsifService;
	app_service_iface.fIfaceVersion           = ESIF_INTERFACE_VERSION;
	app_service_iface.fIfaceSize              = (UInt16)sizeof(EsifInterface);

	app_service_iface.fGetConfigFuncPtr       = EsifSvcConfigGet;
	app_service_iface.fSetConfigFuncPtr       = EsifSvcConfigSet;
	app_service_iface.fPrimitiveFuncPtr       = EsifSvcPrimitiveExec;
	app_service_iface.fWriteLogFuncPtr        = EsifSvcWriteLog;
	app_service_iface.fRegisterEventFuncPtr   = EsifSvcEventRegister;
	app_service_iface.fUnregisterEventFuncPtr = EsifSvcEventUnregister;

	/* GetApplicationInterface Handleshake send ESIF receive APP Interface */
	rc = ifaceFuncPtr(&appPtr->fInterface);
	if (ESIF_OK != rc) {
		goto exit;
	}

	/* Check EsifAppInterface */
	if (appPtr->fInterface.fIfaceType != eIfaceTypeApplication ||
		appPtr->fInterface.fIfaceSize != (UInt16)sizeof(AppInterface) ||
		appPtr->fInterface.fIfaceVersion != APP_INTERFACE_VERSION ||

		/* Functions Pointers */
		appPtr->fInterface.fAppAllocateHandleFuncPtr == NULL ||
		appPtr->fInterface.fAppCreateFuncPtr == NULL ||
		appPtr->fInterface.fAppDestroyFuncPtr == NULL ||
		appPtr->fInterface.fAppCommandFuncPtr == NULL ||
		appPtr->fInterface.fAppEventFuncPtr == NULL ||
		appPtr->fInterface.fAppGetAboutFuncPtr == NULL ||
		appPtr->fInterface.fAppGetBannerFuncPtr == NULL ||
		appPtr->fInterface.fAppGetDescriptionFuncPtr == NULL ||
		appPtr->fInterface.fAppGetGuidFuncPtr == NULL ||
		appPtr->fInterface.fAppGetNameFuncPtr == NULL ||
		appPtr->fInterface.fAppGetStatusFuncPtr == NULL ||
		appPtr->fInterface.fAppGetVersionFuncPtr == NULL ||
		appPtr->fInterface.fParticipantAllocateHandleFuncPtr == NULL ||
		appPtr->fInterface.fParticipantCreateFuncPtr == NULL ||
		appPtr->fInterface.fParticipantDestroyFuncPtr == NULL ||
		appPtr->fInterface.fParticipantSetStateFuncPtr == NULL ||
		appPtr->fInterface.fDomainAllocateHandleFuncPtr == NULL ||
		appPtr->fInterface.fDomainCreateFuncPtr == NULL ||
		appPtr->fInterface.fDomainDestroyFuncPtr == NULL ||
		appPtr->fInterface.fDomainSetStateFuncPtr == NULL ||
		appPtr->fInterface.fAppSetStateFuncPtr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Callback for application information */
	rc = appPtr->fInterface.fAppGetNameFuncPtr(&data_name);
	if (ESIF_OK != rc) {
		goto exit;
	}
	rc = appPtr->fInterface.fAppGetDescriptionFuncPtr(&data_desc);
	if (ESIF_OK != rc) {
		goto exit;
	}
	rc = appPtr->fInterface.fAppGetVersionFuncPtr(&data_version);
	if (ESIF_OK != rc) {
		goto exit;
	}
	app_type_ptr = "plugin";


	ESIF_TRACE_DEBUG("\n\n"
					 "Application Name   : %s\n"
					 "Application Desc   : %s\n"
					 "Application Type   : %s\n"
					 "Application Version: %s\n\n",
					 (esif_string)data_name.buf_ptr,
					 (esif_string)data_desc.buf_ptr,
					 (esif_string)app_type_ptr,
					 (esif_string)data_version.buf_ptr);

	/* Ask for the application handle to be allocated */
	rc = appPtr->fInterface.fAppAllocateHandleFuncPtr(&appPtr->fHandle);
	if (ESIF_OK != rc) {
		goto exit;
	}

	app_data_ptr = CreateAppData(path_buf);
	if (NULL == app_data_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	/* Create the application */
	rc = appPtr->fInterface.fAppCreateFuncPtr(&app_service_iface,
											  appPtr,
											  appPtr->fHandle,
											  app_data_ptr,
											  eAppStateEnabled);
	esif_ccb_free(app_data_ptr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	rc = appPtr->fInterface.fAppGetBannerFuncPtr(appPtr->fHandle, &data_banner);
	if (ESIF_OK != rc) {
		goto exit;
	}

	CMD_OUT("%s\n", (esif_string)data_banner.buf_ptr);

exit:

	return rc;
}


#define ASSIGN_DATA_STRING(field, buffer, buffer_len) \
	field.buf_ptr  = buffer; \
	field.buf_len  = buffer_len; \
	field.data_len = (UInt32)(esif_ccb_strlen(buffer, buffer_len) + 1); \
	field.type     = ESIF_DATA_STRING;

#define ASSIGN_DATA_GUID(field, buffer) \
	field.buf_ptr  = buffer; \
	field.buf_len  = ESIF_GUID_LEN; \
	field.data_len = ESIF_GUID_LEN; \
	field.type     = ESIF_DATA_GUID;


/* Data For Interface Marshaling */
static AppDomainDataPtr CreateDomainData(const struct esif_fpc_domain *domainPtr, const EsifUpDomainPtr upDomainPtr)
{
	AppDomainDataPtr dom_data_ptr = (AppDomainDataPtr)esif_ccb_malloc(sizeof(AppDomainData));

	ESIF_TRACE_DEBUG("%s\n", domainPtr->descriptor.name);

	if (NULL == dom_data_ptr) {
		goto exit;
	}

	dom_data_ptr->fName.buf_ptr  = (void *)domainPtr->descriptor.name;
	dom_data_ptr->fName.buf_len  = ESIF_NAME_LEN;
	dom_data_ptr->fName.data_len = (UInt32)esif_ccb_strlen(domainPtr->descriptor.name, ESIF_NAME_LEN);
	dom_data_ptr->fName.type     = ESIF_DATA_STRING;

	dom_data_ptr->fDescription.buf_ptr  = (void *)domainPtr->descriptor.description;
	dom_data_ptr->fDescription.buf_len  = ESIF_DESC_LEN;
	dom_data_ptr->fDescription.data_len = (UInt32)esif_ccb_strlen(domainPtr->descriptor.description, ESIF_DESC_LEN);
	dom_data_ptr->fDescription.type     = ESIF_DATA_STRING;

	dom_data_ptr->fGuid.buf_ptr  = (void *)domainPtr->descriptor.guid;
	dom_data_ptr->fGuid.buf_len  = ESIF_GUID_LEN;
	dom_data_ptr->fGuid.data_len = ESIF_GUID_LEN;
	dom_data_ptr->fGuid.type     = ESIF_DATA_GUID;

	dom_data_ptr->fVersion    = APP_DOMAIN_VERSION;
	dom_data_ptr->fType       = (enum esif_domain_type)domainPtr->descriptor.domainType;
	dom_data_ptr->fCapability = upDomainPtr->capability_for_domain.capability_flags;
	esif_ccb_memcpy(dom_data_ptr->fCapabilityBytes, upDomainPtr->capability_for_domain.capability_mask, 32);

exit:

	return dom_data_ptr;
}


static eEsifError CreateDomain(
	UInt8 domainId,
	EsifAppPtr appPtr,
	AppParticipantDataMapPtr participantDataMapPtr,
	struct esif_fpc_domain *domainPtr,
	EsifUpDomainPtr upDomainPtr
	)
{
	eEsifError rc = ESIF_OK;
	AppDomainDataPtr domain_data_ptr = NULL;
	void *domain_handle = NULL;

	ESIF_TRACE_DEBUG("Create Domain %s\n", domainPtr->descriptor.name);
	if (NULL == appPtr || NULL == domainPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	domain_data_ptr = CreateDomainData(domainPtr, upDomainPtr);
	if (NULL == domain_data_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Have Domain Data %d\n", domainId);

	rc = appPtr->fInterface.fDomainAllocateHandleFuncPtr(
			appPtr->fHandle,
			participantDataMapPtr->fAppParticipantHandle,
			&domain_handle);

	if (ESIF_OK == rc && NULL == domain_handle) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	} else if (ESIF_OK == rc && domainId < sizeof(g_qualifiers) / sizeof(esif_string)) {
		participantDataMapPtr->fDomainData[domainId].fAppDomainId      = domainId;
		participantDataMapPtr->fDomainData[domainId].fAppDomainHandle  = domain_handle;
		participantDataMapPtr->fDomainData[domainId].fAppDomainDataPtr = domain_data_ptr;
		participantDataMapPtr->fDomainData[domainId].fQualifier = g_qualifiers[domainId];
		participantDataMapPtr->fDomainData[domainId].fQualifierId      = *(u16 *)g_qualifiers[domainId];

		ESIF_TRACE_DEBUG("DomainMap(%u): Name %s Esif(%s) %p Mapped To Handle 0x%p\n",
						 domainId,
						 (esif_string)domain_data_ptr->fName.buf_ptr,
						 participantDataMapPtr->fDomainData[domainId].fQualifier,
						 participantDataMapPtr->fDomainData[domainId].fAppDomainDataPtr,
						 participantDataMapPtr->fDomainData[domainId].fAppDomainHandle);

		rc = appPtr->fInterface.fDomainCreateFuncPtr(
				appPtr->fHandle,
				participantDataMapPtr->fAppParticipantHandle,
				domain_handle,
				domain_data_ptr,
				eDomainStateEnabled);
	} else {
		ESIF_TRACE_DEBUG("DomainMap(%u): Domain Mapping Error %s(%d)\n",
			domainId,
			esif_rc_str(rc), rc);
	}

exit:
	esif_ccb_free(domain_data_ptr);
	return rc;
}


static eEsifError CreateDomains(
	EsifAppPtr appPtr,
	EsifUpPtr upPtr,
	AppParticipantDataMapPtr participantDataMapPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifDspPtr dspPtr = NULL;
	UInt32 domainCount = 0;
	UInt32 domainIndex = 0;
	EsifUpDomainPtr upDomainPtr = NULL;

	ESIF_ASSERT(appPtr != NULL);
	ESIF_ASSERT(upPtr != NULL);
	ESIF_ASSERT(participantDataMapPtr != NULL);

	ESIF_TRACE_DEBUG("Create Domains\n");

	dspPtr = EsifUp_GetDsp(upPtr);
	if (NULL == dspPtr) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}

	domainCount = dspPtr->get_domain_count(dspPtr);
	for (domainIndex = 0; domainIndex < domainCount; domainIndex++) {
		EsifFpcDomainPtr domainPtr = dspPtr->get_domain(dspPtr, domainIndex);
		if (NULL == domainPtr) {
			continue;
		}

		upDomainPtr = EsifUp_GetDomainById(upPtr, domainPtr->descriptor.domain);
		if (upDomainPtr) {
			rc = CreateDomain((UInt8)domainIndex, appPtr, participantDataMapPtr, domainPtr, upDomainPtr);
			if (ESIF_OK != rc) {
				goto exit;
			}
			ESIF_TRACE_DEBUG("Create Domain %s\n", domainPtr->descriptor.name);
		}
	}
exit:
	return rc;
}


/* Data For Interface Marshaling */
static AppParticipantDataPtr CreateParticipantData(
	const EsifUpPtr upPtr,
	const EsifUpDataPtr upDataPtr
	)
{
	AppParticipantDataPtr appDataPtr = NULL;
	EsifDspPtr dspPtr = NULL;

	ESIF_ASSERT(upPtr != NULL);
	ESIF_ASSERT(upDataPtr != NULL);

	dspPtr = EsifUp_GetDsp(upPtr);
	if (dspPtr == NULL) {
		goto exit;
	}

	appDataPtr = (AppParticipantDataPtr)esif_ccb_malloc(sizeof(AppParticipantData));
	if (NULL == appDataPtr) {
		goto exit;
	}

	/* Common */
	appDataPtr->fVersion = upDataPtr->fVersion;
	ASSIGN_DATA_GUID(appDataPtr->fDriverType, upDataPtr->fDriverType);
	ASSIGN_DATA_GUID(appDataPtr->fDeviceType, upDataPtr->fDriverType);
	ASSIGN_DATA_STRING(appDataPtr->fName, upDataPtr->fName, ESIF_NAME_LEN);
	ASSIGN_DATA_STRING(appDataPtr->fDesc, upDataPtr->fDesc, ESIF_DESC_LEN);

	ASSIGN_DATA_STRING(appDataPtr->fDriverName, upDataPtr->fDriverName, ESIF_NAME_LEN);
	ASSIGN_DATA_STRING(appDataPtr->fDeviceName, upDataPtr->fDeviceName, ESIF_NAME_LEN);
	ASSIGN_DATA_STRING(appDataPtr->fDevicePath, upDataPtr->fDevicePath, ESIF_PATH_LEN);

	appDataPtr->fDomainCount   = (UInt8)dspPtr->get_domain_count(dspPtr);
	appDataPtr->fBusEnumerator = upDataPtr->fEnumerator;

	/* ACPI Device */
	ASSIGN_DATA_STRING(appDataPtr->fAcpiDevice, upDataPtr->fAcpiDevice, ESIF_NAME_LEN);
	ASSIGN_DATA_STRING(appDataPtr->fAcpiScope, upDataPtr->fAcpiScope, ESIF_SCOPE_LEN);
	appDataPtr->fAcpiType = upDataPtr->fAcpiType;
	ASSIGN_DATA_STRING(appDataPtr->fAcpiUID, upDataPtr->fAcpiUID, sizeof(appDataPtr->fAcpiUID));

	/* PCI Device */
	appDataPtr->fPciVendor    = upDataPtr->fPciVendor;
	appDataPtr->fPciDevice    = upDataPtr->fPciDevice;
	appDataPtr->fPciBus       = upDataPtr->fPciBus;
	appDataPtr->fPciBusDevice = upDataPtr->fPciBusDevice;
	appDataPtr->fPciFunction  = upDataPtr->fPciFunction;
	appDataPtr->fPciRevision  = upDataPtr->fPciRevision;
	appDataPtr->fPciClass     = upDataPtr->fPciClass;
	appDataPtr->fPciSubClass  = upDataPtr->fPciSubClass;
	appDataPtr->fPciProgIf    = upDataPtr->fPciProgIf;

exit:
	return appDataPtr;
}


eEsifError EsifAppCreateParticipant(
	const EsifAppPtr appPtr,
	const EsifUpPtr upPtr
	)
{
	eEsifError rc = ESIF_OK;
	void *participant_handle = NULL;
	AppParticipantDataPtr participant_data_ptr = NULL;
	AppParticipantDataMapPtr participantDataMapPtr = NULL;

	if (NULL == appPtr || NULL == upPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	participantDataMapPtr = &appPtr->fParticipantData[EsifUp_GetInstance(upPtr)];

	// Exit if llready registered in this app
	if (participantDataMapPtr->fUpPtr != NULL) {
		goto exit;
	}

	/* Create participant metadata to marshall though interface */
	participant_data_ptr = CreateParticipantData(upPtr, EsifUp_GetMetadata(upPtr));
	if (NULL == participant_data_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	/* Create particpant Handle */
	rc = appPtr->fInterface.fParticipantAllocateHandleFuncPtr(
			appPtr->fHandle,
			&participant_handle);

	if ((rc != ESIF_OK) || (NULL == participant_handle)) {
		esif_ccb_free(participant_data_ptr);
		participant_data_ptr = NULL;

		if (NULL == participant_handle) {
			rc = ESIF_E_INVALID_HANDLE;
		} else {
			ESIF_TRACE_DEBUG("Participant(%u) Mapping Error %s(%d)\n", EsifUp_GetInstance(upPtr), esif_rc_str(rc), rc);
		}
		goto exit;
	}

	/* get reference on participant since we save a copy of pointer for later use*/
	rc = EsifUp_GetRef(upPtr);
	if (ESIF_OK != rc) {
		participantDataMapPtr->fUpPtr = NULL;
		goto exit;
	}

	participantDataMapPtr->fAppParticipantHandle = participant_handle;	/* Application Participant */
	participantDataMapPtr->fUpPtr = upPtr;								/* ESIF Participant */

	ESIF_TRACE_DEBUG("Participant(%u) Esif 0x%p Mapped To Handle 0x%p\n",
						EsifUp_GetInstance(upPtr),
						participantDataMapPtr->fUpPtr,
						participantDataMapPtr->fAppParticipantHandle);

	/* Call through the interface to create the participant instance in the app. */
	rc = appPtr->fInterface.fParticipantCreateFuncPtr(
			appPtr->fHandle,
			participant_handle,
			participant_data_ptr,
			eParticipantStateEnabled);
	if (ESIF_OK != rc) {
		EsifUp_PutRef(upPtr);
		participantDataMapPtr->fAppParticipantHandle = NULL;	/* Application Participant */
		participantDataMapPtr->fUpPtr = NULL;					/* ESIF Participant */
		goto exit;
	}

	rc = CreateDomains(appPtr, upPtr, participantDataMapPtr);
exit:
	if (participant_data_ptr) {
		esif_ccb_free(participant_data_ptr);
		participant_data_ptr = NULL;
	}
	return rc;
}


static eEsifError DestroyDomain(
	EsifAppPtr appPtr,
	AppParticipantDataMapPtr participantDataMapPtr,
	AppDomainDataMapPtr domainDataMapPtr
	)
{
	eEsifError rc = ESIF_OK;

	rc = appPtr->fInterface.fDomainDestroyFuncPtr(
			appPtr->fHandle,
			participantDataMapPtr->fAppParticipantHandle,
			domainDataMapPtr->fAppDomainHandle);

	if (ESIF_OK == rc) {
		ESIF_TRACE_DEBUG("DomainMap(%u) Esif 0x%p UnMapped From Handle 0x%p\n",
						 domainDataMapPtr->fAppDomainId,
						 domainDataMapPtr->fAppDomainDataPtr,
						 domainDataMapPtr->fAppDomainHandle);
	} else {
		ESIF_TRACE_DEBUG("DomainMap(%u) Esif UnMapping Error %s(%d)\n",
						 domainDataMapPtr->fAppDomainId,
						 esif_rc_str(rc), rc);
	}

	memset(domainDataMapPtr, 0, sizeof(*domainDataMapPtr));
	return rc;
}


static eEsifError DestroyDomains(
	EsifAppPtr appPtr,
	AppParticipantDataMapPtr participantDataMapPtr
	)
{
	eEsifError rc = ESIF_OK;
	UInt8 i = 0;

	ESIF_TRACE_DEBUG("Destroy Domains\n");
	for (i = 0; i < MAX_DOMAIN_ENTRY; i++) {
		AppDomainDataMapPtr domainDataMapPtr = &participantDataMapPtr->fDomainData[i];
		if (NULL == domainDataMapPtr->fAppDomainHandle) {
			continue;
		}
		rc = DestroyDomain(appPtr, participantDataMapPtr, domainDataMapPtr);
	}
	return rc;
}


eEsifError EsifAppDestroyParticipant(
	const EsifAppPtr appPtr,
	const EsifUpPtr upPtr
	)
{
	eEsifError rc = ESIF_OK;
	AppParticipantDataMapPtr participant_data_map_ptr = NULL;

	if (NULL == appPtr || NULL == upPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	participant_data_map_ptr = &appPtr->fParticipantData[EsifUp_GetInstance(upPtr)];

	// If created as NULL no need for callback.
	if (NULL == participant_data_map_ptr ||
		NULL == participant_data_map_ptr->fAppParticipantHandle) {
		goto exit;
	}

	rc = DestroyDomains(appPtr, participant_data_map_ptr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	rc = appPtr->fInterface.fParticipantDestroyFuncPtr(
			appPtr->fHandle,
			participant_data_map_ptr->fAppParticipantHandle);

	if (ESIF_OK == rc) {
		ESIF_TRACE_DEBUG("ParticipantMap(%u) Esif 0x%p UnMapped From Handle 0x%p\n",
						 EsifUp_GetInstance(participant_data_map_ptr->fUpPtr),
						 participant_data_map_ptr->fUpPtr,
						 participant_data_map_ptr->fAppParticipantHandle);
	} else {
		ESIF_TRACE_DEBUG("ParticipantMap(%u) UnMapping Error %s(%d)\n",
						 EsifUp_GetInstance(participant_data_map_ptr->fUpPtr),
						 esif_rc_str(rc), rc);
	}

exit:
	if (participant_data_map_ptr != NULL) {
		if (participant_data_map_ptr->fUpPtr != NULL) {
			/* release reference on participant since we get reference on it in EsifAppCreateParticipant */
			EsifUp_PutRef(participant_data_map_ptr->fUpPtr);
		}

		memset(participant_data_map_ptr, 0, sizeof(*participant_data_map_ptr));
	}

	return rc;
}


/*
** PUBLIC
*/

eEsifError EsifAppStart(EsifAppPtr appPtr)
{
	ESIF_TRACE_ENTRY_INFO();

	eEsifError rc = ESIF_OK;
	GetIfaceFuncPtr iface_func_ptr = NULL;
	esif_string iface_func_name    = GET_APPLICATION_INTERFACE_FUNCTION;

	char libPath[ESIF_LIBPATH_LEN];
	ESIF_TRACE_DEBUG("name=%s\n", appPtr->fLibNamePtr);
	esif_build_path(libPath, ESIF_LIBPATH_LEN, ESIF_PATHTYPE_DLL, appPtr->fLibNamePtr, ESIF_LIB_EXT);
	appPtr->fLibHandle = esif_ccb_library_load(libPath);

	if (NULL == appPtr->fLibHandle || NULL == appPtr->fLibHandle->handle) {
		rc = esif_ccb_library_error(appPtr->fLibHandle);
		ESIF_TRACE_DEBUG("esif_ccb_library_load() %s failed [%s (%d)]: %s\n", libPath, esif_rc_str(rc), rc, esif_ccb_library_errormsg(appPtr->fLibHandle));
		goto exit;
	}

	ESIF_TRACE_DEBUG("esif_ccb_library_load() %s completed.\n", libPath);

	iface_func_ptr = (GetIfaceFuncPtr)esif_ccb_library_get_func(appPtr->fLibHandle, (char*)iface_func_name);
	if (NULL == iface_func_ptr) {
		rc = esif_ccb_library_error(appPtr->fLibHandle);
		ESIF_TRACE_DEBUG("esif_ccb_library_get_func() %s failed [%s (%d)]: %s\n", iface_func_name, esif_rc_str(rc), rc, esif_ccb_library_errormsg(appPtr->fLibHandle));
		goto exit;
	}

	ESIF_TRACE_DEBUG("esif_ccb_library_get_func() %s completed.\n", iface_func_name);
	rc = AppCreate(appPtr, iface_func_ptr);
	if (ESIF_OK != rc) {
		ESIF_TRACE_DEBUG("AppCreate failed.\n");
		goto exit;
	}
	ESIF_TRACE_DEBUG("AppCreate completed.\n");

	rc = EsifApp_RegisterParticipantsWithApp(appPtr);
	if (ESIF_OK != rc) {
		ESIF_TRACE_DEBUG("EsifApp_RegisterParticipantsWithApp failed.\n");
		goto exit;
	}
	ESIF_TRACE_DEBUG("EsifApp_RegisterParticipantsWithApp completed.\n");

exit:

	if (ESIF_OK != rc) {
		esif_ccb_library_unload(appPtr->fLibHandle);
		appPtr->fLibHandle = NULL;
	}
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


static eEsifError EsifApp_RegisterParticipantsWithApp(
	EsifAppPtr self
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpPtr upPtr = NULL;
	UfPmIterator upIter = {0};

	ESIF_ASSERT(self != NULL);

	rc = EsifUpPm_InitIterator(&upIter);
	if (rc!= ESIF_OK) {
		goto exit;
	}

	/* Skip participant 0: ESIF treats this as a participant no one else does */
	rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	if (rc!= ESIF_OK) {
		goto exit;
	}

	rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	while (ESIF_OK == rc) {
		rc = EsifAppCreateParticipant(self, upPtr);
		if (ESIF_OK != rc) {
			break;
		}
		rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}
exit:
	EsifUp_PutRef(upPtr);	
	if (ESIF_E_ITERATION_DONE == rc) {
		rc = ESIF_OK;
	} 
	ESIF_TRACE_INFO("Register participants with App, status = %s\n", esif_rc_str(rc));
	return rc;
}


eEsifError EsifAppStop(EsifAppPtr appPtr)
{
	eEsifError rc = ESIF_OK;
	ESIF_ASSERT(appPtr != NULL);

	rc = EsifApp_DestroyParticipants(appPtr);
	ESIF_TRACE_DEBUG("EsifUpManagerDestroyParticipantsInApp completed.\n");

	rc = appPtr->fInterface.fAppDestroyFuncPtr(appPtr->fHandle);
	if (ESIF_OK == rc) {
		esif_ccb_library_unload(appPtr->fLibHandle);
		esif_ccb_free(appPtr->fLibNamePtr);
		memset(appPtr, 0, sizeof(*appPtr));
	}

	return rc;
}


static eEsifError EsifApp_DestroyParticipants(EsifAppPtr self)
{
	eEsifError rc = ESIF_OK;
	UfPmIterator upIter = {0};
	EsifUpPtr upPtr = NULL;

	ESIF_ASSERT(self != NULL);

	rc = EsifUpPm_InitIterator(&upIter);
	if (rc != ESIF_OK) {
		goto exit;
	}

	rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	while (ESIF_OK == rc) {
		EsifAppDestroyParticipant(self, upPtr);
		rc = EsifUpPm_GetNextUp(&upIter, &upPtr);
	}
exit:
	EsifUp_PutRef(upPtr);	
	ESIF_TRACE_INFO("Destroy participants in App\n");
	return ESIF_OK;
}


static u8 isEventRegistered(
	u64 RegisteredEvents,
	enum esif_event_type eventType
	)
{
	u8 returnValue = ESIF_FALSE;
	u64 bitMask    = 1;

	//
	// left shift the bitMask so the '1' is in the correct position, and then
	// inspect the bit at that position.
	//
	bitMask     = bitMask << (u32)eventType;
	returnValue = (RegisteredEvents & bitMask) ? 1 : 0;

	return returnValue;
}


/* Lookup participant data for a instance */
static AppParticipantDataMapPtr EsifApp_GetParticipantDataMapFromInstance(
	const EsifAppPtr appPtr,
	const u8 participantId
	)
{
	UInt8 i = 0;
	AppParticipantDataMapPtr upDataMapPtr = NULL;

	/* First Find Upper Framework Pointer */
	EsifUpPtr upPtr = EsifUpPm_GetAvailableParticipantByInstance(participantId);

	if (NULL == upPtr) {
		goto exit;
	}

	ESIF_TRACE_DEBUG("Have Event For Participant %s\n", EsifUp_GetName(upPtr));

	/*
	 * Okay now we have to find the correct particpant handle based on the appPtr.  Note each
	 * app will have a different particpant handle.
	 */
	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		if (NULL == appPtr->fParticipantData[i].fUpPtr) {
			continue;
		}

		if (appPtr->fParticipantData[i].fUpPtr == upPtr) {
			upDataMapPtr = &appPtr->fParticipantData[i];

			ESIF_TRACE_DEBUG("Found participant data map for %s\n", EsifUp_GetName(upPtr));
			break;
		}
	}

exit:
	if (upPtr != NULL) {
		EsifUp_PutRef(upPtr);
	}

	return upDataMapPtr;
}

/* Event handler for events registered for by the application */
static eEsifError ESIF_CALLCONV EsifApp_EventCallback(
	void *appHandle,
	UInt8 participantId,
	UInt16 domainId,
	EsifFpcEventPtr fpcEventPtr,
	EsifDataPtr eventDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr appPtr = NULL;
	EsifUpPtr upPtr = NULL;
	AppParticipantDataMapPtr upDataMapPtr = NULL;
	void *participantHandle = NULL;
	void *domainHandle = NULL;
	EsifData dataGuid     = {ESIF_DATA_GUID, NULL, sizeof(fpcEventPtr->event_guid), sizeof(fpcEventPtr->event_guid)};
	char guidStr[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guidStr);
	UNREFERENCED_PARAMETER(domainId);

	if (NULL == fpcEventPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	appPtr = GetAppFromHandle(appHandle);
	if (appPtr == NULL) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}
	if (NULL == appPtr->fInterface.fAppEventFuncPtr) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	dataGuid.buf_ptr = &fpcEventPtr->event_guid;

	ESIF_TRACE_DEBUG("\n");

	if (ESIF_INSTANCE_LF == participantId) {

		/* Find Our Participant 0 */
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(ESIF_INSTANCE_LF);
		if (NULL == upPtr) {
			rc = ESIF_E_UNSPECIFIED;
			goto exit;
		}
		ESIF_TRACE_DEBUG("Using Participant %s\n", EsifUp_GetName(upPtr));
		ESIF_TRACE_DEBUG("Ring Manager Door Bell:\n");

		EsifUp_PutRef(upPtr);
	} else {
		upDataMapPtr = EsifApp_GetParticipantDataMapFromInstance(appPtr, participantId);
		if (NULL == upDataMapPtr) {
			rc = ESIF_E_PARTICIPANT_NOT_FOUND;
			goto exit;
		}

		participantHandle = upDataMapPtr->fAppParticipantHandle;

		ESIF_TRACE_DEBUG("Using Participant %s\n", EsifUp_GetName(upDataMapPtr->fUpPtr));
		ESIF_TRACE_DEBUG("Ring Participant Door Bell:\n");
	}

	ESIF_TRACE_DEBUG(
		"  appHandle:         %p\n"
		"  participantHandle: %p\n"
		"  domainHandle:      %p\n"
		"  eventGuid:         %s\n",
		appHandle,
		participantHandle,
		domainHandle,
		esif_guid_print((esif_guid_t *)dataGuid.buf_ptr, guidStr));

	rc = appPtr->fInterface.fAppEventFuncPtr(
			appHandle,
			participantHandle,
			NULL,
			eventDataPtr,
			&dataGuid);
exit:
	return rc;
}


/* Provide registration for ESIF event */
eEsifError EsifApp_RegisterEvent(
	const void *esifHandle,
	const void *appHandle,
	const void *upHandle,
	const void *domainHandle,
	const EsifDataPtr eventGuidPtr
	)
{
	eEsifError rc = ESIF_OK;
	char guidStr[ESIF_GUID_PRINT_SIZE];
	UNREFERENCED_PARAMETER(guidStr);

	if (NULL == esifHandle) {
		ESIF_TRACE_ERROR("Invalid esif handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		ESIF_TRACE_ERROR("Invalid app handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if ((NULL == eventGuidPtr) || (NULL == eventGuidPtr->buf_ptr) || (ESIF_DATA_GUID != eventGuidPtr->type)) {
		ESIF_TRACE_ERROR("Invalid event GUID\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("Registering App Event\n\n"
					 "ESIF Handle          : %p\n"
					 "App Handle           : %p\n"
					 "Participant Handle   : %p\n"
					 "Domain Handle        : %p\n"
					 "Event GUID           : %s\n\n",
					 esifHandle,
					 appHandle,
					 upHandle,
					 domainHandle,
					 esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr));

	/* Determine what to do based on provided parameters */
	switch (EsifApp_CategorizeEvent(appHandle, upHandle, domainHandle)) {
	case EVENT_CATEGORY_APP:
	{
		EsifAppPtr appPtr = NULL;
		EsifUpPtr upPtr   = NULL;

		/* Find Our Participant 0 */
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(ESIF_INSTANCE_LF);
		if (NULL == upPtr) {
			ESIF_TRACE_WARN("Participant 0 was not found in UF participant manager\n");
			rc = ESIF_E_UNSPECIFIED;
			goto exit;
		}
		ESIF_TRACE_DEBUG("Using Participant %s\n", EsifUp_GetName(upPtr));

		EsifUp_PutRef(upPtr);

		appPtr = GetAppFromHandle(appHandle);
		if (NULL == appPtr) {
			ESIF_TRACE_ERROR("The app data was not found from app handle\n");
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		rc = EsifEventMgr_RegisterEventByGuid(eventGuidPtr->buf_ptr, 0, EVENT_MGR_DOMAIN_D0, EsifApp_EventCallback, appPtr->fHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_PARTICIPANT:
	{
		/* Register with participant */
		AppParticipantDataMapPtr upMapPtr = NULL;
		EsifAppPtr appPtr = NULL;

		appPtr = GetAppFromHandle(appHandle);
		if (NULL == appPtr) {
			ESIF_TRACE_ERROR("The app data was not found from app handle\n");
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		upMapPtr = EsifApp_GetParticipantDataMapFromHandle(appPtr, upHandle);
		if (NULL == upMapPtr) {
			ESIF_TRACE_ERROR("The app participant data was not found from participant handle\n");
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}
		ESIF_TRACE_DEBUG("Using Participant %s\n", EsifUp_GetName(upMapPtr->fUpPtr));

		rc = EsifEventMgr_RegisterEventByGuid(eventGuidPtr->buf_ptr, EsifUp_GetInstance(upMapPtr->fUpPtr), EVENT_MGR_MATCH_ANY, EsifApp_EventCallback, appPtr->fHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_DOMAIN:
	{
		/* Register with domain */
		AppParticipantDataMapPtr upMapPtr = NULL;
		AppDomainDataMapPtr domainPtr   = NULL;
		EsifAppPtr appPtr = NULL;

		appPtr = GetAppFromHandle(appHandle);
		if (NULL == appPtr) {
			ESIF_TRACE_ERROR("The app data was not found from app handle\n");
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		upMapPtr = EsifApp_GetParticipantDataMapFromHandle(appPtr, upHandle);
		if (NULL == upMapPtr) {
			ESIF_TRACE_ERROR("The app participant data was not found from participant handle\n");
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		domainPtr = EsifApp_GetDomainDataMapByHandle(upMapPtr, domainHandle);
		if (NULL == domainPtr) {
			ESIF_TRACE_ERROR("The app domain data was not found from domain handle\n");
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}
		ESIF_TRACE_DEBUG("Using Participant %s\n", EsifUp_GetName(upMapPtr->fUpPtr));

		rc = EsifEventMgr_RegisterEventByGuid(eventGuidPtr->buf_ptr, EsifUp_GetInstance(upMapPtr->fUpPtr), domainPtr->fQualifierId, EsifApp_EventCallback, appPtr->fHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	default:
		ESIF_TRACE_ERROR("Unknown category type\n");
		rc = ESIF_E_INVALID_HANDLE;
		break;
	}
exit:
	return rc;
}


/* Provide unregistration for previously registered ESIF event */
eEsifError EsifApp_UnregisterEvent(
	const void *esifHandle,
	const void *appHandle,
	const void *upHandle,
	const void *domainHandle,
	const EsifDataPtr eventGuidPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr appPtr = NULL;
	EsifUpPtr upPtr = NULL;
	AppParticipantDataMapPtr upMapPtr = NULL;
	AppDomainDataMapPtr domainPtr   = NULL;
	char guidStr[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guidStr);

	if (NULL == esifHandle) {
		ESIF_TRACE_ERROR("Invalid esif handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if (NULL == appHandle) {
		ESIF_TRACE_ERROR("Invalid app handle\n");
		return ESIF_E_INVALID_HANDLE;
	}

	if ((NULL == eventGuidPtr) || (NULL == eventGuidPtr->buf_ptr) || (ESIF_DATA_GUID != eventGuidPtr->type)) {
		ESIF_TRACE_ERROR("Invalid event GUID\n");
		return ESIF_E_PARAMETER_IS_NULL;
	}

	ESIF_TRACE_DEBUG("Unregistering App event:\n\n"
					 "  ESIF Handle          : %p\n"
					 "  App Handle           : %p\n"
					 "  Participant Handle   : %p\n"
					 "  Domain Handle        : %p\n"
					 "  Event GUID           : %s\n\n",
					 esifHandle,
					 appHandle,
					 upHandle,
					 domainHandle,
					 esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr));

	appPtr = GetAppFromHandle(appHandle);
	if (NULL == appPtr) {
		ESIF_TRACE_ERROR("The app data was not found from app handle\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	/* Determine what to do based on provided parameters */
	switch (EsifApp_CategorizeEvent(appHandle, upHandle, domainHandle)) {
	case EVENT_CATEGORY_APP:
	{
		/* Unregister app  */
		upPtr = EsifUpPm_GetAvailableParticipantByInstance(ESIF_INSTANCE_LF);
		if (NULL == upPtr) {
			ESIF_TRACE_WARN("Participant 0 was not found in UF participant manager\n");
			rc = ESIF_E_UNSPECIFIED;
			goto exit;
		}
		ESIF_TRACE_DEBUG("Using Participant %s\n", EsifUp_GetName(upPtr));

		EsifUp_PutRef(upPtr);

		rc = EsifEventMgr_UnregisterEventByGuid(eventGuidPtr->buf_ptr, 0, EVENT_MGR_DOMAIN_D0, EsifApp_EventCallback, appPtr->fHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_PARTICIPANT:
	{
		/* Unregister participant */
		upMapPtr = EsifApp_GetParticipantDataMapFromHandle(appPtr, upHandle);
		if (NULL == upMapPtr) {
			ESIF_TRACE_ERROR("The app participant data was not found from participant handle\n");
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}
		ESIF_TRACE_DEBUG("Using Participant %s\n", EsifUp_GetName(upMapPtr->fUpPtr));

		rc = EsifEventMgr_UnregisterEventByGuid(eventGuidPtr->buf_ptr, EsifUp_GetInstance(upMapPtr->fUpPtr), EVENT_MGR_MATCH_ANY, EsifApp_EventCallback, appPtr->fHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	case EVENT_CATEGORY_DOMAIN:
	{
		/* Unregister domain */

		upMapPtr = EsifApp_GetParticipantDataMapFromHandle(appPtr, upHandle);
		if (NULL == upMapPtr) {
			ESIF_TRACE_ERROR("The app participant data was not found from participant handle\n");
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}

		domainPtr = EsifApp_GetDomainDataMapByHandle(upMapPtr, domainHandle);
		if (NULL == domainPtr) {
			ESIF_TRACE_ERROR("The app domain data was not found from domain handle\n");
			rc = ESIF_E_INVALID_HANDLE;
			goto exit;
		}
		ESIF_TRACE_DEBUG("Using Participant %s\n", EsifUp_GetName(upMapPtr->fUpPtr));

		rc = EsifEventMgr_UnregisterEventByGuid(eventGuidPtr->buf_ptr, EsifUp_GetInstance(upMapPtr->fUpPtr), domainPtr->fQualifierId, EsifApp_EventCallback, appPtr->fHandle);
		if (ESIF_OK != rc) {
			goto exit;
		}
	}
	break;

	default:
		ESIF_TRACE_ERROR("Unknown category type\n");
		break;
	}
exit:
	return rc;
}


/* Lookup participant data for a handle */
AppParticipantDataMapPtr EsifApp_GetParticipantDataMapFromHandle(
	const EsifAppPtr appPtr,
	const void *participantHandle
	)
{
	UInt8 i = 0;
	AppParticipantDataMapPtr upDataMapPtr = NULL;

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		if (appPtr->fParticipantData[i].fAppParticipantHandle == participantHandle) {
			upDataMapPtr = &appPtr->fParticipantData[i];
			break;
		}
	}

	return upDataMapPtr;
}


static AppDomainDataMapPtr EsifApp_GetDomainDataMapByHandle(
	const AppParticipantDataMapPtr upMapPtr,
	const void *domainHandle
	)
{
	UInt8 i = 0;
	AppDomainDataMapPtr domain_data_map_ptr = NULL;

	for (i = 0; i < MAX_DOMAIN_ENTRY; i++) {
		if (upMapPtr->fDomainData[i].fAppDomainHandle == domainHandle) {
			domain_data_map_ptr = &upMapPtr->fDomainData[i];
			break;
		}
	}

	return domain_data_map_ptr;
}


static ESIF_INLINE EventCategory EsifApp_CategorizeEvent(
	const void *appHandle,
	const void *upHandle,
	const void *domainHandle
	)
{
	EventCategory category = EVENT_CATEGORY_NONE;

	if (NULL != appHandle && NULL == upHandle && NULL == domainHandle) {
		category = EVENT_CATEGORY_APP;
	} else if (NULL != appHandle && NULL != upHandle && NULL == domainHandle) {
		category = EVENT_CATEGORY_PARTICIPANT;
	} else if (NULL != appHandle && NULL != upHandle && NULL != domainHandle) {
		category = EVENT_CATEGORY_DOMAIN;
	} else {
		// This should never happen
	}
	return category;
}


eEsifError EsifAppInit()
{
	return ESIF_OK;
}


void EsifAppExit()
{
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
