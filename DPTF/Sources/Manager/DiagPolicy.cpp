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

DiagPolicy::DiagPolicy()
{
	policyEnum["crt"] = crt;
	policyEnum["list"] = lstPlcy;
}


string DiagPolicy::listSupportedPolicies()
{
	return ("Supported Policies:\n"
		    "    Critical Policy (crt)\n"
	);
}

Policy* DiagPolicy::getPolicyPointer(string policyName)
{
	Policy* policy;
	policyManager = dptfManager->getPolicyManager();
	policyManager->createAllPolicies(dptfManager->getDptfPolicyDirectoryPath());
	auto allPolicyIndexes = policyManager->getPolicyIndexes();
	for (auto policyIndex = allPolicyIndexes.begin(); policyIndex != allPolicyIndexes.end(); ++policyIndex)
	{
		policy = policyManager->getPolicyPtr(*policyIndex);
		if (policy->getName() == policyName)
		{
			return policy;
		}
	}
	return NULL;
}

pair<esif_error_t, string> DiagPolicy::executeCritical()
{
	Policy* policy;
	pair<esif_error_t, string> response;
	response.first = ESIF_OK;
	if ((policy = getPolicyPointer("Critical Policy")) != NULL)
	{
		response.second = policy->getDiagnosticsAsXml();
	}
	else
	{
		response.first = ESIF_E_NOT_FOUND;
		response.second = "Critical Policy not found.\n";
	}
	return response;
}

pair<esif_error_t, string> DiagPolicy::executeCommand()
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
		auto it = policyEnum.find(argvString[2]);
		if (it == policyEnum.end())
		{
			response.first = ESIF_E_COMMAND_DATA_INVALID;
			response.second = "Policy not currently supported. Type 'app cmd dptf help' for currently supported policies.\n";
		}
		else
		{
			switch (policyEnum[argvString[2]])
			{
				case crt:
					response = executeCritical();
					break;
				case lstPlcy:
					response.first = ESIF_E_UNSPECIFIED;
					response.second = listSupportedPolicies();
					break;
				default:
					response.first = ESIF_E_COMMAND_DATA_INVALID;
					response.second = "Policy not currently supported. Type 'app cmd dptf help' for currently supported policies.\n";
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

DiagPolicy::~DiagPolicy()
{
}