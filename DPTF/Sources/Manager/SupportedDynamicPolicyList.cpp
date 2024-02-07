/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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

#include "SupportedDynamicPolicyList.h"
#include "EsifServicesInterface.h"
#include "ManagerLogger.h"
#include "ManagerMessage.h"
#include "DataManager.h"
#include "TableObjectType.h"
#include "StringConverter.h"

using namespace std;

static constexpr UIntN UuidStringLength = 36;

struct EsifDataBinarySupportedDynamicPolicyListPackage
{
	esif_data_variant uuid;
	esif_data_variant templateGuid;
	esif_data_variant name;
};

DynamicIdspTableEntry::DynamicIdspTableEntry(const string& uuid, const string& uuidTemplate, const string& name)
{
	m_uuid = Guid::fromUnmangledString(uuid);
	m_templateGuid = Guid::fromUnmangledString(uuidTemplate);
	m_name = name;
	m_uuidString = StringConverter::toLower(uuid);
}

Bool DynamicIdspTableEntry::isSameAs(const DynamicIdspTableEntry& tableEntry) const
{
	return (
		(m_uuid == tableEntry.m_uuid) && (m_templateGuid == tableEntry.m_templateGuid)
		&& (m_name == tableEntry.m_name));
}

Guid DynamicIdspTableEntry::getUuid() const
{
	return m_uuid;
}

Guid DynamicIdspTableEntry::getTemplateGuid() const
{
	return m_templateGuid;
}

string DynamicIdspTableEntry::getName() const
{
	return m_name;
}

string DynamicIdspTableEntry::getUuidString() const
{
	return m_uuidString;
}

SupportedDynamicPolicyList::SupportedDynamicPolicyList(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
{
	SupportedDynamicPolicyList::update();
}

UIntN SupportedDynamicPolicyList::getCount() const
{
	return static_cast<UIntN>(m_dynamicPolicyList.size());
}

DynamicIdspTableEntry SupportedDynamicPolicyList::get(UIntN index) const
{
	return m_dynamicPolicyList.at(index);
}

void SupportedDynamicPolicyList::update()
{
	try
	{
		const DptfBuffer buffer = m_dptfManager->getDataManager()
		                                       ->getTableObject(TableObjectType::Dynamic_Idsp, Constants::EmptyString)
		                                       .getData();
		m_dynamicPolicyList = parseBufferForDynamicPolicyUuids(buffer);
	}
	catch (const exception& ex)
	{
		MANAGER_LOG_MESSAGE_WARNING_EX(
			{
				stringstream message;
				message << ex.what();
				ManagerMessage warningMessage = ManagerMessage(m_dptfManager, _file, _line, _function, message.str());
				return warningMessage;
			});

		m_dynamicPolicyList.clear();
	}
}

vector<DynamicIdspTableEntry> SupportedDynamicPolicyList::parseBufferForDynamicPolicyUuids(const DptfBuffer& buffer)
{
	if (buffer.size() == 0)
	{
		throw dptf_exception("There is no data to process.");
	}

	auto data = buffer.get();
	const UIntN rowCount = countRows(buffer.size(), data);

	data = buffer.get();
	data += sizeof(esif_data_variant); // Ignore revision field

	auto currentRow = reinterpret_cast<struct EsifDataBinarySupportedDynamicPolicyListPackage*>(data);

	vector<DynamicIdspTableEntry> entries;
	for (UIntN i = 0; i < rowCount; i++)
	{
		auto uuid = StringParser::trimNulls(string(
			reinterpret_cast<const char*>(&(currentRow->uuid)) + sizeof(union esif_data_variant),
			currentRow->uuid.string.length));

		data += currentRow->uuid.string.length;
		currentRow = reinterpret_cast<EsifDataBinarySupportedDynamicPolicyListPackage*>(data);

		auto uuidTemplate = StringParser::trimNulls(string(
			reinterpret_cast<const char*>(&(currentRow->templateGuid)) + sizeof(union esif_data_variant),
			currentRow->templateGuid.string.length));

		data += currentRow->templateGuid.string.length;
		currentRow = reinterpret_cast<EsifDataBinarySupportedDynamicPolicyListPackage*>(data);

		const char* nameAddress = reinterpret_cast<const char*>(&currentRow->name) + sizeof(union esif_data_variant);
		auto name = StringParser::trimNulls(string(nameAddress, currentRow->name.string.length));

		data += currentRow->name.string.length;
		currentRow = reinterpret_cast<EsifDataBinarySupportedDynamicPolicyListPackage*>(data);

		throwIfInvalidUuidLength(uuid);
		throwIfInvalidUuidLength(uuidTemplate);

		DynamicIdspTableEntry temp(uuid, uuidTemplate, name);

		Bool isDuplicateEntry = false;
		for (const auto& entry : entries)
		{
			if (temp.isSameAs(entry))
			{
				isDuplicateEntry = true;
				break;
			}
		}

		if (isDuplicateEntry == false)
		{
			entries.push_back(temp);
		}

		data += sizeof(EsifDataBinarySupportedDynamicPolicyListPackage);
		currentRow = reinterpret_cast<EsifDataBinarySupportedDynamicPolicyListPackage*>(data);
	}
	return entries;
}

UIntN SupportedDynamicPolicyList::countRows(UInt32 size, UInt8* data)
{
	IntN bytesRemaining = size;
	UIntN rows = 0;

	// Remove revision field
	const esif_data_variant* revision = reinterpret_cast<esif_data_variant*>(data);
	if ((revision->type == esif_data_type::ESIF_DATA_UINT32) || 
		(revision->type == esif_data_type::ESIF_DATA_UINT64))
	{
		data += sizeof(esif_data_variant);
		bytesRemaining -= sizeof(esif_data_variant);
	}
	else
	{
		throw dptf_exception("Revision field is missing");
	}
	throwIfOutOfRange(bytesRemaining);

	const EsifDataBinarySupportedDynamicPolicyListPackage* currentRow =
		reinterpret_cast<struct EsifDataBinarySupportedDynamicPolicyListPackage*>(data);

	while (bytesRemaining > 0)
	{
		throwIfOutOfRange(currentRow->uuid.string.length); // Verify string length not negative or will become
														   // positive when subtracted from bytesRemaining
		bytesRemaining -= currentRow->uuid.string.length;
		throwIfOutOfRange(bytesRemaining);

		data += currentRow->uuid.string.length;
		currentRow = reinterpret_cast<EsifDataBinarySupportedDynamicPolicyListPackage*>(data);

		throwIfOutOfRange(currentRow->templateGuid.string.length); // Verify string length not negative or will become
																   // positive when subtracted from bytesRemaining
		bytesRemaining -= currentRow->templateGuid.string.length;
		throwIfOutOfRange(bytesRemaining);

		data += currentRow->templateGuid.string.length;
		currentRow = reinterpret_cast<EsifDataBinarySupportedDynamicPolicyListPackage*>(data);

		throwIfOutOfRange(currentRow->name.string.length); // Verify string length not negative or will become
														   // positive when subtracted from bytesRemaining
		bytesRemaining -= currentRow->name.string.length;
		throwIfOutOfRange(bytesRemaining);

		data += currentRow->name.string.length;
		currentRow = reinterpret_cast<EsifDataBinarySupportedDynamicPolicyListPackage*>(data);

		rows++;

		bytesRemaining -= sizeof(EsifDataBinarySupportedDynamicPolicyListPackage);
		throwIfOutOfRange(bytesRemaining);

		data += sizeof(EsifDataBinarySupportedDynamicPolicyListPackage);
		currentRow = reinterpret_cast<EsifDataBinarySupportedDynamicPolicyListPackage*>(data);
	}

	return rows;
}

void SupportedDynamicPolicyList::throwIfOutOfRange(IntN bytesRemaining)
{
	if (bytesRemaining < 0)
	{
		throw dptf_exception("Expected binary data size mismatch.");
	}
}

void SupportedDynamicPolicyList::throwIfInvalidUuidLength(const string& uuid)
{
	if (uuid.length() != UuidStringLength)
	{
		throw dptf_exception("Expected uuid string length mismatch.");
	}
}

EsifServicesInterface* SupportedDynamicPolicyList::getEsifServices() const
{
	return m_dptfManager->getEsifServices();
}
