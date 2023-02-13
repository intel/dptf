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

#include "SupportedPolicyList.h"
#include "EsifServicesInterface.h"
#include "esif_ccb_memory.h"
#include "esif_sdk_primitive_type.h"
#include "esif_sdk_data.h"
#include "ManagerLogger.h"
#include "ManagerMessage.h"
#include <algorithm>

using namespace std;

static const UIntN GuidSize = 16;

#pragma pack(push, 1)
typedef struct _AcpiEsifGuid
{
	union esif_data_variant esifDataVariant;
	UInt8 uuid[GuidSize];
} AcpiEsifGuid;
#pragma pack(pop)
set<Guid> parseGuids(const size_t guidCount, const AcpiEsifGuid* data);

// System Configuration policy always enabled
const Guid SystemConfigurationPolicy{0x65, 0x10, 0x95, 0x92, 0x27, 0x64, 0x30, 0x4B, 0x97, 0x37, 0x13, 0xCC, 0x70, 0xEF, 0xA2, 0x1A};

SupportedPolicyList::SupportedPolicyList(DptfManagerInterface* dptfManager, const set<Guid>& defaultGuids)
	: m_dptfManager(dptfManager)
	, m_defaultGuids(defaultGuids)
{
	update();
}

Bool SupportedPolicyList::isPolicySupported(const Guid& guid) const
{
#ifdef DISABLE_VALID_POLICY_CHECK

	return true;

#else

	return (m_supportedPolicies.end() != m_supportedPolicies.find(guid));

#endif
}

void SupportedPolicyList::update(void)
{
	set<Guid> supportedPolicies = readPoliciesFromIdsp();
	addDefaultPoliciesIfEmpty(supportedPolicies);
	addSystemPolicy(supportedPolicies);
	m_supportedPolicies = supportedPolicies;
	postMessageWithSupportedGuids();
}

void SupportedPolicyList::addDefaultPoliciesIfEmpty(set<Guid>& supportedPolicies) const
{
	if (supportedPolicies.empty())
	{
		MANAGER_LOG_MESSAGE_INFO(
			{ return ManagerMessage(m_dptfManager, _file, _line, _function, "Using default policies."); });
		for (const auto& g : m_defaultGuids)
		{
			supportedPolicies.emplace(g);
		}
	}
}

void SupportedPolicyList::addSystemPolicy(set<Guid>& supportedPolicies) const
{
	supportedPolicies.emplace(SystemConfigurationPolicy);
}

set<Guid> SupportedPolicyList::getGuids() const
{
	return m_supportedPolicies;
}

UInt64 SupportedPolicyList::getCount() const
{
	return m_supportedPolicies.size();
}

set<Guid> SupportedPolicyList::readPoliciesFromIdsp() const
{
	try
	{
		const auto buffer = m_dptfManager->getEsifServices()->primitiveExecuteGet(
			esif_primitive_type::GET_SUPPORTED_POLICIES, ESIF_DATA_BINARY);
		throwIfIdspDataValid(buffer);
		const auto guidCount = buffer.size() / sizeof(AcpiEsifGuid);
		const auto acpiEsifGuid = reinterpret_cast<AcpiEsifGuid*>(buffer.get());
		return parseGuids(guidCount, acpiEsifGuid);
	}
	catch (const exception& ex)
	{
		MANAGER_LOG_MESSAGE_WARNING_EX(
			{ return ManagerMessage(m_dptfManager, _file, _line, _function, string(ex.what())); });
		return {};
	}
}

set<Guid> parseGuids(const size_t guidCount, const AcpiEsifGuid* data)
{
	set<Guid> guids;
	for (UInt32 i = 0; i < guidCount; i++)
	{
		guids.emplace(data[i].uuid);
	}
	return guids;
}

void SupportedPolicyList::postMessageWithSupportedGuids() const
{
	MANAGER_LOG_MESSAGE_INFO({
		stringstream message;
		if (!m_supportedPolicies.empty())
		{
			for (const auto& g : m_supportedPolicies)
			{
				message << "Supported GUID: " << g.toString() << endl;
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

Bool SupportedPolicyList::isIdspDataValid(const DptfBuffer& buffer)
{
	return ((buffer.size() % sizeof(AcpiEsifGuid)) == 0);
}

void SupportedPolicyList::throwIfIdspDataValid(const DptfBuffer& buffer) const
{
	if (!isIdspDataValid(buffer))
	{
		stringstream message;
		message << "Received invalid data length ["s << buffer.size() << "]"s
				<< " from GET_SUPPORTED_POLICIES buffer."s;
		throw dptf_exception(message.str());
	}
}

EsifServicesInterface* SupportedPolicyList::getEsifServices() const
{
	return m_dptfManager->getEsifServices();
}
