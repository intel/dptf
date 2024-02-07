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
#include "PolicyManagerInterface.h"
#include "CaptureDataGenerator.h"
#include "FileIo.h"

class dptf_export CaptureCommand : public CommandHandler
{
public:
	CaptureCommand(
		DptfManagerInterface* dptfManager,
		const std::shared_ptr<IFileIo>& fileIo,
		const std::shared_ptr<TimeStampGenerator>& timeStampGenerator);	
	std::string getCommandName() const override;
	void execute(const CommandArguments& arguments) override;

private:
	std::shared_ptr<IFileIo> m_fileIo;
	std::shared_ptr<TimeStampGenerator> m_timeStampGenerator;
	std::list<std::shared_ptr<CaptureDataGenerator>> m_dataGenerators;
	static Bool exportFileNameProvided(const CommandArguments& arguments);
	std::string generateExportPath(const CommandArguments& arguments) const;
	std::string getExportFileName(const CommandArguments& arguments) const;
	std::string generateCaptureData() const;
	static void throwIfBadArgumentCount(const CommandArguments& arguments);
	static void throwIfBadFileNameGiven(const CommandArguments& arguments);
};
