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

DiagParticipant::DiagParticipant()
{
	partEnum["list"] = lstPart;
}

string DiagParticipant::listSupportedParticipants()
{
	return ("Supported Participants:\n"
		"    None\n"
		);
}

pair<esif_error_t, string> DiagParticipant::executeCommand()
{
	pair<esif_error_t, string> response;
	UInt32 temp = 0;
	response.first = ESIF_OK;
	if (argc < 3 || (temp = loadStringVariables()) != argc)
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
		auto it = partEnum.find(argvString[2]);
		if (it == partEnum.end())
		{
			response.first = ESIF_E_NOT_IMPLEMENTED;
			response.second = "DiagParticipant Not Implemented.\n";
		}
		else
		{
			switch (partEnum[argvString[2]])
			{
				case lstPart:
					response.first = ESIF_E_UNSPECIFIED;
					response.second = listSupportedParticipants();
					break;
				default:
					response.first = ESIF_E_NOT_IMPLEMENTED;
					response.second = "DiagParticipant Not Implemented.\n";;
					break;
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

DiagParticipant::~DiagParticipant()
{
}