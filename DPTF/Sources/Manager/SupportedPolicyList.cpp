/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "SupportedPolicyList.h"
#include "EsifServicesInterface.h"
#include "esif_ccb_memory.h"
#include "esif_sdk_primitive_type.h"
#include "esif_sdk_data.h"
#include "ManagerLogger.h"
#include "ManagerMessage.h"

static const UIntN GuidSize = 16;

#pragma pack(push, 1)
typedef struct _AcpiEsifGuid
{
	union esif_data_variant esifDataVariant;
	UInt8 uuid[GuidSize];
} AcpiEsifGuid;
#pragma pack(pop)

SupportedPolicyList::SupportedPolicyList(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
{
	update();
}

UIntN SupportedPolicyList::getCount(void) const
{
	return static_cast<UIntN>(m_guid.size());
}

Guid SupportedPolicyList::operator[](UIntN index) const
{
	return get(index);
}

Guid SupportedPolicyList::get(UIntN index) const
{
	return m_guid.at(index);
}

Bool SupportedPolicyList::isPolicySupported(const Guid& guid) const
{
#ifdef DISABLE_VALID_POLICY_CHECK

	return true;

#else

	Bool supported = false;

	for (auto it = m_guid.begin(); it != m_guid.end(); it++)
	{
		if (*it == guid)
		{
			supported = true;
			break;
		}
	}

	return supported;

#endif
}

void SupportedPolicyList::update(void)
{
	try
	{
		DptfBuffer buffer = m_dptfManager->getEsifServices()->primitiveExecuteGet(
			esif_primitive_type::GET_SUPPORTED_POLICIES, ESIF_DATA_BINARY);
		if (isBufferValid(buffer))
		{
			m_guid = parseBufferForPolicyGuids(buffer);
		}
		else
		{
			m_guid.clear();
			std::stringstream message;
			message << "Received invalid data length [" << buffer.size()
				<< "] from primitive call: GET_SUPPORTED_POLICIES";
			throw dptf_exception(message.str());
		}
	}
	catch (const std::exception& ex)
	{
		MANAGER_LOG_MESSAGE_WARNING_EX({
			std::stringstream message;
			message << ex.what();
			ManagerMessage warningMessage = ManagerMessage(m_dptfManager, _file, _line, _function, message.str());
			return warningMessage;
			});

		m_guid.clear();
	}

	postMessageWithSupportedGuids();
}

Bool SupportedPolicyList::isBufferValid(const DptfBuffer& buffer) const
{
	return ((buffer.size() % sizeof(AcpiEsifGuid)) == 0);
}

std::vector<Guid> SupportedPolicyList::parseBufferForPolicyGuids(const DptfBuffer& buffer)
{
	UInt32 guidCount = buffer.size() / sizeof(AcpiEsifGuid);
	AcpiEsifGuid* acpiEsifGuid = reinterpret_cast<AcpiEsifGuid*>(buffer.get());

	std::vector<Guid> guids;
	for (UInt32 i = 0; i < guidCount; i++)
	{
		UInt8 guidByteArray[GuidSize] = { 0 };
		esif_ccb_memcpy(guidByteArray, &acpiEsifGuid[i].uuid, GuidSize);
		guids.push_back(Guid(guidByteArray));
	}
	return guids;
}

void SupportedPolicyList::postMessageWithSupportedGuids() const
{
	MANAGER_LOG_MESSAGE_INFO({
		std::stringstream message;
		if (m_guid.size() > 0)
		{
			for (auto g = m_guid.begin(); g != m_guid.end(); ++g)
			{
				message << "Supported GUID: " << g->toString() << "\n";
			}
		}
		else
		{
			message << "No supported GUIDs found";
		}
		ManagerMessage infoMessage = ManagerMessage(m_dptfManager, _file, _line, _function, message.str());
		return infoMessage;
		});
}


EsifServicesInterface* SupportedPolicyList::getEsifServices() const
{
	return m_dptfManager->getEsifServices();
}
