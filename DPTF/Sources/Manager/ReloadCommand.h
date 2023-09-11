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
#pragma once
#include "CommandHandler.h"
#include "PolicyManagerInterface.h"

class dptf_export ReloadCommand : public CommandHandler
{
public:
	ReloadCommand(DptfManagerInterface* dptfManager);
	std::string getCommandName() const override;
	void reloadSinglePolicy(const std::string& policyName);
	void execute(const CommandArguments& arguments) override;

private:
	void reloadAllPolicies();
	void throwIfBadArguments(const CommandArguments& arguments);
	void bindAllParticipants(const std::set<UIntN>& participantIndexList) const;
	void unbindAllParticipants(const std::set<UIntN>& participantIndexList) const;
	void recreateAllPolicies(PolicyManagerInterface* policyManager);
};
