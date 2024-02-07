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
#include "UiGetModuleDataCommand.h"
#include "DptfManagerInterface.h"
#include "ParticipantManagerInterface.h"
#include "PlatformRequestHandler.h"
#include "PolicyManagerInterface.h"
#include "StatusFormat.h"
#include "StringConverter.h"
#include "UiCommandTypes.h"
#include "WorkItemQueueManagerInterface.h"
using namespace std;

UiGetModuleDataCommand::UiGetModuleDataCommand(DptfManagerInterface* dptfManager)
	: UiSubCommand(dptfManager)
{
}

UiGetModuleDataCommand::~UiGetModuleDataCommand()
{
}

string UiGetModuleDataCommand::getCommandName() const
{
	return "getmoduledata";
}

void UiGetModuleDataCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfBadArgumentCount(arguments);
		throwIfBadArgumentData(arguments);
		throwIfBadGroupNumber(arguments);
		throwIfBadModuleNumber(arguments);
		const UInt32 groupNumber = StringConverter::toUInt32(arguments[1].getDataAsString());
		const UInt32 moduleNumber = StringConverter::toUInt32(arguments[2].getDataAsString());
		const std::string result = getModuleData(groupNumber, moduleNumber);
		setResultCode(ESIF_OK);
		setResultMessage(result);
	}
	catch (const command_failure& e)
	{
		setResultCode(e.getErrorCode());
		setResultMessage(e.getDescription());
	}
}

std::string UiGetModuleDataCommand::getModuleData(const UInt32 group, const UInt32 module) const
{
	switch (group)
	{
	case UiCommandGroupType::Policies:
		return getModuleDataForPolicy(module);
	case UiCommandGroupType::Participants:
		return getModuleDataForParticipant(module);
	case UiCommandGroupType::Framework:
		return getModuleDataForFramework(module);
	case UiCommandGroupType::Arbitrator:
		return getModuleDataForArbitrator(module);
	case UiCommandGroupType::System:
		return getModuleDataForSystem(module);
	default:
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, "Invalid group ID specified.");
	}
}

std::string UiGetModuleDataCommand::getModuleDataForPolicy(const UInt32 module) const
{
	try
	{
		const auto policy = m_dptfManager->getPolicyManager()->getPolicyPtr(module);
		return policy->getStatusAsXml();
	}
	catch (...)
	{
		throw command_failure(ESIF_E_UNSPECIFIED, "Error getting policy status data.");
	}
}

std::string UiGetModuleDataCommand::getModuleDataForParticipant(const UInt32 module) const
{
	const auto participantDomainsList = buildParticipantDomainsList();
	const auto participantId = participantDomainsList[module].first;
	const auto participantManager = m_dptfManager->getParticipantManager();
	const auto participant = participantManager->getParticipantPtr(participantId);
	const auto domainId = participantDomainsList[module].second;
	return participant->getStatusAsXml(domainId)->toString();
}

std::string UiGetModuleDataCommand::getModuleDataForFramework(const UInt32 module) const
{
	try
	{
		std::string status;
		switch (module)
		{
		case UiCommandManagerModuleType::Policies:
			status = getModuleDataForFrameworkPolicies();
			break;
		case UiCommandManagerModuleType::Participants:
			status = getModuleDataForFrameworkParticipants();
			break;
		case UiCommandManagerModuleType::Events:
			status = getModuleDataForFrameworkEvents();
			break;
		case UiCommandManagerModuleType::Statistics:
			status = getModuleDataForFrameworkStatistics();
			break;
		default:
			throw command_failure(ESIF_E_UNSPECIFIED, "Invalid module selected for Framework status");
		}
		return status;
	}
	catch (const command_failure&)
	{
		throw;
	}
	catch (...)
	{
		throw command_failure(ESIF_E_UNSPECIFIED, "Error generating status for framework module.");
	}
}

// clang-format off
const Guid ManagerPolicyStatusFormatId(0x3E, 0x58, 0x63, 0x46, 0xF8, 0xF7, 0x45, 0x4A, 0xA8, 0xF7, 0xDE, 0x7E, 0xC6, 0xF7, 0x61, 0xA8);
// clang-format on

std::string UiGetModuleDataCommand::getModuleDataForFrameworkPolicies() const
{
	const auto managerPolicyRoot = XmlNode::createRoot();
	managerPolicyRoot->addChild(XmlNode::createComment("format_id=" + ManagerPolicyStatusFormatId.toString()));
	const auto policyStatus = XmlNode::createWrapperElement("manager_policy_status");
	policyStatus->addChild(getXmlForFrameworkLoadedPolicies());
	policyStatus->addChild(getXmlForPlatformRequests());
	managerPolicyRoot->addChild(policyStatus);
	return managerPolicyRoot->toString();
}

void UiGetModuleDataCommand::addXmlForFrameworkPolicy(const std::shared_ptr<XmlNode>& root) const
{
	const auto policyManager = m_dptfManager->getPolicyManager();
	const auto policyIndexes = policyManager->getPolicyIndexes();
	for (auto i = policyIndexes.begin(); i != policyIndexes.end(); ++i)
	{
		try
		{
			// do not want to add System Configuration as part of the Policies table
			const auto policy = policyManager->getPolicyPtr(*i);
			std::string name = policy->getName();
			if (name != UiCommandSystemModuleType::ToString(UiCommandSystemModuleType::SystemConfiguration))
			{
				const auto policyRoot = XmlNode::createWrapperElement("policy");
				const auto policyIndex = XmlNode::createDataElement("policy_index", std::to_string(*i));
				policyRoot->addChild(policyIndex);
				const auto policyName = XmlNode::createDataElement("policy_name", name);
				policyRoot->addChild(policyName);
				root->addChild(policyRoot);
			}
		}
		catch (...)
		{
			// Policy not available, do not add.
		}
	}
}

std::shared_ptr<XmlNode> UiGetModuleDataCommand::getXmlForFrameworkLoadedPolicies() const
{
	const auto policyManager = m_dptfManager->getPolicyManager();
	const auto policyIndexes = policyManager->getPolicyIndexes();
	auto policiesRoot = XmlNode::createWrapperElement("policies");
	policiesRoot->addChild(XmlNode::createDataElement("policy_count", std::to_string(policyIndexes.size())));
	addXmlForFrameworkPolicy(policiesRoot);
	return policiesRoot;
}

std::shared_ptr<XmlNode> UiGetModuleDataCommand::getXmlForPlatformRequests() const
{
	const auto requestHandler = std::dynamic_pointer_cast<PlatformRequestHandlerInterface>(m_dptfManager->getPlatformRequestHandler());
	if (requestHandler == nullptr)
		throw dptf_exception("Failed to get platform request handler.");
	return requestHandler->getXml();
}

// clang-format off
const Guid ManagerParticipantStatusFormatId(0x98, 0x3F, 0x90, 0x5D, 0x9E, 0x39, 0x93, 0x4D, 0xBC, 0xCD, 0xA1, 0xA5, 0x8F, 0x61, 0x18, 0x5F);
// clang-format on

std::string UiGetModuleDataCommand::getModuleDataForFrameworkParticipants() const
{
	const auto managerParticipantRoot = XmlNode::createRoot();
	managerParticipantRoot->addChild(
		XmlNode::createComment("format_id=" + ManagerParticipantStatusFormatId.toString()));
	const auto participantStatus = XmlNode::createWrapperElement("manager_participant_status");
	participantStatus->addChild(getXmlForFrameworkLoadedParticipants());
	managerParticipantRoot->addChild(participantStatus);
	return managerParticipantRoot->toString();
}

std::shared_ptr<XmlNode> UiGetModuleDataCommand::getXmlForFrameworkLoadedParticipants() const
{
	const auto participantManager = m_dptfManager->getParticipantManager();
	auto participantsRoot = XmlNode::createWrapperElement("participants");
	const auto participantIndexList = participantManager->getParticipantIndexes();
	for (auto i = participantIndexList.begin(); i != participantIndexList.end(); ++i)
	{
		try
		{
			const auto participant = participantManager->getParticipantPtr(*i);
			participantsRoot->addChild(participant->getXml(Constants::Invalid));
		}
		catch (...)
		{
			// Participant not available
		}
	}

	return participantsRoot;
}

std::string UiGetModuleDataCommand::getModuleDataForFrameworkEvents() const
{
	const auto policyManager = m_dptfManager->getPolicyManager();
	const auto policyManagerStatus = policyManager->getStatusAsXml();
	return policyManagerStatus->toString();
}

std::string UiGetModuleDataCommand::getModuleDataForFrameworkStatistics() const
{
	const auto workItemQueueManager = m_dptfManager->getWorkItemQueueManager();
	const auto workItemQueueManagerStatus =	workItemQueueManager->getStatusAsXml();
	return workItemQueueManagerStatus->toString();
}

void UiGetModuleDataCommand::addXmlForArbitratorPolicy(
	ControlFactoryType::Type type, const std::shared_ptr<XmlNode>& root) const
{
	const auto participantManager = m_dptfManager->getParticipantManager();
	const auto participantIndexList = participantManager->getParticipantIndexes();
	const auto policyManager = m_dptfManager->getPolicyManager();
	auto policyIndexes = policyManager->getPolicyIndexes();
	for (auto policyIndex = policyIndexes.begin(); policyIndex != policyIndexes.end(); ++policyIndex)
	{
		try
		{
			const auto policy = policyManager->getPolicyPtr(*policyIndex);
			const auto policyRoot = XmlNode::createWrapperElement("policy");
			const auto policyName = XmlNode::createDataElement("policy_name", policy->getName());
			policyRoot->addChild(policyName);

			for (auto participantIndex = participantIndexList.begin(); participantIndex != participantIndexList.end();
				 ++participantIndex)
			{
				try
				{
					const Participant* participant = participantManager->getParticipantPtr(*participantIndex);
					policyRoot->addChild(participant->getArbitrationXmlForPolicy(*policyIndex, type));
				}
				catch (...)
				{
					// Participant not available
				}
			}
			root->addChild(policyRoot);
		}
		catch (...)
		{
			// Policy not available, do not add.
		}
	}
}

std::string UiGetModuleDataCommand::getModuleDataForArbitrator(const UInt32 module) const
{
	const auto root = XmlNode::createRoot();
	const auto type = (ControlFactoryType::Type)module;
	root->addChild(XmlNode::createComment("format_id=" + 
		ControlFactoryType::getArbitratorFormatId(type).toString()));

	const auto arbitratorRoot = XmlNode::createWrapperElement("arbitrator_status");
	const auto numberOfUniqueDomains = buildParticipantDomainsList().size();
	arbitratorRoot->addChild(XmlNode::createDataElement(
	"number_of_domains", StatusFormat::friendlyValue((UInt32)numberOfUniqueDomains)));
	addXmlForArbitratorPolicy(type, arbitratorRoot);
	root->addChild(arbitratorRoot);
	return root->toString();
}

std::string UiGetModuleDataCommand::getModuleDataForSystem(const UInt32 module) const
{
	// System group "modules" are implemented as though they are policies to leverage currently implemented interfaces
	// As they are not really a policy, we do not want them to appear under the Policies group
	try
	{
		const auto policyManager = m_dptfManager->getPolicyManager();
		const auto policy = policyManager->getPolicy(
			UiCommandSystemModuleType::ToString(UiCommandSystemModuleType::SystemConfiguration));
		return policy->getStatusAsXml();
	}
	catch (...)
	{
		throw command_failure(ESIF_E_UNSPECIFIED, "Error retrieving system module data.");
	}
}

void UiGetModuleDataCommand::throwIfBadArgumentCount(const CommandArguments& arguments)
{
	if (arguments.size() != 3)
	{
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, "Invalid argument count.");
	}
}

void UiGetModuleDataCommand::throwIfBadArgumentData(const CommandArguments& arguments)
{
	try
	{
		StringConverter::toUInt32(arguments[1].getDataAsString());
		StringConverter::toUInt32(arguments[2].getDataAsString());
	}
	catch (...)
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, 
		"Argument given is not a valid group or module ID.");
	}
}

void UiGetModuleDataCommand::throwIfBadGroupNumber(const CommandArguments& arguments)
{
	const UInt32 groupNumber = StringConverter::toUInt32(arguments[1].getDataAsString());
	if (groupNumber >= UiCommandGroupType::MAX)
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, 
		"Invalid group ID specified.");
	}
}

void UiGetModuleDataCommand::throwIfBadModuleNumber(const CommandArguments& arguments) const
{
	const UInt32 group = StringConverter::toUInt32(arguments[1].getDataAsString());
	switch (group)
	{
	case UiCommandGroupType::Policies:
		throwIfBadPolicyModule(arguments);
		return;
	case UiCommandGroupType::Participants:
		throwIfBadParticipantModule(arguments);
		return;
	case UiCommandGroupType::Framework:
		throwIfBadFrameworkModule(arguments);
		return;
	case UiCommandGroupType::Arbitrator:
		throwIfBadArbitratorModule(arguments);
		return;
	case UiCommandGroupType::System:
		throwIfBadSystemModule(arguments);
		return;
	default:
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, "Invalid group ID specified.");
	}
}

void UiGetModuleDataCommand::throwIfBadPolicyModule(const CommandArguments& arguments) const
{
	const UInt32 module = StringConverter::toUInt32(arguments[2].getDataAsString());
	const auto policyIndexes = m_dptfManager->getPolicyManager()->getPolicyIndexes();
	const auto it = policyIndexes.find(module);
	if (it == policyIndexes.end())
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, "Module ID for Policy is invalid.");
	}

	// SystemConfiguration has its own getXml function, and does not go through this
	const auto policy = m_dptfManager->getPolicyManager()->getPolicyPtr(module);
	if (policy->getName() == UiCommandSystemModuleType::ToString(UiCommandSystemModuleType::SystemConfiguration))
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, "Invalid Module ID.");
	}

}

void UiGetModuleDataCommand::throwIfBadParticipantModule(const CommandArguments& arguments) const
{
	const auto participantDomainsList = buildParticipantDomainsList();
	const UInt32 moduleId = StringConverter::toUInt32(arguments[2].getDataAsString());
	if (moduleId >= participantDomainsList.size())
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, "Invalid participant ID given.");
	}
}

void UiGetModuleDataCommand::throwIfBadFrameworkModule(const CommandArguments& arguments)
{
	const auto module = StringConverter::toUInt32(arguments[2].getDataAsString());
	if (module >= UiCommandManagerModuleType::MAX)
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, "Invalid framework module given.");
	}
}

void UiGetModuleDataCommand::throwIfBadArbitratorModule(const CommandArguments& arguments)
{
	const auto module = StringConverter::toUInt32(arguments[2].getDataAsString());
	if (module >= ControlFactoryType::MAX)
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, "Invalid arbitrator control type given.");
	}
}

void UiGetModuleDataCommand::throwIfBadSystemModule(const CommandArguments& arguments)
{
	const auto module = StringConverter::toUInt32(arguments[2].getDataAsString());
	if (module != UiCommandSystemModuleType::SystemConfiguration)
	{
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, "Invalid system module ID given.");
	}
}
