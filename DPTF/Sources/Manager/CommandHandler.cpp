/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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
#include "CommandHandler.h"
#include "esif_ccb_string.h"
#include "EsifTime.h"
#include <time.h>

void CommandHandler::loadVariables(const UInt32 argCount, const EsifDataPtr argv, DptfManagerInterface* manager)
{
	argc = argCount;
	argvRaw = argv;
	dptfManager = manager;
}

// Returns the number of arguments successfully loaded to argvString.
UInt32 CommandHandler::loadStringVariables()
{
	string temp;
	UInt32 count;
	for (count = 0; count < argc; count++)
	{
		if ((temp = getArgumentAsString(count)) != "\0")
		{
			argvString.push_back(temp);
		}
		else
		{
			break;
		}
	}
	return count;
}

// Returns the number of arguments successfully freed from argvString.
UInt32 CommandHandler::emptyStringVariables()
{
	UInt32 count = 0;
	while (!argvString.empty())
	{
		argvString.pop_back();
		count++;
	}
	return count;
}

string CommandHandler::getArgumentAsString(UInt32 argNumber)
{
	string argString;
	if (argvRaw != NULL && argNumber < argc && argvRaw[argNumber].type == ESIF_DATA_STRING)
	{
		argString = (const char*)argvRaw[argNumber].buf_ptr;
	}
	else
	{
		argString = "\0";
	}
	return argString;
}

pair<esif_error_t, string> CommandHandler::formatXml(pair<esif_error_t, string> result)
{
	return result;
}

pair<string, int> CommandHandler::createFilename(void)
{
	string filename;
	string extension = ".txt";
	pair<string, int> result;
	char datetime[MAX_FILENAME] = {0};
	int defaultFileName = 0;

	if ((argvString[argc - 2] == "filename") && (argvString[argc - 1].find_first_of("*<>:/\\|?", 0) == string::npos) && (argvString[argc - 1].find_first_not_of('.') != string::npos))
	{
		filename = argvString[argc - 1];
	}
	else
	{
		defaultFileName = 1;
		EsifTime timestamp;
		auto localTime = timestamp.getLocalTime();
		esif_ccb_sprintf(sizeof(datetime), datetime, "DPTF_Settings_From_%04d-%02d-%02d-%02d%02d%02d",
			localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday, localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
		filename = datetime;
	}
	if (filename.find_first_of('.', 0) == string::npos)
	{
		filename += extension;
	}
	result.first = filename;
	result.second = defaultFileName;
	return result;
}

pair<esif_error_t, string> CommandHandler::writeToFile(pair<esif_error_t, string> result, string format)
{
	/*
	Create filename to write to.
	If filename provided, use filename. Otherwise format filename with
	local date and time (DPTF_Settings_From_yyyy-mm-dd-hhmmss).
	*/

	fstream fp;
	string writeToFile = result.second;
	pair<string, int> filename = createFilename();
	string filePath = dptfManager->getDptfReportDirectoryPath() + filename.first;
	result.second = "";
	if (filename.second)
	{
		result.second = "No valid filename provided. Continuing with default filename.\n";
	}

	// Open file
	fp.open(filePath, ios::out);
	if (!fp.is_open())
	{
		result.first = ESIF_E_IO_OPEN_FAILED;
		result.second += "Could not open " + filename.first + "at location " + filePath + ".\n";
		goto exit;
	}

	fp << writeToFile;

	if (result.first == ESIF_OK)
	{
		result.second += "DPTF Configuration written to " + filePath + "\n";
	}

	fp.close();

exit:
	return result;
}

CommandHandler::~CommandHandler()
{
}