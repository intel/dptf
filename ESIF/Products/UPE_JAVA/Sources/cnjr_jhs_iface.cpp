/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "esif_sdk_iface_conjure.h"	/* Conjure Interface */
#include "esif_sdk_class_guid.h"	/* Class GUIDs */

#include "esif_ccb_library.h"
#include "esif_ccb_memory.h"
#include "esif_ccb_string.h"

#include "esif_ccb_thread.h"
#include "conjure.h"

#ifdef ESIF_ATTR_OS_ANDROID
#include "jhs_binder_service.h"
#include <android/log.h>
using namespace jhs;
#endif

using namespace std;

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "..\ESIF\Products\ESIF_CM\Sources\win\banned.h"
#endif

// Application Description
#define CONJURE_LIB_NAME    "Conjure1"
#define CONJURE_LIB_DESC    "First Conjure Library"
#define CONJURE_LIB_GUID    {0x75, 0x04, 0x6a, 0x5a, 0xd7, 0x3f, 0x43, 0x7a, 0x8d, 0xfc, 0x3a, 0x8a, 0x75, 0x43, 0x6f, 0x7a}

#define DEFAULT_DRIVER_NAME	""
#define DEFAULT_DEVICE_NAME	""

/* TODO: Update with the version of the conjure (informational only) */
#define CONJURE_CODE_VERSION 1

// Debug
#define IFACE_TAG "esif_uf_cnj_iface -->"
#ifdef ESIF_ATTR_OS_ANDROID
	#define TRACE_LEVEL_ERROR	ANDROID_LOG_ERROR
	#define TRACE_LEVEL_INFORMATION	ANDROID_LOG_INFO
	#define TRACE_LEVEL_VERBOSE	ANDROID_LOG_DEBUG
	#define DBG_ESIF_ACT_APP	"UPE_JAVA"
	#define DBG_ESIF_CNJ_LIB	"UPE_JAVA"
	#define ESIF_TRACE(level, type, format, ...) __android_log_print(level, type, format, ##__VA_ARGS__)
#else
	#define ESIF_TRACE(level, type, format, ...)
#endif

static void* cjrClientService(void *ptr);

static esif_thread_t g_jhcs_thread;

EsifConjureServiceInterface g_cnjService;
void *g_esifHandle = NULL;


struct fake_conjure {
	u8  dummy;
};

// The SIGUSR1 handler below is the alternative
// solution for Android where the pthread_cancel()
// is not implemented in Android NDK

/* SIGUSR1 Signal Handler */
static void sigusr1_handler(int signum)
{
        pthread_exit(0);
}

/* Enable or Disable SIGTERM Handler */
static void sigusr1_enable()
{
        struct sigaction action={0};
        action.sa_handler = sigusr1_handler;
        sigaction(SIGUSR1, &action, NULL);
}


// Conjure Instance Create
static eEsifError ConjureCreate (
	EsifConjureServiceInterfacePtr cnjServiceInterfacePtr,
	const void *esifHandle,
	void * *cnjHandleLocation
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE(TRACE_LEVEL_INFORMATION, DBG_ESIF_ACT_APP, "%s %s[%s]", IFACE_TAG, CONJURE_LIB_NAME, ESIF_FUNC);
	ESIF_TRACE(TRACE_LEVEL_VERBOSE, DBG_ESIF_ACT_APP, "%s cnjServiceInterfacePtr [0x%p]", ESIF_FUNC, cnjServiceInterfacePtr);
	ESIF_TRACE(TRACE_LEVEL_VERBOSE, DBG_ESIF_ACT_APP, "%s esifHandle [0x%p]", ESIF_FUNC, esifHandle);
	ESIF_TRACE(TRACE_LEVEL_VERBOSE, DBG_ESIF_ACT_APP, "%s cnjHandleLocation [0x%p]", ESIF_FUNC, cnjHandleLocation);

	ESIF_ASSERT(cnjServiceInterfacePtr != NULL);
	ESIF_ASSERT(cnjHandleLocation != NULL);

	/* make a copy of the EsifAppServicesInterface */
	esif_ccb_memcpy(&g_cnjService, cnjServiceInterfacePtr, sizeof(*cnjServiceInterfacePtr));

	/* Verify EsifAppServices Interface */
	if (g_cnjService.fIfaceType != eIfaceTypeConjureService ||
		g_cnjService.fIfaceSize != (UInt16)sizeof(EsifConjureServiceInterface) ||
		g_cnjService.fIfaceVersion != 1 ||

		/* Functions Pointers */
		g_cnjService.fRegisterParticipantFuncPtr == NULL ||
		g_cnjService.fUnRegisterParticipantFuncPtr == NULL) {
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	*cnjHandleLocation = esif_ccb_malloc(sizeof(struct fake_conjure));
	ESIF_TRACE(TRACE_LEVEL_VERBOSE, DBG_ESIF_ACT_APP, "%s cnjHandle [0x%p]", ESIF_FUNC, *cnjHandleLocation);
	g_esifHandle = (void*)esifHandle;

	esif_ccb_thread_create(&g_jhcs_thread, cjrClientService, NULL);

exit:

	return rc;
}

static void* cjrClientService(void *ptr) {
#ifdef ESIF_ATTR_OS_ANDROID
	UNREFERENCED_PARAMETER(ptr);

	sigusr1_enable();

	sp<IServiceManager> sm = defaultServiceManager();
	ESIF_ASSERT(sm != NULL);
	sm->addService(String16(JHS_CLIENT_SERVICE_NAME), new JhsClientService());
	ProcessState::self()->startThreadPool();
	ESIF_TRACE(TRACE_LEVEL_INFORMATION, DBG_ESIF_ACT_APP, "JHS client service started...");
	IPCThreadState::self()->joinThreadPool();
#endif
	return NULL;
}


// Destroy Conjure Instance
static eEsifError ConjureDestroy (void *cnjHandle)
{
	ESIF_TRACE(TRACE_LEVEL_INFORMATION, DBG_ESIF_ACT_APP, "%s %s[%s]", IFACE_TAG, CONJURE_LIB_NAME, ESIF_FUNC);
	ESIF_TRACE(TRACE_LEVEL_VERBOSE, DBG_ESIF_ACT_APP, "%s cnjHandle [0x%p]", ESIF_FUNC, cnjHandle);

	ESIF_ASSERT(cnjHandle != NULL);

	esif_ccb_free(cnjHandle);

	return ESIF_OK;
}


esif_handle_t RegisterParticipant(const EsifParticipantIfacePtr piPtr)
{
	esif_handle_t participantInstance = (esif_handle_t)ESIF_INSTANCE_INVALID;

	ESIF_TRACE(TRACE_LEVEL_INFORMATION, DBG_ESIF_CNJ_LIB, "%s\n", ESIF_FUNC);

	g_cnjService.fRegisterParticipantFuncPtr(piPtr, &participantInstance);

	return participantInstance;
}

eEsifError UnRegisterParticipant(const esif_handle_t participantHandle)
{
	ESIF_TRACE(TRACE_LEVEL_INFORMATION, DBG_ESIF_CNJ_LIB, "%s\n", ESIF_FUNC);

	g_cnjService.fUnRegisterParticipantFuncPtr(participantHandle);

	return ESIF_OK;
}



///////////////////////////////////////////////////////////////////////////////
// Participant
///////////////////////////////////////////////////////////////////////////////



// Get Application Interface Pointers
extern "C" ESIF_EXPORT eEsifError ESIF_CALLCONV GetConjureInterface(EsifConjureInterfacePtr ifacePtr)
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE(TRACE_LEVEL_INFORMATION, DBG_ESIF_ACT_APP, "%s %s[%s]", IFACE_TAG, CONJURE_LIB_NAME, ESIF_FUNC);
	ESIF_TRACE(TRACE_LEVEL_VERBOSE, DBG_ESIF_ACT_APP, "%s cnjInterface [0x%p]", ESIF_FUNC, ifacePtr);

	if (NULL == ifacePtr) {
		ESIF_TRACE(TRACE_LEVEL_ERROR, DBG_ESIF_ACT_APP, "Conjure interface pointer is NULL");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if ((ifacePtr->hdr.fIfaceType != eIfaceTypeConjure) ||
		(ifacePtr->hdr.fIfaceSize < sizeof(*ifacePtr))) {
		ESIF_TRACE(TRACE_LEVEL_ERROR, DBG_ESIF_ACT_APP, "Interface not supported: Type = %d, size = %d; Req %d:%lu",
			ifacePtr->hdr.fIfaceType,
			ifacePtr->hdr.fIfaceSize,
			eIfaceTypeConjure,
			(unsigned long)sizeof(*ifacePtr));
		rc = ESIF_E_NOT_SUPPORTED;
		goto exit;
	}

	ifacePtr->hdr.fIfaceVersion = CONJURE_IFACE_VERSION;
	ifacePtr->hdr.fIfaceSize    = (UInt16)sizeof(*ifacePtr);

	ifacePtr->cnjVersion = CONJURE_CODE_VERSION;

	esif_ccb_strcpy(ifacePtr->name, CONJURE_LIB_NAME, sizeof(ifacePtr->name));
	esif_ccb_strcpy(ifacePtr->desc, CONJURE_LIB_DESC, sizeof(ifacePtr->desc));

	// Conjure
	ifacePtr->fConjureCreateFuncPtr  = ConjureCreate;
	ifacePtr->fConjureDestroyFuncPtr = ConjureDestroy;
exit:
	return rc;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
