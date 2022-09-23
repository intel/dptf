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
#include "UiGetModulesInGroupCommand.h"
#include "DptfManagerInterface.h"
#include "ParticipantManagerInterface.h"
#include "PolicyManagerInterface.h"
#include "StringConverter.h"
#include "UiCommandTypes.h"
using namespace std;

UiGetModulesInGroupCommand::UiGetModulesInGroupCommand(DptfManagerInterface* dptfManager)
	: UiSubCommand(dptfManager)
{
}

UiGetModulesInGroupCommand::~UiGetModulesInGroupCommand()
{
}

string UiGetModulesInGroupCommand::getCommandName() const
{
	return "getmodulesingroup";
}

void UiGetModulesInGroupCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfBadArgumentNumber(arguments);
		throwIfBadArgumentData(arguments);
		throwIfBadGroupNumber(arguments);
		const UInt32 groupNumber = StringConverter::toUInt32(arguments[1].getDataAsString());
		const std::string result = getModulesInGroup(groupNumber);
		setResultCode(ESIF_OK);
		setResultMessage(result);
	}
	catch (const command_failure& e)
	{
		setResultCode(e.getErrorCode());
		setResultMessage(e.getDescription());
	}
}

std::string UiGetModulesInGroupCommand::getModulesInGroup(UInt32 groupNumber) const
{
	switch (groupNumber)
	{
	case UiCommandGroupType::Policies:
		return getPoliciesGroup();
	case UiCommandGroupType::Participants:
		return getParticipantsGroup();
	case UiCommandGroupType::Framework:
		return getFrameworkGroup();
	case UiCommandGroupType::Arbitrator:
		return getArbitratorGroup();
	case UiCommandGroupType::System:
		return getSystemGroup();
	default:
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID , "Invalid group ID specified.");
	}
}

std::string UiGetModulesInGroupCommand::getPoliciesGroup() const
{
	const auto policyManager = m_dptfManager->getPolicyManager();
	const auto modules = XmlNode::createWrapperElement("modules");
	const auto policyIndexes = policyManager->getPolicyIndexes();
	for (const auto policyIndex : policyIndexes)
	{
		try
		{
			auto const module = XmlNode::createWrapperElement("module");
			module->addChild(XmlNode::createDataElement("id", std::to_string(policyIndex)));
			module->addChild(XmlNode::createDataElement("name", policyManager->getPolicyPtr(policyIndex)->getName()));
			modules->addChild(module);
		}
		catch (...)
		{
			// policy not available, don't add it to the list
		}
	}

	return modules->toString();
}

std::string UiGetModulesInGroupCommand::getParticipantsGroup() const
{
	const auto participantDomainsList = buildParticipantDomainsList();
	const auto participantManager = m_dptfManager->getParticipantManager();
	const auto modules = XmlNode::createWrapperElement("modules");
	for (UIntN i = 0; i < participantDomainsList.size(); i++)
	{
		try
		{
			const auto participant = participantManager->getParticipantPtr(participantDomainsList[i].first);

			std::stringstream name;
			name << participant->getParticipantName();

			if (participant->getDomainCount() > 1)
			{
				name << '(' << participantDomainsList[i].second << ')';
			}

			const auto module = XmlNode::createWrapperElement("module");
			module->addChild(XmlNode::createDataElement("id", std::to_string(i)));
			module->addChild(XmlNode::createDataElement("name", name.str()));
			modules->addChild(module);
		}
		catch (dptf_exception&)
		{
			// Participant Not available.
		}
	}

	return modules->toString();
}

std::string UiGetModulesInGroupCommand::getFrameworkGroup() const
{
	const auto modules = XmlNode::createWrapperElement("modules");
	for (UIntN moduleType = 0; moduleType < (UIntN)UiCommandManagerModuleType::Statistics; ++moduleType)
	{
		const auto module = XmlNode::createWrapperElement("module");
		module->addChild(XmlNode::createDataElement("id", std::to_string(moduleType)));
		module->addChild(XmlNode::createDataElement("name", UiCommandManagerModuleType::ToString((UiCommandManagerModuleType::Type)moduleType)));
		modules->addChild(module);
	}

#ifdef INCLUDE_WORK_ITEM_STATISTICS
	// Work Item Statistics
	const auto module = XmlNode::createWrapperElement("module");
	module->addChild(XmlNode::createDataElement("id", std::to_string(UiCommandManagerModuleType::Statistics)));
	module->addChild(XmlNode::createDataElement("name", UiCommandManagerModuleType::ToString(UiCommandManagerModuleType::Statistics)));
	modules->addChild(module);
#endif

	return modules->toString();
}

std::string UiGetModulesInGroupCommand::getArbitratorGroup() const
{
	const std::list<std::pair<int, std::string>> arbitratorModules = {
		{ControlFactoryType::Active, ControlFactoryType::getArbitratorString(ControlFactoryType::Active)},
		{ControlFactoryType::Core, ControlFactoryType::getArbitratorString(ControlFactoryType::Core)},
		{ControlFactoryType::Display, ControlFactoryType::getArbitratorString(ControlFactoryType::Display)},
		{ControlFactoryType::PeakPowerControl,
		 ControlFactoryType::getArbitratorString(ControlFactoryType::PeakPowerControl)},
		{ControlFactoryType::Performance, ControlFactoryType::getArbitratorString(ControlFactoryType::Performance)},
		{ControlFactoryType::PowerControl, ControlFactoryType::getArbitratorString(ControlFactoryType::PowerControl)},
		{ControlFactoryType::ProcessorControl,
		 ControlFactoryType::getArbitratorString(ControlFactoryType::ProcessorControl)},
		{ControlFactoryType::SystemPower, ControlFactoryType::getArbitratorString(ControlFactoryType::SystemPower)},
		{ControlFactoryType::Temperature, ControlFactoryType::getArbitratorString(ControlFactoryType::Temperature)}};

	auto modules = XmlNode::createWrapperElement("modules");
	for (const auto& arbitratorModule : arbitratorModules)
	{
		const auto module = XmlNode::createWrapperElement("module");
		module->addChild(XmlNode::createDataElement("id", std::to_string(arbitratorModule.first)));
		module->addChild(XmlNode::createDataElement("name", arbitratorModule.second));
		modules->addChild(module);
	}
	return modules->toString();
}

std::string UiGetModulesInGroupCommand::getSystemGroup() const
{
	const auto modules = XmlNode::createWrapperElement("modules");
	const auto module = XmlNode::createWrapperElement("module");
	module->addChild(XmlNode::createDataElement("id", std::to_string((UIntN)UiCommandSystemModuleType::SystemConfiguration)));
	module->addChild(XmlNode::createDataElement("name", UiCommandSystemModuleType::ToString(UiCommandSystemModuleType::SystemConfiguration)));
	modules->addChild(module);
	return modules->toString();
}

void UiGetModulesInGroupCommand::throwIfBadArgumentNumber(const CommandArguments& arguments)
{
	if (arguments.size() != 2)
	{
		string description = string(
			"Invalid argument count given to 'ui getmodulesingroup' command.  "
			"Run 'dptf help' command for more information.");
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}

void UiGetModulesInGroupCommand::throwIfBadArgumentData(const CommandArguments& arguments)
{
	try
	{
		StringConverter::toUInt32(arguments[1].getDataAsString());
	}
	catch (...)
	{
		string description = string(
			"Invalid argument given to 'ui getmodulesingroup' command.  "
			"Run 'dptf help' command for more information.");
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}

void UiGetModulesInGroupCommand::throwIfBadGroupNumber(const CommandArguments& arguments)
{
	UInt32 groupNumber = StringConverter::toUInt32(arguments[1].getDataAsString());
	if (groupNumber >= UiCommandGroupType::MAX)
	{
		string description = string(
			"Invalid group specified to 'ui getmodulesingroup' command.  "
			"Run 'dptf help' command for more information.");
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}
