/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "ActiveRelationshipTable.h"
#include "BinaryParse.h"
#include "TableStringParser.h"

ActiveRelationshipTable::ActiveRelationshipTable(
	const std::vector<std::shared_ptr<RelationshipTableEntryBase>>& entries)
	: RelationshipTableBase(entries)
{
}

ActiveRelationshipTable::ActiveRelationshipTable()
	: RelationshipTableBase()
{
}

ActiveRelationshipTable::~ActiveRelationshipTable()
{
}

ActiveRelationshipTable ActiveRelationshipTable::createArtFromDptfBuffer(const DptfBuffer& buffer)
{
	std::vector<std::shared_ptr<RelationshipTableEntryBase>> entries;

	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct EsifDataBinaryArtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

	if (buffer.size() == 0)
	{
		throw dptf_exception("There is no data to process.");
	}

	UIntN rows = countArtRows(buffer.size(), data);

	// Reset currentRow to point to the beginning of the data block
	data = reinterpret_cast<UInt8*>(buffer.get());
	data += sizeof(esif_data_variant); // Ignore revision field
	currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

	for (UIntN i = 0; i < rows; i++)
	{
		// Since the ART has 2 strings in it, the process for extracting them is:
		//  1. Extract the source at the beginning of the structure
		//  2. Since the actual string data is placed between the source and target, the pointer needs moved
		//  3. Move the pointer past the source string data and set current row
		//  4. Now the targetDevice field will actually point to the right spot
		//  5. Extract target device
		//  6. Move the pointer as before (past the targetDevice string data) and set current row
		//  7. Extract the remaining fields
		//  8. Point data and currentRow to the next row

		std::string source = TableStringParser::getDeviceString((currentRow->sourceDevice));

		data += currentRow->sourceDevice.string.length;
		currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

		std::string target = TableStringParser::getDeviceString((currentRow->targetDevice));

		data += currentRow->targetDevice.string.length;
		currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

		std::vector<UInt32> acEntries;
		acEntries.push_back(static_cast<UInt32>(currentRow->ac0MaxFanSpeed.integer.value));
		acEntries.push_back(static_cast<UInt32>(currentRow->ac1MaxFanSpeed.integer.value));
		acEntries.push_back(static_cast<UInt32>(currentRow->ac2MaxFanSpeed.integer.value));
		acEntries.push_back(static_cast<UInt32>(currentRow->ac3MaxFanSpeed.integer.value));
		acEntries.push_back(static_cast<UInt32>(currentRow->ac4MaxFanSpeed.integer.value));
		acEntries.push_back(static_cast<UInt32>(currentRow->ac5MaxFanSpeed.integer.value));
		acEntries.push_back(static_cast<UInt32>(currentRow->ac6MaxFanSpeed.integer.value));
		acEntries.push_back(static_cast<UInt32>(currentRow->ac7MaxFanSpeed.integer.value));
		acEntries.push_back(static_cast<UInt32>(currentRow->ac8MaxFanSpeed.integer.value));
		acEntries.push_back(static_cast<UInt32>(currentRow->ac9MaxFanSpeed.integer.value));

		auto newArtEntry = std::make_shared<ActiveRelationshipTableEntry>(
			BinaryParse::normalizeAcpiScope(source),
			BinaryParse::normalizeAcpiScope(target),
			static_cast<UInt32>(currentRow->weight.integer.value),
			acEntries);

		if (newArtEntry)
		{
			// Check for duplicate entries. Don't add entry if previous entry exists with same target/source pair
			Bool isDuplicateEntry = false;
			for (auto e = entries.begin(); e != entries.end(); e++)
			{
				auto artEntry = std::dynamic_pointer_cast<ActiveRelationshipTableEntry>(*e);
				if (artEntry && newArtEntry->isSameAs(*artEntry))
				{
					isDuplicateEntry = true;
					break;
				}
			}

			if (isDuplicateEntry == false)
			{
				entries.push_back(newArtEntry);
			}
		}

		// Since we've already accounted for the strings, we now move the pointer by the size of the structure
		//  to get to the next row.
		data += sizeof(struct EsifDataBinaryArtPackage);
		currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);
	}

	return ActiveRelationshipTable(entries);
}

UIntN ActiveRelationshipTable::countArtRows(UInt32 size, UInt8* data)
{
	IntN bytesRemaining = size;
	UIntN rows = 0;

	// Remove revision field
	esif_data_variant* revision = reinterpret_cast<esif_data_variant*>(data);
	if ((revision->type == esif_data_type::ESIF_DATA_UINT32) || (revision->type == esif_data_type::ESIF_DATA_UINT64))
	{
		data += sizeof(esif_data_variant);
		bytesRemaining -= sizeof(esif_data_variant);
	}
	else
	{
		throw dptf_exception("Revision Field is Missing. (ART)");
	}

	throwIfOutOfRange(bytesRemaining);

	struct EsifDataBinaryArtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

	while (bytesRemaining > 0)
	{
		bytesRemaining -= sizeof(struct EsifDataBinaryArtPackage);
		throwIfOutOfRange(bytesRemaining);

		bytesRemaining -= currentRow->sourceDevice.string.length;
		throwIfOutOfRange(bytesRemaining);

		data += currentRow->sourceDevice.string.length;
		currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

		bytesRemaining -= currentRow->targetDevice.string.length;
		throwIfOutOfRange(bytesRemaining);

		data += currentRow->targetDevice.string.length;
		currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);

		// The math done here will vary based on the number of strings in the BIOS object
		rows++;

		data += sizeof(struct EsifDataBinaryArtPackage);
		currentRow = reinterpret_cast<struct EsifDataBinaryArtPackage*>(data);
	}

	return rows;
}

void ActiveRelationshipTable::throwIfOutOfRange(IntN bytesRemaining)
{
	if (bytesRemaining < 0)
	{
		throw dptf_exception("Expected binary data size mismatch. (ART)");
	}
}

std::vector<std::shared_ptr<ActiveRelationshipTableEntry>> ActiveRelationshipTable::getEntriesForTarget(UIntN target)
{
	std::vector<std::shared_ptr<ActiveRelationshipTableEntry>> entries;
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		if ((*entry)->getTargetDeviceIndex() == target)
		{
			auto artEntry = std::dynamic_pointer_cast<ActiveRelationshipTableEntry>(*entry);
			if (artEntry)
			{
				entries.push_back(artEntry);
			}
		}
	}
	return entries;
}

std::vector<std::shared_ptr<ActiveRelationshipTableEntry>> ActiveRelationshipTable::getEntriesForSource(UIntN source)
{
	std::vector<std::shared_ptr<ActiveRelationshipTableEntry>> entries;
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		if ((*entry)->getSourceDeviceIndex() == source)
		{
			auto artEntry = std::dynamic_pointer_cast<ActiveRelationshipTableEntry>(*entry);
			if (artEntry)
			{
				entries.push_back(artEntry);
			}
		}
	}
	return entries;
}

std::vector<UIntN> ActiveRelationshipTable::getAllSources(void) const
{
	std::set<UIntN> sources;
	for (UIntN row = 0; row < m_entries.size(); ++row)
	{
		if (m_entries[row]->getSourceDeviceIndex() != Constants::Invalid)
		{
			sources.insert(m_entries[row]->getSourceDeviceIndex());
		}
	}
	return std::vector<UIntN>(sources.begin(), sources.end());
}

std::vector<UIntN> ActiveRelationshipTable::getAllTargets(void) const
{
	std::set<UIntN> targets;
	for (UIntN row = 0; row < m_entries.size(); ++row)
	{
		if (m_entries[row]->getTargetDeviceIndex() != Constants::Invalid)
		{
			targets.insert(m_entries[row]->getTargetDeviceIndex());
		}
	}
	return std::vector<UIntN>(targets.begin(), targets.end());
}

std::shared_ptr<XmlNode> ActiveRelationshipTable::getXml()
{
	auto status = XmlNode::createWrapperElement("art");
	for (auto entry = m_entries.begin(); entry != m_entries.end(); entry++)
	{
		auto artEntry = std::dynamic_pointer_cast<ActiveRelationshipTableEntry>(*entry);
		if (artEntry)
		{
			status->addChild(artEntry->getXml());
		}
	}
	return status;
}

Bool ActiveRelationshipTable::operator==(const ActiveRelationshipTable& art) const
{
	if (getNumberOfEntries() != art.getNumberOfEntries())
	{
		return false;
	}

	for (UIntN i = 0; i < getNumberOfEntries(); i++)
	{
		auto lhsArtEntry = std::dynamic_pointer_cast<ActiveRelationshipTableEntry>(m_entries[i]);
		auto rhsArtEntry = std::dynamic_pointer_cast<ActiveRelationshipTableEntry>(art.m_entries[i]);
		if (!lhsArtEntry || !rhsArtEntry || !(*lhsArtEntry == *rhsArtEntry))
		{
			return false;
		}
	}

	return true;
}

DptfBuffer ActiveRelationshipTable::toArtBinary() const
{
	esif_data_variant revisionField;
	revisionField.integer.type = esif_data_type::ESIF_DATA_UINT64;
	revisionField.integer.value = 1;

	DptfBuffer packages;
	UInt32 offset = 0;
	for (auto entry = m_entries.begin(); entry != m_entries.end(); entry++)
	{
		auto artEntry = std::dynamic_pointer_cast<ActiveRelationshipTableEntry>(*entry);
		if (artEntry)
		{
			UInt32 sourceScopeLength = (UInt32)(*entry)->getSourceDeviceScope().size();
			UInt32 targetScopeLength = (UInt32)(*entry)->getTargetDeviceScope().size();

			DptfBuffer packageBuffer;
			packageBuffer.allocate(sizeof(EsifDataBinaryArtPackage) + sourceScopeLength + targetScopeLength);

			EsifDataBinaryArtPackage entryPackage;
			UInt32 dataAddress = 0;

			// Source Scope
			entryPackage.sourceDevice.string.length = sourceScopeLength;
			entryPackage.sourceDevice.type = esif_data_type::ESIF_DATA_STRING;
			packageBuffer.put(dataAddress, (UInt8*)(&(entryPackage.sourceDevice)), sizeof(entryPackage.sourceDevice));
			dataAddress += sizeof(entryPackage.sourceDevice);
			packageBuffer.put(dataAddress, (UInt8*)((*entry)->getSourceDeviceScope().c_str()), sourceScopeLength);
			dataAddress += sourceScopeLength;

			// Target Scope
			entryPackage.targetDevice.string.length = targetScopeLength;
			entryPackage.targetDevice.type = esif_data_type::ESIF_DATA_STRING;
			packageBuffer.put(dataAddress, (UInt8*)(&(entryPackage.targetDevice)), sizeof(entryPackage.targetDevice));
			dataAddress += sizeof(entryPackage.targetDevice);
			packageBuffer.put(dataAddress, (UInt8*)((*entry)->getTargetDeviceScope().c_str()), targetScopeLength);
			dataAddress += targetScopeLength;

			// Weight
			entryPackage.weight.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.weight.integer.value = artEntry->getWeight();
			packageBuffer.put(dataAddress, (UInt8*)(&(entryPackage.weight)), sizeof(entryPackage.weight));
			dataAddress += sizeof(entryPackage.weight);

			// AC0
			entryPackage.ac0MaxFanSpeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.ac0MaxFanSpeed.integer.value = artEntry->ac(0);
			packageBuffer.put(
				dataAddress, (UInt8*)(&(entryPackage.ac0MaxFanSpeed)), sizeof(entryPackage.ac0MaxFanSpeed));
			dataAddress += sizeof(entryPackage.ac0MaxFanSpeed);

			// AC1
			entryPackage.ac1MaxFanSpeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.ac1MaxFanSpeed.integer.value = artEntry->ac(1);
			packageBuffer.put(
				dataAddress, (UInt8*)(&(entryPackage.ac1MaxFanSpeed)), sizeof(entryPackage.ac1MaxFanSpeed));
			dataAddress += sizeof(entryPackage.ac1MaxFanSpeed);

			// AC2
			entryPackage.ac2MaxFanSpeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.ac2MaxFanSpeed.integer.value = artEntry->ac(2);
			packageBuffer.put(
				dataAddress, (UInt8*)(&(entryPackage.ac2MaxFanSpeed)), sizeof(entryPackage.ac2MaxFanSpeed));
			dataAddress += sizeof(entryPackage.ac2MaxFanSpeed);

			// AC3
			entryPackage.ac3MaxFanSpeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.ac3MaxFanSpeed.integer.value = artEntry->ac(3);
			packageBuffer.put(
				dataAddress, (UInt8*)(&(entryPackage.ac3MaxFanSpeed)), sizeof(entryPackage.ac3MaxFanSpeed));
			dataAddress += sizeof(entryPackage.ac3MaxFanSpeed);

			// AC4
			entryPackage.ac4MaxFanSpeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.ac4MaxFanSpeed.integer.value = artEntry->ac(4);
			packageBuffer.put(
				dataAddress, (UInt8*)(&(entryPackage.ac4MaxFanSpeed)), sizeof(entryPackage.ac4MaxFanSpeed));
			dataAddress += sizeof(entryPackage.ac4MaxFanSpeed);

			// AC5
			entryPackage.ac5MaxFanSpeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.ac5MaxFanSpeed.integer.value = artEntry->ac(5);
			packageBuffer.put(
				dataAddress, (UInt8*)(&(entryPackage.ac5MaxFanSpeed)), sizeof(entryPackage.ac5MaxFanSpeed));
			dataAddress += sizeof(entryPackage.ac5MaxFanSpeed);

			// AC6
			entryPackage.ac6MaxFanSpeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.ac6MaxFanSpeed.integer.value = artEntry->ac(6);
			packageBuffer.put(
				dataAddress, (UInt8*)(&(entryPackage.ac6MaxFanSpeed)), sizeof(entryPackage.ac6MaxFanSpeed));
			dataAddress += sizeof(entryPackage.ac6MaxFanSpeed);

			// AC7
			entryPackage.ac7MaxFanSpeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.ac7MaxFanSpeed.integer.value = artEntry->ac(7);
			packageBuffer.put(
				dataAddress, (UInt8*)(&(entryPackage.ac7MaxFanSpeed)), sizeof(entryPackage.ac7MaxFanSpeed));
			dataAddress += sizeof(entryPackage.ac7MaxFanSpeed);

			// AC8
			entryPackage.ac8MaxFanSpeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.ac8MaxFanSpeed.integer.value = artEntry->ac(8);
			packageBuffer.put(
				dataAddress, (UInt8*)(&(entryPackage.ac8MaxFanSpeed)), sizeof(entryPackage.ac8MaxFanSpeed));
			dataAddress += sizeof(entryPackage.ac8MaxFanSpeed);

			// AC9
			entryPackage.ac9MaxFanSpeed.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.ac9MaxFanSpeed.integer.value = artEntry->ac(9);
			packageBuffer.put(
				dataAddress, (UInt8*)(&(entryPackage.ac9MaxFanSpeed)), sizeof(entryPackage.ac9MaxFanSpeed));
			dataAddress += sizeof(entryPackage.ac9MaxFanSpeed);

			packages.put(offset, packageBuffer.get(), packageBuffer.size());
			offset += packageBuffer.size();
		}
	}

	UInt32 sizeOfRevision = (UInt32)sizeof(revisionField);
	DptfBuffer buffer(sizeOfRevision + packages.size());
	buffer.put(0, (UInt8*)&revisionField, sizeOfRevision);
	buffer.put(sizeOfRevision, packages.get(), packages.size());
	return buffer;
}
