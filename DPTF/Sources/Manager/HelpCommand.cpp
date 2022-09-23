/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
	const string message =
		"DPTF Help Command. Version " VERSION_STR " [" ESIF_ATTR_OS " " ESIF_PLATFORM_TYPE " " ESIF_BUILD_TYPE "]"
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
		"                                      Runs diagnostics on a participant\n"
		"dptf tableobject get <tablename> [dynamic policy uuid]\n"
		"                                      Gets table from DataVault\n"
		"dptf tableobject get <tablename> <datavault> <key>\n"
		"                                      Gets table from alternative DataVault source and key\n"
		"dptf tableobject get <tablename> <participant name>\n"
		"                                      Gets participant table from DataVault\n"
		"dptf tableobject set <tablename> <value> [dynamic policy uuid]\n"
		"                                      Sets table value to DataVault\n"
		"dptf tableobject set <tablename> <value> <datavault> <key>\n"
		"                                      Sets table value to alternative DataVault source and key\n"
		"dptf tableobject set <tablename> <participant name> <value>\n"
		"                                      Sets participant table value to DataVault\n"
		"dptf tableobject delete <tablename> [dynamic policy uuid]\n"
		"                                      Deletes table from DataVault\n"
		"dptf tableobject delete <tablename> [dynamic policy uuid] <all>\n"
		"                                      Deletes table from all DataVaults\n"
		"dptf tableobject delete <tablename> <participant name>\n"
		"                                      Deletes participant table from DataVault\n"
		"dptf tableobject delete <tablename> <participant name> <all>\n"
		"                                      Deletes participant table from all DataVault\n"
		"dptf config delete <datavault> <key>\n"
		"                                      Deletes DataVault key\n"
		"dptf ui getgroups\n"
		"                                      Returns list of high level groups for UI menu\n"
		"dptf ui getmodulesingroup <group ID>\n"
		"                                      Returns list of module IDs for the groupID given\n"
		"dptf ui getmoduledata <group ID> <module ID>\n"
		"                                      Returns detailed XML data for a given group ID and module ID\n"
		"dptf capture [file name]              Export settings to file specified\n";

	setResultMessage(message);
}
