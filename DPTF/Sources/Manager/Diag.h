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
#include <map>

enum Policies 
{
	crt,							/* Critical Policy */
	lstPlcy							/* List all currentlysupported policies*/
};

enum Participants
{
	lstPart							/* List all currentlysupported policies*/
};

static map <string, Policies> policyEnum;
static map <string, Participants> partEnum;

class dptf_export DiagAll : public DiagCommand
{
public:
	pair<esif_error_t, string> executeCommand() override;
	~DiagAll();
};

class dptf_export DiagPolicy : public DiagCommand
{
public:
	DiagPolicy();
	string listSupportedPolicies();
	Policy* getPolicyPointer(string policyName);
	pair<esif_error_t, string> executeCommand() override;
	pair<esif_error_t, string> executeCritical();
	~DiagPolicy();
};

class dptf_export DiagParticipant : public DiagCommand
{
public:
	DiagParticipant();
	string listSupportedParticipants();
	pair<esif_error_t, string> executeCommand() override;
	~DiagParticipant();
};
