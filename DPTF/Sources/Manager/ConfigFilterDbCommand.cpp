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
#include "ConfigFilterDbCommand.h"
#include "DptfManagerInterface.h"

using namespace std;

ConfigFilterDbCommand::ConfigFilterDbCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

ConfigFilterDbCommand::~ConfigFilterDbCommand()
{
}

string ConfigFilterDbCommand::getCommandName() const
{
	return "filterdb";
}

string ConfigFilterDbCommand::getFilteredContentByCpuId(
	const shared_ptr<ConfigurationFileContentInterface>& cs,
	const regex regularExp) const
{
	const auto environmentProfile = m_dptfManager->getEnvironmentProfile();
	return cs->toFilteredDatabaseString(environmentProfile.cpuIdWithoutStepping, regularExp);
}

void ConfigFilterDbCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfBadArguments(arguments);
		const auto configManager = m_dptfManager->getConfigurationManager();
		const auto configName = arguments[1].getDataAsString();
		regex filterRegex =  regex(arguments[2].getDataAsString());

		if (configManager->contentExists(configName))
		{
			const auto cs = configManager->getContent(configName);
			const auto result = getFilteredContentByCpuId(cs, filterRegex);
			setResultCode(ESIF_OK);
			setResultMessage(result);
		}
		else
		{
			throw command_failure(ESIF_E_NOT_FOUND, "Configuration filter content for "s + configName + " not loaded."s);
		}
	}
	catch (const command_failure& e)
	{
		setResultCode(e.getErrorCode());
		setResultMessage(e.getDescription());
	}
}

void ConfigFilterDbCommand::throwIfBadArguments(const CommandArguments& arguments) const
{
	// filterdb <filename> <regex>
	if (arguments.size() != 3)
	{
		stringstream description;
		description << "Invalid argument count given to "s;
		description << getCommandName() << " command. ";
		description << "Run 'dptf help' command for more information."s;
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description.str());
	}
}