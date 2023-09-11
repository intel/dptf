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

#include "SwOemVariables.h"
#include "esif_sdk_data.h"
#include "StatusFormat.h"
#include "StringConverter.h"
#include "XmlNode.h"

using namespace std;
using namespace StatusFormat;

enum {SW_OEM_VARIABLES_REVISION = 1};

SwOemVariables::SwOemVariables()
	: m_variables()
{
}

SwOemVariables::SwOemVariables(const std::map<UInt32, UInt32>& variables)
{
	m_variables = variables;
}

UIntN SwOemVariables::getNumberOfVariables(void) const
{
	return static_cast<UIntN>(m_variables.size());
}

std::map<UInt32, UInt32> SwOemVariables::getSwOemVariables(void) const
{
	return m_variables;
}

UInt32 SwOemVariables::getValueForVariableId(UInt32 variableId) const
{
	try
	{
		return m_variables.at(variableId);
	}
	catch (...) 
	{
		throw dptf_exception("No value identified for SW OEM variable "s + std::to_string(variableId));
	}
}

Bool SwOemVariables::operator==(const SwOemVariables& swOemVariables) const
{
	return (m_variables == swOemVariables.m_variables);
}

DptfBuffer SwOemVariables::toSwOemVariablesBinary() const
{
	DptfBuffer swOemVariablesBuffer;
	const auto bufferSize = static_cast<UInt32>(sizeof(UInt32) * getNumberOfVariables());
	swOemVariablesBuffer.allocate(bufferSize);
	UInt32 dataAddress = 0;

	for (const auto& [id, value] : m_variables)
	{
		swOemVariablesBuffer.put(dataAddress, reinterpret_cast<const UInt8*>(&id), sizeof(id));
		dataAddress += sizeof(id);

		swOemVariablesBuffer.put(dataAddress, reinterpret_cast<const UInt8*>(&value), sizeof(value));
		dataAddress += sizeof(value);
	}

	return swOemVariablesBuffer;
}

DptfBuffer SwOemVariables::toSwOemVariablesDvBinary() const
{
	esif_data_variant revisionField{};
	revisionField.integer.type = esif_data_type::ESIF_DATA_UINT64;
	revisionField.integer.value = SW_OEM_VARIABLES_REVISION;
	DptfBuffer swOemVariablesBuffer;

	const auto bufferSize = static_cast<UInt32>(sizeof(esif_data_variant) * getNumberOfVariables());
	swOemVariablesBuffer.allocate(bufferSize);
	UInt32 dataAddress = 0;

	for (const auto& [id, value] : m_variables)
	{
		esif_data_variant variableField{};
		variableField.integer.type = esif_data_type::ESIF_DATA_UINT64;
		variableField.integer.value = id;
		swOemVariablesBuffer.put(dataAddress, reinterpret_cast<UInt8*>(&variableField), sizeof(variableField));
		dataAddress += sizeof(variableField);

		variableField.integer.type = esif_data_type::ESIF_DATA_UINT64;
		variableField.integer.value = value;
		swOemVariablesBuffer.put(dataAddress, reinterpret_cast<UInt8*>(&variableField), sizeof(variableField));
		dataAddress += sizeof(variableField);
	}
	constexpr auto sizeOfRevision = static_cast<UInt32>(sizeof(revisionField));
	DptfBuffer buffer(sizeOfRevision + swOemVariablesBuffer.size());
	buffer.put(0, reinterpret_cast<UInt8*>(&revisionField), sizeOfRevision);
	buffer.put(sizeOfRevision, swOemVariablesBuffer.get(), swOemVariablesBuffer.size());

	return buffer;
}

Bool SwOemVariables::isSwOemCondition(UInt64 condition)
{
	if ((condition >= ConditionType::SwOemVariable0 && condition <= ConditionType::SwOemVariable5)
		|| (condition >= ConditionType::SwOemConditionBaseId
			&& condition < ConditionType::ParticipantConditionBaseId))
	{
		return true;
	}

	return false;
}

ConditionType::Type SwOemVariables::getConditionForVariable(UInt32 variableId)
{
	switch (variableId)
	{
	case 0:
		return ConditionType::SwOemVariable0;
	case 1:
		return ConditionType::SwOemVariable1;
	case 2:
		return ConditionType::SwOemVariable2;
	case 3:
		return ConditionType::SwOemVariable3;
	case 4:
		return ConditionType::SwOemVariable4;
	case 5:
		return ConditionType::SwOemVariable5;
	default:
		return getConditionForNewSwOemVariable(variableId);
	}
}

ConditionType::Type SwOemVariables::getConditionForNewSwOemVariable(UInt32 variableId)
{
	const auto newSwOemVarId = variableId - DefaultSwOemVarCount;
	constexpr auto conditionId = static_cast<UInt32>(ConditionType::SwOemConditionBaseId);
	return static_cast<ConditionType::Type>(conditionId + newSwOemVarId);
}

std::shared_ptr<XmlNode> SwOemVariables::getXml() const
{
	auto status = XmlNode::createWrapperElement("sw_oem_variables"s);
	for (const auto& [id, value] : m_variables)
	{
		const auto swOemVariablesEntry = XmlNode::createWrapperElement("sw_oem_variables_entry"s);
		swOemVariablesEntry->addChild(XmlNode::createDataElement("variable_id"s, friendlyValue(id)));
		swOemVariablesEntry->addChild(XmlNode::createDataElement("variable"s, friendlyValue(value)));
		status->addChild(swOemVariablesEntry);
	}
	
	return status;
}

void SwOemVariables::add(const SwOemVariables& newSwOemVariables)
{
	for (const auto& [id, value] : newSwOemVariables.m_variables)
	{
		m_variables[id] = value;
	}
}

SwOemVariables SwOemVariables::createFromAppBroadcastData(const DptfBuffer& swOemVariablesData) 
{
	std::map<UInt32, UInt32> variables;

	if (swOemVariablesData.size() == 0)
	{
		return {variables};
	}
	
	try
	{
		UInt32 bufferOffset = 0;
		while (bufferOffset < swOemVariablesData.size())
		{
			const auto id = extractUInt32FromBroadcastData(swOemVariablesData, bufferOffset);
			const auto value = extractUInt32FromBroadcastData(swOemVariablesData, bufferOffset);
			variables[id] = value;
		}
	}
	catch (...)
	{
		throw dptf_exception("Improper SW OEM Variables format from appBroadcast"s);
	}

	return {variables};
}

SwOemVariables SwOemVariables::createFromTableObjectData(const DptfBuffer& swOemVariablesData)
{
	std::map<UInt32, UInt32> variables;

	try
	{
		UInt32 bufferOffset = 0;
		const UInt64 revision = extractUInt64FromVariant(swOemVariablesData, bufferOffset);
		if (!isRevisionSupported(revision))
		{
			throw dptf_exception("Unsupported SW OEM Variables revision");
		}

		while (bufferOffset < swOemVariablesData.size()) 
		{
			const auto id = static_cast<UInt32>(extractUInt64FromVariant(swOemVariablesData, bufferOffset));
			const auto value = static_cast<UInt32>(extractUInt64FromVariant(swOemVariablesData, bufferOffset));
			variables[id] = value;
		}
	}
	catch (...)
	{
		variables.clear();
	}

	return {variables};
}

UInt64 SwOemVariables::extractUInt64FromVariant(const DptfBuffer& buffer, UInt32& bufferOffset) 
{
	const auto remainingBuffer = (buffer.size() - bufferOffset);
	if (bufferOffset > buffer.size() || remainingBuffer < sizeof(esif_data_variant))
	{
		throw dptf_exception("Buffer boundary reached");
	}

	auto data = buffer.get();
	data += bufferOffset;
	const esif_data_variant* entry = reinterpret_cast<esif_data_variant*>(data);
	if (entry->type != ESIF_DATA_UINT64) 
	{
		throw dptf_exception("Unexpected data type");
	}

	bufferOffset += sizeof(esif_data_variant);
	return entry->integer.value;
}

UInt32 SwOemVariables::extractUInt32FromBroadcastData(const DptfBuffer& buffer, UInt32& bufferOffset)
{
	const auto remainingBuffer = (buffer.size() - bufferOffset);
	if (bufferOffset > buffer.size() || remainingBuffer < sizeof(UInt32))
	{
		throw dptf_exception("Buffer boundary reached");
	}

	auto data = buffer.get();
	data += bufferOffset;
	const UInt32* value = reinterpret_cast<UInt32*>(data);
	bufferOffset += sizeof(UInt32);
	
	return *value;
}

Bool SwOemVariables::isRevisionSupported(UInt64 revision)
{
	return (revision <= SW_OEM_VARIABLES_REVISION);
}