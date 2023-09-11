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
#include "ReloadCommand.h"
#include "DptfManagerInterface.h"
#include "WorkItemQueueManagerInterface.h"
#include "ParticipantManagerInterface.h"
#include "PolicyManagerInterface.h"
#include "EsifDataString.h"

using namespace std;

const auto ReloadAllPoliciesCommandName = "policies"s;
const auto ReloadSinglePolicyCommandName = "policy"s;

ReloadCommand::ReloadCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

string ReloadCommand::getCommandName() const
{
	return "reload"s;
}

void ReloadCommand::execute(const CommandArguments& arguments)
{
	throwIfBadArguments(arguments);
	if (arguments[1].getDataAsString() == ReloadAllPoliciesCommandName)
	{
		reloadAllPolicies();
	}
	else if (arguments[1].getDataAsString() == ReloadSinglePolicyCommandName)
	{
		const auto policyName = arguments[2].getDataAsString();
		reloadSinglePolicy(policyName);
	}
	else
	{
		setResultCode(ESIF_E_COMMAND_DATA_INVALID);
		setResultMessage("Invalid command parameters"s);
	}
}

void ReloadCommand::reloadAllPolicies()
{
	const auto policyManager = m_dptfManager->getPolicyManager();
	const auto participantIndexList = m_dptfManager->getParticipantManager()->getParticipantIndexes();
	unbindAllParticipants(participantIndexList);
	recreateAllPolicies(policyManager);
	bindAllParticipants(participantIndexList);
	setResultCode(ESIF_OK);
	setResultMessage("DPTF Policies Reloaded"s);
}

void ReloadCommand::reloadSinglePolicy(const std::string& policyName)
{
	const auto policyManager = m_dptfManager->getPolicyManager();
	if (policyManager->policyExists(policyName))
	{
		policyManager->reloadPolicy(policyName);
		const auto policy = policyManager->getPolicy(policyName);
		m_dptfManager->bindAllParticipantsToPolicy(policy->getPolicyIndex());
		setResultCode(ESIF_OK);
		setResultMessage(policyName + " reloaded"s);
	}
	else
	{
		setResultCode(ESIF_E_NOT_FOUND);
		setResultMessage(policyName + " could not be found."s);
	}
}

void ReloadCommand::bindAllParticipants(const set<UIntN>& participantIndexList) const
{
	for (const auto participantIndex : participantIndexList)
	{
		m_dptfManager->bindParticipantToPolicies(participantIndex);
		m_dptfManager->bindDomainsToPolicies(participantIndex);
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
		setResultMessage("Failed to reload all policies. "s + ex.getDescription());
	}
}

void ReloadCommand::unbindAllParticipants(const set<UIntN>& participantIndexList) const
{
	for (const unsigned int participantIndex : participantIndexList)
	{
		m_dptfManager->unbindDomainsFromPolicies(participantIndex);
		m_dptfManager->unbindParticipantFromPolicies(participantIndex);
	}
}

void ReloadCommand::throwIfBadArguments(const CommandArguments& arguments)
{
	if (arguments.size() < 2)
	{
		const auto description = "Invalid argument count given."s;
		setResultMessage(description);
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}

	if (arguments[1].isDataTypeString() == false)
	{
		const auto description = "Invalid argument type given.  Expected a string."s;
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}

	if (!(arguments[1].getDataAsString() == ReloadAllPoliciesCommandName 
		|| arguments[1].getDataAsString() == ReloadSinglePolicyCommandName))
	{
		const auto description = "Invalid argument given for reload target."s;
		setResultMessage(description);
		throw command_failure(ESIF_E_COMMAND_DATA_INVALID, description);
	}
}
