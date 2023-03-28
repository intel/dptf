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
#include "ConfigListCommand.h"
#include "DptfManagerInterface.h"

using namespace std;

#define COLUMN_NAME_WIDTH 9
#define COLUMN_SEGMENTS_WIDTH 5
#define COLUMN_DATA_WIDTH 9
#define COLUMN_PATH_WIDTH 93
#define MAX_WIDTH 119

ConfigListCommand::ConfigListCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

ConfigListCommand::~ConfigListCommand()
{
}

string ConfigListCommand::getCommandName() const
{
	return "list";
}

void ConfigListCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfBadArguments(arguments);
		stringstream resultMessage;
		resultMessage << generateHeader();
		resultMessage << generateRows();
		setResultMessage(resultMessage.str());
		setResultCode(ESIF_OK);
	}
	catch (const command_failure& e)
	{
		setResultCode(e.getErrorCode());
		setResultMessage(e.getDescription());
	}
}

string ConfigListCommand::generateHeader()
{
	stringstream resultMessage;
	resultMessage << left << setw(COLUMN_NAME_WIDTH) << "Name"s;
	resultMessage << left << setw(COLUMN_SEGMENTS_WIDTH) << "Segs"s;
	resultMessage << left << setw(COLUMN_DATA_WIDTH) << "Bytes"s;
	resultMessage << left << setw(COLUMN_PATH_WIDTH) << "Source File Path"s;
	resultMessage << endl;
	resultMessage << setw(MAX_WIDTH) << setfill('-') << '-';
	resultMessage << endl;
	return resultMessage.str();
}

string ConfigListCommand::generateRows() const
{
	throwIfInvalidConfigurationManager(m_dptfManager->getConfigurationManager());
	const auto configManager = m_dptfManager->getConfigurationManager();
	const auto names = configManager->getContentNames();
	stringstream rows;
	if (names.empty())
	{
		rows << "<No config files loaded>"s << endl;
	}
	else
	{
		for (const auto& name : names)
		{
			const auto configSpace = configManager->getContent(name);
			const auto& sourceFilePath = configSpace->metaData().sourceFilePath;
			const auto numSegments = to_string(configSpace->dataSegments().size());
			const auto numTotalBytes = to_string(configSpace->metaData().dataLengthInBytes);
			const auto row = generateRow(
				{name, numSegments, numTotalBytes, sourceFilePath}, 
				{COLUMN_NAME_WIDTH, COLUMN_SEGMENTS_WIDTH, COLUMN_DATA_WIDTH, COLUMN_PATH_WIDTH});
			rows << row << endl;
		}
	}
	return rows.str();
}

string ConfigListCommand::generateRow(const vector<string>& values, const vector<unsigned>& widths)
{
	throwIfValueCountInvalid(values, widths);

	stringstream row;
	for (unsigned i = 0; i < values.size(); ++i)
	{
		string value(values[i]);
		if (value.size() > widths[i])
		{
			// chop off the first chunk of characters from the left
			const auto offset = values[i].size() - widths[i] + 4;
			value = "..."s + value.substr(offset);
		}
		value.resize(widths[i] - 1, ' ');
		row << left << setw(widths[i]) << value;
	}
	return row.str();
}


void ConfigListCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() != 1)
	{
		const auto description = R"(
			Invalid argument count given to 'config list' command. Run 'dptf help' command for more information.)";
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}

void ConfigListCommand::throwIfInvalidConfigurationManager(
	const shared_ptr<ConfigurationFileManagerInterface>& configurationManager)
{
	if (!configurationManager)
	{
		throw logic_error("Invalid configuration manager");
	}
}

void ConfigListCommand::throwIfValueCountInvalid(const vector<string>& values, const vector<unsigned>& widths)
{
	if (values.size() != widths.size())
	{
		throw command_failure(ESIF_E_UNSPECIFIED, "Unexpected error generating result data."s);
	}
}
