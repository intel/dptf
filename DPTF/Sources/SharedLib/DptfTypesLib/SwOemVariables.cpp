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

using namespace StatusFormat;

#define SW_OEM_VARIABLES_REVISION 1

SwOemVariables::SwOemVariables()
	: m_variables()
{
}

SwOemVariables::SwOemVariables(const std::map<UInt32, UInt32>& variables)
{
	m_variables = variables;
}

SwOemVariables::~SwOemVariables()
{
}

UIntN SwOemVariables::getNumberOfVariables(void) const
{
	return (UIntN)m_variables.size();
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
		throw dptf_exception("No value identified for SW OEM variable " + std::to_string(variableId));
	}
}

Bool SwOemVariables::operator==(const SwOemVariables& swOemVariables) const
{
	return (m_variables == swOemVariables.m_variables);
}

DptfBuffer SwOemVariables::toSwOemVariablesBinary() const
{
	DptfBuffer swOemVariablesBuffer;
	auto bufferSize = UInt32(sizeof(UInt32) * getNumberOfVariables());
	swOemVariablesBuffer.allocate(bufferSize);
	UInt32 dataAddress = 0;

	for (auto variable = m_variables.begin(); variable != m_variables.end(); variable++)
	{
		UInt32 variableId = variable->first;
		swOemVariablesBuffer.put(dataAddress, (UInt8*)(&variableId), sizeof(variableId));
		dataAddress += sizeof(variableId);

		UInt32 variableValue = variable->second;
		swOemVariablesBuffer.put(dataAddress, (UInt8*)(&variableValue), sizeof(variableValue));
		dataAddress += sizeof(variableValue);
	}

	return swOemVariablesBuffer;
}

DptfBuffer SwOemVariables::toSwOemVariablesDvBinary() const
{
	esif_data_variant revisionField;
	revisionField.integer.type = esif_data_type::ESIF_DATA_UINT64;
	revisionField.integer.value = SW_OEM_VARIABLES_REVISION;
	DptfBuffer swOemVariablesBuffer;

	auto bufferSize = UInt32(sizeof(esif_data_variant) * getNumberOfVariables());
	swOemVariablesBuffer.allocate(bufferSize);
	UInt32 dataAddress = 0;

	for (auto variable = m_variables.begin(); variable != m_variables.end(); variable++)
	{
		esif_data_variant variableField;
		variableField.integer.type = esif_data_type::ESIF_DATA_UINT64;
		variableField.integer.value = variable->first;
		swOemVariablesBuffer.put(dataAddress, (UInt8*)(&variableField), sizeof(variableField));
		dataAddress += sizeof(variableField);

		variableField.integer.type = esif_data_type::ESIF_DATA_UINT64;
		variableField.integer.value = variable->second;
		swOemVariablesBuffer.put(dataAddress, (UInt8*)(&variableField), sizeof(variableField));
		dataAddress += sizeof(variableField);
	}
	UInt32 sizeOfRevision = (UInt32)sizeof(revisionField);
	DptfBuffer buffer(sizeOfRevision + swOemVariablesBuffer.size());
	buffer.put(0, (UInt8*)&revisionField, sizeOfRevision);
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
	auto newSwOemVarId = variableId - DefaultSwOemVarCount;
	auto conditionId = (UInt32)(ConditionType::SwOemConditionBaseId);
	return (ConditionType::Type)(conditionId + newSwOemVarId);
}

std::shared_ptr<XmlNode> SwOemVariables::getXml(void) const
{
	auto status = XmlNode::createWrapperElement("sw_oem_variables");
	for (auto variable = m_variables.begin(); variable != m_variables.end(); ++variable)
	{
		auto swOemVariablesEntry = XmlNode::createWrapperElement("sw_oem_variables_entry");
		swOemVariablesEntry->addChild(XmlNode::createDataElement("variable_id", friendlyValue(variable->first)));
		swOemVariablesEntry->addChild(XmlNode::createDataElement("variable", friendlyValue(variable->second)));
		status->addChild(swOemVariablesEntry);
	}
	
	return status;
}

void SwOemVariables::add(const SwOemVariables& newSwOemVariables)
{
	for (auto newVars = newSwOemVariables.m_variables.begin(); newVars != newSwOemVariables.m_variables.end();
		 newVars++)
	{
		m_variables[newVars->first] = newVars->second;
	}
}

SwOemVariables SwOemVariables::createFromAppBroadcastData(const DptfBuffer& swOemVariablesData) 
{
	std::map<UInt32, UInt32> variables;

	if (swOemVariablesData.size() == 0)
	{
		return SwOemVariables(variables);
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
		throw dptf_exception("Improper SW OEM Variables format from appBroadcast");
	}

	return SwOemVariables(variables);
}

SwOemVariables SwOemVariables::createFromTableObjectData(const DptfBuffer& swOemVariablesData)
{
	std::map<UInt32, UInt32> variables;

	try
	{
		UInt32 bufferOffset = 0;
		UInt64 revision = extractUInt64FromVariant(swOemVariablesData, bufferOffset);
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

	return SwOemVariables(variables);
}

UInt64 SwOemVariables::extractUInt64FromVariant(const DptfBuffer& buffer, UInt32& bufferOffset) 
{
	UInt32 remainingBuffer = (buffer.size() - bufferOffset);
	if (bufferOffset > buffer.size() || remainingBuffer < sizeof(esif_data_variant))
	{
		throw dptf_exception("Buffer boundary reached");
	}

	UInt8* data = buffer.get();
	data += bufferOffset;
	esif_data_variant* entry = reinterpret_cast<esif_data_variant*>(data);
	if (entry->type != ESIF_DATA_UINT64) 
	{
		throw dptf_exception("Unexpected data type");
	}

	bufferOffset += sizeof(esif_data_variant);
	return entry->integer.value;
}

UInt32 SwOemVariables::extractUInt32FromBroadcastData(const DptfBuffer& buffer, UInt32& bufferOffset)
{
	UInt32 remainingBuffer = (buffer.size() - bufferOffset);
	if (bufferOffset > buffer.size() || remainingBuffer < sizeof(UInt32))
	{
		throw dptf_exception("Buffer boundary reached");
	}

	UInt8* data = buffer.get();
	data += bufferOffset;
	UInt32* value = reinterpret_cast<UInt32*>(data);
	bufferOffset += sizeof(UInt32);
	
	return *value;
}

Bool SwOemVariables::isRevisionSupported(UInt64 revision)
{
	return (revision <= SW_OEM_VARIABLES_REVISION);
}