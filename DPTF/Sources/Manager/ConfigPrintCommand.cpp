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
#include "ConfigPrintCommand.h"
#include "DptfManagerInterface.h"
#include "DttConfiguration.h"
using namespace std;

ConfigPrintCommand::ConfigPrintCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

ConfigPrintCommand::~ConfigPrintCommand()
{
}

string ConfigPrintCommand::getCommandName() const
{
	return "print";
}

string ConfigPrintCommand::getPrintedContent(const shared_ptr<ConfigurationFileContentInterface>& cs) const
{
	const auto dttConfiguration = DttConfiguration(cs);
	const auto segments = dttConfiguration.getSegmentsWithEnvironmentProfile(m_dptfManager->getEnvironmentProfile());
	return segments.empty() ? Constants::EmptyString : segments.front().toJsonString();
}

void ConfigPrintCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfBadArguments(arguments);
		throwIfInvalidConfigurationManager(m_dptfManager->getConfigurationManager());

		const auto configManager = m_dptfManager->getConfigurationManager();
		const auto configName = arguments[1].getDataAsString();
		if (configManager->contentExists(configName))
		{
			const auto cs = configManager->getContent(configName);
			const auto result = getPrintedContent(cs);
			setResultCode(ESIF_OK);
			setResultMessage(result);
		}
		else
		{
			throw command_failure(ESIF_E_NOT_FOUND, 
				"Configuration content for "s + configName + " not loaded."s);
		}
	}
	catch (const command_failure& e)
	{
		setResultCode(e.getErrorCode());
		setResultMessage(e.getDescription());
	}
}



void ConfigPrintCommand::throwIfBadArguments(const CommandArguments& arguments) const
{
	// print <name>
	if (arguments.size() != 2)
	{
		stringstream description;
		description << "Invalid argument count given to "s;
		description << getCommandName() << " command. ";
		description << "Run 'dptf help' command for more information."s;
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description.str());
	}
}

void ConfigPrintCommand::throwIfInvalidConfigurationManager(
	const shared_ptr<ConfigurationFileManagerInterface>& configurationManager) const
{
	if (!configurationManager)
	{
		throw logic_error("Invalid configuration manager");
	}
}