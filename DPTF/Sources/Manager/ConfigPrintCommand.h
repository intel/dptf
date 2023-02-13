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
#include <memory>
#include "CommandHandler.h"
#include "ConfigurationFileContent.h"

class ConfigurationFileManagerInterface;

class dptf_export ConfigPrintCommand : public CommandHandler
{
public:
	ConfigPrintCommand(DptfManagerInterface* dptfManager);
	virtual ~ConfigPrintCommand();
	std::string getCommandName() const override;
	void execute(const CommandArguments& arguments) override;

protected:
	virtual std::string getPrintedContent(const std::shared_ptr<ConfigurationFileContentInterface>& cs) const;
	virtual void throwIfBadArguments(const CommandArguments& arguments) const;
	virtual void throwIfInvalidConfigurationManager(
		const std::shared_ptr<ConfigurationFileManagerInterface>& configurationManager) const;
};
