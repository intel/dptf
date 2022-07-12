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

#include "ThermalRelationshipTable.h"
#include "EsifDataBinaryTrtPackage.h"
#include "BinaryParse.h"
#include "TableStringParser.h"

ThermalRelationshipTable::ThermalRelationshipTable(
	const std::vector<std::shared_ptr<RelationshipTableEntryBase>>& entries)
	: RelationshipTableBase(entries)
{
}

ThermalRelationshipTable::ThermalRelationshipTable()
	: RelationshipTableBase()
{
}

ThermalRelationshipTable::~ThermalRelationshipTable()
{
}

ThermalRelationshipTable ThermalRelationshipTable::createTrtFromDptfBuffer(const DptfBuffer& buffer)
{
	std::vector<std::shared_ptr<RelationshipTableEntryBase>> entries;
	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct EsifDataBinaryTrtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

	if (buffer.size() != 0)
	{
		UIntN rows = countTrtRows(buffer.size(), data);

		// Reset currentRow to point to the beginning of the data block
		data = reinterpret_cast<UInt8*>(buffer.get());
		currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

		for (UIntN i = 0; i < rows; i++)
		{
			// Since the TRT has 2 strings in it, the process for extracting them is:
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
			currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

			std::string target = TableStringParser::getDeviceString((currentRow->targetDevice));

			data += currentRow->targetDevice.string.length;
			currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

			auto newTrtEntry = std::make_shared<ThermalRelationshipTableEntry>(
				BinaryParse::normalizeAcpiScope(source),
				BinaryParse::normalizeAcpiScope(target),
				static_cast<UInt32>(currentRow->thermalInfluence.integer.value),
				TimeSpan::createFromTenthSeconds(static_cast<UInt32>(currentRow->thermalSamplingPeriod.integer.value)));

			if (newTrtEntry)
			{
				// Check for duplicate entries. Don't add entry if previous entry exists with same target/source pair
				Bool isDuplicateEntry = false;
				for (auto e = entries.begin(); e != entries.end(); e++)
				{
					auto trtEntry = std::dynamic_pointer_cast<ThermalRelationshipTableEntry>(*e);
					if (trtEntry && newTrtEntry->isSameAs(*trtEntry))
					{
						isDuplicateEntry = true;
						break;
					}
				}

				if (isDuplicateEntry == false)
				{
					entries.push_back(newTrtEntry);
				}
			}

			// Since we've already accounted for the strings, we now move the pointer by the size of the structure
			//  to get to the next row.
			data += sizeof(struct EsifDataBinaryTrtPackage);
			currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);
		}
	}
	return ThermalRelationshipTable(entries);
}

std::vector<std::shared_ptr<ThermalRelationshipTableEntry>> ThermalRelationshipTable::getEntriesForTarget(
	UIntN targetIndex)
{
	std::vector<std::shared_ptr<ThermalRelationshipTableEntry>> entries;
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		if ((*entry)->getTargetDeviceIndex() == targetIndex)
		{
			auto trtEntry = std::dynamic_pointer_cast<ThermalRelationshipTableEntry>(*entry);
			if (trtEntry)
			{
				entries.push_back(trtEntry);
			}
		}
	}
	return entries;
}

TimeSpan ThermalRelationshipTable::getMinimumActiveSamplePeriodForSource(
	UIntN sourceIndex,
	std::set<UIntN> activeTargets)
{
	auto minimumSamplePeriod = TimeSpan::createInvalid();
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		if (((*entry)->getSourceDeviceIndex() == sourceIndex)
			&& (activeTargets.find((*entry)->getTargetDeviceIndex()) != activeTargets.end()))
		{
			auto trtEntry = std::dynamic_pointer_cast<ThermalRelationshipTableEntry>(*entry);
			if (trtEntry)
			{
				auto samplingPeriod = trtEntry->thermalSamplingPeriod();
				if (minimumSamplePeriod.isInvalid() || samplingPeriod < minimumSamplePeriod)
				{
					minimumSamplePeriod = samplingPeriod;
				}
			}
		}
	}
	return minimumSamplePeriod;
}

TimeSpan ThermalRelationshipTable::getShortestSamplePeriodForTarget(UIntN target)
{
	auto shortestSamplePeriod = TimeSpan::createInvalid();
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		if ((*entry)->getTargetDeviceIndex() == target)
		{
			auto trtEntry = std::dynamic_pointer_cast<ThermalRelationshipTableEntry>(*entry);
			if (trtEntry)
			{
				auto samplingPeriod = trtEntry->thermalSamplingPeriod();
				if (shortestSamplePeriod.isInvalid() || samplingPeriod < shortestSamplePeriod)
				{
					shortestSamplePeriod = samplingPeriod;
				}
			}
		}
	}

	return shortestSamplePeriod;
}

TimeSpan ThermalRelationshipTable::getSampleTimeForRelationship(UIntN target, UIntN source) const
{
	for (auto entry = m_entries.begin(); entry != m_entries.end(); ++entry)
	{
		if (((*entry)->getTargetDeviceIndex() == target) && ((*entry)->getSourceDeviceIndex() == source))
		{
			auto trtEntry = std::dynamic_pointer_cast<ThermalRelationshipTableEntry>(*entry);
			if (trtEntry)
			{
				return trtEntry->thermalSamplingPeriod();
			}
		}
	}
	throw dptf_exception("No match found for target and source in TRT.");
}

std::shared_ptr<XmlNode> ThermalRelationshipTable::getXml()
{
	auto status = XmlNode::createWrapperElement("trt");
	for (auto entry = m_entries.begin(); entry != m_entries.end(); entry++)
	{
		auto trtEntry = std::dynamic_pointer_cast<ThermalRelationshipTableEntry>(*entry);
		if (trtEntry)
		{
			status->addChild(trtEntry->getXml());
		}
	}
	return status;
}

UIntN ThermalRelationshipTable::countTrtRows(UInt32 size, UInt8* data)
{
	IntN bytesRemaining = size;
	UIntN rows = 0;

	struct EsifDataBinaryTrtPackage* currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

	while (bytesRemaining > 0)
	{
		bytesRemaining -= sizeof(struct EsifDataBinaryTrtPackage);
		throwIfOutOfRange(bytesRemaining);
		throwIfOutOfRange(currentRow->sourceDevice.string.length); // Verify string length not negative or will become positive when subtracted from bytesRemaining below
		bytesRemaining -= currentRow->sourceDevice.string.length;
		throwIfOutOfRange(bytesRemaining);

		data += currentRow->sourceDevice.string.length;
		currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

		throwIfOutOfRange(currentRow->targetDevice.string.length); // Verify string length not negative or will become
																   // positive when subtracted from bytesRemaining below
		bytesRemaining -= currentRow->targetDevice.string.length;
		throwIfOutOfRange(bytesRemaining);

		data += currentRow->targetDevice.string.length;
		currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);

		// The math done here will vary based on the number of strings in the BIOS object
		rows++;

		data += sizeof(struct EsifDataBinaryTrtPackage);
		currentRow = reinterpret_cast<struct EsifDataBinaryTrtPackage*>(data);
	}

	return rows;
}

Bool ThermalRelationshipTable::operator==(const ThermalRelationshipTable& trt) const
{
	if (getNumberOfEntries() != trt.getNumberOfEntries())
	{
		return false;
	}

	for (UIntN i = 0; i < getNumberOfEntries(); i++)
	{
		auto lhsTrtEntry = std::dynamic_pointer_cast<ThermalRelationshipTableEntry>(m_entries[i]);
		auto rhsTrtEntry = std::dynamic_pointer_cast<ThermalRelationshipTableEntry>(trt.m_entries[i]);
		if (!lhsTrtEntry || !rhsTrtEntry || !(*lhsTrtEntry == *rhsTrtEntry))
		{
			return false;
		}
	}

	return true;
}

Bool ThermalRelationshipTable::operator!=(const ThermalRelationshipTable& trt) const
{
	return !(*this == trt);
}

DptfBuffer ThermalRelationshipTable::toTrtBinary() const
{
	DptfBuffer packages;
	UInt32 offset = 0;
	for (auto entry = m_entries.begin(); entry != m_entries.end(); entry++)
	{
		auto trtEntry = std::dynamic_pointer_cast<ThermalRelationshipTableEntry>(*entry);
		if (trtEntry)
		{
			UInt32 sourceScopeLength = (UInt32)(*entry)->getSourceDeviceScope().size();
			UInt32 targetScopeLength = (UInt32)(*entry)->getTargetDeviceScope().size();

			DptfBuffer packageBuffer;
			packageBuffer.allocate(sizeof(EsifDataBinaryTrtPackage) + sourceScopeLength + targetScopeLength);

			EsifDataBinaryTrtPackage entryPackage;
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

			// Thermal Influence
			entryPackage.thermalInfluence.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.thermalInfluence.integer.value = trtEntry->thermalInfluence();
			packageBuffer.put(
				dataAddress, (UInt8*)(&(entryPackage.thermalInfluence)), sizeof(entryPackage.thermalInfluence));
			dataAddress += sizeof(entryPackage.thermalInfluence);

			// Sampling Period
			entryPackage.thermalSamplingPeriod.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.thermalSamplingPeriod.integer.value = trtEntry->thermalSamplingPeriod().asTenthSecondsInt();
			packageBuffer.put(
				dataAddress,
				(UInt8*)(&(entryPackage.thermalSamplingPeriod)),
				sizeof(entryPackage.thermalSamplingPeriod));
			dataAddress += sizeof(entryPackage.thermalSamplingPeriod);

			// Reserved1
			entryPackage.reserved1.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.reserved1.integer.value = 0;
			packageBuffer.put(dataAddress, (UInt8*)(&(entryPackage.reserved1)), sizeof(entryPackage.reserved1));
			dataAddress += sizeof(entryPackage.reserved1);

			// Reserved2
			entryPackage.reserved2.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.reserved2.integer.value = 0;
			packageBuffer.put(dataAddress, (UInt8*)(&(entryPackage.reserved2)), sizeof(entryPackage.reserved2));
			dataAddress += sizeof(entryPackage.reserved2);

			// Reserved3
			entryPackage.reserved3.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.reserved3.integer.value = 0;
			packageBuffer.put(dataAddress, (UInt8*)(&(entryPackage.reserved3)), sizeof(entryPackage.reserved3));
			dataAddress += sizeof(entryPackage.reserved3);

			// Reserved4
			entryPackage.reserved4.integer.type = esif_data_type::ESIF_DATA_UINT64;
			entryPackage.reserved4.integer.value = 0;
			packageBuffer.put(dataAddress, (UInt8*)(&(entryPackage.reserved4)), sizeof(entryPackage.reserved4));
			dataAddress += sizeof(entryPackage.reserved4);

			packages.put(offset, packageBuffer.get(), packageBuffer.size());
			offset += packageBuffer.size();
		}
	}

	DptfBuffer buffer(packages.size());
	buffer.put(0, packages.get(), packages.size());
	return buffer;
}

void ThermalRelationshipTable::throwIfOutOfRange(IntN bytesRemaining)
{
	if (bytesRemaining < 0)
	{
		throw dptf_exception("Expected binary data size mismatch. (TRT)");
	}
}
