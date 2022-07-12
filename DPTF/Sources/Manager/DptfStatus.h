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

#pragma once

#include "Dptf.h"
#include "DptfStatusInterface.h"
#include "ControlFactoryType.h"

class XmlNode;
class Indent;
class Participant;
class ParticipantStatusMap;

class DptfStatus : public DptfStatusInterface
{
public:
	DptfStatus(DptfManagerInterface* dptfManager);
	~DptfStatus();

	virtual std::pair<std::string, eEsifError> getStatus(const eAppStatusCommand command, const UInt32 appStatusIn)
		override;
	virtual void clearCache() override;

private:
	DptfManagerInterface* m_dptfManager;
	PolicyManagerInterface* m_policyManager;
	ParticipantManagerInterface* m_participantManager;
	ParticipantStatusMap* m_participantStatusMap;

	std::string getGroupsXml(eEsifError* returnCode);
	std::string getModulesInGroup(const UInt32 appStatusIn, eEsifError* returnCode);
	std::string getPoliciesGroup();
	std::string getParticipantsGroup();
	std::string getFrameworkGroup();
	std::string getArbitratorGroup();
	std::string getSystemGroup();
	std::shared_ptr<XmlNode> getArbitratorModuleInGroup(ControlFactoryType::Type type);
	std::string getModuleData(const UInt32 appStatusIn, eEsifError* returnCode);
	std::string getXmlForPolicy(UInt32 policyIndex, eEsifError* returnCode);
	std::string getXmlForParticipant(UInt32 mappedIndex, eEsifError* returnCode);
	std::string getXmlForFramework(UInt32 moduleIndex, eEsifError* returnCode);
	std::string getXmlForArbitrator(UInt32 moduleIndex, eEsifError* returnCode);
	std::string getXmlForSystem(UInt32 moduleIndex, eEsifError* returnCode);
	std::shared_ptr<XmlNode> getXmlForFrameworkLoadedPolicies();
	std::shared_ptr<XmlNode> getXmlForFrameworkLoadedParticipants();
	std::shared_ptr<XmlNode> getArbitratorXmlForLoadedParticipants(UInt32 moduleIndex);
	std::shared_ptr<XmlNode> getXmlForPlatformRequests();
	void checkAndDropSymLink(std::string fileName);

	void fillEsifString(EsifDataPtr outputLocation, std::string inputString, eEsifError* returnCode);
	UIntN getNumberOfUniqueDomains(std::set<UIntN> participantIndexList);

	// KW error resolution
	DptfStatus(const DptfStatus&);

	DptfStatus& operator=(const DptfStatus&)
	{
		return *this;
	}
};
