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

string HelpCommand::getCommandName() const
{
	return "help";
}

void HelpCommand::execute(const CommandArguments& arguments)
{
	// TODO:  possibly require each command to report its own help and this
	// command would just compile all of the helps for each command

	const string title = "DTT Help Command. Version " VERSION_STR " [" ESIF_ATTR_OS " " ESIF_PLATFORM_TYPE " " ESIF_BUILD_TYPE "]";
	const string key = R"(
Key:  <>  Required parameters
      []  Optional parameters
)";
	const string commands = getCommandHelpText();

	setResultMessage(title + key + commands);
}

std::string HelpCommand::getCommandHelpText() const
{
	return R"(
help                                                          Shows help info for available commands
reload policies                                               Reloads all policies
reload policy <policy name>                                   Reloads a single policy.  Use '' around names with spaces
policies enabled                                              Returns a list of policies that are currently enabled
diag all                                                      Runs diagnostics on all policies and participants
diag policy <policy name> [file name]                         Runs diagnostics on a policy
diag part <participant name> [file name]                      Runs diagnostics on a participant
tableobject get <tablename> [dynamic policy uuid]             Gets table from DataVault
tableobject get <tablename> <datavault> <key>                 Gets table from alternative DataVault source and key
tableobject get <tablename> <participant name>                Gets participant table from DataVault
tableobject set <tablename> <value> [dynamic policy uuid]     Sets table value to DataVault
tableobject set <tablename> <value> <datavault> <key>         Sets table value to alternative DataVault source and key
tableobject set <tablename> <participant name> <value>        Sets participant table value to DataVault
tableobject delete <tablename> [dynamic policy uuid]          Deletes table from DataVault
tableobject delete <tablename> [dynamic policy uuid] <all>    Deletes table from all DataVaults
tableobject delete <tablename> <participant name>             Deletes participant table from DataVault
tableobject delete <tablename> <participant name> <all>       Deletes participant table from all DataVaults
config delete <datavault> <key>                               Deletes DataVault key
config filterdb <configuration name> <regex>                  Displays config content filtered by regex as key-values
config list                                                   Shows all loaded configurations
config print <configuration name>                             Displays contents of a configuration as JSON
config printdb <configuration name>                           Displays contents of a configuration as key-values
config reload                                                 Rescans directories for configurations
ui getgroups                                                  Returns groups for top level UI menu
ui getmodulesingroup <group ID>                               Returns module IDs for group ID
ui getmoduledata <group ID> <module ID>                       Returns XML data for group ID and module ID
capture [file name]                                           Export settings to file
getCpuId                                                      Returns platform CPU ID without stepping
)";
}