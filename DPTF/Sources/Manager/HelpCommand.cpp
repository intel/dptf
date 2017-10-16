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
using namespace std;

pair<esif_error_t, string> HelpCommand::executeCommand()
{
	pair<esif_error_t, string> response;
	response.first = ESIF_OK;
	response.second = 
		"\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
		"DDDDDD    PPPPPP   TTTTTTTT  FFFFFFF      HH    HH  EEEEEE  LL       PPPPPP\n"
		"DD   DD   PP   PP     TT     FF           HH    HH  EE      LL       PP   PP\n"
		"DD    DD  PPPPPP      TT     FFFFF        HHHHHHHH  EEEE    LL       PPPPPP\n"
		"DD   DD   PP          TT     FF           HH    HH  EE      LL       PP           Format:Text\n"
		"DDDDDD    PP          TT     FF           HH    HH  EEEEEE  LLLLLLL  PP           Version " VERSION_STR "\n\n"
		"Key:  <>-Required parameters\n"
		"      []-Optional parameters\n"
		"       |-Choice of parameters\n"
		"     ...-Repeated parameters\n\n"
		"GENERAL COMMANDS:\n"
		"help                                                                    Displays text about available commands in DPTF\n"
		"reload policies                                                         Reloads all policies in DPTF\n"
		"diag all                                                                Runs diagnostics for all policies and all participants in DPTF.\n"
		"diag <policy policy_name>|<part part_name> ... [filename file_name]     Runs diagnostics for a particular policy or a particular participant in DPTF.\n"
		"                                                                        ***Currently Supported Policies***\n"
		"                                                                              Critical Policy (crt)\n"
		"                                                                        ***Currently Supported Participants***\n"
		"                                                                              None\n"
		"diag <policy|part> list                                                 List all policies or participants that are currently supported under diag.\n"
		"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	return response;
}

HelpCommand::~HelpCommand() 
{
}
