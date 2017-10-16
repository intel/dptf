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
#include "EsifDataString.h"
#include "DptfVer.h"
#include "DptfManagerInterface.h"
#include "WIPolicyReloadAll.h"
#include "WorkItemQueueManagerInterface.h"
using namespace std;

pair<esif_error_t, string> ReloadCommand::executeCommand()
{
	UInt32 cmdCount = 2;
	UInt32 temp = 0;
	pair<esif_error_t, string> response;
	response.first = ESIF_OK;
	if ((temp = loadStringVariables()) != argc)
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
		if (argc == cmdCount && argvString[1] == "policies") {
			// reload policies
			try
			{
				WorkItem* workItem = new WIPolicyReloadAll(dptfManager);
				dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
				response.second = "DPTF Policies Reloaded\n";
			}
			catch (...)
			{
				response.first = ESIF_E_UNSPECIFIED;
				response.second = "Failed to reload policies.\n";
			}
		}
		else {
			response.first = ESIF_E_COMMAND_DATA_INVALID;
			response.second = "Command not supported. Type 'app cmd dptf help' for available commands.\n";
		}

		if (emptyStringVariables() != argc)
		{
			response.first = ESIF_E_UNSPECIFIED;
			response.second += "String variable not successfully emptied. May cause complications with future calls to reload.\n";
		}
	}
	return response;
}

ReloadCommand::~ReloadCommand()
{
}