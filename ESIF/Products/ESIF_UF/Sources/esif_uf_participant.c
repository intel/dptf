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
#define ESIF_TRACE_ID	ESIF_TRACEMODULE_PARTICIPANT

#include "esif_uf.h"
#include "esif_uf_primitive.h"
#include "esif_uf_ipc.h"	/* IPC                       */
#include "esif_uf_actmgr.h"	/* Action Manager            */
#include "esif_uf_xform.h"

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/*
 * PRIVATE FUNCTION PROTOTYPES
 */
static eEsifError EsifUp_CreateParticipantByLpEventData(
	struct esif_ipc_event_data_create_participant *lpCreateDataPtr,
	UInt8 upInstance,
	EsifUpPtr *upPtr
	);

static eEsifError EsifUp_CreateParticipantByUpInterface(
	EsifParticipantIfacePtr upInterfacePtr,
	UInt8 upInstance,
	EsifUpPtr *upPtr
	);

static eEsifError EsifUp_ReInitializeParticipantByLpEventData(
	EsifUpPtr self,
	struct esif_ipc_event_data_create_participant *lpCreateDataPtr
	);

static eEsifError EsifUp_ReInitializeParticipantByUpInterface(
	EsifUpPtr self,
	EsifParticipantIfacePtr upInterfacePtr
	);

static eEsifError EsifUp_SelectDspByLpEventData(
	EsifUpPtr self,
	struct esif_ipc_event_data_create_participant *lpCreateDataPtr
	);

static eEsifError EsifUp_SelectDspByUpInterface(
	EsifUpPtr self,
	EsifParticipantIfacePtr upInterfacePtr
	);

static eEsifError EsifUp_InitDomains(EsifUpPtr self);

static eEsifError EsifUp_ExecuteLfSetAction(
	EsifUpPtr self,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const UInt16 kernActionNum,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);
static eEsifError EsifUp_ExecuteLfGetAction(
	EsifUpPtr self,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const UInt16 kernActionNum,
	EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	);
static eEsifError EsifUp_ExecuteUfSetAction(
	EsifUpPtr self,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const EsifDataPtr requestPtr
	);
static eEsifError EsifUp_ExecuteUfGetAction(
	EsifUpPtr self,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	);
static eEsifError EsifUp_ExecuteAction(
	EsifUpPtr self,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	UInt16 kernelActNum,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	);
static EsifString EsifUp_SelectDsp(
	EsifUpPtr self
	);
/*
 * FUNCTION DEFINITIONS
 */


eEsifError EsifUp_DspReadyInit(
	EsifUpPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 domainCount;
	UInt32 domainIndex;
	char domain_str[8] = "";

	if ((NULL == self) ||
		(NULL == self->fDspPtr) ||
		(NULL == self->fDspPtr->get_domain_count)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	domainCount = self->fDspPtr->get_domain_count(self->fDspPtr);

	for (domainIndex = 0; domainIndex < domainCount; domainIndex++) {
		UInt32 period = 0;
		EsifPrimitiveTuple periodTuple = {GET_PARTICIPANT_SAMPLE_PERIOD, self->domains[domainIndex].domain, 255};
		EsifPrimitiveTuple behaviorTuple = {SET_PARTICIPANT_SAMPLE_BEHAVIOR, self->domains[domainIndex].domain, 255};
		EsifData periodData = {ESIF_DATA_TIME, &period, sizeof(period), 0};

		rc = EsifUp_ExecutePrimitive(self, &periodTuple, NULL, &periodData);
		if (ESIF_OK == rc) {
			ESIF_TRACE_DEBUG("Setting polling period for %s:%s to %d\n", self->fMetadata.fName, domain_str, period);

			rc = EsifUp_ExecutePrimitive(self, &behaviorTuple, &periodData, NULL);
			if (rc != ESIF_OK) {
				ESIF_TRACE_WARN("Failed to set polling period for %s:%s\n", self->fMetadata.fName, domain_str);
			}
		}

		EsifUpDomain_DspReadyInit(&self->domains[domainIndex]);
	}
	rc = ESIF_OK; /* Domain loop above best-effort */
exit:
	return rc;
}


eEsifError EsifUp_UpdatePolling(
	EsifUpPtr self,
	UInt16 domainValue,
	UInt32 period
	)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple behaviorTuple = {SET_PARTICIPANT_SAMPLE_BEHAVIOR, domainValue, 255};
	EsifData periodData = {ESIF_DATA_TIME, &period, sizeof(period), 0};
	
	if (self == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Updating polling period for %s:0x%04X to %d\n", self->fMetadata.fName, domainValue, period);
	rc = EsifUp_ExecutePrimitive(self, &behaviorTuple, &periodData, NULL);
	if (rc != ESIF_OK) {
		ESIF_TRACE_WARN("Failed to set polling period for %s:0x%04X\n", self->fMetadata.fName, domainValue);
	}

exit:
	return rc;
}

eEsifError EsifUp_UpdateHysteresis(
	EsifUpPtr self,
	UInt16 domainValue,
	esif_temp_t hysteresis_val
	)
{
	eEsifError rc = ESIF_OK;
	EsifPrimitiveTuple hystTuple = {SET_PARTICIPANT_HYSTERESIS_BEHAVIOR, domainValue, 255};
	struct esif_data hystData = {ESIF_DATA_TIME, &hysteresis_val, sizeof(hysteresis_val), 0};
	
	if (self == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	ESIF_TRACE_DEBUG("Updating hysteresis for %s:0x%04X to %d\n", self->fMetadata.fName, domainValue, hysteresis_val);
	rc = EsifUp_ExecutePrimitive(self, &hystTuple, &hystData, NULL);
	if (rc != ESIF_OK) {
		ESIF_TRACE_INFO("Failed to set hysteresis for %s:0x%04X\n", self->fMetadata.fName, domainValue);
	}

exit:
	return rc;
}


EsifFpcEventPtr EsifUp_GetFpcEventByType(
	EsifUpPtr self,
	eEsifEventType eventType
	)
{
	EsifFpcEventPtr eventPtr = NULL;

	if (NULL == self) {
		goto exit;
	}
	eventPtr = self->fDspPtr->get_event_by_type(self->fDspPtr, eventType);

exit:
	return eventPtr;
}


EsifFpcEventPtr EsifUp_GetFpcEventByGuid(
	EsifUpPtr self,
	esif_guid_t *guidPtr
	)
{
	EsifFpcEventPtr eventPtr = NULL;

	if ((NULL == self) || (NULL == guidPtr)) {
		goto exit;
	}
	eventPtr = self->fDspPtr->get_event_by_guid(self->fDspPtr, *guidPtr);

exit:
	return eventPtr;
}


EsifString EsifUp_GetName(
	EsifUpPtr self
	)
{
	if (self != NULL) {
		return self->fMetadata.fName;
	}
	else {
		return "UNK";
	}
}


EsifUpDomainPtr EsifUp_GetDomainById(
	EsifUpPtr self,
	UInt16 domainId
	)
{
	EsifUpDomainPtr domainPtr = NULL;
	eEsifError rc = ESIF_OK;
	UInt8 domainIndex = 0;

	if (NULL == self) {
		goto exit;
	}

	rc = EsifDomainIdToIndex(domainId, &domainIndex);
	if ((rc != ESIF_OK) || (domainIndex >= self->domainCount)) {
		goto exit;
	}

	domainPtr = &self->domains[domainIndex];
exit:
	return domainPtr;
}

void EsifUp_RegisterParticipantForPolling(EsifUpPtr self)
{
	UInt16 j = 0;
	UInt8 domainCount = 0;
	EsifUpDomainPtr domainPtr = NULL;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(self->fDspPtr != NULL);
	ESIF_ASSERT(self->fDspPtr->get_domain_count != NULL);

	domainCount = (u8) self->fDspPtr->get_domain_count(self->fDspPtr);

	for (j = 0; j < domainCount; j++) {
		domainPtr = EsifUp_GetDomainById(self, self->domains[j].domain);

		if (NULL == domainPtr) {
			continue;
		}

		EsifUpDomain_RegisterForTempPoll(domainPtr, ESIF_POLL_ECONO);
		EsifUpDomain_RegisterForStatePoll(domainPtr, ESIF_POLL_ECONO);
	}
}

void EsifUp_UnRegisterParticipantForPolling(EsifUpPtr self)
{
	UInt16 j = 0;
	UInt8 domainCount = 0;
	EsifUpDomainPtr domainPtr = NULL;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(self->fDspPtr != NULL);
	ESIF_ASSERT(self->fDspPtr->get_domain_count != NULL);

	domainCount = (u8) self->fDspPtr->get_domain_count(self->fDspPtr);

	for (j = 0; j < domainCount; j++) {
		domainPtr = EsifUp_GetDomainById(self, self->domains[j].domain);

		if (NULL == domainPtr) {
			continue;
		}

		EsifUpDomain_UnRegisterForTempPoll(domainPtr);
		EsifUpDomain_UnRegisterForStatePoll(domainPtr);
	}
}

void EsifUp_PollParticipant(EsifUpPtr self)
{
	UInt16 j = 0;
	UInt8 domainCount = 0;
	EsifUpDomainPtr domainPtr = NULL;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(self->fDspPtr != NULL);
	ESIF_ASSERT(self->fDspPtr->get_domain_count != NULL);

	domainCount = (u8) self->fDspPtr->get_domain_count(self->fDspPtr);

	for (j = 0; j < domainCount; j++) {
		domainPtr = EsifUp_GetDomainById(self, self->domains[j].domain);

		if (NULL == domainPtr) {
			continue;
		}

		EsifUpDomain_Poll(domainPtr);
	}
}



static eEsifError EsifUp_CreateParticipantByLpEventData(
	struct esif_ipc_event_data_create_participant *lpCreateDataPtr,
	UInt8 upInstance,
	EsifUpPtr *upPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpPtr newUpPtr = NULL;

	ESIF_ASSERT(lpCreateDataPtr != NULL);
	ESIF_ASSERT(upPtr != NULL);

	*upPtr = NULL;

	/* Allocate a particpant */
	newUpPtr = (EsifUpPtr)esif_ccb_malloc(sizeof(EsifUp));
	if (NULL == newUpPtr) {
		ESIF_TRACE_ERROR("Fail to allocate EsifUp\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	/* Life control */
	newUpPtr->refCount = 1;
	newUpPtr->markedForDelete = ESIF_FALSE;
	esif_ccb_event_init(&newUpPtr->deleteEvent);
	esif_ccb_lock_init(&newUpPtr->objLock);

	/* origin of creation */
	newUpPtr->fOrigin = eParticipantOriginLF;

	/* store Uppter Framework Instance */
	newUpPtr->fInstance = upInstance;

	/* Store Lower Framework Instance. */
	newUpPtr->fLpInstance = lpCreateDataPtr->id;

	/* Meta data */
	newUpPtr->fMetadata.fVersion = lpCreateDataPtr->version;
	newUpPtr->fMetadata.fEnumerator = (enum esif_participant_enum)lpCreateDataPtr->enumerator;
	newUpPtr->fMetadata.fFlags = lpCreateDataPtr->flags;

	esif_ccb_memcpy(&newUpPtr->fMetadata.fDriverType, &lpCreateDataPtr->class_guid, ESIF_GUID_LEN);

	esif_ccb_strcpy(newUpPtr->fMetadata.fName, lpCreateDataPtr->name, ESIF_NAME_LEN);
	esif_ccb_strcpy(newUpPtr->fMetadata.fDesc, lpCreateDataPtr->desc, ESIF_DESC_LEN);
	esif_ccb_strcpy(newUpPtr->fMetadata.fDriverName, lpCreateDataPtr->driver_name, ESIF_NAME_LEN);
	esif_ccb_strcpy(newUpPtr->fMetadata.fDeviceName, lpCreateDataPtr->device_name, ESIF_NAME_LEN);
	esif_ccb_strcpy(newUpPtr->fMetadata.fDevicePath, lpCreateDataPtr->device_path, ESIF_PATH_LEN);

	/* ACPI */
	esif_ccb_strcpy(newUpPtr->fMetadata.fAcpiUID, lpCreateDataPtr->acpi_uid, sizeof(newUpPtr->fMetadata.fAcpiUID));
	newUpPtr->fMetadata.fAcpiType = lpCreateDataPtr->acpi_type;
	esif_ccb_strcpy(newUpPtr->fMetadata.fAcpiDevice, lpCreateDataPtr->acpi_device, ESIF_NAME_LEN);
	esif_ccb_strcpy(newUpPtr->fMetadata.fAcpiScope, lpCreateDataPtr->acpi_scope, ESIF_SCOPE_LEN);

	/* PCI */
	newUpPtr->fMetadata.fPciVendor = (u16)lpCreateDataPtr->pci_vendor;
	newUpPtr->fMetadata.fPciDevice = (u16)lpCreateDataPtr->pci_device;
	newUpPtr->fMetadata.fPciBus = lpCreateDataPtr->pci_bus;
	newUpPtr->fMetadata.fPciBusDevice = lpCreateDataPtr->pci_bus_device;
	newUpPtr->fMetadata.fPciFunction = lpCreateDataPtr->pci_function;
	newUpPtr->fMetadata.fPciRevision = lpCreateDataPtr->pci_revision;
	newUpPtr->fMetadata.fPciClass = lpCreateDataPtr->pci_class;
	newUpPtr->fMetadata.fPciSubClass = lpCreateDataPtr->pci_sub_class;
	newUpPtr->fMetadata.fPciProgIf = lpCreateDataPtr->pci_prog_if;

	/* DSP */
	rc = EsifUp_SelectDspByLpEventData(newUpPtr, lpCreateDataPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	rc = EsifUp_InitDomains(newUpPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

exit:
	if (rc == ESIF_OK) {
		ESIF_TRACE_INFO("The participant data of %s is created in ESIF UF, instance = %d\n", newUpPtr->fMetadata.fName, upInstance);
		*upPtr = newUpPtr;
	} else if (newUpPtr != NULL) {
		EsifUp_DestroyParticipant(newUpPtr);
	}

	return rc;
}


static eEsifError EsifUp_CreateParticipantByUpInterface(
	EsifParticipantIfacePtr upInterfacePtr,
	UInt8 upInstance,
	EsifUpPtr *upPtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifUpPtr newUpPtr = NULL;

	ESIF_ASSERT(upInterfacePtr != NULL);
	ESIF_ASSERT(upPtr != NULL);

	*upPtr = NULL;

	/* Allocate a particpant */
	newUpPtr = (EsifUpPtr)esif_ccb_malloc(sizeof(EsifUp));
	if (NULL == newUpPtr) {
		ESIF_TRACE_ERROR("Fail to allocate EsifUp\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	/* Life control */
	newUpPtr->refCount = 1;
	newUpPtr->markedForDelete = ESIF_FALSE;
	esif_ccb_event_init(&newUpPtr->deleteEvent);
	esif_ccb_lock_init(&newUpPtr->objLock);

	/* origin of creation */
	newUpPtr->fOrigin = eParticipantOriginUF;

	/* Store Uppter Framework Instance */
	newUpPtr->fInstance = upInstance;

	/* Store Lower Framework Instance */
	newUpPtr->fLpInstance = ESIF_INSTANCE_INVALID;	/* Not Used */

	/* Meta data */
	newUpPtr->fMetadata.fVersion = upInterfacePtr->version;
	newUpPtr->fMetadata.fEnumerator = (enum esif_participant_enum)upInterfacePtr->enumerator;
	newUpPtr->fMetadata.fFlags = upInterfacePtr->flags;

	esif_ccb_memcpy(&newUpPtr->fMetadata.fDriverType, &upInterfacePtr->class_guid, ESIF_GUID_LEN);

	esif_ccb_strcpy(newUpPtr->fMetadata.fName, upInterfacePtr->name, ESIF_NAME_LEN);
	esif_ccb_strcpy(newUpPtr->fMetadata.fDesc, upInterfacePtr->desc, ESIF_DESC_LEN);
	esif_ccb_strcpy(newUpPtr->fMetadata.fDriverName, upInterfacePtr->driver_name, ESIF_NAME_LEN);
	esif_ccb_strcpy(newUpPtr->fMetadata.fDeviceName, upInterfacePtr->device_name, ESIF_NAME_LEN);
	esif_ccb_strcpy(newUpPtr->fMetadata.fAcpiDevice, upInterfacePtr->device_name, ESIF_NAME_LEN);
	esif_ccb_strcpy(newUpPtr->fMetadata.fDevicePath, upInterfacePtr->device_path, ESIF_PATH_LEN);
	esif_ccb_strcpy(newUpPtr->fMetadata.fAcpiScope, upInterfacePtr->object_id, ESIF_SCOPE_LEN);

	rc = EsifUp_SelectDspByUpInterface(newUpPtr, upInterfacePtr);
	if (rc != ESIF_OK) {
		goto exit;
	}
	rc = EsifUp_InitDomains(newUpPtr);
exit:
	if (rc == ESIF_OK) {
		ESIF_TRACE_INFO("The participant data of %s is created in ESIF UF, instance = %d\n", newUpPtr->fMetadata.fName, upInstance);
		*upPtr = newUpPtr;
	}
	else if (newUpPtr != NULL) {
		EsifUp_DestroyParticipant(newUpPtr);
	}

	return rc;
}


static eEsifError EsifUp_InitDomains(
	EsifUpPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt8 domainIndex = 0;
	struct esif_fpc_domain *fpcDomainPtr = NULL;

	if ((NULL == self) ||
		(NULL == self->fDspPtr) ||
		(NULL == self->fDspPtr->get_domain_count) ||
		(NULL == self->fDspPtr->get_domain)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	
	self->domainCount = (UInt8) self->fDspPtr->get_domain_count(self->fDspPtr);

	fpcDomainPtr = self->fDspPtr->get_domain(self->fDspPtr, domainIndex + 1);

	while (fpcDomainPtr != NULL) {
		EsifUpDomain_InitDomain(&self->domains[domainIndex], self, fpcDomainPtr);
		fpcDomainPtr = self->fDspPtr->get_domain(self->fDspPtr, ++domainIndex + 1);
	}

exit:
	return rc;
}


eEsifError EsifUp_CreateParticipant(
	const eEsifParticipantOrigin origin,
	UInt8 upInstance,
	const void *metadataPtr,
	EsifUpPtr *upPtr
	)
{
	eEsifError rc = ESIF_OK;

	if ((NULL == upPtr) || (NULL == metadataPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	switch (origin) {
	case eParticipantOriginLF:
		rc = EsifUp_CreateParticipantByLpEventData((struct esif_ipc_event_data_create_participant *)metadataPtr, upInstance, upPtr);
		break;

	case eParticipantOriginUF:
		rc = EsifUp_CreateParticipantByUpInterface((EsifParticipantIfacePtr)metadataPtr, upInstance, upPtr);
		break;

	default:
		ESIF_TRACE_ERROR("Unknown origin is specified\n");
		*upPtr = NULL;
		rc = ESIF_E_UNSPECIFIED;
		break;
	}

exit:
	return rc;
}


static eEsifError EsifUp_ReInitializeParticipantByLpEventData(
	EsifUpPtr self,
	struct esif_ipc_event_data_create_participant *lpCreateDataPtr
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(lpCreateDataPtr != NULL);

	self->fLpInstance = lpCreateDataPtr->id;

	rc = EsifUp_SelectDspByLpEventData(self, lpCreateDataPtr);

	return rc;
}


static eEsifError EsifUp_ReInitializeParticipantByUpInterface(
    EsifUpPtr self,
    EsifParticipantIfacePtr upInterfacePtr
    )
{
    eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(upInterfacePtr != NULL);

	self->fLpInstance = ESIF_INSTANCE_INVALID;

	rc = EsifUp_SelectDspByUpInterface(self, upInterfacePtr);

    return rc;
}


eEsifError EsifUp_ReInitializeParticipant(
	EsifUpPtr self,
	const eEsifParticipantOrigin origin,
	const void *metadataPtr
	)
{
	eEsifError rc = ESIF_OK;

	if ((NULL == self) || (NULL == metadataPtr)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	switch (origin) {
	case eParticipantOriginLF:
		rc = EsifUp_ReInitializeParticipantByLpEventData(self, (struct esif_ipc_event_data_create_participant *)metadataPtr);
		break;

	case eParticipantOriginUF:
		rc = EsifUp_ReInitializeParticipantByUpInterface(self, (EsifParticipantIfacePtr)metadataPtr);
		break;

	default:
		ESIF_TRACE_ERROR("Unknown origin is specified\n");
		rc = ESIF_E_UNSPECIFIED;
		break;
	}

exit:
	return rc;
}


void EsifUp_DestroyParticipant(
	EsifUpPtr self
	)
{
	if (self != NULL) {
		self->markedForDelete = ESIF_TRUE;
		EsifUp_PutRef(self);

		ESIF_TRACE_INFO("Destroy participant %d : wait for delete event...\n", self->fInstance);
		esif_ccb_event_wait(&self->deleteEvent);

		esif_ccb_event_uninit(&self->deleteEvent);
		esif_ccb_lock_uninit(&self->objLock);

		esif_ccb_free(self);
	}

	ESIF_TRACE_INFO("Participant destroyed\n");
}


static eEsifError EsifUp_SelectDspByLpEventData(
	EsifUpPtr self,
	struct esif_ipc_event_data_create_participant *lpCreateDataPtr
	)
{
	eEsifError rc = ESIF_OK;
	char edpFilePath[MAX_PATH] = { 0 };
	EsifString dspName = NULL;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(lpCreateDataPtr != NULL);

	dspName = EsifUp_SelectDsp(self);
	if (dspName == NULL) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}

	esif_build_path(edpFilePath, sizeof(edpFilePath), ESIF_PATHTYPE_DSP, dspName, ".edp");
	ESIF_TRACE_DEBUG("Selected DSP Delivery Method EDP(CPC): %s\n", edpFilePath);

	/* send DSP to ESIF LF */
	rc = esif_send_dsp(edpFilePath, lpCreateDataPtr->id);
	if ((rc != ESIF_OK) && (rc != ESIF_E_DSP_ALREADY_LOADED)) {
		ESIF_TRACE_ERROR("Fail to send DSP to ESIF LF: %s\n", edpFilePath);
		goto exit;
	}
	rc = ESIF_OK;

	self->fDspPtr = esif_uf_dm_select_dsp_by_code(dspName);
	if (self->fDspPtr == NULL) {
		ESIF_TRACE_ERROR("Missed DSP lookup (%s).\n", dspName);
		rc = ESIF_E_NEED_DSP;
	}
	else {
		ESIF_TRACE_DEBUG("Selected DSP (%s) for participant: %s. \n", dspName, self->fMetadata.fName);
	}

exit:
	return rc;
}


static eEsifError EsifUp_SelectDspByUpInterface(
	EsifUpPtr self,
	EsifParticipantIfacePtr upInterfacePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifString dspName = NULL;

	UNREFERENCED_PARAMETER(upInterfacePtr);

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(upInterfacePtr != NULL);

	dspName = EsifUp_SelectDsp(self);
	if (dspName == NULL) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}

	self->fDspPtr = esif_uf_dm_select_dsp_by_code(dspName);
	if (self->fDspPtr == NULL) {
		ESIF_TRACE_ERROR("Missed DSP lookup (%s).\n", dspName);
		rc = ESIF_E_NEED_DSP;
	}
	else {
		ESIF_TRACE_DEBUG("Selected DSP (%s) for participant: %s. \n", dspName, self->fMetadata.fName);
	}

exit:
	return rc;
}

EsifString EsifUp_SelectDsp(
	EsifUpPtr self
	)
{
	EsifDspQuery query = { 0 };
	char vendorId[MAX_VENDOR_ID_STRING_LENGTH] = { 0 };
	char deviceId[MAX_DEVICE_ID_STRING_LENGTH] = { 0 };
	char participantEnum[ENUM_TO_STRING_LEN] = { 0 };
	char participantPType[ENUM_TO_STRING_LEN] = { 0 };

	esif_ccb_sprintf(MAX_VENDOR_ID_STRING_LENGTH, vendorId, "0x%04X", self->fMetadata.fPciVendor);
	esif_ccb_sprintf(MAX_DEVICE_ID_STRING_LENGTH, deviceId, "0x%04X", self->fMetadata.fPciDevice);
	esif_ccb_sprintf(ENUM_TO_STRING_LEN, participantEnum, "%d", self->fMetadata.fEnumerator);
	esif_ccb_sprintf(ENUM_TO_STRING_LEN, participantPType, "%d", self->fMetadata.fAcpiType);

	query.vendorId = (EsifString) vendorId;
	query.deviceId = (EsifString) deviceId;
	query.participantType = (EsifString) participantEnum;
	query.hid = (EsifString) self->fMetadata.fAcpiDevice;
	query.ptype = (EsifString) participantPType;
	query.participantName = (EsifString) self->fMetadata.fName;

	return EsifDspMgr_SelectDsp(query);
}


UInt8 EsifUp_GetInstance(
	EsifUpPtr self
	)
{
	UInt8 instance = ESIF_PARTICIPANT_INVALID_INSTANCE;

	if (self != NULL) {
		instance = self->fInstance;
	}

	return instance;
}


eEsifError EsifUp_SuspendParticipant(
	EsifUpPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 domainCount;
	UInt32 domainIndex;

	if ((NULL == self) ||
		(NULL == self->fDspPtr) ||
		(NULL == self->fDspPtr->get_domain_count)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	domainCount = self->fDspPtr->get_domain_count(self->fDspPtr);

	for (domainIndex = 0; domainIndex < domainCount; domainIndex++) {
		if (self->domains[domainIndex].tempPollInitialized == ESIF_TRUE) {
			EsifUpDomain_StopTempPoll(&self->domains[domainIndex]);
		}
	}

exit:
	return rc;
}

eEsifError EsifUp_ResumeParticipant(
	EsifUpPtr self
	)
{
	eEsifError rc = ESIF_OK;
	UInt32 domainCount;
	UInt32 domainIndex;

	if ((NULL == self) ||
		(NULL == self->fDspPtr) ||
		(NULL == self->fDspPtr->get_domain_count)) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	domainCount = self->fDspPtr->get_domain_count(self->fDspPtr);

	for (domainIndex = 0; domainIndex < domainCount; domainIndex++) {
	}

exit:
	return rc;
}


eEsifError EsifUp_GetRef(
	EsifUpPtr self
	)
{
	eEsifError rc = ESIF_OK;
	Bool isLocked = ESIF_FALSE;

	if (self == NULL) {
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	esif_ccb_write_lock(&self->objLock);
	isLocked = ESIF_TRUE;

	if (self->markedForDelete == ESIF_TRUE) {
		ESIF_TRACE_DEBUG("Driver marked for delete\n");
		rc = ESIF_E_UNSPECIFIED;
		goto exit;
	}

	self->refCount++;

	ESIF_TRACE_DEBUG("ref = %d\n", self->refCount);

exit:
	if (isLocked) {
		esif_ccb_write_unlock(&self->objLock);
	}

	return rc;
}

void EsifUp_PutRef(
	EsifUpPtr self
	)
{
	UInt8 needRelease = ESIF_FALSE;

	if (self != NULL) {
		esif_ccb_write_lock(&self->objLock);

		self->refCount--;

		ESIF_TRACE_DEBUG("ref = %d\n", self->refCount);

		if ((self->refCount == 0) && (self->markedForDelete)) {
			needRelease = ESIF_TRUE;
		}

		esif_ccb_write_unlock(&self->objLock);

		if (needRelease == ESIF_TRUE) {
			ESIF_TRACE_DEBUG("Signal delete event\n");
			esif_ccb_event_set(&self->deleteEvent);
		}
	}
}

/*
 * Execute Primitive
 * NOTE: This version should only be called by functions within the
 * participant /domain while Participant Mangager or participant locks are
 * already or from within the participant when executing in a
 * known/guaranteed state.  EsifExecutePrimitive should be called when executing
 * outside the context of the participant.
 */
eEsifError EsifUp_ExecutePrimitive(
	EsifUpPtr self,
	EsifPrimitiveTuplePtr tuplePtr,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifDspPtr dspPtr = NULL;
	EsifFpcPrimitivePtr primitivePtr = NULL;
	EsifFpcActionPtr actionPtr = NULL;
	UInt16 kernAct;
	UInt32 i;
	UInt32 reqAuto  = ESIF_FALSE;
	UInt32 rspAuto = ESIF_FALSE;

	if (NULL == self) {
		ESIF_TRACE_ERROR("Participant pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (NULL == tuplePtr) {
		ESIF_TRACE_ERROR("Tuple pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	ESIF_TRACE_DEBUG("\n\n"
		"Primitive Request:\n"
		"  Participant ID       : %u\n"
		"  Primitive            : %s(%u)\n"
		"  Domain               : 0x%04X\n"
		"  Instance             : %u\n\n"
		"  Request              : %p\n"
		"  Response             : %p\n",
		self->fInstance,
		esif_primitive_str((enum esif_primitive_type)tuplePtr->id), tuplePtr->id,
		tuplePtr->id,
		tuplePtr->instance,
		requestPtr,
		responsePtr);

	dspPtr = self->fDspPtr;
	if (NULL == dspPtr) {
		ESIF_TRACE_ERROR("DSP is not found for participant %d\n", self->fInstance);
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}	

	if (dspPtr->type == NULL) {
		rc = ESIF_E_NEED_DSP;
		goto exit;
	}

	primitivePtr = dspPtr->get_primitive(dspPtr, tuplePtr);
	if (NULL == primitivePtr) {
		ESIF_TRACE_DEBUG("The primitive id %d is not found in DSP\n", tuplePtr->id);
		rc = ESIF_E_PRIMITIVE_NOT_FOUND_IN_DSP;
		goto exit;
	}

	/* Print */
	EsifUfDumpPrimitive(dspPtr, primitivePtr);

	if (primitivePtr->num_actions == 0) {
		ESIF_TRACE_ERROR("No action is supported for primitive %d\n", tuplePtr->id);
		rc = ESIF_E_PRIMITIVE_NO_ACTION_AVAIL;
		goto exit;
	}

	/* Automatically Pick Up The Right Data Type, If Asked To Do So */
	if (requestPtr != NULL) {
		ESIF_DATA_RETYPE(requestPtr->type,
			ESIF_DATA_AUTO,
			(enum esif_data_type)primitivePtr->request_type,
			reqAuto);
	}
	if (responsePtr != NULL) {
		ESIF_DATA_RETYPE(responsePtr->type,
			ESIF_DATA_AUTO,
			(enum esif_data_type)primitivePtr->result_type,
			rspAuto);

		/* Work around for now */
		if (ESIF_DATA_BIT == responsePtr->type ||
			ESIF_DATA_UINT8 == responsePtr->type ||
			ESIF_DATA_UINT16 == responsePtr->type) {
			responsePtr->type = ESIF_DATA_UINT32;
		}
		if (ESIF_DATA_UNICODE == responsePtr->type) {
			responsePtr->type = ESIF_DATA_STRING;
		}

		if ((responsePtr->buf_ptr == NULL) && (ESIF_DATA_ALLOCATE == responsePtr->buf_len)) {
			int size = 4;

			if (ESIF_DATA_UINT64 == responsePtr->type ||
				ESIF_DATA_FREQUENCY == responsePtr->type) {
				size = 8;
			}
			if (ESIF_DATA_STRING == responsePtr->type) {
				size = 128;
			}
			if (ESIF_DATA_UNICODE == responsePtr->type) {
				size = 256;
			}
			if (ESIF_DATA_BINARY == responsePtr->type) {
				size = 4096;
			}

			responsePtr->buf_ptr = esif_ccb_malloc(size);
			responsePtr->buf_len = size;
			if (NULL == responsePtr->buf_ptr) {
				ESIF_TRACE_ERROR("Fail to allocate response buffer\n");
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}

			ESIF_TRACE_DEBUG("Auto allocated response buffer %p size %u\n",
				responsePtr->buf_ptr,
				responsePtr->buf_len);
		}
	}

	/*
	 * Execute each action in the primitive until one succeeds
	 */
	for (kernAct = 0, i = 0; i < (int)primitivePtr->num_actions; i++) {
		actionPtr = dspPtr->get_action(dspPtr, primitivePtr, (u8)i);

		if (NULL == actionPtr) {
			ESIF_TRACE_DEBUG("No actions succeeded for primitive (%d), current action %d\n", tuplePtr->id, i);
			rc = ESIF_E_PRIMITIVE_NO_ACTION_SUCCESSFUL;
			goto exit;
		}

		rc = EsifUp_ExecuteAction(self, primitivePtr, actionPtr, kernAct, requestPtr, responsePtr);
		if (ESIF_OK == rc ) {
			break;
		}
		
		/*
		 * When Default Buffer Size Is Too Small, Retry Once Using Response Data Size.
		 * This Only Applies To ESIF_DATA_AUTO.
		 */
		if (ESIF_E_NEED_LARGER_BUFFER == rc) {
			/*
			 * If autosizing is not enabled, break after the first action that
			 * requires a larger buffer.
			 */
			if (ESIF_FALSE == rspAuto) {
				break;
			}
			ESIF_TRACE_DEBUG("Auto re-malloc: original %u new buffer size %u byte\n",
				responsePtr->buf_len,
				responsePtr->data_len);

			esif_ccb_free(responsePtr->buf_ptr);

			responsePtr->buf_ptr = esif_ccb_malloc(responsePtr->data_len);
			responsePtr->buf_len = responsePtr->data_len;

			if (NULL == responsePtr->buf_ptr) {
				ESIF_TRACE_ERROR("Fail to allocate response buffer\n");
				rc = ESIF_E_NO_MEMORY;
				goto exit;
			}
			rc = EsifUp_ExecuteAction(self, primitivePtr, actionPtr, kernAct, requestPtr, responsePtr);
			if (ESIF_OK == rc ) {
				break;
			}
		}

		if (actionPtr->is_kernel > 0) {
			kernAct++;
		}
	}

exit:
	ESIF_TRACE_DEBUG("Primitive result = %s\n", esif_rc_str(rc));
	return rc;
}


static eEsifError EsifUp_ExecuteAction(
	EsifUpPtr self,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	UInt16 kernelActNum,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(primitivePtr != NULL);
	ESIF_ASSERT(actionPtr != NULL);

	if (actionPtr->is_kernel > 0) {
		/* ESIF LF via IPC */
		if (ESIF_PRIMITIVE_OP_GET == primitivePtr->operation) {
			rc = EsifUp_ExecuteLfGetAction(self,
				primitivePtr,
				actionPtr,
				kernelActNum,
				requestPtr,
				responsePtr);
		} else {
			rc = EsifUp_ExecuteLfSetAction(self,
				primitivePtr,
				actionPtr,
				kernelActNum,
				requestPtr,
				responsePtr);
		}
		ESIF_TRACE_DEBUG("Used ESIF LF for kernel action %d; rc = %s\n",
			kernelActNum,
			esif_rc_str(rc));

	} else {
		/* User-Level Only */
		if (ESIF_PRIMITIVE_OP_GET == primitivePtr->operation) {
			rc = EsifUp_ExecuteUfGetAction(self,
				primitivePtr,
				actionPtr,
				requestPtr,
				responsePtr);
		} else {
			rc = EsifUp_ExecuteUfSetAction(self,
				primitivePtr,
				actionPtr,
				requestPtr);
		}
		ESIF_TRACE_DEBUG("Used User-Level service for action %u; rc = %s\n", kernelActNum, esif_rc_str(rc));
	}
	return rc;
}


static eEsifError EsifUp_ExecuteUfGetAction(
	EsifUpPtr self,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	enum esif_action_type actionType;
	EsifActTypePtr actiontypePtr = NULL;
	struct esif_data voidRequest  = {ESIF_DATA_VOID, NULL, 0, 0};

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(primitivePtr != NULL);
	ESIF_ASSERT(actionPtr != NULL);

	if (NULL == responsePtr) {
		ESIF_TRACE_ERROR("Response pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (NULL == requestPtr) {
		requestPtr = &voidRequest;
	}

	actionType = actionPtr->type;

	/* Find Action From Action Type LIST */
	actiontypePtr = g_actMgr.GetActType(&g_actMgr, actionType);
	if (NULL == actiontypePtr) {
		rc = ESIF_E_UNSUPPORTED_ACTION_TYPE;
		ESIF_TRACE_DEBUG("Action For Type %d NOT FOUND Skipping...\n", actionType);
		goto exit;
	}

	/* Validate Action Type */
	if (ESIF_TRUE == actiontypePtr->fIsKernel) {
		ESIF_TRACE_DEBUG("Invalid action : Kernel action\n");
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	/* Have Action So Execute */
	ESIF_TRACE_DEBUG("Have Action %s(%d)\n",
		esif_action_type_str(actionType),
		actionType);

	/*
	 * Perform a sanity check at the topmost level between the "integer"
	 * data types and buffer sizes.  This allows simplification of the
	 * checks required within subordinate code.
	 */
	if ((responsePtr->buf_len < (int) esif_data_type_sizeof(responsePtr->type)) ||
		(requestPtr->buf_len < (int) esif_data_type_sizeof(requestPtr->type))) {
		rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		goto exit;
	}

	if(actiontypePtr->fIsPlugin == ESIF_FALSE) {
		/* Validate Action Pointer */
		if (NULL == actiontypePtr->fGetFuncPtr) {
			ESIF_TRACE_DEBUG("Invalid action : Action function pointer is NULL\n");
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			goto exit;
		}

		rc = actiontypePtr->fGetFuncPtr(actiontypePtr->fHandle,
			self,
			primitivePtr,
			actionPtr,
			requestPtr,
			responsePtr);
	} else {
		rc = EsifActCallPluginGet(actiontypePtr->fHandle,
			self,
			actionPtr,
			actiontypePtr->fActGetFuncPtr,
			requestPtr,
			responsePtr);
	}
	ESIF_TRACE_DEBUG("USER rc %s, Buffer Len %d, Data Len %d\n",
		esif_rc_str(rc),
		responsePtr->buf_len,
		responsePtr->data_len);

	/* UF Transform*/
	if (rc == ESIF_OK) {
		EsifUfExecuteTransform(
			responsePtr,
			actionType,
			self->fDspPtr,
			ESIF_PRIMITIVE_OP_GET);
	}
exit:
	return rc;
}


static eEsifError EsifUp_ExecuteUfSetAction(
	EsifUpPtr self,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const EsifDataPtr requestPtr
	)
{
	eEsifError rc    = ESIF_OK;
	enum esif_action_type actionType;
	EsifActTypePtr actiontypePtr = NULL;

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(primitivePtr != NULL);
	ESIF_ASSERT(actionPtr != NULL);

	if (NULL == requestPtr) {
		ESIF_TRACE_ERROR("Request pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	/* Find Action From Action Type LIST */
	actionType = actionPtr->type;
	actiontypePtr = g_actMgr.GetActType(&g_actMgr, actionType);
	if (NULL == actiontypePtr) {
		rc = ESIF_E_UNSUPPORTED_ACTION_TYPE;
		ESIF_TRACE_DEBUG("Action For Type %d NOT FOUND Skipping...\n", actionType);
		goto exit;
	}

	/* Validate Action */
	if (ESIF_TRUE == actiontypePtr->fIsKernel) {
		ESIF_TRACE_DEBUG("Invalid action : Kernel action\n");
		rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
		goto exit;
	}

	/* Have Action So Execute */
	ESIF_TRACE_DEBUG("Have Action %s(%d)\n",
		esif_action_type_str(actionType),
		actionType);

	ESIF_TRACE_DEBUG("Buffer Len %d, Data Len %d\n",
		requestPtr->buf_len,
		requestPtr->data_len);

	/*
	 * Perform a sanity check at the topmost level between the "integer"
	 * data types and buffer sizes.  This allows simplification of the
	 * checks required within subordinate code.
	 */
	if (requestPtr->buf_len < (int) esif_data_type_sizeof(requestPtr->type)) {
		rc = ESIF_E_REQ_SIZE_TYPE_MISTMATCH;
		goto exit;
	}

	/* UF Transform*/
	EsifUfExecuteTransform(requestPtr,
		actionType,
		self->fDspPtr,
		ESIF_PRIMITIVE_OP_SET);

	if(actiontypePtr->fIsPlugin == ESIF_FALSE) {
		/* Validate Action Pointer */
		if (NULL == actiontypePtr->fSetFuncPtr) {
			ESIF_TRACE_DEBUG("Invalid action : Action function pointer is NULL\n");
			rc = ESIF_E_PARAMETER_IS_OUT_OF_BOUNDS;
			goto exit;
		}

		rc = actiontypePtr->fSetFuncPtr(actiontypePtr->fHandle,
			self,
			primitivePtr,
			actionPtr,
			requestPtr);
	} else {
		rc = EsifActCallPluginSet(actiontypePtr->fHandle,
			self,
			actionPtr,
			actiontypePtr->fActSetFuncPtr,
			requestPtr);
	}
	ESIF_TRACE_DEBUG("USER rc %s, Buffer Len %d, Data Len %d\n",
		esif_rc_str(rc),
		requestPtr->buf_len,
		requestPtr->data_len);
exit:
	return rc;
}


static eEsifError EsifUp_ExecuteLfGetAction(
	EsifUpPtr self,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const UInt16 kernActionNum,
	EsifDataPtr requestPtr,
	const EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifIpcPtr ipcPtr = NULL;
	EsifIpcPrimitivePtr ipcPrimPtr = NULL;
	u8 *addr = NULL;
	struct esif_data voidRequest  = {ESIF_DATA_VOID, NULL, 0, 0};

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(primitivePtr != NULL);
	ESIF_ASSERT(actionPtr != NULL);

	ESIF_TRACE_DEBUG("Send To LOWER_FRAMEWORK/KERNEL\n");

	if (NULL == responsePtr) {
		ESIF_TRACE_ERROR("Response pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (NULL == requestPtr) {
		requestPtr = &voidRequest;
	}

	ipcPtr = esif_ipc_alloc_primitive(&ipcPrimPtr, (responsePtr->buf_len + requestPtr->buf_len));
	if (NULL == ipcPtr || NULL == ipcPrimPtr) {
		ESIF_TRACE_ERROR("Fail to allocate EsifIpc/EsifIpcPrimitive for IPC privimitive execution\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	ipcPrimPtr->id              = (u32)primitivePtr->tuple.id;
	ipcPrimPtr->domain          = primitivePtr->tuple.domain;
	ipcPrimPtr->instance        = (u8)primitivePtr->tuple.instance;
	ipcPrimPtr->src_id          = ESIF_INSTANCE_UF;
	ipcPrimPtr->dst_id          = self->fLpInstance;
	ipcPrimPtr->kern_action     = kernActionNum;
	ipcPrimPtr->action_type     = (enum esif_action_type)actionPtr->type;

	ipcPrimPtr->rsp_data_type   = responsePtr->type;
	ipcPrimPtr->rsp_data_offset = 0;
	ipcPrimPtr->rsp_data_len    = responsePtr->buf_len;

	if (requestPtr->buf_len == 0) {
		ipcPrimPtr->req_data_type   = ESIF_DATA_VOID;
		ipcPrimPtr->req_data_offset = 0;
		ipcPrimPtr->req_data_len    = 0;
	} else {
		ipcPrimPtr->req_data_type   = requestPtr->type;
		ipcPrimPtr->req_data_offset = responsePtr->buf_len;	// Data Format: {{rsp_data}, {req_data}}
		ipcPrimPtr->req_data_len    = requestPtr->buf_len;
		addr = (u8 *)((u8 *)(ipcPrimPtr + 1) + (responsePtr->buf_len));
		esif_ccb_memcpy(addr, requestPtr->buf_ptr, requestPtr->buf_len);
	}

	rc = ipc_execute(ipcPtr);
	if (rc != ESIF_OK) {
		goto exit;
	}

	responsePtr->data_len = (u16)ipcPrimPtr->rsp_data_len;
	ESIF_TRACE_DEBUG("IPC rc %s, Primitive rc %s, Buffer Len %d, Data Len %d\n",
		esif_rc_str(ipcPtr->return_code),
		esif_rc_str(ipcPrimPtr->return_code),
		responsePtr->buf_len,
		responsePtr->data_len);

	if (ESIF_OK != ipcPtr->return_code) {
		rc = ipcPtr->return_code;
		goto exit;
	}

	if (ESIF_OK != ipcPrimPtr->return_code) {
		rc = ipcPrimPtr->return_code;
		goto exit;
	}

	// Assign Data
	esif_ccb_memcpy(responsePtr->buf_ptr, (ipcPrimPtr + 1), responsePtr->data_len);
exit:
	if (NULL != ipcPtr) {
		esif_ipc_free(ipcPtr);
	}
	return rc;
}


static eEsifError EsifUp_ExecuteLfSetAction(
	EsifUpPtr self,
	const EsifFpcPrimitivePtr primitivePtr,
	const EsifFpcActionPtr actionPtr,
	const UInt16 kernActionNum,
	const EsifDataPtr requestPtr,
	EsifDataPtr responsePtr
	)
{
	eEsifError rc = ESIF_OK;
	EsifIpcPtr ipcPtr = NULL;
	EsifIpcPrimitivePtr ipcPrimPtr = NULL;
	struct esif_data voidResponse  = {ESIF_DATA_VOID, NULL, 0, 0};

	ESIF_ASSERT(self != NULL);
	ESIF_ASSERT(primitivePtr != NULL);
	ESIF_ASSERT(actionPtr != NULL);

	ESIF_TRACE_DEBUG("Send To LOWER_FRAMEWORK/KERNEL\n");

	if (NULL == requestPtr) {
		ESIF_TRACE_ERROR("Request pointer is NULL\n");
		rc = ESIF_E_PARAMETER_IS_NULL;
		goto exit;
	}

	if (NULL == responsePtr) {
		responsePtr = &voidResponse;
	}

	ipcPtr = esif_ipc_alloc_primitive(&ipcPrimPtr, requestPtr->buf_len);
	if (NULL == ipcPtr || NULL == ipcPrimPtr) {
		ESIF_TRACE_ERROR("Fail to allocate EsifIpc/EsifIpcPrimitive for IPC privimitive execution\n");
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	ipcPrimPtr->id              = primitivePtr->tuple.id;
	ipcPrimPtr->domain          = primitivePtr->tuple.domain;
	ipcPrimPtr->instance        = (u8)primitivePtr->tuple.instance;
	ipcPrimPtr->src_id          = ESIF_INSTANCE_UF;
	ipcPrimPtr->dst_id          = self->fLpInstance;
	ipcPrimPtr->kern_action     = kernActionNum;
	ipcPrimPtr->req_data_type   = requestPtr->type;
	ipcPrimPtr->action_type     = (enum esif_action_type)actionPtr->type;
	ipcPrimPtr->req_data_offset = 0;
	ipcPrimPtr->req_data_len    = requestPtr->buf_len;
	ipcPrimPtr->rsp_data_type   = responsePtr->type;
	ipcPrimPtr->rsp_data_offset = 0;
	ipcPrimPtr->rsp_data_len    = responsePtr->buf_len;

	esif_ccb_memcpy((ipcPrimPtr + 1), requestPtr->buf_ptr, requestPtr->buf_len);

	rc = ipc_execute(ipcPtr);
	if (ESIF_OK != rc) {
		goto exit;
	}

	if (ESIF_OK != ipcPtr->return_code) {
		rc = ipcPtr->return_code;
		ESIF_TRACE_DEBUG("ipc_ptr return_code failure - %s\n", esif_rc_str(rc));
		goto exit;
	}

	if (ESIF_OK != ipcPrimPtr->return_code) {
		rc = ipcPrimPtr->return_code;
		ESIF_TRACE_DEBUG("ipcPrimPtr return_code failure - %s\n", esif_rc_str(rc));
		goto exit;
	}
exit:
	if (NULL != ipcPtr) {
		esif_ipc_free(ipcPtr);
	}
	return rc;
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
