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
#include "Commands.h"
#include "Diag.h"
using namespace std;

map<string, DiagCommand*> diagCommandDispatch;

DiagCommand::DiagCommand()
{
}

pair<esif_error_t, string> DiagCommand::executeCommand()
{
	UInt32 temp = 0;
	pair<esif_error_t, string> response;
	response.first = ESIF_OK;
	if (argc <= 1 || (temp = loadStringVariables()) != argc)
	{
		response.first = ESIF_E_INVALID_ARGUMENT_COUNT;
		response.second = "Invalid input type. Type 'app cmd dptf help' for available commands.\n";
		if (emptyStringVariables() != temp)
		{
			response.first = ESIF_E_UNSPECIFIED;
			response.second += "String variable not successfully emptied. Terminating...\n";
		}
	}
	else
	{
		auto it = diagCommandDispatch.find(argvString[1]);
		if (it == diagCommandDispatch.end())
		{
			response.first = ESIF_E_COMMAND_DATA_INVALID;
			response.second = "Command not supported. Type 'app cmd dptf help' for available commands.\n";
		}
		else
		{
			diagCommandDispatch[argvString[1]]->loadVariables(argc, argvRaw, dptfManager);
			response = diagCommandDispatch[argvString[1]]->executeCommand();
			if (response.first == ESIF_OK)
			{
				response = writeToFile(response, FORMAT_XML);
			}
		}
		if (emptyStringVariables() != argc)
		{
			response.first = ESIF_E_UNSPECIFIED;
			response.second += "String variable not successfully emptied. Terminating...\n";
		}
	}
	
	return response;
}

DiagCommand::~DiagCommand()
{
}