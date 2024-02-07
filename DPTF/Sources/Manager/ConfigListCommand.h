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
#include "ConfigurationFileManager.h"

class dptf_export ConfigListCommand : public CommandHandler
{
public:
	ConfigListCommand(DptfManagerInterface* dptfManager);
	virtual ~ConfigListCommand();
	virtual std::string getCommandName() const override;
	virtual void execute(const CommandArguments& arguments) override;

private:
	static std::string generateHeader();
	std::string generateRows() const;
	static std::string generateRow(const std::vector<std::string>& values, const std::vector<unsigned>& widths);
	static void throwIfBadArguments(const CommandArguments& arguments);
	static void throwIfInvalidConfigurationManager(
		const std::shared_ptr<ConfigurationFileManagerInterface>& configurationManager);
	static void throwIfValueCountInvalid(const std::vector<std::string>& values, const std::vector<unsigned>& widths);
};
