/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
#include "upe.h"
#include "jhs_binder_service.h"
#include <binder/IPCThreadState.h>
#include <binder/Parcel.h>

#include "esif_sdk.h"
#include "esif_sdk_iface_conjure.h"	/* Conjure Interface */
#include "esif_sdk_class_guid.h"
#include "esif_ccb_memory.h"
#include "esif_lib_esifdata.h"

#include "conjure.h"

// Private macro helper to assemble event data
#define EVENTDATA_ENCODE(p1, p2) (((u32) (p1) << 16) | (p2))

namespace jhs {
	// Helper function translating device HID and PType to ACPI 4-letter name
	static void getDefaultParticipantName(char *participantHID, UInt32 pType, char *defaultParticipantName)
	{
		if (!esif_ccb_strcmp(participantHID, "INT3400")) {
			esif_ccb_strcpy(defaultParticipantName, "IETM", ESIF_ACPI_NAME_LEN);
		}
		else if (!esif_ccb_strcmp(participantHID, "INT3406")) {
			esif_ccb_strcpy(defaultParticipantName, "TDSP", ESIF_ACPI_NAME_LEN);
		}
		else if (!esif_ccb_strcmp(participantHID, "INT3408") && pType == 15) {
			esif_ccb_strcpy(defaultParticipantName, "WWAN", ESIF_ACPI_NAME_LEN);
		}
		else if (!esif_ccb_strcmp(participantHID, "INT3408") && pType == 7) {
			esif_ccb_strcpy(defaultParticipantName, "WIFI", ESIF_ACPI_NAME_LEN);
		}
		else if (!esif_ccb_strcmp(participantHID, "INT340A")) {
			esif_ccb_strcpy(defaultParticipantName, "ASTG", ESIF_ACPI_NAME_LEN);
		}
		else {
			esif_ccb_strcpy(defaultParticipantName, "FGEN", ESIF_ACPI_NAME_LEN);
		}
	}

	// Helper function converting from wide string to string
	static void wstr2str(int16_t *wstr, char *str, int len)
	{
		int i = 0;

		for (i = 0; i < len; ++i) {
			*str++ = (char) *wstr++;
		}
	}

	IMPLEMENT_META_INTERFACE(JhsClientService, JHS_CLIENT_META_INTERFACE_NAME);
	BpJhsClientService::BpJhsClientService(const sp<IBinder>& impl) : BpInterface<IJhsClientService>(impl)
	{
		UPE_TRACE_DEBUG("JHCS: Should not be called");
	}

	uint32_t JhsClientService::participantActive(DptfParticipant participant)
	{
		esif_guid_t classGuid = ESIF_PARTICIPANT_PLAT_CLASS_GUID;
		char const *devicePath = "/sys/class/thermal/";
		char const *driverName = "N/A";
		char const *desc = "Partipant from JHS";
		char *participantScope = NULL;
		char *targetACPIName = NULL;
		EsifParticipantIface newPart;
		int guid_element_counter = 0;
		int handle = 0; // 0 is an invalid handle. Any positive number is a good handle

		targetACPIName = (char *)esif_ccb_malloc(ESIF_ACPI_NAME_LEN);
		if(NULL == targetACPIName) {
			goto exit;
		}
		participantScope = (char *)esif_ccb_malloc(ESIF_SCOPE_LEN);
		if(NULL == participantScope) {
			goto exit2;
		}

		getDefaultParticipantName(participant.mEisaId, participant.mPtype, targetACPIName);
		esif_ccb_sprintf(ESIF_SCOPE_LEN, participantScope, "\\_SB_.%s", targetACPIName);

		for (guid_element_counter = 0; guid_element_counter < ESIF_GUID_LEN; guid_element_counter++) {
			newPart.class_guid[guid_element_counter] = *(classGuid + guid_element_counter);
		}

		newPart.version = ESIF_EVENT_DATA_PARTICIPANT_CREATE_UF_VERSION;
		newPart.enumerator = ESIF_PARTICIPANT_ENUM_SYSFS;
		newPart.flags = 0x0;
		newPart.send_event = NULL;
		newPart.recv_event = NULL;
		newPart.acpi_type = participant.mPtype;
		esif_ccb_strncpy(newPart.name, targetACPIName, ESIF_NAME_LEN);
		esif_ccb_strncpy(newPart.desc, desc, ESIF_DESC_LEN);
		esif_ccb_strncpy(newPart.object_id, participantScope, ESIF_SCOPE_LEN);
		esif_ccb_strncpy(newPart.device_path, devicePath, ESIF_PATH_LEN);
		esif_ccb_strncpy(newPart.driver_name, driverName, ESIF_NAME_LEN);
		esif_ccb_strncpy(newPart.device_name, participant.mEisaId, ESIF_NAME_LEN);

		handle = (int) RegisterParticipant(&newPart);

		esif_ccb_free(participantScope);
exit2:
		esif_ccb_free(targetACPIName);

exit:
		return handle;
	}


	status_t JhsClientService::participantInActive(JhsParticipantHandle handle)
	{
		UnRegisterParticipant((esif_handle_t)handle.mParticipantId);
		return NO_ERROR;
	}

	status_t JhsClientService::sendEvent(JhsEvent event)
	{
		// Todo: translate (if necessary) JhsEvent to ESIF event and signal upper layers of the event
		eEsifError rc = ESIF_OK;
		esif_handle_t participantId = (esif_handle_t) event.mEventParticipant;
		u32 buf = 0;
		EsifData esifEventData = {ESIF_DATA_UINT32, &buf, sizeof(buf), sizeof(buf)};

		// Create additional ESIF Data to be passed to ESIF/DPTF
		switch (event.mEventType) {
		case ESIF_EVENT_OS_MOBILE_NOTIFICATION:
			buf = EVENTDATA_ENCODE(event.mEventSubType, event.mEventSubTypeParam);
			break;
		case ESIF_EVENT_OS_BATTERY_PERCENT_CHANGED:
		case ESIF_EVENT_POWER_SOURCE_CHANGED:
		case ESIF_EVENT_OS_DOCK_MODE_CHANGED:
		case ESIF_EVENT_DISPLAY_ORIENTATION_CHANGED:
			buf = event.mEventSubType;
			break;
		default:
			// Exit for un-recognized event
			goto exit;
		}
		rc = ActionSendEvent(participantId, (esif_event_type) event.mEventType, &esifEventData);

exit:
		return NO_ERROR;
	}

	status_t BnJhsClientService::onTransact(uint32_t code, const Parcel& data, Parcel *reply, uint32_t flags)
	{
		status_t rc = NO_ERROR;
		int32_t dummy = 0;
		CHECK_INTERFACE(IJhsClientService, data, reply);
		switch(code) {
			case PART_ACTIVE: {
				int16_t jhsString[ESIA_ID_LEN] = { 0 };
				DptfParticipant part = { 0 };
				JhsParticipantHandle handle = { 0 };

				dummy = data.readInt32();
				dummy = data.readInt32();
				data.read(jhsString, ESIA_ID_LEN * 2); // Twice the length due to wide chars
				wstr2str(jhsString, part.mEisaId, ESIA_ID_LEN); // Convert back to chars
				part.mEisaId[ESIA_ID_LEN - 1] = '\0'; // Ensure that string is NULL terminated

				dummy = data.readInt32();
				part.mPtype = data.readInt32();

				UPE_TRACE_DEBUG("JHCS: Dispatch to participantActive(): mEsiaId: %s, mPtype: %d", part.mEisaId, part.mPtype);
				handle.mParticipantId = participantActive(part);
				UPE_TRACE_DEBUG("JHCS: ESIF returning participant ID: %d", handle.mParticipantId);
				reply->writeInt32(0); // a dummy write
				reply->writeInt32(handle.mParticipantId);
				} break;

			case PART_INACTIVE: {
				JhsParticipantHandle handle = { 0 };

				dummy = data.readInt32();
				handle.mParticipantId = data.readInt32();

				UPE_TRACE_DEBUG("JHCS: Dispatch to participantInActive(), handle: %d", handle.mParticipantId);
				rc = participantInActive(handle);
				} break;

			case SEND_EVENT: {
				JhsEvent event = { 0 };

				dummy = data.readInt32();
				event.mEventType = data.readInt32();
				event.mEventParticipant = data.readInt32();
				event.mEventSubType = data.readInt32();
				event.mEventSubTypeParam = data.readInt32();

				UPE_TRACE_DEBUG("JHCS: Dispatch to sendEvent(), mEventType: %d, mEventParticipant: %d, mEventSubType: %d, mEventSubTypeParam: %d",
					event.mEventType, event.mEventParticipant, event.mEventSubType, event.mEventSubTypeParam);
				rc = sendEvent(event);
				} break;
			default:
				// Set some error code here?
				break;
		}

		return rc;
	}

}

