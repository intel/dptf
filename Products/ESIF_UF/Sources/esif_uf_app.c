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

#include "esif_uf.h"				/* Upper Framework */
#include "esif_uf_appmgr.h"			/* Application Manager */
#include "esif_uf_cfgmgr.h"			/* Configuration Manager */
#include "esif_uf_log.h"			/* Logging */
#include "esif_uf_esif_iface.h"		/* ESIF Service Interface */
#include "esif_uf_service.h"		/* ESIF Service */
#include "esif_uf_fpc.h"
#include "esif_dsp.h"
#include "esif_pm.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif


/* DEBUG? */
#define APP_DEBUG ESIF_DEBUG

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

/* Data For Interface Marshaling */
static AppDataPtr CreateAppData (esif_string pathBuf)
{
	AppDataPtr app_data_ptr = NULL;
	esif_string full_path   = NULL;

	if (NULL == pathBuf) {
		goto exit;
	}

	full_path = esif_build_path(pathBuf, ESIF_PATH_LEN, ESIF_DIR_PRG, NULL);
	if (NULL == full_path) {
		goto exit;
	}

	app_data_ptr = (AppDataPtr)esif_ccb_malloc(sizeof(AppData));
	if (NULL == app_data_ptr) {
		goto exit;
	}

	app_data_ptr->fPathHome.buf_ptr  = (void*)full_path;
	app_data_ptr->fPathHome.buf_len  = ESIF_PATH_LEN;
	app_data_ptr->fPathHome.data_len = (UInt32)esif_ccb_strlen(full_path, ESIF_PATH_LEN);
	app_data_ptr->fPathHome.type     = ESIF_DATA_STRING;

exit:

	return app_data_ptr;
}


/*
    IMPLEMENT EsifAppInterface
 */
typedef eEsifError (*GetIfaceFuncPtr)(AppInterfacePtr);

static eEsifError AppCreate (
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


	ESIF_TRACE_DEBUG("%s\n\n"
					 "Application Name   : %s\n"
					 "Application Desc   : %s\n"
					 "Application Type   : %s\n"
					 "Application Version: %s\n\n",
					 ESIF_FUNC,
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
	if (ESIF_OK != rc) {
		goto exit;
	}

	if (NULL != app_data_ptr) {
		esif_ccb_free(app_data_ptr);
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
	field.buf_ptr = buffer; \
	field.buf_len = buffer_len; \
	field.data_len = (UInt32)(esif_ccb_strlen(buffer, buffer_len) + 1); \
	field.type    = ESIF_DATA_STRING;

#define ASSIGN_DATA_GUID(field, buffer) \
	field.buf_ptr = buffer; \
	field.buf_len = ESIF_GUID_LEN; \
	field.data_len = ESIF_GUID_LEN; \
	field.type = ESIF_DATA_GUID;


/* Data For Interface Marshaling */
static AppDomainDataPtr CreateDomainData (const struct esif_fpc_domain *domainPtr)
{
	AppDomainDataPtr dom_data_ptr = (AppDomainDataPtr)esif_ccb_malloc(sizeof(AppDomainData));

	ESIF_TRACE_DEBUG("%s: %s\n", ESIF_FUNC, domainPtr->descriptor.name);

	if (NULL == dom_data_ptr) {
		goto exit;
	}

	dom_data_ptr->fName.buf_ptr  = (void*)domainPtr->descriptor.name;
	dom_data_ptr->fName.buf_len  = ESIF_NAME_LEN;
	dom_data_ptr->fName.data_len = (UInt32)esif_ccb_strlen(domainPtr->descriptor.name, ESIF_NAME_LEN);
	dom_data_ptr->fName.type     = ESIF_DATA_STRING;

	dom_data_ptr->fDescription.buf_ptr  = (void*)domainPtr->descriptor.description;
	dom_data_ptr->fDescription.buf_len  = ESIF_DESC_LEN;
	dom_data_ptr->fDescription.data_len = (UInt32)esif_ccb_strlen(domainPtr->descriptor.description, ESIF_DESC_LEN);
	dom_data_ptr->fDescription.type     = ESIF_DATA_STRING;

	dom_data_ptr->fGuid.buf_ptr  = (void*)domainPtr->descriptor.guid;
	dom_data_ptr->fGuid.buf_len  = ESIF_GUID_LEN;
	dom_data_ptr->fGuid.data_len = ESIF_GUID_LEN;
	dom_data_ptr->fGuid.type     = ESIF_DATA_GUID;

	dom_data_ptr->fVersion    = APP_DOMAIN_VERSION;
	dom_data_ptr->fType       = (enum esif_domain_type)domainPtr->descriptor.domainType;
	dom_data_ptr->fCapability = domainPtr->capability_for_domain.capability_flags;
	esif_ccb_memcpy(dom_data_ptr->fCapabilityBytes, domainPtr->capability_for_domain.capability_mask, 32);

exit:

	return dom_data_ptr;
}


static eEsifError CreateDomain (
	UInt8 domainId,
	EsifAppPtr appPtr,
	AppParticipantDataMapPtr participantDataMapPtr,
	struct esif_fpc_domain *domainPtr
	)
{
	eEsifError rc = ESIF_OK;
	AppDomainDataPtr domain_data_ptr = NULL;
	void *domain_handle = NULL;

	ESIF_TRACE_DEBUG("%s: Create Domain %s\n", ESIF_FUNC, domainPtr->descriptor.name);
	if (NULL == appPtr || NULL == domainPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	domain_data_ptr = CreateDomainData(domainPtr);
	if (NULL == domain_data_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s: Have Domain Data %d\n", ESIF_FUNC, domainId);

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
		participantDataMapPtr->fDomainData[domainId].fQualifierId      = *(u16*)g_qualifiers[domainId];

		ESIF_TRACE_DEBUG("%s: DomainMap(%u): Name %s Esif(%s) %p Mapped To Handle 0x%p\n", ESIF_FUNC,
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
		ESIF_TRACE_DEBUG("%s: DomainMap(%u): Domain Mapping Error %s(%d)\n", ESIF_FUNC,
						 domainId,
						 esif_rc_str(rc), rc);
	}

exit:
	esif_ccb_free(domain_data_ptr);
	return rc;
}


static eEsifError CreateDomains (
	EsifAppPtr appPtr,
	EsifUpPtr upPtr,
	AppParticipantDataMapPtr participantDataMapPtr
	)
{
	eEsifError rc = ESIF_OK;
	UInt8 i = 0;

	UNREFERENCED_PARAMETER(participantDataMapPtr);
	UNREFERENCED_PARAMETER(appPtr);

	ESIF_TRACE_DEBUG("%s: Create Domains\n", ESIF_FUNC);
	for (i = 0; i < upPtr->fDspPtr->get_domain_count(upPtr->fDspPtr); i++) {
		struct esif_fpc_domain *domain_ptr = upPtr->fDspPtr->get_domain(upPtr->fDspPtr, i + 1);
		if (NULL == domain_ptr) {
			continue;
		}

		rc = CreateDomain(i, appPtr, participantDataMapPtr, domain_ptr);
		if (ESIF_OK != rc) {
			goto exit;
		}
		ESIF_TRACE_DEBUG("Create Domain %s\n", domain_ptr->descriptor.name);
	}

exit:

	return rc;
}


/* Data For Interface Marshaling */
static AppParticipantDataPtr CreateParticipantData (
	const EsifUpPtr upPtr,
	const EsifUpDataPtr upDataPtr
	)
{
	AppParticipantDataPtr app_data_ptr = NULL;

	if (upPtr->fDspPtr == NULL) {
		goto exit;
	}

	app_data_ptr = (AppParticipantDataPtr)esif_ccb_malloc(sizeof(AppParticipantData));
	if (NULL == app_data_ptr) {
		goto exit;
	}

	/* Common */
	app_data_ptr->fVersion = upDataPtr->fVersion;
	ASSIGN_DATA_GUID(app_data_ptr->fDriverType, upDataPtr->fDriverType);
	ASSIGN_DATA_GUID(app_data_ptr->fDeviceType, upDataPtr->fDriverType);
	ASSIGN_DATA_STRING(app_data_ptr->fName, upDataPtr->fName, ESIF_NAME_LEN);
	ASSIGN_DATA_STRING(app_data_ptr->fDesc, upDataPtr->fDesc, ESIF_DESC_LEN);

	ASSIGN_DATA_STRING(app_data_ptr->fDriverName, upDataPtr->fDriverName, ESIF_NAME_LEN);
	ASSIGN_DATA_STRING(app_data_ptr->fDeviceName, upDataPtr->fDeviceName, ESIF_NAME_LEN);
	ASSIGN_DATA_STRING(app_data_ptr->fDevicePath, upDataPtr->fDevicePath, ESIF_PATH_LEN);

	app_data_ptr->fDomainCount   = (u8)upPtr->fDspPtr->get_domain_count(upPtr->fDspPtr);
	app_data_ptr->fBusEnumerator = upDataPtr->fEnumerator;

	/* ACPI Device */
	ASSIGN_DATA_STRING(app_data_ptr->fAcpiDevice, upDataPtr->fAcpiDevice, ESIF_NAME_LEN);
	ASSIGN_DATA_STRING(app_data_ptr->fAcpiScope, upDataPtr->fAcpiScope, ESIF_SCOPE_LEN);
	app_data_ptr->fAcpiType = upDataPtr->fAcpiType;
	app_data_ptr->fAcpiUID  = upDataPtr->fAcpiUID;

	/* PCI Device */
	app_data_ptr->fPciVendor    = upDataPtr->fPciVendor;
	app_data_ptr->fPciDevice    = upDataPtr->fPciDevice;
	app_data_ptr->fPciBus       = upDataPtr->fPciBus;
	app_data_ptr->fPciBusDevice = upDataPtr->fPciBusDevice;
	app_data_ptr->fPciFunction  = upDataPtr->fPciFunction;
	app_data_ptr->fPciRevision  = upDataPtr->fPciRevision;
	app_data_ptr->fPciClass     = upDataPtr->fPciClass;
	app_data_ptr->fPciSubClass  = upDataPtr->fPciSubClass;
	app_data_ptr->fPciProgIf    = upDataPtr->fPciProgIf;

exit:

	return app_data_ptr;
}


eEsifError EsifAppCreateParticipant (
	const EsifAppPtr appPtr,
	const EsifUpPtr upPtr
	)
{
	eEsifError rc = ESIF_OK;
	void *participant_handle = NULL;
	AppParticipantDataPtr participant_data_ptr = NULL;

	if (NULL == appPtr || NULL == upPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Create participant metadata to marshall though interface */
	participant_data_ptr = CreateParticipantData(upPtr, &upPtr->fMetadata);
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
			ESIF_TRACE_DEBUG("%s: Participant(%u) Mapping Error %s(%d)\n", ESIF_FUNC, upPtr->fInstance, esif_rc_str(rc), rc);
		}
		goto exit;
	}

	{
		AppParticipantDataMapPtr participantDataMapPtr = &appPtr->fParticipantData[upPtr->fInstance];
		participantDataMapPtr->fAppParticipantHandle = participant_handle;	/* Application Participant */
		participantDataMapPtr->fUpPtr = upPtr;								/* ESIF Participant */

		ESIF_TRACE_DEBUG("%s: Participant(%u) Esif 0x%p Mapped To Handle 0x%p\n", ESIF_FUNC,
						 upPtr->fInstance,
						 participantDataMapPtr->fUpPtr,
						 participantDataMapPtr->fAppParticipantHandle);

		/* Call through the interface to create the participant instance in the app. */
		rc = appPtr->fInterface.fParticipantCreateFuncPtr(
				appPtr->fHandle,
				participant_handle,
				participant_data_ptr,
				eParticipantStateEnabled);
		if (ESIF_OK != rc) {
			esif_ccb_free(participant_data_ptr);
			participant_data_ptr = NULL;
			goto exit;
		}

		rc = CreateDomains(appPtr, upPtr, participantDataMapPtr);
	}

exit:
	if (participant_data_ptr) {
		esif_ccb_free(participant_data_ptr);
		participant_data_ptr = NULL;
	}
	return rc;
}


static eEsifError DestroyDomain (
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
		ESIF_TRACE_DEBUG("%s: DomainMap(%u) Esif 0x%p UnMapped From Handle 0x%p\n", ESIF_FUNC,
						 domainDataMapPtr->fAppDomainId,
						 domainDataMapPtr->fAppDomainDataPtr,
						 domainDataMapPtr->fAppDomainHandle);

		memset(domainDataMapPtr, 0, sizeof(*domainDataMapPtr));
	} else {
		ESIF_TRACE_DEBUG("%s: DomainMap(%u) Esif UnMapping Error %s(%d)\n", ESIF_FUNC,
						 domainDataMapPtr->fAppDomainId,
						 esif_rc_str(rc), rc);
	}
	return rc;
}


static eEsifError DestroyDomains (
	EsifAppPtr appPtr,
	AppParticipantDataMapPtr participantDataMapPtr
	)
{
	eEsifError rc = ESIF_OK;
	UInt8 i = 0;

	ESIF_TRACE_DEBUG("%s: Destroy Domains\n", ESIF_FUNC);
	for (i = 0; i < MAX_DOMAIN_ENTRY; i++) {
		AppDomainDataMapPtr domainDataMapPtr = &participantDataMapPtr->fDomainData[i];
		if (NULL == domainDataMapPtr->fAppDomainHandle) {
			continue;
		}
		rc = DestroyDomain(appPtr, participantDataMapPtr, domainDataMapPtr);
	}
	return rc;
}


eEsifError EsifAppDestroyParticipant (
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

	participant_data_map_ptr = &appPtr->fParticipantData[upPtr->fInstance];

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
		ESIF_TRACE_DEBUG("%s: ParticipantMap(%u) Esif 0x%p UnMapped From Handle 0x%p\n", ESIF_FUNC,
						 participant_data_map_ptr->fUpPtr->fInstance,
						 participant_data_map_ptr->fUpPtr,
						 participant_data_map_ptr->fAppParticipantHandle);
						 memset(participant_data_map_ptr, 0, sizeof(*participant_data_map_ptr));
	} else {
		ESIF_TRACE_DEBUG("%s: ParticipantMap(%u) UnMapping Error %s(%d)\n", ESIF_FUNC,
						 participant_data_map_ptr->fUpPtr->fInstance,
						 esif_rc_str(rc), rc);
	}

exit:

	return rc;
}


/*
** PUBLIC
*/

eEsifError EsifAppStart (EsifAppPtr appPtr)
{
	eEsifError rc = ESIF_OK;
	GetIfaceFuncPtr iface_func_ptr = NULL;
	esif_string iface_func_name    = GET_APPLICATION_INTERFACE_FUNCTION;

	char libPath[ESIF_LIBPATH_LEN];
	esif_lib_t lib_handle = 0;

	ESIF_TRACE_DEBUG("%s name=%s\n", ESIF_FUNC, appPtr->fLibNamePtr);
	esif_ccb_sprintf(ESIF_LIBPATH_LEN, libPath, "%s.%s", esif_build_path(libPath, ESIF_LIBPATH_LEN, ESIF_DIR_PRG, appPtr->fLibNamePtr), ESIF_LIB_EXT);

	lib_handle = esif_ccb_library_load(libPath);

	if (0 == lib_handle) {
		rc = ESIF_E_UNSPECIFIED;
		ESIF_TRACE_DEBUG("%s esif_ccb_library_load() %s failed.\n", ESIF_FUNC, libPath);
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s esif_ccb_library_load() %s completed.\n", ESIF_FUNC, libPath);

	iface_func_ptr = (GetIfaceFuncPtr)esif_ccb_library_get_func(lib_handle, (char*)iface_func_name);
	if (NULL == iface_func_ptr) {
		rc = ESIF_E_UNSPECIFIED;
		ESIF_TRACE_DEBUG("%s esif_ccb_library_get_func() %s failed.\n", ESIF_FUNC, iface_func_name);
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s esif_ccb_library_get_func() %s completed.\n", ESIF_FUNC, iface_func_name);
	rc = AppCreate(appPtr, iface_func_ptr);
	if (ESIF_OK != rc) {
		ESIF_TRACE_DEBUG("%s AppCreate failed.\n", ESIF_FUNC);
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s AppCreate completed.\n", ESIF_FUNC);
	rc = EsifUpManagerRegisterParticipantsWithApp(appPtr);
	ESIF_TRACE_DEBUG("%s EsifUpManagerRegisterParticipantsWithApp completed.\n", ESIF_FUNC);

exit:

	return rc;
}


eEsifError EsifAppStop (EsifAppPtr appPtr)
{
	eEsifError rc = ESIF_OK;
	ESIF_ASSERT(appPtr != NULL);

	rc = EsifUpManagerDestroyParticipantsInApp(appPtr);
	ESIF_TRACE_DEBUG("%s EsifUpManagerDestroyParticipantsInApp completed.\n", ESIF_FUNC);

	rc = appPtr->fInterface.fAppDestroyFuncPtr(appPtr->fHandle);
	if (ESIF_OK == rc) {
		memset(appPtr, 0, sizeof(*appPtr));
	}
	return rc;
}


static u8 isEventRegistered (
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
	bitMask     = bitMask << (u32) eventType;
	returnValue = (RegisteredEvents & bitMask) ? 1 : 0;

	return returnValue;
}


/* Lookup participant data for a instance */
static AppParticipantDataMapPtr find_participant_data_map_from_instance (
	const EsifAppPtr appPtr,
	const u8 participantId
	)
{
	UInt8 i = 0;
	AppParticipantDataMapPtr participant_data_map_ptr = NULL;

	/* First Find Upper Framework Pointer */
	EsifUpPtr participant_ptr = EsifUpManagerGetAvailableParticipantByInstance(participantId);

	if (NULL == participant_ptr) {
		goto exit;
	}

	ESIF_TRACE_DEBUG("%s: Have Event For Participant %s\n", ESIF_FUNC,
					 participant_ptr->fMetadata.fName);

	/*
	** Okay now we have to find the correct particpant handle based on the appPtr.  Note each
	** app will have a different particpant handle.
	*/
	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		if (NULL == appPtr->fParticipantData[i].fUpPtr) {
			continue;
		}

		if (appPtr->fParticipantData[i].fUpPtr == participant_ptr) {
			participant_data_map_ptr = &appPtr->fParticipantData[i];

			ESIF_TRACE_DEBUG("%s: Found participant data map for %s\n", ESIF_FUNC,
							 participant_ptr->fMetadata.fName);
			break;
		}
	}

exit:

	return participant_data_map_ptr;
}


eEsifError EsifAppEvent (
	EsifAppPtr theAppPtr,
	UInt8 participantId,
	UInt16 domainId,
	EsifDataPtr eventData,
	eEsifEventType eventType
	)
{
	void *appHandle    = NULL;
	void *participantHandle = NULL;
	void *domainHandle = NULL;
	u8 send_event = ESIF_FALSE;

	eEsifError rc = ESIF_OK;
	esif_guid_t event_guid = {0};
	EsifData data_guid     = {ESIF_DATA_GUID, &event_guid, sizeof(event_guid), sizeof(event_guid)};

	UNREFERENCED_PARAMETER(domainHandle);
	UNREFERENCED_PARAMETER(domainId);
	ESIF_ASSERT(theAppPtr != NULL);

	ESIF_TRACE_DEBUG("%s:\n", ESIF_FUNC);

	if (0 == participantId) {
		// Application Event?
		if (isEventRegistered(theAppPtr->fRegisteredEvents, eventType)) {
			struct esif_fpc_event *event_ptr = NULL;
			EsifUpPtr up_ptr = NULL;
			appHandle  = theAppPtr->fHandle;
			send_event = ESIF_TRUE;

			/* Find Our Participant 0 */
			up_ptr = EsifUpManagerGetAvailableParticipantByInstance(0);
			if (NULL == up_ptr) {
				rc = ESIF_E_UNSPECIFIED;
				goto exit;
			}
			ESIF_TRACE_DEBUG("%s: Using Participant %s\n", ESIF_FUNC, up_ptr->fMetadata.fName);

			/* Find The Event Associated With Participant 0 */
			event_ptr = up_ptr->fDspPtr->get_event_by_type(up_ptr->fDspPtr, eventType);
			if (NULL == event_ptr) {
				rc = ESIF_E_NOT_FOUND;
				goto exit;
			}
			esif_ccb_memcpy(&event_guid, &event_ptr->event_guid, ESIF_GUID_LEN);
			ESIF_TRACE_DEBUG("%s: Ring Manager Door Bell:\n", ESIF_FUNC);
		}
	} else {
		AppParticipantDataMapPtr app_participant_data_map_ptr = find_participant_data_map_from_instance(theAppPtr, participantId);
		if (NULL == app_participant_data_map_ptr) {
			rc = ESIF_E_PARAMETER_IS_NULL;
			goto exit;
		}

		// Participant Event?
		if (isEventRegistered(app_participant_data_map_ptr->fRegisteredEvents, eventType)) {
			struct esif_fpc_event *event_ptr = NULL;
			appHandle  = theAppPtr->fHandle;
			participantHandle = app_participant_data_map_ptr->fAppParticipantHandle;
			send_event = ESIF_TRUE;

			ESIF_TRACE_DEBUG("%s: Using Participant %s\n", ESIF_FUNC, app_participant_data_map_ptr->fUpPtr->fMetadata.fName);

			/* Find The Event Associated With Participant 0 */
			event_ptr = app_participant_data_map_ptr->fUpPtr->fDspPtr->get_event_by_type(
					app_participant_data_map_ptr->fUpPtr->fDspPtr, eventType);
			if (NULL == event_ptr) {
				rc = ESIF_E_NOT_FOUND;
				goto exit;
			}
			esif_ccb_memcpy(&event_guid, &event_ptr->event_guid, ESIF_GUID_LEN);
			ESIF_TRACE_DEBUG("%s: Ring Participant Door Bell:\n", ESIF_FUNC);
		}
	}

	if (NULL != theAppPtr->fInterface.fAppEventFuncPtr && send_event) {
		char guid_str[ESIF_GUID_PRINT_SIZE];
		UNREFERENCED_PARAMETER(guid_str);

		ESIF_TRACE_DEBUG("%s:\n\n"
						 "appHandle:         %p\n"
						 "participantHandle: %p\n"
						 "domainHandle:      %p\n"
						 "eventGuid:         %s\n"
						 "eventData:         %p\n\n",
						 ESIF_FUNC, appHandle, participantHandle, domainHandle,
						 esif_guid_print((esif_guid_t*)data_guid.buf_ptr, guid_str),
						 eventData);

		rc = theAppPtr->fInterface.fAppEventFuncPtr(
				appHandle,
				participantHandle,
				NULL,
				eventData,
				&data_guid);
	}

exit:

	return rc;
}


eEsifError EsifAppInit ()
{
	return ESIF_OK;
}


void EsifAppExit ()
{
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
