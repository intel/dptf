/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#include "ReloadCommand.h"
#include "DptfManagerInterface.h"
#include "WorkItemQueueManagerInterface.h"
#include "ParticipantManagerInterface.h"
#include "PolicyManagerInterface.h"
#include "EsifDataString.h"

using namespace std;

ReloadCommand::ReloadCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

ReloadCommand::~ReloadCommand()
{
}

std::string ReloadCommand::getCommandName() const
{
	return "reload";
}

void ReloadCommand::execute(const CommandArguments& arguments)
{

	throwIfBadArguments(arguments);
	if (arguments[1].getDataAsString() == "policies")
	{
		auto policyManager = m_dptfManager->getPolicyManager();
		auto participantIndexList = m_dptfManager->getParticipantManager()->getParticipantIndexes();
		unbindAllParticipants(participantIndexList);
		recreateAllPolicies(policyManager);
		bindAllParticipants(participantIndexList);
		setResultCode(ESIF_OK);
		setResultMessage("DPTF Policies Reloaded");
	}
}

void ReloadCommand::bindAllParticipants(const std::set<UIntN>& participantIndexList)
{
	for (auto participantIndex = participantIndexList.begin(); participantIndex != participantIndexList.end();
		 ++participantIndex)
	{
		m_dptfManager->bindParticipantToPolicies(*participantIndex);
		m_dptfManager->bindDomainsToPolicies(*participantIndex);
	}
}

void ReloadCommand::recreateAllPolicies(PolicyManagerInterface* policyManager)
{
	try
	{
		policyManager->destroyAllPolicies();
		policyManager->getSupportedPolicyList()->update();
		policyManager->createAllPolicies(m_dptfManager->getDptfPolicyDirectoryPath());
	}
	catch (const dptf_exception& ex)
	{
		setResultMessage(std::string("Failed to reload all policies. ") + ex.getDescription());
	}
}

void ReloadCommand::unbindAllParticipants(const std::set<UIntN>& participantIndexList)
{
	for (auto participantIndex = participantIndexList.begin(); participantIndex != participantIndexList.end();
		 ++participantIndex)
	{
		m_dptfManager->unbindDomainsFromPolicies(*participantIndex);
		m_dptfManager->unbindParticipantFromPolicies(*participantIndex);
	}
}

void ReloadCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		string description = string("Invalid argument count given.");
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

	if (arguments[1].isDataTypeString() == false)
	{
		string description = string("Invalid argument type given.  Expected a string.");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}

	if (arguments[1].getDataAsString().compare("policies") != 0)
	{
		string description = string("Invalid argument given for reload target.");
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}
