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

#include "DptfStatus.h"
#include "Indent.h"
#include "esif_sdk_iface_app.h"
#include "Participant.h"
#include "Policy.h"
#include <fstream>
#include "PolicyManagerInterface.h"
#include "ParticipantManagerInterface.h"
#include "WorkItemQueueManagerInterface.h"
#include "BinaryParse.h"
#include "XmlNode.h"
#include "ParticipantStatusMap.h"
#include "EsifDataString.h"
#include <StatusFormat.h>
#include "PlatformRequestHandler.h"

// clang-format off
const Guid ManagerStatusFormatId(0x3E, 0x58, 0x63, 0x46, 0xF8, 0xF7, 0x45, 0x4A, 0xA8, 0xF7, 0xDE, 0x7E, 0xC6, 0xF7, 0x61, 0xA8);
// clang-format on

DptfStatus::DptfStatus(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
{
	m_policyManager = m_dptfManager->getPolicyManager();
	m_participantManager = m_dptfManager->getParticipantManager();
	m_participantStatusMap = new ParticipantStatusMap(m_participantManager);
}

DptfStatus::~DptfStatus()
{
	DELETE_MEMORY_TC(m_participantStatusMap);
}

namespace GroupType
{
	enum Type
	{
		Policies = 0,
		Participants = 1,
		Framework = 2,
		Arbitrator = 3,
		System = 4
	};
}

namespace ManagerModuleType
{
	enum Type
	{
		Events = 0,
		Manager = 1,
		Statistics = 2
	};

	std::string ToString(ManagerModuleType::Type type)
	{
		switch (type)
		{
		case Events:
			return "Event Status";
		case Manager:
			return "Manager Status";
		case Statistics:
			return "Work Item Statistics";
		default:
			return Constants::InvalidString;
		}
	}
}

namespace SystemModuleType
{
	enum Type
	{
		SystemConfiguration = 0
	};

	std::string ToString(SystemModuleType::Type type)
	{
		switch (type)
		{
		case SystemConfiguration:
			// it is important that this name matches the name in the "policy" cpp file
			return "System Configuration";
		default:
			return Constants::InvalidString;
		}
	}
}

std::pair<std::string, eEsifError> DptfStatus::getStatus(const eAppStatusCommand command, const UInt32 appStatusIn)
{
	std::string response;
	std::pair<std::string, eEsifError> statusResult;
	eEsifError returnCode = ESIF_OK;

	switch (command)
	{
	case eAppStatusCommandGetXSLT:
		response = getXsltContent(&returnCode);
		break;
	case eAppStatusCommandGetGroups:
		response = getGroupsXml(&returnCode);
		break;
	case eAppStatusCommandGetModulesInGroup:
		response = getModulesInGroup(appStatusIn, &returnCode);
		break;
	case eAppStatusCommandGetModuleData:
		response = getModuleData(appStatusIn, &returnCode);
		break;
	default:
		returnCode = ESIF_E_UNSPECIFIED;
		throw dptf_exception("Received invalid command status code.");
	}

	statusResult.first = response;
	statusResult.second = returnCode;

	return statusResult;
}

void DptfStatus::clearCache()
{
	m_participantStatusMap->clearCachedData();
}

std::string DptfStatus::getFileContent(std::string fileName)
{
	// Try to find file in current directory
	std::ifstream file(fileName, std::ios::in | std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		std::string content;
		content.resize(static_cast<UIntN>(file.tellg()));
		file.seekg(0, std::ios::beg);
		file.read(&content[0], content.size());
		file.close();
		return content;
	}
	else
	{
		throw dptf_exception("File not found.");
	}
}

std::string DptfStatus::getXsltContent(eEsifError* returnCode)
{
	try
	{
		return getFileContent(m_dptfManager->getDptfHomeDirectoryPath() + "combined.xsl");
	}
	catch (dptf_exception&)
	{
		// Could not find file, try from Resources/
		*returnCode = ESIF_E_UNSPECIFIED;
		throw;
	}
}

std::string DptfStatus::getGroupsXml(eEsifError* returnCode)
{
	auto groups = XmlNode::createWrapperElement("groups");

	auto group0 = XmlNode::createWrapperElement("group");
	groups->addChild(group0);
	group0->addChild(XmlNode::createDataElement("id", "0"));
	group0->addChild(XmlNode::createDataElement("name", "Policies"));

	auto group1 = XmlNode::createWrapperElement("group");
	groups->addChild(group1);
	group1->addChild(XmlNode::createDataElement("id", "1"));
	group1->addChild(XmlNode::createDataElement("name", "Participants"));

	auto group2 = XmlNode::createWrapperElement("group");
	groups->addChild(group2);
	group2->addChild(XmlNode::createDataElement("id", "2"));
	group2->addChild(XmlNode::createDataElement("name", "Manager"));

	auto group3 = XmlNode::createWrapperElement("group");
	groups->addChild(group3);
	group3->addChild(XmlNode::createDataElement("id", "3"));
	group3->addChild(XmlNode::createDataElement("name", "Arbitrator"));

	auto group4 = XmlNode::createWrapperElement("group");
	groups->addChild(group4);
	group4->addChild(XmlNode::createDataElement("id", "4"));
	group4->addChild(XmlNode::createDataElement("name", "System"));

	std::string s = groups->toString();

	return s;
}

std::string DptfStatus::getModulesInGroup(const UInt32 appStatusIn, eEsifError* returnCode)
{
	std::string modulesInGroup;

	switch (appStatusIn)
	{
	case GroupType::Policies:
		modulesInGroup = getPoliciesGroup();
		break;
	case GroupType::Participants:
		modulesInGroup = getParticipantsGroup();
		break;
	case GroupType::Framework:
		modulesInGroup = getFrameworkGroup();
		break;
	case GroupType::Arbitrator:
		modulesInGroup = getArbitratorGroup();
		break;
	case GroupType::System:
		modulesInGroup = getSystemGroup();
		break;
	default:
		*returnCode = ESIF_E_UNSPECIFIED;
		throw dptf_exception("Invalid group ID specified.");
	}
	return modulesInGroup;
}

std::string DptfStatus::getPoliciesGroup()
{
	auto modules = XmlNode::createWrapperElement("modules");

	auto policyIndexes = m_policyManager->getPolicyIndexes();
	for (auto policyIndex = policyIndexes.begin(); policyIndex != policyIndexes.end(); ++policyIndex)
	{
		try
		{
			// Get the policy variables before adding nodes.  This forces
			// exceptions to be thrown first.
			auto policy = m_policyManager->getPolicyPtr(*policyIndex);
			std::string name = policy->getName();

			auto module = XmlNode::createWrapperElement("module");
			modules->addChild(module);

			auto policyId = XmlNode::createDataElement("id", std::to_string(*policyIndex));
			module->addChild(policyId);

			auto policyName = XmlNode::createDataElement("name", name);
			module->addChild(policyName);
		}
		catch (...)
		{
			// policy not available, don't add it to the list
		}
	}

	std::string s = modules->toString();

	return s;
}

std::string DptfStatus::getParticipantsGroup()
{
	return m_participantStatusMap->getGroupsString();
}

std::string DptfStatus::getFrameworkGroup()
{
	auto modules = XmlNode::createWrapperElement("modules");

	for (UIntN moduleType = 0; moduleType < (UIntN)ManagerModuleType::Statistics; ++moduleType)
	{
		auto module = XmlNode::createWrapperElement("module");
		modules->addChild(module);

		auto moduleId = XmlNode::createDataElement("id", std::to_string(moduleType));
		module->addChild(moduleId);

		auto moduleName =
			XmlNode::createDataElement("name", ManagerModuleType::ToString((ManagerModuleType::Type)moduleType));
		module->addChild(moduleName);
	}

#ifdef INCLUDE_WORK_ITEM_STATISTICS

	// Work Item Statistics

	module = XmlNode::createWrapperElement("module");
	modules->addChild(module);

	moduleId = XmlNode::createDataElement("id", std::to_string(ManagerModuleType::Statistics));
	module->addChild(moduleId);

	moduleName = XmlNode::createDataElement("name", ManagerModuleType::ToString(ManagerModuleType::Statistics));
	module->addChild(moduleName);

#endif

	std::string s = modules->toString();

	return s;
}

std::string DptfStatus::getArbitratorGroup()
{
	auto modules = XmlNode::createWrapperElement("modules");

	// in alphabetical order
	modules->addChild(getArbitratorModuleInGroup(ControlFactoryType::Active));
	modules->addChild(getArbitratorModuleInGroup(ControlFactoryType::ConfigTdp));
	modules->addChild(getArbitratorModuleInGroup(ControlFactoryType::Core));
	modules->addChild(getArbitratorModuleInGroup(ControlFactoryType::Display));
	modules->addChild(getArbitratorModuleInGroup(ControlFactoryType::PeakPowerControl));
	modules->addChild(getArbitratorModuleInGroup(ControlFactoryType::Performance));
	modules->addChild(getArbitratorModuleInGroup(ControlFactoryType::PlatformPowerControl));
	modules->addChild(getArbitratorModuleInGroup(ControlFactoryType::PowerControl));
	modules->addChild(getArbitratorModuleInGroup(ControlFactoryType::ProcessorControl));
	modules->addChild(getArbitratorModuleInGroup(ControlFactoryType::SystemPower));
	modules->addChild(getArbitratorModuleInGroup(ControlFactoryType::Temperature));

	std::string s = modules->toString();

	return s;
}

std::string DptfStatus::getSystemGroup()
{
	auto modules = XmlNode::createWrapperElement("modules");
	auto module = XmlNode::createWrapperElement("module");
	modules->addChild(module);

	auto moduleId = XmlNode::createDataElement("id", std::to_string((UIntN)SystemModuleType::SystemConfiguration));
	module->addChild(moduleId);

	auto moduleName = XmlNode::createDataElement("name", SystemModuleType::ToString(SystemModuleType::SystemConfiguration));
	module->addChild(moduleName);

	return modules->toString();
}

std::shared_ptr<XmlNode> DptfStatus::getArbitratorModuleInGroup(ControlFactoryType::Type type)
{
	auto module = XmlNode::createWrapperElement("module");
	auto moduleId = XmlNode::createDataElement("id", std::to_string(type));
	module->addChild(moduleId);

	auto moduleName =
		XmlNode::createDataElement("name", ControlFactoryType::getArbitratorString((ControlFactoryType::Type)type));
	module->addChild(moduleName);
	return module;
}

std::string DptfStatus::getModuleData(const UInt32 appStatusIn, eEsifError* returnCode)
{
	std::string moduleData;

	UInt32 groupId = static_cast<UInt32>(BinaryParse::extractBits(32, 16, appStatusIn));
	UInt32 moduleId = static_cast<UInt32>(BinaryParse::extractBits(15, 0, appStatusIn));

	switch (groupId)
	{
	case GroupType::Policies:
		moduleData = getXmlForPolicy(moduleId, returnCode);
		break;
	case GroupType::Participants:
		moduleData = getXmlForParticipant(moduleId, returnCode);
		break;
	case GroupType::Framework:
		moduleData = getXmlForFramework(moduleId, returnCode);
		break;
	case GroupType::Arbitrator:
		moduleData = getXmlForArbitrator(moduleId, returnCode);
		break;
	case GroupType::System:
		moduleData = getXmlForSystem(moduleId, returnCode);
		break;
	default:
		*returnCode = ESIF_E_UNSPECIFIED;
		throw dptf_exception("Invalid group ID specified.");
	}
	return moduleData;
}

std::string DptfStatus::getXmlForPolicy(UInt32 policyIndex, eEsifError* returnCode)
{
	try
	{
		auto policy = m_policyManager->getPolicyPtr(policyIndex);
		return policy->getStatusAsXml();
	}
	catch (policy_index_invalid&)
	{
		*returnCode = ESIF_E_UNSPECIFIED;
		throw dptf_exception("Invalid policy status requested.");
	}
	catch (...)
	{
		*returnCode = ESIF_E_UNSPECIFIED;
		throw;
	}
}

std::string DptfStatus::getXmlForParticipant(UInt32 mappedIndex, eEsifError* returnCode)
{
	auto participantIndexList = m_participantManager->getParticipantIndexes();
	UIntN totalDomainCount = 0;

	// Total # of participants + domains for error checking
	for (auto i = participantIndexList.begin(); i != participantIndexList.end(); ++i)
	{
		try
		{
			Participant* participant = m_participantManager->getParticipantPtr(*i);
			totalDomainCount += participant->getDomainCount();
		}
		catch (...)
		{
			// If participant pointer couldn't be returned or couldn't get domain count
			// Most likely, a participant was removed and left bind a null pointer
		}
	}

	if (mappedIndex >= totalDomainCount)
	{
		*returnCode = ESIF_E_UNSPECIFIED;
		throw dptf_exception("Invalid participant status requested.");
	}

	try
	{
		auto participantData = m_participantStatusMap->getStatusAsXml(mappedIndex);
		std::string s = participantData->toString();

		return s;
	}
	catch (...)
	{
		*returnCode = ESIF_E_UNSPECIFIED;
		throw;
	}
}

std::string DptfStatus::getXmlForFramework(UInt32 moduleIndex, eEsifError* returnCode)
{
	switch (moduleIndex)
	{
	case ManagerModuleType::Manager:
	{
		*returnCode = ESIF_OK;
		auto frameworkRoot = XmlNode::createRoot();
		frameworkRoot->addChild(XmlNode::createComment("format_id=" + ManagerStatusFormatId.toString()));
		auto dppmRoot = XmlNode::createWrapperElement("manager_status");
		dppmRoot->addChild(getXmlForFrameworkLoadedPolicies());
		dppmRoot->addChild(getXmlForFrameworkLoadedParticipants());
		dppmRoot->addChild(getXmlForPlatformRequests());
		frameworkRoot->addChild(dppmRoot);
		return frameworkRoot->toString();
	}
	case ManagerModuleType::Events:
	{
		*returnCode = ESIF_OK;
		auto frameworkRoot = m_policyManager->getStatusAsXml();
		return frameworkRoot->toString();
	}
	case ManagerModuleType::Statistics:
	{
		*returnCode = ESIF_OK;
		auto frameworkRoot = m_dptfManager->getWorkItemQueueManager()->getStatusAsXml();
		return frameworkRoot->toString();
	}
	default:
		*returnCode = ESIF_E_UNSPECIFIED;
		return std::string();
	}
}

std::string DptfStatus::getXmlForArbitrator(UInt32 moduleIndex, eEsifError* returnCode)
{
	*returnCode = ESIF_OK;
	auto frameworkRoot = getArbitratorXmlForLoadedParticipants(moduleIndex);
	return frameworkRoot->toString();
}

std::string DptfStatus::getXmlForSystem(UInt32 moduleIndex, eEsifError* returnCode)
{
	// System group "modules" are implemented as though they are policies to leverage currently implemented interfaces
	// As they are not really a policy, we do not want them to appear under the Policies group
	switch (moduleIndex)
	{
	case SystemModuleType::SystemConfiguration:
	{
		*returnCode = ESIF_OK;

		try
		{
			auto policy = m_policyManager->getPolicy(SystemModuleType::ToString(SystemModuleType::SystemConfiguration));
			return policy->getStatusAsXml();
		}
		catch (policy_index_invalid&)
		{
			*returnCode = ESIF_E_UNSPECIFIED;
			throw dptf_exception("Invalid policy status requested.");
		}
		catch (...)
		{
			*returnCode = ESIF_E_UNSPECIFIED;
			throw;
		}
	}
	default:
		*returnCode = ESIF_E_UNSPECIFIED;
		return std::string();
	}
}

std::shared_ptr<XmlNode> DptfStatus::getXmlForFrameworkLoadedPolicies()
{
	auto policiesRoot = XmlNode::createWrapperElement("policies");

	auto policyIndexes = m_policyManager->getPolicyIndexes();
	policiesRoot->addChild(XmlNode::createDataElement("policy_count", std::to_string(policyIndexes.size())));

	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		try
		{
			auto policy = m_policyManager->getPolicyPtr(*i);
			std::string name = policy->getName();

			if (name == SystemModuleType::ToString(SystemModuleType::SystemConfiguration))
			{
				// do not want to add System Configuration as part of the Policies table
				continue;
			}

			auto policyRoot = XmlNode::createWrapperElement("policy");

			auto policyIndex = XmlNode::createDataElement("policy_index", std::to_string(*i));
			policyRoot->addChild(policyIndex);

			auto policyName = XmlNode::createDataElement("policy_name", name);
			policyRoot->addChild(policyName);

			policiesRoot->addChild(policyRoot);
		}
		catch (...)
		{
			// Policy not available, do not add.
		}
	}

	return policiesRoot;
}

std::shared_ptr<XmlNode> DptfStatus::getXmlForFrameworkLoadedParticipants()
{
	auto participantsRoot = XmlNode::createWrapperElement("participants");

	auto participantIndexList = m_participantManager->getParticipantIndexes();
	for (auto i = participantIndexList.begin(); i != participantIndexList.end(); ++i)
	{
		try
		{
			Participant* participant = m_participantManager->getParticipantPtr(*i);
			participantsRoot->addChild(participant->getXml(Constants::Invalid));
		}
		catch (...)
		{
			// Participant not available
		}
	}

	return participantsRoot;
}

std::shared_ptr<XmlNode> DptfStatus::getXmlForPlatformRequests()
{
	return std::dynamic_pointer_cast<PlatformRequestHandler>(m_dptfManager->getPlatformRequestHandler())->getXml();
}

void DptfStatus::fillEsifString(EsifDataPtr outputLocation, std::string inputString, eEsifError* returnCode)
{
	*returnCode = FillDataPtrWithString(outputLocation, inputString);
}

UIntN DptfStatus::getNumberOfUniqueDomains(std::set<UIntN> participantIndexList)
{
	UIntN numberOfUniqueDomains = 0;
	for (auto participantIndex = participantIndexList.begin(); participantIndex != participantIndexList.end();
		 ++participantIndex)
	{
		try
		{
			Participant* participant = m_participantManager->getParticipantPtr(*participantIndex);
			numberOfUniqueDomains = numberOfUniqueDomains + participant->getDomainCount();
		}
		catch (...)
		{
			// Participant not available
		}
	}

	return numberOfUniqueDomains;
}

std::shared_ptr<XmlNode> DptfStatus::getArbitratorXmlForLoadedParticipants(UInt32 moduleIndex)
{
	auto root = XmlNode::createRoot();
	auto type = (ControlFactoryType::Type)moduleIndex;
	root->addChild(XmlNode::createComment("format_id=" + ControlFactoryType::getArbitratorFormatId(type).toString()));

	auto arbitratorRoot = XmlNode::createWrapperElement("arbitrator_status");
	auto participantIndexList = m_participantManager->getParticipantIndexes();
	auto numberOfUniqueDomains = getNumberOfUniqueDomains(participantIndexList);

	arbitratorRoot->addChild(
		XmlNode::createDataElement("number_of_domains", StatusFormat::friendlyValue(numberOfUniqueDomains)));

	auto policyIndexes = m_policyManager->getPolicyIndexes();
	for (auto policyIndex = policyIndexes.begin(); policyIndex != policyIndexes.end(); ++policyIndex)
	{
		try
		{
			auto policy = m_policyManager->getPolicyPtr(*policyIndex);
			std::string name = policy->getName();
			auto policyRoot = XmlNode::createWrapperElement("policy");
			auto policyName = XmlNode::createDataElement("policy_name", name);
			policyRoot->addChild(policyName);

			for (auto participantIndex = participantIndexList.begin(); participantIndex != participantIndexList.end();
				 ++participantIndex)
			{
				try
				{
					Participant* participant = m_participantManager->getParticipantPtr(*participantIndex);
					policyRoot->addChild(participant->getArbitrationXmlForPolicy(*policyIndex, type));
				}
				catch (...)
				{
					// Participant not available
				}
			}

			arbitratorRoot->addChild(policyRoot);
		}
		catch (...)
		{
			// Policy not available, do not add.
		}
	}

	root->addChild(arbitratorRoot);
	return root;
}
