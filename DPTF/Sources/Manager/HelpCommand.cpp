/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
#include "HelpCommand.h"
#include "EsifDataString.h"
#include "DptfVer.h"
using namespace std;

HelpCommand::HelpCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

HelpCommand::~HelpCommand()
{
}

std::string HelpCommand::getCommandName() const
{
	return "help";
}

void HelpCommand::execute(const CommandArguments& arguments)
{
	// TODO:  possibly require each command to report its own help and this
	// command would just compile all of the helps for each command
	string message = "DPTF Help Command.  Application version " VERSION_STR
					 "\n"
					 "Key:  <>  Required parameters\n"
					 "      []  Optional parameters\n"
					 "\n"
					 "help                                  Shows help info for available commands\n"
					 "reload policies                       Reloads all policies\n"
					 "diag all                              Runs diagnostics on all policies and\n"
					 "                                      participants\n"
					 "diag policy <policy name> [file name]\n"
					 "                                      Runs diagnostics on a policy\n"
					 "diag part <participant name> [file name]\n"
					 "                                      Runs diagnostics on a participant\n";
	setResultMessage(message);
}
