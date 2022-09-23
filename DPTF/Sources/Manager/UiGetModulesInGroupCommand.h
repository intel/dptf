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
#pragma once
#include "UiSubCommand.h"

class dptf_export UiGetModulesInGroupCommand : public UiSubCommand
{
public:
	UiGetModulesInGroupCommand(DptfManagerInterface* dptfManager);
	virtual ~UiGetModulesInGroupCommand();
	virtual std::string getCommandName() const override;
	
	virtual void execute(const CommandArguments& arguments) override;

private:
	static void throwIfBadArgumentNumber(const CommandArguments& arguments);
	static void throwIfBadArgumentData(const CommandArguments& arguments);
	static void throwIfBadGroupNumber(const CommandArguments& arguments);
	std::string getModulesInGroup(UInt32 groupNumber) const;
	std::string getPoliciesGroup() const;
	std::string getParticipantsGroup() const;
	std::string getFrameworkGroup() const;
	std::string getArbitratorGroup() const;
	std::string getSystemGroup() const;
};
