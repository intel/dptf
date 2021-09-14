/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include "esif_uf_ccb_thermalapi.h"
#include "esif_uf_handlemgr.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

#define ESIF_APP_CLIENT_INDICATOR	'@'	// AppNames that start with this indicate out-of-process (non-restartable) client apps

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

//
// FRIEND MEMBER FUNCTIONS
//
eEsifError EsifApp_Create(
	const EsifString appName,
	EsifAppPtr *appPtr
	);

eEsifError EsifApp_Stop(EsifAppPtr self);

void EsifApp_Destroy(EsifAppPtr self);

//
// FRIEND FUNCTIONS
//
#if defined(ESIF_FEAT_OPT_ARBITRATOR_ENABLED)
/* Remove arbitration requests for the given ESIF application */ 
void EsifArbMgr_RemoveApp(
	esif_handle_t appHandle
);
#else
#define EsifArbMgr_RemoveApp(app)
#endif


/*
* Takes an additional reference on the app which prevents the app from being
* destroyed until all references are released.
*/
eEsifError EsifApp_GetRef(EsifAppPtr self);

/*
* Releases a reference on the app and destroys the app if the reference count
* reaches 0.
*/
void EsifApp_PutRef(EsifAppPtr self);

eEsifError EsifApp_Start(EsifAppPtr self);

eEsifError EsifApp_SuspendApp(EsifAppPtr self);

eEsifError EsifApp_ResumeApp(EsifAppPtr self);

eEsifError EsifApp_CreateParticipant(
	const EsifAppPtr self,
	const EsifUpPtr upPtr
	);

eEsifError EsifApp_DestroyParticipant(
	const EsifAppPtr self,
	const EsifUpPtr upPtr
	);

//
// PRIVATE MEMBER FUNCTIONS
//
static eEsifError EsifApp_Load(EsifAppPtr self);

static eEsifError EsifApp_RegisterParticipantsWithApp(EsifAppPtr self);
static eEsifError EsifApp_DestroyParticipants(EsifAppPtr self);


static AppParticipantDataMapPtr EsifApp_GetParticipantDataMapFromInstance(
	const EsifAppPtr self,
	const esif_handle_t participantId
	);

static AppParticipantDataMapPtr EsifApp_FindEmptyDataMap(
	const EsifAppPtr self
	);

static void EsifApp_WaitForAccessCompletion(EsifAppPtr self);

static void EsifApp_ClearParticipantDataMap(AppParticipantDataMapPtr participantDataMapPtr);

static void EsifApp_StripInvalid(EsifString buffer, size_t buf_len)
{
	const EsifString banned = "\r\n\t,|";
	size_t j = 0; 
	while (buffer[j] && j < buf_len) {
		if (esif_ccb_strchr(banned, buffer[j])) {
			esif_ccb_memmove(&buffer[j], &buffer[j + 1], buf_len - j - 1);
		}
		else {
			j++;
		}
	}
}

//
// FUNCTION IMPLEMENTATIONS
//
eEsifError EsifApp_Create(
	const EsifString name,
	EsifAppPtr *appPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifAppPtr self = NULL;
	size_t i = 0;
	size_t j = 0;
	EsifString appName = (EsifString)name;

	if ((NULL == appName) || (NULL == appPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/*
	 * AppName format: [@]appname[=library] where:
	 * 1. AppNames starting with "@" indicate an out-of-process Client Application (i.e., not Restartable)
	 * 2. Appnames ending with optional "=library" use library.dll (or .so) instead of appname.dll
	 */
	Bool isClientApp = ESIF_FALSE;
	if (*appName == ESIF_APP_CLIENT_INDICATOR) {
		appName++;
		isClientApp = ESIF_TRUE;
	}
	size_t appNameLen = esif_ccb_strlen(appName, APPNAME_MAXLEN);
	EsifString libName = esif_ccb_strchr(appName, APPNAME_SEPARATOR);
	if (libName) {
		appNameLen -= esif_ccb_strlen(libName, APPNAME_MAXLEN - (size_t)(libName - appName));
		libName++;
	}

	self = (EsifAppPtr)esif_ccb_malloc(sizeof(*self));
	if (NULL == self) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	esif_ccb_event_init(&self->deleteEvent);
	esif_ccb_lock_init(&self->objLock);
	self->refCount = 1;

	self->fAppNamePtr = (esif_string)esif_ccb_malloc(appNameLen + 1);
	self->fLibNamePtr = (esif_string)esif_ccb_strdup((libName ? libName : appName));
	if (NULL == self->fLibNamePtr || NULL == self->fAppNamePtr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}
	esif_ccb_strcpy(self->fAppNamePtr, appName, appNameLen + 1);
	EsifApp_StripInvalid(self->fAppNamePtr, appNameLen + 1);
	self->isRestartable = !isClientApp;

	for (i = 0; i < (sizeof(self->fParticipantData) / sizeof(*self->fParticipantData)); i++) {
		self->fParticipantData[i].fAppParticipantHandle = ESIF_INVALID_HANDLE;
		for (j = 0; j < (sizeof(self->fParticipantData[i].fDomainData) / sizeof(*self->fParticipantData[i].fDomainData)); j++) {
			self->fParticipantData[i].fDomainData[j].fAppDomainHandle = ESIF_INVALID_HANDLE;
		}
	}
	*appPtr = self;
exit:
	if (rc != ESIF_OK) {
		EsifApp_Destroy(self);
	}
	return rc;
}


void EsifApp_Destroy(
	EsifAppPtr self
	)
{
	if (NULL == self) {
		goto exit;
	}

	
	self->markedForDelete = ESIF_TRUE;
	EsifApp_PutRef(self);

	ESIF_TRACE_INFO("Waiting for delete event...\n");
	esif_ccb_event_wait(&self->deleteEvent);
	ESIF_TRACE_INFO("Destroying...\n");

	/* Release any reference to participants taken when init was paused */
	EsifUp_PutRef(self->upPtr);
	self->upPtr = NULL;
	self->iteratorValid = ESIF_FALSE;

	EsifEventMgr_UnregisterAllForApp(EsifSvcEventCallback, self->fHandle);
	
	esif_ccb_library_unload(self->fLibHandle);
	esif_ccb_free(self->fAppNamePtr);
	esif_ccb_free(self->fAppDescPtr);
	esif_ccb_free(self->fAppVersionPtr);
	esif_ccb_free(self->fAppIntroPtr);
	esif_ccb_free(self->fLibNamePtr);
	self->isRestartable = ESIF_FALSE;
	esif_ccb_event_uninit(&self->deleteEvent);
	esif_ccb_lock_uninit(&self->objLock);
	EsifHandleMgr_PutHandle(self->fHandle);
	esif_ccb_free(self);
exit:
	return;
}


eEsifError EsifApp_GetRef(
	EsifAppPtr self
	)
{
	eEsifError rc = ESIF_OK;

	if (self == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_write_lock(&self->objLock);

	if (self->markedForDelete == ESIF_TRUE) {
		esif_ccb_write_unlock(&self->objLock);
		ESIF_TRACE_DEBUG("Marked for delete\n");
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	self->refCount++;
	esif_ccb_write_unlock(&self->objLock);
exit:
	return rc;
}


void EsifApp_PutRef(
	EsifAppPtr self
	)
{
	Bool needRelease = ESIF_FALSE;
	Bool accessComplete = ESIF_FALSE;

	if (self != NULL) {
		esif_ccb_write_lock(&self->objLock);

		self->refCount--;

		if (self->stoppingApp && (self->refCount <= 1)) {
			accessComplete = ESIF_TRUE;
		}

		if ((self->refCount == 0) && (self->markedForDelete)) {
			needRelease = ESIF_TRUE;
		}

		esif_ccb_write_unlock(&self->objLock);

		if (accessComplete) {
			ESIF_TRACE_DEBUG("Signal activity done event\n");
			esif_ccb_event_set(&self->accessCompleteEvent);
		}
		if (needRelease) {
			ESIF_TRACE_DEBUG("Signal delete event\n");
			esif_ccb_event_set(&self->deleteEvent);
		}
	}
}


/* Data For Interface Marshaling */
static AppDataPtr CreateAppData(
	EsifAppPtr self,
	esif_string pathBuf,
	size_t bufLen
	)
{
	AppDataPtr app_data_ptr = NULL;

	if (NULL == self) {
		goto exit;
	}

	if (NULL == pathBuf) {
		ESIF_TRACE_ERROR("Path buffer is NULL\n");
		goto exit;
	}

	// Build path for ESIF Apps: "HomeDir"
	esif_build_path(pathBuf, bufLen, ESIF_PATHTYPE_HOME, NULL, NULL);

	ESIF_TRACE_DEBUG("pathBuf=%s\n", (esif_string)pathBuf);

	app_data_ptr = (AppDataPtr)esif_ccb_malloc(sizeof(AppData));
	if (NULL == app_data_ptr) {
		ESIF_TRACE_ERROR("Fail to allocate AppData\n");
		goto exit;
	}

	app_data_ptr->fPathHome.buf_ptr  = (void *)pathBuf;
	app_data_ptr->fPathHome.buf_len  = (UInt32)bufLen;
	app_data_ptr->fPathHome.data_len = (UInt32)esif_ccb_strlen(pathBuf, bufLen) + 1;
	app_data_ptr->fPathHome.type     = ESIF_DATA_STRING;
	app_data_ptr->fLogLevel          = (eLogType) g_traceLevel;
exit:
	return app_data_ptr;
}


/*
	IMPLEMENT EsifAppInterface
 */
typedef eEsifError (ESIF_CALLCONV *GetIfaceFuncPtr)(AppInterfaceSetPtr);

static eEsifError EsifApp_CreateApp(
	EsifAppPtr self,
	GetIfaceFuncPtr ifaceFuncPtr
	)
{
	eEsifError rc = ESIF_OK;
	esif_handle_t esifHandle = ESIF_INVALID_HANDLE;
	EsifString originalAppName = NULL;

	AppDataPtr app_data_ptr = NULL;
	char path_buf[ESIF_PATH_LEN * 3] = { 0 };

	char name[ESIF_NAME_LEN] = { 0 };
	ESIF_DATA(data_name, ESIF_DATA_STRING, name, ESIF_NAME_LEN);

	char desc[ESIF_DESC_LEN] = { 0 };
	ESIF_DATA(data_desc, ESIF_DATA_STRING, desc, ESIF_DESC_LEN);

	char version[ESIF_NAME_LEN] = { 0 };
	ESIF_DATA(data_version, ESIF_DATA_STRING, version, ESIF_NAME_LEN);

	#define BANNER_LEN 1024
	char banner[BANNER_LEN] = { 0 };
	ESIF_DATA(data_banner, ESIF_DATA_STRING, banner, BANNER_LEN);

	esif_string app_type_ptr = NULL;
	AppInterfaceSet appIfaceSet = { 0 };

	ESIF_TRACE_ENTRY_INFO();

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(ifaceFuncPtr != NULL);

	/* Assign the EsifInterface Functions */
	appIfaceSet.hdr.fIfaceType              = eIfaceTypeApplication;
	appIfaceSet.hdr.fIfaceVersion           = APP_INTERFACE_VERSION;
	appIfaceSet.hdr.fIfaceSize              = (UInt16)sizeof(appIfaceSet);

	/* GetApplicationInterfaceV2 Handleshake send ESIF receive APP Interface */
	rc = ifaceFuncPtr(&appIfaceSet);
	if (ESIF_OK != rc) {
		goto exit;
	}

	/* Check EsifAppInterface */
	if (appIfaceSet.hdr.fIfaceType != eIfaceTypeApplication ||
		appIfaceSet.hdr.fIfaceSize != (UInt16)sizeof(appIfaceSet) ||
		appIfaceSet.hdr.fIfaceVersion != APP_INTERFACE_VERSION) {
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	/* Functions Pointers */
	if (appIfaceSet.appIface.fAppCreateFuncPtr == NULL ||
		appIfaceSet.appIface.fAppDestroyFuncPtr == NULL ||
		appIfaceSet.appIface.fAppGetNameFuncPtr == NULL ||
		appIfaceSet.appIface.fParticipantCreateFuncPtr == NULL ||
		appIfaceSet.appIface.fParticipantDestroyFuncPtr == NULL ||
		appIfaceSet.appIface.fDomainCreateFuncPtr == NULL ||
		appIfaceSet.appIface.fDomainDestroyFuncPtr == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_memcpy(&self->fInterface, &appIfaceSet.appIface, sizeof(self->fInterface));

	// Pass Logical appname to App in the initial GetName/GetDesription/GetVersion calls
	if (data_name.buf_ptr && data_name.buf_len && self->fAppNamePtr) {
		esif_ccb_strcpy(data_name.buf_ptr, self->fAppNamePtr, data_name.buf_len);
		EsifApp_StripInvalid(data_name.buf_ptr, data_name.buf_len);
		data_name.data_len = (u32)esif_ccb_strlen(data_name.buf_ptr, data_name.buf_len) + 1;
	}

	// App may Rename itself if it is an in-process App without an explicit AppName
	if (self->isRestartable && self->fAppNamePtr && self->fLibNamePtr && esif_ccb_stricmp(self->fAppNamePtr, self->fLibNamePtr) == 0) {
		originalAppName = esif_ccb_strdup(self->fAppNamePtr);
	}

	// Callback for application information
	rc = self->fInterface.fAppGetNameFuncPtr(&data_name);
	if (ESIF_OK != rc) {
		goto exit;
	}

	// Rename App if the name returned by AppGetName is different than the original AppName
	if (originalAppName && data_name.buf_ptr && esif_ccb_stricmp(originalAppName, data_name.buf_ptr) != 0) {
		rc = EsifAppMgr_AppRename(self->fAppNamePtr, data_name.buf_ptr);
		if (ESIF_E_APP_ALREADY_STARTED == rc) {
			rc = ESIF_E_SESSION_ALREADY_STARTED;
		}
		if (ESIF_OK != rc) {
			goto exit;
		}
	}

	//optional
	rc = EsifApp_GetDescription(self, &data_desc);
	if (ESIF_OK != rc && ESIF_E_NOT_IMPLEMENTED != rc) {
		goto exit;
	}

	//optional
	rc = EsifApp_GetVersion(self, &data_version);
	if (ESIF_OK != rc && ESIF_E_NOT_IMPLEMENTED != rc) {
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


	rc = EsifHandleMgr_GetNextHandle(&esifHandle);
	if (rc != ESIF_OK) {
		goto exit;
	}
	self->fHandle = esifHandle;

	app_data_ptr = CreateAppData(self, path_buf, sizeof(path_buf));
	if (NULL == app_data_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	// Fill in the service pointers and then create the app
	appIfaceSet.esifIface.fGetConfigFuncPtr = EsifSvcConfigGet;
	appIfaceSet.esifIface.fSetConfigFuncPtr = EsifSvcConfigSet;
	appIfaceSet.esifIface.fPrimitiveFuncPtr = EsifSvcPrimitiveExec;
	appIfaceSet.esifIface.fWriteLogFuncPtr = EsifSvcWriteLog;
	appIfaceSet.esifIface.fRegisterEventFuncPtr = EsifSvcEventRegister;
	appIfaceSet.esifIface.fUnregisterEventFuncPtr = EsifSvcEventUnregister;

	/* Version 2 */
	appIfaceSet.esifIface.fSendEventFuncPtr = EsifSvcEventReceive;

	/* Version 3 */
	appIfaceSet.esifIface.fSendCommandFuncPtr = EsifSvcCommandReceive;

	// Create the application 
	rc = self->fInterface.fAppCreateFuncPtr(&appIfaceSet,
											  esifHandle,
											  &self->fAppCtxHandle,
											  app_data_ptr,
											  eAppStateEnabled);
	esif_ccb_free(app_data_ptr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	//optional
	rc = EsifApp_GetIntro(self, &data_banner);
	if (ESIF_OK != rc && ESIF_E_NOT_IMPLEMENTED != rc) {
		goto exit;
	}

	// Only display Intro Banner for Restartable Apps (not Remote Clients)
	if (self->isRestartable) {
		CMD_OUT("%s\n", (esif_string)data_banner.buf_ptr);
	}

	self->appCreationDone = ESIF_TRUE;
	ESIF_TRACE_DEBUG("Application creation completed.\n");
exit:
	esif_ccb_free(originalAppName);
	if (rc != ESIF_OK) {
		/* If creation fails, use of the interface is not allowed */
		esif_ccb_memset(&self->fInterface, 0, sizeof(self->fInterface));
		ESIF_TRACE_DEBUG("Application creation failed.\n");
	}
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
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


static eEsifError EsifApp_CreateDomain(
	EsifAppPtr self,
	UInt8 domainId,
	AppParticipantDataMapPtr participantDataMapPtr,
	struct esif_fpc_domain *domainPtr,
	EsifUpDomainPtr upDomainPtr
	)
{
	eEsifError rc = ESIF_OK;
	AppDomainDataPtr domain_data_ptr = NULL;
	esif_handle_t domainHandle = ESIF_INVALID_HANDLE;

	ESIF_TRACE_DEBUG("Create Domain %s\n", domainPtr->descriptor.name);
	if (NULL == self || NULL == domainPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	domain_data_ptr = CreateDomainData(domainPtr, upDomainPtr);
	if (NULL == domain_data_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Have Domain Data %d\n", domainId);

/* Create domain Handle */
	rc = EsifHandleMgr_GetNextHandle(&domainHandle); 
	if (rc != ESIF_OK) {
		goto exit;
	}

	if (ESIF_OK == rc && ESIF_INVALID_HANDLE == domainHandle) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	} else if (ESIF_OK == rc && domainId < sizeof(g_qualifiers) / sizeof(esif_string)) {
		participantDataMapPtr->fDomainData[domainId].fAppDomainId      = domainId;
		participantDataMapPtr->fDomainData[domainId].fAppDomainHandle  = domainHandle;
		participantDataMapPtr->fDomainData[domainId].fAppDomainDataPtr = domain_data_ptr;
		participantDataMapPtr->fDomainData[domainId].fQualifier = g_qualifiers[domainId];
		participantDataMapPtr->fDomainData[domainId].fQualifierId      = *(u16 *)g_qualifiers[domainId];

		ESIF_TRACE_DEBUG("DomainMap(%u): Name %s Esif(%s) %p Mapped To Handle 0x%p\n",
						 domainId,
						 (esif_string)domain_data_ptr->fName.buf_ptr,
						 participantDataMapPtr->fDomainData[domainId].fQualifier,
						 participantDataMapPtr->fDomainData[domainId].fAppDomainDataPtr,
						 participantDataMapPtr->fDomainData[domainId].fAppDomainHandle);

		rc = self->fInterface.fDomainCreateFuncPtr(
				self->fAppCtxHandle,
				participantDataMapPtr->fAppParticipantHandle,
				domainHandle,
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


static eEsifError EsifApp_CreateDomains(
	EsifAppPtr self,
	EsifUpPtr upPtr,
	AppParticipantDataMapPtr participantDataMapPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifDspPtr dspPtr = NULL;
	UInt32 domainCount = 0;
	UInt32 domainIndex = 0;
	EsifUpDomainPtr upDomainPtr = NULL;

	ESIF_ASSERT(self != NULL);
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
			rc = EsifApp_CreateDomain(self, (UInt8)domainIndex, participantDataMapPtr, domainPtr, upDomainPtr);
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
	ASSIGN_DATA_STRING(appDataPtr->fName, upDataPtr->fName, sizeof(upDataPtr->fName));
	ASSIGN_DATA_STRING(appDataPtr->fDesc, upDataPtr->fDesc, sizeof(upDataPtr->fDesc));

	ASSIGN_DATA_STRING(appDataPtr->fDriverName, upDataPtr->fDriverName, sizeof(upDataPtr->fDriverName));
	ASSIGN_DATA_STRING(appDataPtr->fDeviceName, upDataPtr->fDeviceName, sizeof(upDataPtr->fDeviceName));
	ASSIGN_DATA_STRING(appDataPtr->fDevicePath, upDataPtr->fDevicePath, sizeof(upDataPtr->fDevicePath));

	appDataPtr->fDomainCount   = (UInt8)dspPtr->get_domain_count(dspPtr);
	appDataPtr->fBusEnumerator = upDataPtr->fEnumerator;

	/* ACPI Device */
	ASSIGN_DATA_STRING(appDataPtr->fAcpiDevice, upDataPtr->fAcpiDevice, sizeof(upDataPtr->fAcpiDevice));
	ASSIGN_DATA_STRING(appDataPtr->fAcpiScope, upDataPtr->fAcpiScope, sizeof(upDataPtr->fAcpiScope));
	appDataPtr->fAcpiType = upDataPtr->fAcpiType;
	ASSIGN_DATA_STRING(appDataPtr->fAcpiUID, upDataPtr->fAcpiUID, sizeof(upDataPtr->fAcpiUID));

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


eEsifError EsifApp_CreateParticipant(
	const EsifAppPtr self,
	const EsifUpPtr upPtr
	)
{
	eEsifError rc = ESIF_OK;
	esif_handle_t participantId = ESIF_INVALID_HANDLE;
	AppParticipantDataPtr participant_data_ptr = NULL;
	AppParticipantDataMapPtr participantDataMapPtr = NULL;

	ESIF_TRACE_ENTRY_INFO();

	if ((NULL == self) || (NULL == upPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	participantId = EsifUp_GetInstance(upPtr);

	// Exit if already registered in this app
	participantDataMapPtr = EsifApp_GetParticipantDataMapFromInstance(self, participantId);
	if (participantDataMapPtr != NULL) {
		goto exit;
	}
	participantDataMapPtr = EsifApp_FindEmptyDataMap(self);
	if (NULL == participantDataMapPtr) {
		rc = ESIF_E_NO_CREATE;
		goto exit;
	}
	/*
	* Don't allow participant registration until creation is complete.
	* Participants will be registered after creation is done.
	*/
	if (!self->appCreationDone) {
		goto exit;
	}

	/* Create participant metadata to marshall though interface */
	participant_data_ptr = CreateParticipantData(upPtr, EsifUp_GetMetadata(upPtr));
	if (NULL == participant_data_ptr) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	participantDataMapPtr->fAppParticipantHandle = participantId;

	/* get reference on participant since we save a copy of pointer for later use*/
	rc = EsifUp_GetRef(upPtr);
	if (ESIF_OK != rc) {
		participantDataMapPtr->fUpPtr = NULL;
		goto exit;
	}

	participantDataMapPtr->fUpPtr = upPtr;								/* ESIF Participant */

	/* Call through the interface to create the participant instance in the app. */
	if (NULL == self->fInterface.fParticipantCreateFuncPtr) {
		rc = ESIF_E_IFACE_DISABLED;
	}
	if (rc == ESIF_OK) {
		rc = self->fInterface.fParticipantCreateFuncPtr(
			self->fAppCtxHandle,
			participantId,
			participant_data_ptr,
			eParticipantStateEnabled);
	}

	if ((ESIF_OK != rc) || (NULL == participantDataMapPtr->fUpPtr)) {
		EsifUp_PutRef(participantDataMapPtr->fUpPtr);
		participantDataMapPtr->fAppParticipantHandle = ESIF_INVALID_HANDLE;	/* Application Participant */
		participantDataMapPtr->fUpPtr = NULL;					/* ESIF Participant */
		goto exit;
	}

	participantDataMapPtr->isValid = ESIF_TRUE;

	rc = EsifApp_CreateDomains(self, upPtr, participantDataMapPtr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	/* Enable participant activity logging if it supports it */
	rc = EsifUpPm_ParticipantActivityLoggingEnable(upPtr);

exit:
	if (participant_data_ptr) {
		esif_ccb_free(participant_data_ptr);
		participant_data_ptr = NULL;
	}
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


static eEsifError EsifApp_DestroyDomain(
	EsifAppPtr self,
	AppParticipantDataMapPtr participantDataMapPtr,
	AppDomainDataMapPtr domainDataMapPtr
	)
{
	eEsifError rc = ESIF_OK;

	rc = self->fInterface.fDomainDestroyFuncPtr(
			self->fAppCtxHandle,
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

	EsifHandleMgr_PutHandle(domainDataMapPtr->fAppDomainHandle);
	esif_ccb_memset(domainDataMapPtr, 0, sizeof(*domainDataMapPtr));
	domainDataMapPtr->fAppDomainHandle = ESIF_INVALID_HANDLE;
	return rc;
}


static eEsifError EsifApp_DestroyDomains(
	EsifAppPtr self,
	AppParticipantDataMapPtr participantDataMapPtr
	)
{
	eEsifError rc = ESIF_OK;
	UInt8 i = 0;

	ESIF_TRACE_DEBUG("Destroy Domains\n");
	for (i = 0; i < MAX_DOMAIN_ENTRY; i++) {
		AppDomainDataMapPtr domainDataMapPtr = &participantDataMapPtr->fDomainData[i];
		if (ESIF_INVALID_HANDLE == domainDataMapPtr->fAppDomainHandle) {
			continue;
		}
		rc = EsifApp_DestroyDomain(self, participantDataMapPtr, domainDataMapPtr);
	}
	return rc;
}


eEsifError EsifApp_DestroyParticipant(
	const EsifAppPtr self,
	const EsifUpPtr upPtr
	)
{
	eEsifError rc = ESIF_OK;
	AppParticipantDataMapPtr participant_data_map_ptr = NULL;
	Bool isValid = ESIF_FALSE;

	if (NULL == self || NULL == upPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	participant_data_map_ptr = EsifApp_GetParticipantDataMapFromInstance(self, EsifUp_GetInstance(upPtr));
		
	// If created as NULL no need for callback.
	if (NULL == participant_data_map_ptr) {
		goto exit;
	}

	esif_ccb_write_lock(&self->objLock);
		isValid = participant_data_map_ptr->isValid;
		participant_data_map_ptr->isValid = ESIF_FALSE;
	esif_ccb_write_unlock(&self->objLock);

	if (!isValid) {
		goto exit;
	}

	// Best effort; ignore return
	EsifApp_DestroyDomains(self, participant_data_map_ptr);

	rc = self->fInterface.fParticipantDestroyFuncPtr(
			self->fAppCtxHandle,
			participant_data_map_ptr->fAppParticipantHandle);

	ESIF_TRACE_DEBUG("Destroyed participant in app " ESIF_HANDLE_FMT "; status %s(%d)\n",
		esif_ccb_handle2llu(participant_data_map_ptr->fAppParticipantHandle), esif_rc_str(rc), rc);
exit:
	if (self != NULL) {
		if ((participant_data_map_ptr != NULL) && isValid) {
			/* release reference on participant since we get reference on it in EsifApp_CreateParticipant */
			EsifUp_PutRef(participant_data_map_ptr->fUpPtr);
			EsifApp_ClearParticipantDataMap(participant_data_map_ptr);
		}
	}

	return rc;
}


static void EsifApp_ClearParticipantDataMap(AppParticipantDataMapPtr participantDataMapPtr)
{
	size_t i = 0;

	ESIF_ASSERT(participantDataMapPtr != NULL);

	esif_ccb_memset(participantDataMapPtr, 0, sizeof(*participantDataMapPtr));
	participantDataMapPtr->fAppParticipantHandle = ESIF_INVALID_HANDLE;

	for (i = 0; i < (sizeof(participantDataMapPtr->fDomainData) / sizeof(*participantDataMapPtr->fDomainData)); i++) {
		participantDataMapPtr->fDomainData[i].fAppDomainHandle = ESIF_INVALID_HANDLE;
	}
}

// Enable or Disable Policy Ativity Logging
void EsifApp_PolicyActivityLoggingEnable(Bool flag)
{
	static atomic_t g_policyLoggingRefCount = ATOMIC_INIT(0);
	if (ESIF_TRUE == flag) {
		atomic_inc(&g_policyLoggingRefCount);
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_DTT_POLICY_ACTIVITY_LOGGING_ENABLED, 0);
	}
	else if (atomic_dec(&g_policyLoggingRefCount) == 0) {
		EsifEventMgr_SignalEvent(ESIF_HANDLE_PRIMARY_PARTICIPANT, EVENT_MGR_DOMAIN_NA, ESIF_EVENT_DTT_POLICY_ACTIVITY_LOGGING_DISABLED, 0);
	}
}

/*
** PUBLIC
*/

eEsifError EsifApp_Start(EsifAppPtr self)
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	rc = EsifApp_Load(self);

	/*
	 * Participant registration with an application is best effort, as we
	 * should not unload an app if there is a failure while registering a
	 * participant.
	 */
	if ((!self->partRegDone) && ((ESIF_OK == rc) || (ESIF_E_APP_ALREADY_STARTED == rc))) {
		EsifApp_RegisterParticipantsWithApp(self);
	}

	/*
	* An app may be stopped while it is still creating.  Many items must be
	* created outside locks, so there may be a race for sections being created
	* and destroyed.  So, we double back after creation and see if we need to
	* stop again to clean everything up.
	*/
	if (self->stoppingApp) {
		EsifApp_Stop(self);
		rc = ESIF_E_NO_CREATE;
	}

	/* Notify App via Command that appstart sequence is completed */
	if ((self->appCreationDone) && ((ESIF_OK == rc) || (ESIF_E_APP_ALREADY_STARTED == rc))) {
		if (self->fInterface.fAppCommandFuncPtr) {
			char app_started[] = "app-started";
			u32 appname_len = (u32)esif_ccb_strlen(self->fAppNamePtr, ESIF_NAME_LEN) + 1;
			char response_buf[MAX_PATH] = { 0 };
			EsifData argv[2] = {
				{ ESIF_DATA_STRING, app_started, sizeof(app_started), sizeof(app_started) },
				{ ESIF_DATA_STRING, self->fAppNamePtr, appname_len, appname_len },
			};
			EsifData response = { ESIF_DATA_STRING, response_buf, sizeof(response_buf), 0 };
			self->fInterface.fAppCommandFuncPtr(self->fAppCtxHandle, ESIF_ARRAY_LEN(argv), argv, &response);
		}
	}

exit:
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


static eEsifError EsifApp_Load(EsifAppPtr self)
{
	eEsifError rc = ESIF_OK;
	GetIfaceFuncPtr iface_func_ptr = NULL;
	esif_string iface_func_name = GET_APPLICATION_INTERFACE_FUNCTION;
	char libPath[ESIF_LIBPATH_LEN] = { 0 };
	char altLibPath[ESIF_LIBPATH_LEN] = { 0 };
	char loadDir[ESIF_LIBPATH_LEN] = { 0 };

	ESIF_ASSERT(self != NULL);

	if (self->appCreationDone) {
		rc = ESIF_E_APP_ALREADY_STARTED;
		goto exit;
	}

	ESIF_TRACE_DEBUG("name=%s lib=%s\n", self->fAppNamePtr, self->fLibNamePtr);
	esif_build_path(libPath, ESIF_LIBPATH_LEN, ESIF_PATHTYPE_DLL, self->fLibNamePtr, ESIF_LIB_EXT);
	esif_build_path(loadDir, ESIF_LIBPATH_LEN, ESIF_PATHTYPE_DLL, NULL, NULL);

	self->fLibHandle = esif_ccb_library_load(libPath);

	if (NULL == self->fLibHandle || NULL == self->fLibHandle->handle) {

		// Try the alternate path for loadable libraries if different from normal path
		esif_build_path(altLibPath, ESIF_LIBPATH_LEN, ESIF_PATHTYPE_DLL_ALT, self->fLibNamePtr, ESIF_LIB_EXT);
		if (esif_ccb_strcmp(altLibPath, libPath) != 0) {
			esif_build_path(loadDir, ESIF_LIBPATH_LEN, ESIF_PATHTYPE_DLL_ALT, NULL, NULL);

			rc = esif_ccb_library_error(self->fLibHandle);
			ESIF_TRACE_WARN("esif_ccb_library_load() %s failed [%s (%d)]: %s\n", libPath, esif_rc_str(rc), rc, esif_ccb_library_errormsg(self->fLibHandle));

			esif_ccb_library_unload(self->fLibHandle);
			self->fLibHandle = NULL;
			self->fLibHandle = esif_ccb_library_load(altLibPath);
		}

		if (NULL == self->fLibHandle || NULL == self->fLibHandle->handle) {
			rc = esif_ccb_library_error(self->fLibHandle);
			ESIF_TRACE_ERROR("esif_ccb_library_load() %s failed [%s (%d)]: %s\n", altLibPath, esif_rc_str(rc), rc, esif_ccb_library_errormsg(self->fLibHandle));
			goto exit;
		}
		esif_ccb_strcpy(libPath, altLibPath, sizeof(libPath));
		rc = ESIF_OK;
	}
	esif_ccb_strcpy(self->loadDir, loadDir, sizeof(self->loadDir));


	ESIF_TRACE_DEBUG("esif_ccb_library_load() %s completed.\n", libPath);

	iface_func_ptr = (GetIfaceFuncPtr)esif_ccb_library_get_func(self->fLibHandle, (char*)iface_func_name);
	if (NULL == iface_func_ptr) {
		rc = esif_ccb_library_error(self->fLibHandle);
		ESIF_TRACE_DEBUG("esif_ccb_library_get_func() %s failed [%s (%d)]: %s\n", iface_func_name, esif_rc_str(rc), rc, esif_ccb_library_errormsg(self->fLibHandle));
		goto exit;
	}

	ESIF_TRACE_DEBUG("esif_ccb_library_get_func() %s completed.\n", iface_func_name);
	rc = EsifApp_CreateApp(self, iface_func_ptr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	// Enable policy activity logging before loading app
	EsifApp_PolicyActivityLoggingEnable(ESIF_TRUE);
	self->policyLoggingEnabled = ESIF_TRUE;
exit:
	if ((ESIF_OK != rc) && (rc != ESIF_E_APP_ALREADY_STARTED)) {
		esif_ccb_library_unload(self->fLibHandle);
		self->fLibHandle = NULL;
	}
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


static eEsifError EsifApp_RegisterParticipantsWithApp(
	EsifAppPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UfPmIteratorPtr upIterPtr = NULL;

	ESIF_ASSERT(self != NULL);

	upIterPtr = &self->upIter;

	if (!self->iteratorValid) {
		rc = EsifUpPm_InitIterator(upIterPtr);
		self->iteratorValid = ESIF_TRUE;
	}

	while (ESIF_OK == rc) {
		if (g_stopEsifUfInit != ESIF_FALSE) {
			ESIF_TRACE_API_INFO("Pausing participant registration with app\n");
			rc = ESIF_I_INIT_PAUSED;
			goto exit;
		}

		rc = EsifUpPm_GetNextUp(upIterPtr, &self->upPtr);
		if (rc != ESIF_OK) {
			break;
		}

		if (!EsifUp_IsPrimaryParticipant(self->upPtr)) {
			ESIF_TRACE_API_INFO("Creating participant in app\n");
			rc = EsifApp_CreateParticipant(self, self->upPtr);
			ESIF_TRACE_API_INFO("Participant creation in app complete (rc = %d)\n", rc);
		}
	};
exit:
	if (rc != ESIF_I_INIT_PAUSED) {
		EsifUp_PutRef(self->upPtr);
		self->upPtr = NULL;
		self->partRegDone = ESIF_TRUE;
		self->iteratorValid = ESIF_FALSE;
	}
	if (ESIF_E_ITERATION_DONE == rc) {
		ESIF_TRACE_DEBUG("EsifApp_RegisterParticipantsWithApp completed.\n");
		rc = ESIF_OK;
	}
	ESIF_TRACE_INFO("Register participants with App, status = %s\n", esif_rc_str(rc));
	return rc;
}


eEsifError EsifApp_Stop(EsifAppPtr self)
{
	eEsifError rc = ESIF_OK;
	esif_guid_t guid = APP_UNLOADING;
	EsifData dataGuid = { ESIF_DATA_GUID, &guid, sizeof(guid), sizeof(guid) };

	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	// Disable policy activity logging
	if (self->policyLoggingEnabled) {
		EsifApp_PolicyActivityLoggingEnable(ESIF_FALSE);
		self->policyLoggingEnabled = ESIF_FALSE;
	}

	/* Send APP_UNLOADED event directly to the App using the App Interface on this
	 * thread instead of using EventMgr Queue so that the App (or ABAT) can perform
	 * any pre-shutdown tasks synchronously before any Participants are destroyed.
	 */
	rc = EsifApp_SendEvent(self, ESIF_HANDLE_PRIMARY_PARTICIPANT, 0, NULL, &dataGuid);

	/*
	* An app may be stopped while it is still starting; however, because some data
	* elements must be accessed outside locks, we provide an indication to the app so
	* that it may rollback changes as needed and signall access completion
	*/
	self->stoppingApp = ESIF_TRUE; 

	rc = EsifApp_DestroyParticipants(self);
	ESIF_TRACE_DEBUG("EsifUpManagerDestroyParticipantsInApp completed.\n");

	/* Wait for any calls into the app to complete */
	EsifApp_WaitForAccessCompletion(self);
	
	if (self->fInterface.fAppDestroyFuncPtr) {
		rc = self->fInterface.fAppDestroyFuncPtr(self->fAppCtxHandle);
	}

	/* Inform arbitrator the ESIF app no longer requires arbitration */
	EsifArbMgr_RemoveApp(self->fHandle);
exit:
	return rc;
}


static void EsifApp_WaitForAccessCompletion(EsifAppPtr self)
{
	Bool mustWait = ESIF_FALSE;

	ESIF_ASSERT(self != NULL);

	esif_ccb_write_lock(&self->objLock);

	/* The manager will still maintain a reference; so check 1 not 0 */
	if (self->refCount > 1) {
		mustWait = ESIF_TRUE;
		esif_ccb_event_reset(&self->accessCompleteEvent);
	}
	esif_ccb_write_unlock(&self->objLock);

	if (mustWait) {
		esif_ccb_event_wait(&self->accessCompleteEvent);
	}
}


static eEsifError EsifApp_DestroyParticipants(EsifAppPtr self)
{
	AppParticipantDataMapPtr participantDataMapPtr = NULL;
	int i = 0;

	ESIF_ASSERT(self != NULL);

	participantDataMapPtr = self->fParticipantData;

	for (i = 0; i < (sizeof(self->fParticipantData) / sizeof(*self->fParticipantData)); i++, participantDataMapPtr++)
	{
		EsifApp_DestroyParticipant(self, participantDataMapPtr->fUpPtr);
	}
	ESIF_TRACE_INFO("Destroy participants in App\n");
	return ESIF_OK;
}


eEsifError EsifApp_SendEvent(
	EsifAppPtr self,
	esif_handle_t participantId,
	UInt16 domainId,
	const EsifDataPtr eventDataPtr,
	const EsifDataPtr eventGuidPtr
	)
{
	eEsifError rc = ESIF_OK;

	UNREFERENCED_PARAMETER(self);
	UNREFERENCED_PARAMETER(domainId);
	UNREFERENCED_PARAMETER(eventDataPtr);
	UNREFERENCED_PARAMETER(eventGuidPtr);

	esif_handle_t localParticipantId = ESIF_INVALID_HANDLE;
	esif_handle_t domainHandle = ESIF_INVALID_HANDLE;
	char guidStr[ESIF_GUID_PRINT_SIZE];

	UNREFERENCED_PARAMETER(guidStr);

	if ((NULL == self) || (NULL == eventGuidPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (!EsifUpPm_IsPrimaryParticipantId(participantId)) {
		rc = EsifApp_GetHandlesByIds(self, participantId, domainId, &localParticipantId, &domainHandle);
		if (rc != ESIF_OK) {
			ESIF_TRACE_WARN("Unable to get handles\n");
			goto exit;
		}
	}

	if (NULL == self->fInterface.fAppEventFuncPtr) {
		rc = ESIF_E_NOT_IMPLEMENTED;
		goto exit;
	}

	rc = self->fInterface.fAppEventFuncPtr(
		self->fAppCtxHandle,
		localParticipantId,
		domainHandle,
		eventDataPtr,
		eventGuidPtr);

	ESIF_TRACE_DEBUG(
		"Sending app event:\n"
		"  appHandle:         %p\n"
		"  localParticipantId: " ESIF_HANDLE_FMT "\n"
		"  domainHandle:     " ESIF_HANDLE_FMT "\n"
		"  eventGuid:         %s\n",
		self->fAppCtxHandle,
		esif_ccb_handle2llu(localParticipantId),
		esif_ccb_handle2llu(domainHandle),
		esif_guid_print((esif_guid_t *)eventGuidPtr->buf_ptr, guidStr));
exit:
	ESIF_TRACE_DEBUG("Exit Code = %d\n", rc);
	return rc;
}


/* Lookup participant data for a handle */
AppParticipantDataMapPtr EsifApp_GetParticipantDataMapFromInstance(
	const EsifAppPtr self,
	const esif_handle_t participantId
)
{
	UInt8 i = 0;
	AppParticipantDataMapPtr upDataMapPtr = NULL;

	if (EsifUpPm_IsPrimaryParticipantId(participantId) || (ESIF_INVALID_HANDLE == participantId)) {
		return upDataMapPtr;
	}

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		if (self->fParticipantData[i].fAppParticipantHandle == participantId) {
			upDataMapPtr = &self->fParticipantData[i];
			break;
		}
	}

	if (NULL == upDataMapPtr) {
		ESIF_TRACE_DEBUG("Unable to find participant data map for participant handle\n");
	}

	return upDataMapPtr;
}


/* Find an empty slot in the participant data map to use */
static AppParticipantDataMapPtr EsifApp_FindEmptyDataMap(
	const EsifAppPtr self
	)
{
	size_t i = 0;
	AppParticipantDataMapPtr upDataMapPtr = NULL;

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		if (ESIF_INVALID_HANDLE == self->fParticipantData[i].fAppParticipantHandle) {
			upDataMapPtr = &self->fParticipantData[i];
			break;
		}
	}

	if (NULL == upDataMapPtr) {
		ESIF_TRACE_DEBUG("Unable to find empty participant data map\n");
	}

	return upDataMapPtr;
}


AppDomainDataMapPtr EsifApp_GetDomainDataMapFromHandle(
	const AppParticipantDataMapPtr upMapPtr,
	const esif_handle_t domainHandle
	)
{
	UInt8 i = 0;
	AppDomainDataMapPtr domainDataMapPtr = NULL;

	for (i = 0; i < MAX_DOMAIN_ENTRY; i++) {
		if (upMapPtr->fDomainData[i].fAppDomainHandle == domainHandle) {
			domainDataMapPtr = &upMapPtr->fDomainData[i];
			break;
		}
	}

	if (NULL == domainDataMapPtr) {
		ESIF_TRACE_DEBUG("Unable to find domain data map for participant handle\n");
	}

	return domainDataMapPtr;
}


eEsifError EsifApp_GetDomainIdByHandle(
	EsifAppPtr self,
	const esif_handle_t upHandle,
	const esif_handle_t domainHandle,
	UInt16 *domainIdPtr
	)
{
	eEsifError rc = ESIF_OK;
	AppParticipantDataMapPtr upMapPtr = NULL;
	AppDomainDataMapPtr domainPtr = NULL;

	if ((NULL == self) || (NULL == domainIdPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	upMapPtr = EsifApp_GetParticipantDataMapFromInstance(self, upHandle);
	if (NULL == upMapPtr) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	domainPtr = EsifApp_GetDomainDataMapFromHandle(upMapPtr, domainHandle);
	if (NULL == domainPtr) {
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	*domainIdPtr = domainPtr->fQualifierId;
exit:
	return rc;
}


char *EsifApp_GetDomainQalifierByHandle(
	EsifAppPtr self,
	const esif_handle_t upHandle, 
	const esif_handle_t domainHandle
	)
{
	char *qualifier = NULL;
	AppParticipantDataMapPtr upMapPtr = NULL;
	AppDomainDataMapPtr domainPtr = NULL;

	if (NULL == self) {
		goto exit;
	}

	upMapPtr = EsifApp_GetParticipantDataMapFromInstance(self, upHandle);
	if (NULL == upMapPtr) {
		goto exit;
	}

	domainPtr = EsifApp_GetDomainDataMapFromHandle(upMapPtr, domainHandle);
	if (NULL == domainPtr) {
		goto exit;
	}

	qualifier = domainPtr->fQualifier;
exit:
	return qualifier;
}


eEsifError EsifApp_GetHandlesByIds(
	EsifAppPtr self,
	esif_handle_t participantId,
	UInt16 domainId,
	esif_handle_t *upHandlePtr,
	esif_handle_t *domainHandlePtr
	)
{
	eEsifError rc = ESIF_OK;
	AppParticipantDataMapPtr upDataMapPtr = NULL;
	UInt8 domainIndex = 0;

	if ((NULL == self) || (NULL == upHandlePtr) || (NULL == domainHandlePtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	upDataMapPtr = EsifApp_GetParticipantDataMapFromInstance(self, participantId);
	if (NULL == upDataMapPtr) {
		rc = ESIF_E_PARTICIPANT_NOT_FOUND;
		goto exit;
	}

	EsifDomainIdToIndex(domainId, &domainIndex);
	if (domainIndex < MAX_DOMAIN_ENTRY) {
		*domainHandlePtr = upDataMapPtr->fDomainData[domainIndex].fAppDomainHandle;
	}

	*upHandlePtr = upDataMapPtr->fAppParticipantHandle;
exit:
	return rc;
}


Bool EsifApp_IsAppName(
	EsifAppPtr self,
	const EsifString name
	)
{
	Bool isName = ESIF_FALSE;
	EsifString appName = (EsifString)name;

	if (self && self->fAppNamePtr) {
		if (*appName == ESIF_APP_CLIENT_INDICATOR) {
			appName++;
		}
		size_t appNameLen = esif_ccb_strlen(appName, APPNAME_MAXLEN); // appName[=libName]
		size_t selfNameLen = esif_ccb_strlen(self->fAppNamePtr, APPNAME_MAXLEN);
		EsifString libName = esif_ccb_strchr(appName, APPNAME_SEPARATOR);
		if (libName) {
			appNameLen -= esif_ccb_strlen(libName, APPNAME_MAXLEN - (size_t)(libName - appName));
		}
		if (selfNameLen == appNameLen && esif_ccb_strnicmp(self->fAppNamePtr, appName, appNameLen) == 0) {
			isName = ESIF_TRUE;
		}
	}
	return isName;
}


Bool EsifApp_IsAppHandle(
	EsifAppPtr self,
	const esif_handle_t handle
	)
{
	Bool isHandle = ESIF_FALSE;

	if (self && (handle == self->fHandle)) {
		isHandle = ESIF_TRUE;
	}
	return isHandle;
}


eEsifError EsifApp_SuspendApp(
	EsifAppPtr self
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (NULL == self->fInterface.fAppSuspendFuncPtr) {
		rc = ESIF_E_NOT_IMPLEMENTED;
		goto exit;
	}

	if (!self->appCreationDone) {
		goto exit;
	}

	rc = self->fInterface.fAppSuspendFuncPtr(self->fAppCtxHandle);

exit:
	return rc;
}


eEsifError EsifApp_ResumeApp(
	EsifAppPtr self
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (NULL == self->fInterface.fAppResumeFuncPtr) {
		rc = ESIF_E_NOT_IMPLEMENTED;
		goto exit;
	}
	if (!self->appCreationDone  && !self->stoppingApp) {
		goto exit;
	}

	rc = self->fInterface.fAppResumeFuncPtr(self->fAppCtxHandle);

exit:
	return rc;
}


eEsifError EsifApp_GetDescription(
	EsifAppPtr self,
	EsifDataPtr descPtr
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (self->fAppDescPtr) {
		u32 data_len = (u32)esif_ccb_strlen(self->fAppDescPtr, ESIF_DESC_LEN);
		if (descPtr == NULL || descPtr->buf_ptr == NULL) {
			rc = ESIF_E_PARAMETER_IS_NULL;
		}
		else if (descPtr->buf_len < data_len) {
			descPtr->data_len = data_len;
			rc = ESIF_E_NEED_LARGER_BUFFER;
		}
		else {
			esif_ccb_strcpy(descPtr->buf_ptr, self->fAppDescPtr, descPtr->buf_len);
			descPtr->data_len = (u32)esif_ccb_strlen(descPtr->buf_ptr, descPtr->buf_len);
			rc = ESIF_OK;
		}
	}
	else {
		if (NULL == self->fInterface.fAppGetDescriptionFuncPtr) {
			rc = ESIF_E_NOT_IMPLEMENTED;
			goto exit;
		}

		if (descPtr && descPtr->buf_ptr && descPtr->buf_len && descPtr->data_len == 0) {
			esif_ccb_strcpy(descPtr->buf_ptr, self->fAppNamePtr, descPtr->buf_len);
			descPtr->data_len = (UInt32)esif_ccb_strlen(descPtr->buf_ptr, descPtr->buf_len);
		}

		rc = self->fInterface.fAppGetDescriptionFuncPtr(descPtr);

		if (rc == ESIF_OK && self->fAppDescPtr == NULL && descPtr && descPtr->buf_ptr) {
			self->fAppDescPtr = esif_ccb_strdup(descPtr->buf_ptr);
		}
	}

exit:
	if (rc != ESIF_OK && rc != ESIF_E_NEED_LARGER_BUFFER && descPtr && descPtr->buf_ptr && descPtr->buf_len) {
		esif_ccb_strcpy(descPtr->buf_ptr, "NA", descPtr->buf_len);
		descPtr->data_len = (UInt32)esif_ccb_strlen(descPtr->buf_ptr, descPtr->buf_len);
	}
	return rc;
}


eEsifError EsifApp_GetVersion(
	EsifAppPtr self,
	EsifDataPtr versionPtr
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (self->fAppVersionPtr) {
		u32 data_len = (u32)esif_ccb_strlen(self->fAppVersionPtr, ESIF_VERSION_LEN);
		if (versionPtr == NULL || versionPtr->buf_ptr == NULL) {
			rc = ESIF_E_PARAMETER_IS_NULL;
		}
		else if (versionPtr->buf_len < data_len) {
			versionPtr->data_len = data_len;
			rc = ESIF_E_NEED_LARGER_BUFFER;
		}
		else {
			esif_ccb_strcpy(versionPtr->buf_ptr, self->fAppVersionPtr, versionPtr->buf_len);
			versionPtr->data_len = (u32)esif_ccb_strlen(versionPtr->buf_ptr, versionPtr->buf_len);
			rc = ESIF_OK;
		}
	}
	else {
		if (NULL == self->fInterface.fAppGetVersionFuncPtr) {
			rc = ESIF_E_NOT_IMPLEMENTED;
			goto exit;
		}

		if (versionPtr && versionPtr->buf_ptr && versionPtr->buf_len && versionPtr->data_len == 0) {
			esif_ccb_strcpy((char *)versionPtr->buf_ptr, self->fAppNamePtr, versionPtr->buf_len);
			versionPtr->data_len = (UInt32)esif_ccb_strlen((char *)versionPtr->buf_ptr, versionPtr->buf_len);
		}

		rc = self->fInterface.fAppGetVersionFuncPtr(versionPtr);

		if (rc == ESIF_OK && self->fAppVersionPtr == NULL && versionPtr && versionPtr->buf_ptr) {
			self->fAppVersionPtr = esif_ccb_strdup(versionPtr->buf_ptr);
		}
	}

exit:
	if (rc != ESIF_OK && rc != ESIF_E_NEED_LARGER_BUFFER && versionPtr && versionPtr->buf_ptr && versionPtr->buf_len) {
		esif_ccb_strcpy(versionPtr->buf_ptr, "NA", versionPtr->buf_len);
		versionPtr->data_len = (UInt32)esif_ccb_strlen(versionPtr->buf_ptr, versionPtr->buf_len);
	}
	return rc;
}

eEsifError EsifApp_GetIntro(
	EsifAppPtr self,
	EsifDataPtr introPtr
)
{
	eEsifError rc = ESIF_OK;

	if (self->fAppIntroPtr) {
		u32 data_len = (u32)esif_ccb_strlen(self->fAppIntroPtr, BANNER_LEN);
		if (introPtr == NULL || introPtr->buf_ptr == NULL) {
			rc = ESIF_E_PARAMETER_IS_NULL;
		}
		else if (introPtr->buf_len < data_len) {
			introPtr->data_len = data_len;
			rc = ESIF_E_NEED_LARGER_BUFFER;
		}
		else {
			esif_ccb_strcpy(introPtr->buf_ptr, self->fAppIntroPtr, introPtr->buf_len);
			introPtr->data_len = (u32)esif_ccb_strlen(introPtr->buf_ptr, introPtr->buf_len);
			rc = ESIF_OK;
		}
	}
	else {
		if (NULL == self->fInterface.fAppGetIntroFuncPtr) {
			rc = ESIF_E_NOT_IMPLEMENTED;
			goto exit;
		}

		rc = self->fInterface.fAppGetIntroFuncPtr(self->fAppCtxHandle, introPtr);

		if (rc == ESIF_OK && self->fAppIntroPtr == NULL && introPtr && introPtr->buf_ptr) {
			self->fAppIntroPtr = esif_ccb_strdup(introPtr->buf_ptr);
		}
	}

exit:
	return rc;
}

eEsifError EsifApp_SendCommand(
	EsifAppPtr self,
	const UInt32 argc,
	EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
)
{
	eEsifError rc = ESIF_OK;

	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (NULL == self->fInterface.fAppCommandFuncPtr) {
		rc = ESIF_E_NOT_IMPLEMENTED;
		goto exit;
	}

	rc = self->fInterface.fAppCommandFuncPtr(self->fAppCtxHandle, argc, requestPtr, responsePtr);
	
exit:
	return rc;
}


eEsifError EsifApp_GetStatus(
	EsifAppPtr self,
	const eAppStatusCommand command,
	const UInt32 statusIn,	/* Command Data (High Word Group, Low Word Module) */
	EsifDataPtr statusPtr	/* Status output string if XML please use ESIF_DATA_XML */
	)
{
	eEsifError rc = ESIF_OK;

	if (NULL == self) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}
	rc = self->fInterface.fAppGetStatusFuncPtr(self->fAppCtxHandle, command, statusIn, statusPtr);

exit:
	return rc;
}



/*
* Used to iterate through the app participant data.
* First call EsifApp_InitPartIterator to initialize the iterator.
* Next, call EsifApp_GetNextPart using the iterator.  Repeat until
* EsifApp_GetNextPart fails. Iteration is complete when ESIF_E_ITERATOR_DONE
* is returned.
*/
eEsifError EsifApp_InitPartIterator(
	EsifAppPartDataIteratorPtr iteratorPtr
)
{
	eEsifError rc = ESIF_OK;

	if (NULL == iteratorPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_memset(iteratorPtr, 0, sizeof(*iteratorPtr));
	iteratorPtr->marker = ESIFAPP_PART_DATA_ITERATOR_MARKER;
	iteratorPtr->index = ESIF_APP_ITERATOR_INVALID;
exit:
	return rc;
}


/* See EsifApp_InitPartIterator for usage */
eEsifError EsifApp_GetNextPart(
	EsifAppPartDataIteratorPtr iteratorPtr,
	EsifAppPtr self,
	AppParticipantDataMapPtr *dataPtr
	)
{
	eEsifError rc = ESIF_OK;
	AppParticipantDataMapPtr nextPtr = NULL;

	if ((NULL == iteratorPtr) || (NULL == self) || (NULL == dataPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Verify the iterator is initialized */
	if (iteratorPtr->marker != ESIFAPP_PART_DATA_ITERATOR_MARKER) {
		ESIF_TRACE_WARN("Iterator invalid\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	iteratorPtr->index++;
	while (iteratorPtr->index < MAX_DOMAIN_ENTRY) {
		nextPtr = &self->fParticipantData[iteratorPtr->index];
		if ((nextPtr->fAppParticipantHandle != ESIF_INVALID_HANDLE) && (nextPtr->fAppParticipantHandle != ESIF_HANDLE_DEFAULT)) {
			*dataPtr = nextPtr;
			goto exit;
		}
		iteratorPtr->index++;
	}
	rc = ESIF_E_ITERATION_DONE;
exit:
	return rc;
}


/*
* Used to iterate through the app participant domain data.
* First call EsifApp_InitDomainIterator to initialize the iterator.
* Next, call EsifApp_GetNextDomain using the iterator.  Repeat until
* EsifApp_GetNextDomain fails. Iteration is complete when ESIF_E_ITERATOR_DONE
* is returned.
*/
eEsifError EsifApp_InitDomainIterator(
	EsifAppDomainDataIteratorPtr iteratorPtr
)
{
	eEsifError rc = ESIF_OK;

	if (NULL == iteratorPtr) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_memset(iteratorPtr, 0, sizeof(*iteratorPtr));
	iteratorPtr->marker = ESIFAPP_DOMAIN_DATA_ITERATOR_MARKER;
	iteratorPtr->index = ESIF_APP_ITERATOR_INVALID;
exit:
	return rc;
}


/* See EsifApp_InitDomainIterator for usage */
eEsifError EsifApp_GetNextDomain(
	EsifAppDomainDataIteratorPtr iteratorPtr,
	AppParticipantDataMapPtr partPtr,
	AppDomainDataMapPtr *dataPtr
)
{
	eEsifError rc = ESIF_OK;
	AppDomainDataMapPtr nextPtr = NULL;

	if ((NULL == iteratorPtr) || (NULL == partPtr) || (NULL == dataPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Verify the iterator is initialized */
	if (iteratorPtr->marker != ESIFAPP_DOMAIN_DATA_ITERATOR_MARKER) {
		ESIF_TRACE_WARN("Iterator invalid\n");
		rc = ESIF_E_INVALID_HANDLE;
		goto exit;
	}

	iteratorPtr->index++;
	while (iteratorPtr->index < MAX_DOMAIN_ENTRY) {
		nextPtr = &partPtr->fDomainData[iteratorPtr->index];
		if ((nextPtr->fAppDomainHandle != ESIF_INVALID_HANDLE) && (nextPtr->fAppDomainHandle != ESIF_HANDLE_DEFAULT)) {
			*dataPtr = nextPtr;
			goto exit;
		}
		iteratorPtr->index++;
	}
	rc = ESIF_E_ITERATION_DONE;
exit:
	return rc;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
