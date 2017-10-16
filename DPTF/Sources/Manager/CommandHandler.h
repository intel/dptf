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

#pragma once
#include "esif_ccb_rc.h"
#include "esif_sdk_data.h"
#include "DptfManagerInterface.h"
#include "ParticipantManagerInterface.h"
#include "PolicyManagerInterface.h"
#include "WorkItem.h"
#include <fstream>

using namespace std;

#define MAX_FILENAME 100
#define FORMAT_XML "xml"

class dptf_export CommandHandler
{
public:
	void loadVariables(const UInt32 argCount, const EsifDataPtr argv, DptfManagerInterface *manager);
	string getArgumentAsString(UInt32 argNumber);
	UInt32 loadStringVariables ();
	UInt32 emptyStringVariables();
	pair<esif_error_t, string> writeToFile(pair<esif_error_t, string> result, string format);
	virtual pair<esif_error_t, string> executeCommand(void) = 0;
	virtual ~CommandHandler();
protected:
	UInt32 argc = 0;
	EsifDataPtr argvRaw = NULL;
	vector<string> argvString;
	DptfManagerInterface *dptfManager;
	ParticipantManagerInterface *participantManager;
	PolicyManagerInterface *policyManager;
private:
	pair<string, int> createFilename(void);
	pair<esif_error_t, string> formatXml(pair<esif_error_t, string> result);
};
