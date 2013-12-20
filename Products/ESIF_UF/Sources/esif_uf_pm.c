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

#define ESIF_TRACE_DEBUG_DISABLED

/* ESIF */
#include "esif_uf.h"		/* Upper Framework */
#include "esif_pm.h"		/* Upper Participant Manager */
#include "esif_ipc.h"		/* IPC Abstraction */
#include "esif_uf_appmgr.h"	/* Application Manager */
#include "esif_dsp.h"		/* Device Support Package */

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* This Module */
EsifUppMgr g_uppMgr = {0};


/*
** ===========================================================================
** PRIVATE
** ===========================================================================
*/
static EsifUpManagerEntryPtr EsifUpManagerGetParticipantEntryFromMetadata (const eEsifParticipantOrigin origin, const void *metadataPtr);
static eEsifError EsifUpManagerDestroyParticipants (void);


/*
**  Map lower participant metadata to upper participant instance.  Every
**  lower participant must have a corresponding upper particpant.  Here we
**  intialize the data from the upper participant from the lower participant
**  registration data.
*/
static void UpInitializeOriginLF (
	const EsifUpPtr upPtr,
	const UInt8 lpInstance,
	const void *lpMetadataPtr
	)
{
	/* Lower Framework Participant Metadata Format */
	struct esif_ipc_event_data_create_participant *metadata_ptr =
		(struct esif_ipc_event_data_create_participant*)lpMetadataPtr;

	ESIF_ASSERT(upPtr != NULL);
	ESIF_ASSERT(metadata_ptr != NULL);

	/* Store Lower Framework Instance. */
	upPtr->fLpInstance = lpInstance;

	/* Common */
	upPtr->fMetadata.fVersion    = metadata_ptr->version;
	upPtr->fMetadata.fEnumerator = (enum esif_participant_enum)metadata_ptr->enumerator;
	upPtr->fMetadata.fFlags = metadata_ptr->flags;

	esif_ccb_memcpy(&upPtr->fMetadata.fDriverType, &metadata_ptr->class_guid, ESIF_GUID_LEN);

	esif_ccb_strcpy(upPtr->fMetadata.fName, metadata_ptr->name, ESIF_NAME_LEN);
	esif_ccb_strcpy(upPtr->fMetadata.fDesc, metadata_ptr->desc, ESIF_DESC_LEN);
	esif_ccb_strcpy(upPtr->fMetadata.fDriverName, metadata_ptr->driver_name, ESIF_NAME_LEN);
	esif_ccb_strcpy(upPtr->fMetadata.fDeviceName, metadata_ptr->device_name, ESIF_NAME_LEN);
	esif_ccb_strcpy(upPtr->fMetadata.fDevicePath, metadata_ptr->device_path, ESIF_PATH_LEN);

	/* ACPI */
	upPtr->fMetadata.fAcpiUID  = metadata_ptr->acpi_uid;
	upPtr->fMetadata.fAcpiType = metadata_ptr->acpi_type;
	esif_ccb_strcpy(upPtr->fMetadata.fAcpiDevice, metadata_ptr->acpi_device, ESIF_NAME_LEN);
	esif_ccb_strcpy(upPtr->fMetadata.fAcpiScope, metadata_ptr->acpi_scope, ESIF_SCOPE_LEN);

	/* PCI */
	upPtr->fMetadata.fPciVendor    = (u16)metadata_ptr->pci_vendor;
	upPtr->fMetadata.fPciDevice    = (u16)metadata_ptr->pci_device;
	upPtr->fMetadata.fPciBus       = metadata_ptr->pci_bus;
	upPtr->fMetadata.fPciBusDevice = metadata_ptr->pci_bus_device;
	upPtr->fMetadata.fPciFunction  = metadata_ptr->pci_function;
	upPtr->fMetadata.fPciRevision  = metadata_ptr->pci_revision;
	upPtr->fMetadata.fPciClass     = metadata_ptr->pci_class;
	upPtr->fMetadata.fPciSubClass  = metadata_ptr->pci_sub_class;
	upPtr->fMetadata.fPciProgIf    = metadata_ptr->pci_prog_if;
}


/*
   Map upper participant metadata to upper particpant instance.  A participant
   that is created from the upperframework will never have a lower framework
   component.  These participants are to cover the case were we either have no
   Kernel corresponding participant or in some cases we may not even have a
   lower frame work present.
 */
static void UpInitializeOriginUF (
	const EsifUpPtr upPtr,
	const void *upMetadataPtr
	)
{
	/* Upper Participant Metadata Format */
	EsifParticipantIfacePtr metadata_ptr =
		(EsifParticipantIfacePtr)upMetadataPtr;

	ESIF_ASSERT(upPtr != NULL);
	ESIF_ASSERT(metadata_ptr != NULL);

	/* Store Lower Framework Instance */
	upPtr->fLpInstance = 255;	/* Not Used */

	/* Common */
	upPtr->fMetadata.fVersion    = metadata_ptr->version;
	upPtr->fMetadata.fEnumerator = (enum esif_participant_enum)metadata_ptr->enumerator;
	upPtr->fMetadata.fFlags = metadata_ptr->flags;

	esif_ccb_memcpy(&upPtr->fMetadata.fDriverType, &metadata_ptr->class_guid, ESIF_GUID_LEN);

	esif_ccb_strcpy(upPtr->fMetadata.fName, metadata_ptr->name, ESIF_NAME_LEN);
	esif_ccb_strcpy(upPtr->fMetadata.fDesc, metadata_ptr->desc, ESIF_DESC_LEN);
	esif_ccb_strcpy(upPtr->fMetadata.fDriverName, metadata_ptr->driver_name, ESIF_NAME_LEN);
	esif_ccb_strcpy(upPtr->fMetadata.fDeviceName, metadata_ptr->device_name, ESIF_NAME_LEN);
	esif_ccb_strcpy(upPtr->fMetadata.fDevicePath, metadata_ptr->device_path, ESIF_PATH_LEN);
	esif_ccb_strcpy(upPtr->fMetadata.fAcpiScope, metadata_ptr->object_id, ESIF_SCOPE_LEN);
}


/*
** ===========================================================================
** PUBLIC
** ===========================================================================
*/

/* Create participant */
EsifUpPtr EsifUpManagerCreateParticipant (
	const eEsifParticipantOrigin origin,
	const void *handle,
	const void *metadataPtr
	)
{
	EsifUpPtr up_ptr = NULL;
	EsifUpManagerEntryPtr entry_ptr = NULL;
	UInt8 i = 0;
	UInt8 lpInstance;

	/* Lock manager */
	esif_ccb_write_lock(&g_uppMgr.fLock);

	/* Validate parameters */
	if (NULL == handle || NULL == metadataPtr) {
		goto exit;
	}

	lpInstance = *(UInt8*)handle;

	/*
	 * Check if a participant has already been created, but was then removed.
	 * In that case, just re-enable the participant.
	 */
	entry_ptr = EsifUpManagerGetParticipantEntryFromMetadata(origin, metadataPtr);
	if(NULL != entry_ptr) {
		up_ptr = entry_ptr->fUpPtr;
		entry_ptr->fState = ESIF_PM_PARTICIPANT_STATE_CREATED;
		g_uppMgr.fEntryCount++;
		goto exit;
	}

	/* Allocate a particpant */
	up_ptr = (EsifUpPtr)esif_ccb_malloc(sizeof(EsifUp));
	if (NULL == up_ptr) {
		ESIF_ASSERT(up_ptr != NULL);
		goto exit;
	}

	/*
	**  Find available slot in participant manager table.  Simple Table Lookup For Now.
	**  Scan table and find first empty slot.  Empty slot indicated by AVAILABLE state.
	*/
	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		if (ESIF_PM_PARTICIPANT_STATE_AVAILABLE == g_uppMgr.fEntries[i].fState) {
			break;
		}
	}

	/* If no available slots return */
	if (i >= MAX_PARTICIPANT_ENTRY) {
		esif_ccb_free(up_ptr);
		up_ptr = NULL;
		goto exit;
	}

	/* Take slot */
	g_uppMgr.fEntries[i].fState = ESIF_PM_PARTICIPANT_STATE_CREATED;
	g_uppMgr.fEntryCount++;

	/* Initialize participant */
	up_ptr->fInstance = i;
	up_ptr->fEnabled  = ESIF_TRUE;

	/* Initialize based on origin */
	switch (origin) {
	case eParticipantOriginLF:
		UpInitializeOriginLF(up_ptr, lpInstance, metadataPtr);
		break;

	case eParticipantOriginUF:
		UpInitializeOriginUF(up_ptr, metadataPtr);
		break;

	default:
		break;
	}

	/* Unlock manager */
	g_uppMgr.fEntries[i].fUpPtr = up_ptr;

exit:
	esif_ccb_write_unlock(&g_uppMgr.fLock);
	return up_ptr;
}


/* Unregister Upper Participant Instance */
eEsifError EsifUpManagerUnregisterParticipant (
	const eEsifParticipantOrigin origin,
	const void *participantHandle
	)
{
	eEsifError rc    = ESIF_OK;
	EsifUpManagerEntryPtr entry_ptr = NULL;
	EsifUpPtr up_ptr = NULL;
	UInt8 instance;

	UNREFERENCED_PARAMETER(origin);

	/* Validate parameters */
	if (NULL == participantHandle) {
		ESIF_ASSERT(participantHandle != NULL);
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	instance = *(UInt8*)participantHandle;

	if(instance > MAX_PARTICIPANT_ENTRY) {
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	esif_ccb_write_lock(&g_uppMgr.fLock);

	entry_ptr = &g_uppMgr.fEntries[instance];
	if (NULL != entry_ptr) {
		up_ptr = entry_ptr->fUpPtr;
		if (NULL != up_ptr) {
			entry_ptr->fState = ESIF_PM_PARTICIPANT_REMOVED;
			g_uppMgr.fEntryCount--;
		}
	}
	esif_ccb_write_unlock(&g_uppMgr.fLock);

	if(NULL != up_ptr) {
		rc = EsifAppMgrDestroyParticipantInAllApps(up_ptr);
	}

exit:
	return rc;
}

eEsifError EsifUpManagerRegisterParticipantsWithApp(EsifAppPtr aAppPtr)
{
	eEsifError rc = ESIF_OK;
	UInt8 i = 0;

	if (NULL == aAppPtr) {
		return ESIF_E_PARAMETER_IS_NULL;
	}

	esif_ccb_read_lock(&g_uppMgr.fLock);

	/* Skip 0 ESIF treats this as a participant no one else does :) */
	for (i = 1; i < MAX_PARTICIPANT_ENTRY; i++) {
		EsifUpPtr up_ptr = g_uppMgr.fEntries[i].fUpPtr;
		if ((NULL != up_ptr) && (g_uppMgr.fEntries[i].fState > ESIF_PM_PARTICIPANT_REMOVED)) {
			rc = EsifAppCreateParticipant(aAppPtr, up_ptr);
			if (ESIF_OK != rc) {
				break;
			}
		}
	}

	esif_ccb_read_unlock(&g_uppMgr.fLock);

	return rc;
}


eEsifError EsifUpManagerDestroyParticipantsInApp (EsifAppPtr aAppPtr)
{
	UInt8 i = 0;

	esif_ccb_read_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		EsifUpPtr up_ptr = g_uppMgr.fEntries[i].fUpPtr;
		if ((NULL != up_ptr) && (g_uppMgr.fEntries[i].fState > ESIF_PM_PARTICIPANT_REMOVED)) {
			EsifAppDestroyParticipant(aAppPtr, up_ptr);
		}
	}

	esif_ccb_read_unlock(&g_uppMgr.fLock);
	return ESIF_OK;
}



/* Get By Instance From ID */
EsifUpPtr EsifUpManagerGetAvailableParticipantByInstance (
	const UInt8 id
	)
{
	EsifUpPtr up_ptr = NULL;
	ESIF_TRACE_DEBUG("%s: instance %d\n", ESIF_FUNC, id);

	if (id >= MAX_PARTICIPANT_ENTRY) {
		ESIF_ASSERT(0);
		goto exit;
	}

	/* Lock manager */
	esif_ccb_read_lock(&g_uppMgr.fLock);

	if (g_uppMgr.fEntries[id].fState > ESIF_PM_PARTICIPANT_REMOVED) {
		up_ptr = g_uppMgr.fEntries[id].fUpPtr;
	}

	/* Unlock Manager */
	esif_ccb_read_unlock(&g_uppMgr.fLock);

exit:

	if (NULL == up_ptr) {
		ESIF_TRACE_DEBUG("%s: instance %d NOT found or OUT OF BOUNDS\n",
				 ESIF_FUNC, id);
	}
	return up_ptr;
}


/* Check if a participant already exists by the name */
Bool EsifUpManagerDoesAvailableParticipantExistByName (
	char* participantName
	)
{
	Bool bRet = ESIF_FALSE;
	EsifUpPtr up_ptr = NULL;
	UInt8 i;

	if(NULL == participantName) {
		goto exit;
	}

	esif_ccb_read_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		up_ptr = EsifUpManagerGetAvailableParticipantByInstance(i);

		if (NULL == up_ptr) {
			continue;
		}

		if ((g_uppMgr.fEntries[i].fState > ESIF_PM_PARTICIPANT_REMOVED) && !strcmp(up_ptr->fMetadata.fName, participantName)) {
			bRet = ESIF_TRUE;
			break;
		}
	}
	esif_ccb_read_unlock(&g_uppMgr.fLock);
exit:
	return bRet;
}


EsifUpPtr EsifUpManagerGetAvailableParticipantByName (
	char* participantName
	)
{
	EsifUpPtr up_ptr = NULL;
	UInt8 i;

	if(NULL == participantName) {
		goto exit;
	}

	esif_ccb_read_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		up_ptr = EsifUpManagerGetAvailableParticipantByInstance(i);

		if (NULL == up_ptr) {
			continue;
		}
		if (!strcmp(participantName, up_ptr->fMetadata.fName)) {
			break;
		}
		up_ptr = NULL;
	}

	esif_ccb_read_unlock(&g_uppMgr.fLock);
exit:
	return up_ptr;
}


static EsifUpManagerEntryPtr EsifUpManagerGetParticipantEntryFromMetadata(
	const eEsifParticipantOrigin origin,
	const void *metadataPtr
	)
{
	EsifUpManagerEntryPtr entry_ptr = NULL;
	char* participantName = "";
	UInt8 i;

	/* Validate parameters */
	if (NULL == metadataPtr) {
		goto exit;
	}

	switch(origin) {
	case eParticipantOriginLF:
		participantName = ((struct esif_ipc_event_data_create_participant*)metadataPtr)->name;
		break;
	case eParticipantOriginUF:
		participantName = ((EsifParticipantIfacePtr)metadataPtr)->name;
		break;

	default:
		goto exit;
		break;
	}


	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		entry_ptr = &g_uppMgr.fEntries[i];

		if (NULL != entry_ptr->fUpPtr) {
			if (!strcmp(entry_ptr->fUpPtr->fMetadata.fName, participantName)) {
				break;
			}
		}
		entry_ptr = NULL;
	}

exit:
	return entry_ptr;
}


/* Initialize manager */
eEsifError EsifUppMgrInit (void)
{
	eEsifError rc = ESIF_OK;
	ESIF_TRACE_DEBUG("%s: Init Upper Participant Manager (PM)", ESIF_FUNC);

	/* Initialize Lock */
	esif_ccb_lock_init(&g_uppMgr.fLock);

	return rc;
}


/* Exit manager */
void EsifUppMgrExit (void)
{
	/* Clean up resources */
	EsifUpManagerDestroyParticipants();

	/* Uninitialize Lock */
	esif_ccb_lock_uninit(&g_uppMgr.fLock);

	ESIF_TRACE_DEBUG("%s: Exit Upper Participant Manager (PM)", ESIF_FUNC);
}


/* This should only be called when shutting down */
static eEsifError EsifUpManagerDestroyParticipants (void)
{
	eEsifError rc = ESIF_OK;
	EsifUpManagerEntryPtr entry_ptr = NULL;
	UInt8 i = 0;

	esif_ccb_write_lock(&g_uppMgr.fLock);

	for (i = 0; i < MAX_PARTICIPANT_ENTRY; i++) {
		entry_ptr = &g_uppMgr.fEntries[i];

		if(entry_ptr->fState > ESIF_PM_PARTICIPANT_REMOVED) {
			g_uppMgr.fEntryCount--;
		}
		entry_ptr->fState = ESIF_PM_PARTICIPANT_STATE_AVAILABLE;

		if(NULL != entry_ptr->fUpPtr) {
			esif_ccb_free(entry_ptr->fUpPtr);
			entry_ptr->fUpPtr = NULL;
		}
	}

	esif_ccb_write_unlock(&g_uppMgr.fLock);

	return rc;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
