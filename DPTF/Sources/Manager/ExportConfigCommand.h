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
#include "CommandDispatcher.h"
#include "DptfManagerInterface.h"
#include "PolicyManagerInterface.h"
#include "FileIo.h"

class dptf_export ExportConfigCommand : public CommandHandler
{
public:
	ExportConfigCommand(DptfManagerInterface* dptfManager, const std::shared_ptr<IFileIo>& fileIo);
	~ExportConfigCommand() override;
	std::string getCommandName() const override;
	void execute(const CommandArguments& arguments) override;

private:
	std::shared_ptr<CommandDispatcher> m_ExportConfigCommandDispatcher;
	std::list<std::shared_ptr<CommandHandler>> m_subCommands;

	void exportPolicyConfiguration(const std::string& policyName, const std::string& fileName) const;
	std::string getResultString(const std::string& fileName) const;
	void throwIfInvalidArguments(const CommandArguments& arguments) const;
	void throwIfInvalidCommand(const CommandArguments& arguments) const;
	void throwIfInvalidSubCommand(const CommandArguments& arguments) const;
	void throwIfInvalidPolicyName(const std::string& policyName) const;
	void throwIfInvalidFileNameArgument(const std::string& fileName) const;
	std::string driverDataFilePath(const std::string& fileName) const;
	static void throwIfInvalidArgumentData(const CommandArguments& arguments);
	static void throwIfInvalidArgumentSubData(const CommandArguments& arguments);
	static void throwIfInvalidArgumentCount(const CommandArguments& arguments);
	std::shared_ptr<IFileIo> m_fileIo;
};
