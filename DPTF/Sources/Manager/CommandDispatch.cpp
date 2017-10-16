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
#include "CommandDispatch.h"
#include "Diag.h"
#include <map>

map<string, CommandHandler*> commandDispatch;
extern map<string, DiagCommand*> diagCommandDispatch;

CommandDispatch::CommandDispatch()
{
	commandDispatch["help"] = new HelpCommand();																		
	commandDispatch["reload"] = new ReloadCommand();																	
	commandDispatch["diag"] = new DiagCommand();																		
	diagCommandDispatch["all"] = new DiagAll();																			
	diagCommandDispatch["policy"] = new DiagPolicy();																	
	diagCommandDispatch["part"] = new DiagParticipant();
}

pair<esif_error_t, string> CommandDispatch::dispatchCommand(string appcmd, UInt32 argc, EsifDataPtr argv, DptfManagerInterface *dptfManager)
{
	pair<esif_error_t, string> result;
	result.first = ESIF_OK;
	auto it = commandDispatch.find(appcmd);
	if (it == commandDispatch.end())
	{
		result.second = "Command not supported. Type 'app cmd dptf help' for available commands.\n";
	}
	else
	{
		commandDispatch[appcmd]->loadVariables(argc, argv, dptfManager);
		result = commandDispatch[appcmd]->executeCommand();
	}
	return result;
}

CommandDispatch::~CommandDispatch()
{
	DELETE_MEMORY(commandDispatch["help"]);																				
	DELETE_MEMORY(commandDispatch["reload"]);																			
	DELETE_MEMORY(commandDispatch["diag"]);																				
	DELETE_MEMORY(diagCommandDispatch["all"]);																			
	DELETE_MEMORY(diagCommandDispatch["policy"]);																		
	DELETE_MEMORY(diagCommandDispatch["part"]);
}