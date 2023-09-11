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

#include "EsifDataCreateParticipant.h"
#include "esif_sdk_iface_conjure.h"

using namespace std;

EsifDataCreateParticipant::EsifDataCreateParticipant(
	const DptfRequestCreateParticipant::RequestData& data)
{
	makeParticipantInfo(data.name, data.description);
}

EsifDataCreateParticipant::EsifDataCreateParticipant(
	const DptfRequestDeleteParticipant::RequestData& data)
{
	makeParticipantInfo(data.name);
}

EsifData EsifDataCreateParticipant::getEsifData() const
{
	return m_esifData;
}

void EsifDataCreateParticipant::makeParticipantInfo(const string& name, const string& description)
{
	m_esifData = {
		ESIF_DATA_STRUCTURE, &m_esifEventData, sizeof(m_esifEventData), sizeof(m_esifEventData)};
	m_esifEventData.hdr.version = ESIF_EVENT_DATA_PARTICIPANT_CREATE_HDR_VERSION;
	m_esifEventData.hdr.dataType = ESIF_EVENT_DATA_CREATE_PARTICIPANT_TYPE_UP;
	m_esifEventData.data.lfData = esif_ipc_event_data_create_participant{};
	m_esifEventData.data.ufData = _t_EsifParticipantIface{};

	EsifParticipantIface* info = &m_esifEventData.data.ufData;
	esif_ccb_strncpy(info->name, name.c_str(), sizeof(info->name));
	esif_ccb_strncpy(info->desc, description.c_str(), sizeof(info->desc));
	info->version = ESIF_EVENT_DATA_PARTICIPANT_CREATE_UF_VERSION;
	info->enumerator = ESIF_PARTICIPANT_ENUM_CONJURE;
	const esif_guid_t newParticipantClassGuid = ESIF_PARTICIPANT_CONJURE_CLASS_GUID;
	esif_ccb_memcpy(info->class_guid, newParticipantClassGuid, sizeof(newParticipantClassGuid));
	info->acpi_type = ESIF_DOMAIN_TYPE_VIRTUAL;
	const char* deviceName = "INT3409";
	esif_ccb_strncpy(info->device_name, deviceName, sizeof(info->device_name));
	stringstream stream;
	stream << "_UP_."s << name;
	esif_ccb_strncpy(info->object_id, stream.str().c_str(), sizeof(info->object_id));
	esif_ccb_strncpy(info->device_path, "/sys/class/thermal/", sizeof(info->device_path));
	esif_ccb_strncpy(info->driver_name, "N/A", sizeof(info->driver_name));
	info->flags = 0;
	info->send_event = nullptr;
	info->recv_event = nullptr;
}