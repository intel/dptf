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

#include "PowerControlDynamicCapsSet.h"
#include "XmlNode.h"
#include "EsifDataBinaryPpccPackage.h"

using namespace std;

PowerControlDynamicCapsSet::PowerControlDynamicCapsSet(
	const std::vector<PowerControlDynamicCaps>& powerControlDynamicCaps)
	: m_capabilities(std::map<PowerControlType::Type, PowerControlDynamicCaps>())
{
	for (auto capability = powerControlDynamicCaps.begin(); capability != powerControlDynamicCaps.end(); capability++)
	{
		m_capabilities[capability->getPowerControlType()] = *capability;
	}
}

PowerControlDynamicCapsSet::PowerControlDynamicCapsSet()
	: m_capabilities(std::map<PowerControlType::Type, PowerControlDynamicCaps>())
{
}

PowerControlDynamicCapsSet::~PowerControlDynamicCapsSet()
{
}

PowerControlDynamicCapsSet PowerControlDynamicCapsSet::createFromPpcc(const DptfBuffer& buffer, Power pl4PowerLimit)
{
	std::vector<PowerControlDynamicCaps> controls;
	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	data += sizeof(esif_data_variant); // Ignore revision field
	struct EsifDataBinaryPpccPackage* currentRow = reinterpret_cast<struct EsifDataBinaryPpccPackage*>(data);
	PowerControlDynamicCaps temp;

	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty PPCC buffer.");
	}

	UIntN rows = (buffer.size() - sizeof(esif_data_variant)) / sizeof(EsifDataBinaryPpccPackage);

	if ((buffer.size() - sizeof(esif_data_variant)) % sizeof(EsifDataBinaryPpccPackage))
	{
		throw dptf_exception("Expected binary data size mismatch. (PPCC)");
	}

	UIntN originalRowCount = rows;
	if (originalRowCount != 4)
	{
		rows = 4;
	}

	for (UIntN i = 0; i < rows; i++)
	{
		auto needDefaultPl2Row = originalRowCount < 2 && i == 1;
		auto needDefaultPl3Row = originalRowCount < 3 && i == 2;
		auto needDefaultPl4Row = originalRowCount < 4 && i == 3;
		
		if (needDefaultPl2Row)
		{
			temp = temp.getDefaultPpccRowValues(PowerControlType::Type::PL2);
		}
		else if (needDefaultPl3Row)
		{
			temp = temp.getDefaultPpccRowValues(PowerControlType::Type::PL3);
		}
		else if (needDefaultPl4Row)
		{
			temp = temp.getDefaultPpccPl4RowValues(pl4PowerLimit);
		}
		else
		{
			temp = temp.getPpccPlRowValues(currentRow);
		}

		if (controls.empty())
		{
			controls.push_back(temp);
		}
		else if (controls.front().getPowerControlType() < temp.getPowerControlType())
		{
			controls.push_back(temp);
		}
		else
		{
			controls.insert(controls.begin(), temp);
		}

		data += sizeof(struct EsifDataBinaryPpccPackage);
		currentRow = reinterpret_cast<struct EsifDataBinaryPpccPackage*>(data);
	}

	return PowerControlDynamicCapsSet(controls);
}

UIntN PowerControlDynamicCapsSet::getPpccDataRows(const DptfBuffer& buffer)
{
	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty PPCC buffer.");
	}

	return (buffer.size() - sizeof(esif_data_variant)) / sizeof(EsifDataBinaryPpccPackage);
}

UInt64 PowerControlDynamicCapsSet::getPpccDataRevision(const DptfBuffer& buffer)
{
	union esif_data_variant* obj = (union esif_data_variant*)buffer.get();

	if (buffer.size() == 0)
	{
		throw dptf_exception("Received empty PPCC buffer.");
	}

	return (UInt64)obj->integer.value;
}

Bool PowerControlDynamicCapsSet::isEmpty() const
{
	return (m_capabilities.size() == 0);
}

Bool PowerControlDynamicCapsSet::hasCapability(PowerControlType::Type controlType) const
{
	auto capability = m_capabilities.find(controlType);
	return (capability != m_capabilities.end());
}

const PowerControlDynamicCaps& PowerControlDynamicCapsSet::getCapability(PowerControlType::Type controlType) const
{
	auto capability = m_capabilities.find(controlType);
	if (capability != m_capabilities.end())
	{
		return capability->second;
	}
	else
	{
		throw dptf_exception("No power control capabilities for " + PowerControlType::ToString(controlType));
	}
}

void PowerControlDynamicCapsSet::setCapability(const PowerControlDynamicCaps& capability)
{
	m_capabilities[capability.getPowerControlType()] = capability;
}

std::set<PowerControlType::Type> PowerControlDynamicCapsSet::getControlTypes() const
{
	std::set<PowerControlType::Type> capabilityTypes;
	for (auto capability = m_capabilities.begin(); capability != m_capabilities.end(); capability++)
	{
		capabilityTypes.insert(capability->first);
	}
	return capabilityTypes;
}

Power PowerControlDynamicCapsSet::snapToCapability(PowerControlType::Type controlType, Power powerValue) const
{
	if (hasCapability(controlType))
	{
		auto capability = getCapability(controlType);
		powerValue = std::min(powerValue, capability.getMaxPowerLimit());
		powerValue = std::max(powerValue, capability.getMinPowerLimit());
	}
	return powerValue;
}

TimeSpan PowerControlDynamicCapsSet::snapToCapability(PowerControlType::Type controlType, TimeSpan timeValue) const
{
	if (hasCapability(controlType))
	{
		auto capability = getCapability(controlType);
		timeValue = std::min(timeValue, capability.getMaxTimeWindow());
		timeValue = std::max(timeValue, capability.getMinTimeWindow());
	}
	return timeValue;
}

DptfBuffer PowerControlDynamicCapsSet::toPpccBinary() const
{
	esif_data_variant revision;
	revision.integer.type = esif_data_type::ESIF_DATA_UINT64;
	revision.integer.value = 2;

	vector<EsifDataBinaryPpccPackage> packages;
	for (auto cap = m_capabilities.begin(); cap != m_capabilities.end(); ++cap)
	{
		EsifDataBinaryPpccPackage package;
		package.powerLimitIndex.integer.type = esif_data_type::ESIF_DATA_UINT64;
		package.powerLimitIndex.integer.value = cap->second.getPowerControlType();
		package.powerLimitMaximum.integer.type = esif_data_type::ESIF_DATA_UINT64;
		
		auto maxPowerLimit = cap->second.getMaxPowerLimit();
		if (maxPowerLimit.isValid())
		{
			package.powerLimitMaximum.integer.value = maxPowerLimit;
		}
		else
		{
			package.powerLimitMaximum.integer.value = Constants::Invalid;
		}

		package.powerLimitMinimum.integer.type = esif_data_type::ESIF_DATA_UINT64;

		auto minPowerLimit = cap->second.getMinPowerLimit();
		if (minPowerLimit.isValid())
		{
			package.powerLimitMinimum.integer.value = minPowerLimit;
		}
		else
		{
			package.powerLimitMinimum.integer.value = Constants::Invalid;
		}

		package.timeWindowMaximum.integer.type = esif_data_type::ESIF_DATA_UINT64;

		auto maxTimeWindow = cap->second.getMaxTimeWindow();
		if (maxTimeWindow.isValid())
		{
			package.timeWindowMaximum.integer.value = maxTimeWindow.asMillisecondsUInt();
		}
		else
		{
			package.timeWindowMaximum.integer.value = Constants::Invalid;
		}

		package.timeWindowMinimum.integer.type = esif_data_type::ESIF_DATA_UINT64;

		auto minTimeWindow = cap->second.getMinTimeWindow();
		if (minTimeWindow.isValid())
		{
			package.timeWindowMinimum.integer.value = minTimeWindow.asMillisecondsUInt();
		}
		else
		{
			package.timeWindowMinimum.integer.value = Constants::Invalid;
		}

		package.stepSize.integer.type = esif_data_type::ESIF_DATA_UINT64;

		auto powerStepSize = cap->second.getPowerStepSize();
		if (powerStepSize.isValid())
		{
			package.stepSize.integer.value = powerStepSize;
		}
		else
		{
			package.stepSize.integer.value = Constants::Invalid;
		}

		packages.push_back(package);
	}

	UInt32 sizeOfRevision = (UInt32)sizeof(revision);
	UInt32 sizeOfPackages = (UInt32)packages.size() * sizeof(EsifDataBinaryPpccPackage);
	DptfBuffer buffer(sizeOfRevision + sizeOfPackages);
	buffer.put(0, (UInt8*)&revision, sizeOfRevision);
	buffer.put(sizeOfRevision, (UInt8*)packages.data(), sizeOfPackages);
	return buffer;
}

Bool PowerControlDynamicCapsSet::operator==(const PowerControlDynamicCapsSet& rhs) const
{
	return (m_capabilities == rhs.m_capabilities);
}

Bool PowerControlDynamicCapsSet::operator!=(const PowerControlDynamicCapsSet& rhs) const
{
	return !(*this == rhs);
}

std::shared_ptr<XmlNode> PowerControlDynamicCapsSet::getXml(void) const
{
	auto root = XmlNode::createWrapperElement("power_control_dynamic_caps_set");
	for (auto capability = m_capabilities.begin(); capability != m_capabilities.end(); capability++)
	{
		root->addChild(capability->second.getXml());
	}
	return root;
}