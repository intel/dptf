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

#pragma once

#include "Dptf.h"
#include "DptfMessage.h"
#include "DptfManagerInterface.h"

class ManagerMessage : public DptfMessage
{
public:
	ManagerMessage(
		const DptfManagerInterface* dptfManager,
		const std::string& fileName,
		UIntN lineNumber,
		const std::string& executingFunctionName);
	ManagerMessage(
		const DptfManagerInterface* dptfManager,
		const std::string& fileName,
		UIntN lineNumber,
		const std::string& executingFunctionName,
		const std::string& message);
	ManagerMessage(const DptfManagerInterface* dptfManager, const DptfMessage& dptfMessage);
	~ManagerMessage() override = default;

	void setFrameworkEvent(FrameworkEvent::Type frameworkEvent);
	void setParticipantIndex(UIntN participantIndex);
	void setParticipantAndDomainIndex(UIntN participantIndex, UIntN domainIndex);
	void setPolicyIndex(UIntN policyIndex);
	void setEsifPrimitive(esif_primitive_type primitive, UInt32 instance);
	void setEsifEventGuid(const Guid& esifEventGuid);
	void setEsifErrorCode(eEsifError esifErrorCode);

	// Allows ManagerMessage to be used anywhere a std::string is required
	operator std::string() const;

private:
	const DptfManagerInterface* m_dptfManager;

	mutable Bool m_outputMessageStringCreated;
	mutable std::string m_outputMessageString;

	static constexpr UIntN KeyLength = 22;

	static std::string getFrameworkEventString(FrameworkEvent::Type frameworkEvent);
	std::string getParticipantName(UIntN participantIndex) const;
	std::string getDomainName(UIntN participantIndex, UIntN domainIndex) const;
	std::string getPolicyName(UIntN policyIndex) const;
	static std::string getEsifPrimitiveName(esif_primitive_type primitive);
	static std::string getEsifErrorCodeString(eEsifError esifErrorCode);

	static std::string createStandardizedKey(const std::string& key);
	std::string createStandardizedIndex(UIntN index) const;
	std::string numToString(UInt64 number) const;
};
