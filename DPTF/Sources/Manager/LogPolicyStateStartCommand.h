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
#include "FileIo.h"

class dptf_export LogPolicyStateStartCommand : public CommandHandler
{
public:
	LogPolicyStateStartCommand(DptfManagerInterface* dptfManager, const std::shared_ptr<IFileIo>& fileIo);
	std::string getCommandName() const override;
	void execute(const CommandArguments& arguments) override;

private:
	std::shared_ptr<IFileIo> m_fileIo;
	void throwIfInvalidArguments(const CommandArguments& arguments) const;
	static void throwIfInvalidArgumentCount(const CommandArguments& arguments);
	static void throwIfInvalidStartArgument(const CommandArguments& arguments);
	static void throwIfPolicyArgumentIsEmpty(const CommandArguments& arguments);
	void throwIfInvalidSamplePeriodArgument(const CommandArguments& arguments) const;
	bool isSamplePeriodInvalid(const std::string& sampleTime) const;
	void startLogger(const CommandArguments& arguments) const;
};
