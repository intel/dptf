/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include <memory>
#include "FileIo.h"
#include "TimeStampGenerator.h"

class CommandDispatcher;

class DiagCommand : public CommandHandler
{
public:
	DiagCommand(
		DptfManagerInterface* dptfManager,
		const std::shared_ptr<IFileIo>& fileIo,
		const std::shared_ptr<TimeStampGenerator>& timeStampGenerator);
	~DiagCommand() override;
	std::string getCommandName() const override;
	void execute(const CommandArguments& arguments) override;

private:
	void throwIfInvalidCommand(const CommandArguments& arguments);
	void throwIfInvalidArgumentData(const CommandArguments& arguments);
	void throwIfInvalidArgumentCount(const CommandArguments& arguments);

	void registerSubCommands() const;
	void createSubCommands();
	std::shared_ptr<CommandDispatcher> m_diagCommandDispatcher;
	std::list<std::shared_ptr<CommandHandler>> m_subCommands;
	std::shared_ptr<IFileIo> m_fileIo;
	std::shared_ptr<TimeStampGenerator> m_timeStampGenerator;
};
