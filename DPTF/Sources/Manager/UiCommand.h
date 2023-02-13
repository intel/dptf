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

class CommandDispatcher;

class dptf_export UiCommand : public CommandHandler
{
public:
	UiCommand(DptfManagerInterface* dptfManager);
	virtual ~UiCommand();
	virtual std::string getCommandName() const override;
	virtual void execute(const CommandArguments& arguments) override;

private:
	void throwIfInvalidCommand(const CommandArguments& arguments) const;
	static void throwIfInvalidArgumentData(const CommandArguments& arguments);
	static void throwIfInvalidArgumentCount(const CommandArguments& arguments);

	void registerSubCommands();
	void createSubCommands();
	std::shared_ptr<CommandDispatcher> m_commandDispatcher;
	std::list<std::shared_ptr<CommandHandler>> m_subCommands;
};
