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
#pragma once

#include "Dptf.h"
#include "DptfBuffer.h"
#include "ConditionType.h"
#include "XmlNode.h"

class dptf_export SwOemVariables
{
public:
	SwOemVariables();
	SwOemVariables(const std::map<UInt32, UInt32>& variables);
	virtual ~SwOemVariables() = default;

	SwOemVariables(const SwOemVariables& other) = default;
	SwOemVariables& operator=(const SwOemVariables& other) = default;
	SwOemVariables(SwOemVariables&& other) = default;
	SwOemVariables& operator=(SwOemVariables&& other) = default;

	std::map<UInt32, UInt32> getSwOemVariables() const;
	void add(const SwOemVariables& newSwOemVariables);
	static SwOemVariables createFromAppBroadcastData(const DptfBuffer& swOemVariablesData);
	static SwOemVariables createFromTableObjectData(const DptfBuffer& swOemVariablesData);
	static ConditionType::Type getConditionForVariable(UInt32 variableId);
	static ConditionType::Type getConditionForNewSwOemVariable(UInt32 variableId);
	UIntN getNumberOfVariables() const;
	UInt32 getValueForVariableId(UInt32 variableId) const;
	Bool operator==(const SwOemVariables& swOemVariables) const;
	DptfBuffer toSwOemVariablesBinary() const;
	DptfBuffer toSwOemVariablesDvBinary() const;
	static Bool isSwOemCondition(UInt64 condition);
	std::shared_ptr<XmlNode> getXml() const;

	static constexpr UInt32 DefaultSwOemVarCount = 6;

private:
	std::map<UInt32, UInt32> m_variables;
	static UInt64 extractUInt64FromVariant(const DptfBuffer& buffer, UInt32& bufferOffset);
	static UInt32 extractUInt32FromBroadcastData(const DptfBuffer& buffer, UInt32& bufferOffset);
	static Bool isRevisionSupported(UInt64 revision);
};

