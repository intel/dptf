/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_UF_APP_
#define _ESIF_UF_APP_

#include "esif.h"
#include "esif_sdk_iface_app.h"
#include "esif_participant.h"
#include "esif_pm.h"

#define MAX_DOMAIN_ENTRY 10
#define APP_DOMAIN_VERSION 1
#define ESIFAPP_PART_DATA_ITERATOR_MARKER 'EAPD'
#define ESIFAPP_DOMAIN_DATA_ITERATOR_MARKER 'EADD'
#define ESIF_APP_ITERATOR_INVALID ((size_t)(-1))
#define	APPNAME_MAXLEN		MAX_PATH
#define	APPNAME_SEPARATOR	'='

/*
** Hierchary
** Application
**      Participants
**          Domain
*/

/* Map App Domain Handle To ESIF Domain Data */
typedef struct _t_AppDomainDataMap {
	UInt8  fAppDomainId;
	esif_handle_t fAppDomainHandle;
	AppDomainDataPtr  fAppDomainDataPtr;
	esif_string       fQualifier;
	UInt16  fQualifierId;
} AppDomainDataMap, *AppDomainDataMapPtr, **AppDomainDataMapPtrLocation;

/* Map App Domain Handle To ESIF Participant Data */
typedef struct _t_AppParticipantDataMap {
	EsifUpPtr  fUpPtr;
	esif_handle_t fAppParticipantHandle;
	//
	// Indicates if the handle is valid; used to prevent destruction by two
	// threads: one for EsifApp_DestroyParticipants during AppStop, and one from
	// the queue during S3 for example
	//
	Bool isValid; 

	/* Each Participant May Have Many Domains */
	AppDomainDataMap  fDomainData[MAX_DOMAIN_ENTRY];
} AppParticipantDataMap, *AppParticipantDataMapPtr, *AppParticipantDataMapPtrLocation;

/* Map App Data To ESIF Prticipants */
typedef struct _t_EsifApp {
	esif_handle_t fHandle;				/* The ESIF handle to associate the app */
	esif_handle_t fAppCtxHandle;		/* The Application Handle (Opaque to ESIF) */
	AppInterface  fInterface;			/* The Application Interface */
	EsifString    fAppNamePtr;			/* The Name of the Application */
	EsifString    fLibNamePtr;			/* The Name Of The Library To Load (same as fAppNamePtr by default) */
	esif_lib_t    fLibHandle;			/* Loadable Library Handle */
	Bool isRestartable;					/* App is Restartable */
	char loadDir[MAX_PATH];				/* Directory were the app was loaded from */

	/* Each Application May Have Many Participants */
	AppParticipantDataMap  fParticipantData[MAX_PARTICIPANT_ENTRY];

	/* State information for pausing initialization */
	Bool appCreationDone;
	Bool partRegDone;
	Bool iteratorValid;
	UfPmIterator upIter;
	EsifUpPtr upPtr;

	/* Indicates if policy logging needs to be disabled */
	Bool policyLoggingEnabled;

	/* Lifetime Management Members */
	UInt32 refCount;
	Bool stoppingApp;
	Bool markedForDelete;
	esif_ccb_event_t deleteEvent;
	esif_ccb_event_t accessCompleteEvent;
	esif_ccb_lock_t objLock;

} EsifApp, *EsifAppPtr, **EsifAppPtrLocation;

typedef struct EsifAppPartDataIterator_s {
	UInt32 marker;
	size_t index;
} EsifAppPartDataIterator, *EsifAppPartDataIteratorPtr;

typedef struct EsifAppDomainDataIterator_s {
	UInt32 marker;
	size_t index;
} EsifAppDomainDataIterator, *EsifAppDomainDataIteratorPtr;


static ESIF_INLINE EsifString EsifApp_GetAppName(
	EsifAppPtr self
)
{
	return (self != NULL) ? self->fAppNamePtr : "UNK";
}

static ESIF_INLINE EsifString EsifApp_GetLibName(
	EsifAppPtr self
	)
{
	return (self != NULL) ? self->fLibNamePtr : "UNK";
}

// Caller responsible for freeing result
static ESIF_INLINE EsifString EsifApp_CopyAppFullName(
	EsifAppPtr self
)
{
	EsifString result = NULL;
	if (self && self->fLibNamePtr && self->fAppNamePtr) {
		size_t appNameLen = esif_ccb_strlen(self->fAppNamePtr, APPNAME_MAXLEN) + 1 + esif_ccb_strlen(self->fLibNamePtr, APPNAME_MAXLEN) + 1;
		result = (EsifString)esif_ccb_malloc(appNameLen);
		if (result) {
			esif_ccb_strcpy(result, self->fAppNamePtr, appNameLen);
			if (esif_ccb_stricmp(self->fAppNamePtr, self->fLibNamePtr) != 0) {
				esif_ccb_sprintf_concat(appNameLen, result, "%c%s", APPNAME_SEPARATOR, self->fLibNamePtr);
			}
		}
	}
	return result;
}

static ESIF_INLINE Bool EsifApp_IsRestartable(
	EsifAppPtr self
)
{
	return (self != NULL) ? self->isRestartable : ESIF_FALSE;
}

static ESIF_INLINE esif_handle_t EsifApp_GetAppHandle(
	EsifAppPtr self
	)
{
	return (self != NULL) ? self->fHandle : ESIF_INVALID_HANDLE;
}

static ESIF_INLINE esif_handle_t EsifApp_GetPartHandle(
	AppParticipantDataMapPtr self
	)
{
	return (self != NULL) ? self->fAppParticipantHandle : ESIF_INVALID_HANDLE;
}

static ESIF_INLINE esif_handle_t EsifApp_GetDomainHandle(
	AppDomainDataMapPtr self
	)
{
	return (self != NULL) ? self->fAppDomainHandle : ESIF_INVALID_HANDLE;
}

static ESIF_INLINE EsifString EsifApp_GetDomainQualifier(
	AppDomainDataMapPtr self
	)
{
	return (self != NULL) ? self->fQualifier : "UNK";
}

static ESIF_INLINE EsifUpPtr EsifApp_GetParticipant(
	AppParticipantDataMapPtr self
	)
{
	return (self != NULL) ? self->fUpPtr : NULL;
}



#ifdef __cplusplus
extern "C" {
#endif

/*
* Used to iterate through the app participant data.
* First call EsifApp_InitPartIterator to initialize the iterator.
* Next, call EsifApp_GetNextPart using the iterator.  Repeat until
* EsifApp_GetNextPart fails. Iteration is complete when ESIF_E_ITERATOR_DONE
* is returned.
*/
eEsifError EsifApp_InitPartIterator(
	EsifAppPartDataIteratorPtr iteratorPtr
	);

eEsifError EsifApp_GetNextPart(
	EsifAppPartDataIteratorPtr iteratorPtr,
	EsifAppPtr self,
	AppParticipantDataMapPtr *dataPtr
	);

/*
* Used to iterate through the app participant domain data.
* First call EsifApp_InitDomainIterator to initialize the iterator.
* Next, call EsifApp_GetNextDomain using the iterator.  Repeat until
* EsifApp_GetNextDomain fails. Iteration is complete when ESIF_E_ITERATOR_DONE
* is returned.
*/
eEsifError EsifApp_InitDomainIterator(
	EsifAppDomainDataIteratorPtr iteratorPtr
	);

eEsifError EsifApp_GetNextDomain(
	EsifAppDomainDataIteratorPtr iteratorPtr,
	AppParticipantDataMapPtr partPtr,
	AppDomainDataMapPtr *dataPtr
	);

eEsifError EsifApp_SendEvent(
	EsifAppPtr self,
	esif_handle_t participantId,
	UInt16 domainId,
	const EsifDataPtr eventDataPtr,
	const EsifDataPtr eventGuidPtr
	);

AppDomainDataMapPtr EsifApp_GetDomainDataMapFromHandle(
	const AppParticipantDataMapPtr upMapPtr,
	const esif_handle_t domainHandle
	);

eEsifError EsifApp_GetDomainIdByHandle(
	EsifAppPtr self,
	const esif_handle_t upHandle,
	const esif_handle_t domainHandle,
	UInt16 *domainIdPtr
);

char *EsifApp_GetDomainQalifierByHandle(
	EsifAppPtr self,
	const esif_handle_t upHandle,
	const esif_handle_t domainHandle
	);

eEsifError EsifApp_GetHandlesByIds(
	EsifAppPtr self,
	esif_handle_t participantId,
	UInt16 domainId,
	esif_handle_t *upHandlePtr,
	esif_handle_t *domainHandlePtr
	);

eEsifError EsifApp_SuspendApp(
	EsifAppPtr self
);

eEsifError EsifApp_ResumeApp(
	EsifAppPtr self
);

eEsifError EsifApp_SendCommand(
	EsifAppPtr self,
	const UInt32 argc,
	EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);

eEsifError EsifApp_GetStatus(
	EsifAppPtr self,
	const eAppStatusCommand command,
	const UInt32 statusIn,	/* Command Data (High Word Group, Low Word Module) */
	EsifDataPtr statusPtr	/* Status output string if XML please use ESIF_DATA_XML */
	);

eEsifError EsifApp_GetDescription(
	EsifAppPtr self,
	EsifDataPtr descPtr
	);

eEsifError EsifApp_GetVersion(
	EsifAppPtr self,
	EsifDataPtr versionPtr
	);

eEsifError EsifApp_GetIntro(
	EsifAppPtr self,
	EsifDataPtr introPtr
	);

#ifdef __cplusplus
}
#endif

#endif	// _ESIF_UF_APP_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

