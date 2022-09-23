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
#include "UiGetGroupsCommand.h"
#include "DptfManagerInterface.h"
#include "PolicyManagerInterface.h"
using namespace std;

UiGetGroupsCommand::UiGetGroupsCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

UiGetGroupsCommand::~UiGetGroupsCommand()
{
}

string UiGetGroupsCommand::getCommandName() const
{
	return "getgroups";
}

void UiGetGroupsCommand::execute(const CommandArguments& arguments)
{
	throwIfBadArguments(arguments);
	setResultMessage(getGroups());
	setResultCode(ESIF_OK);
}

void UiGetGroupsCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() != 1)
	{
		setResultCode(ESIF_E_INVALID_ARGUMENT_COUNT);
		string description = string(
			"Invalid argument count given to 'ui getgroups' command.  "
			"Run 'dptf help' command for more information.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}

std::string UiGetGroupsCommand::getGroups() const
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

	return groups->toString();
}
