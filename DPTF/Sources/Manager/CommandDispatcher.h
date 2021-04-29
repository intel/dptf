/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include <string>
#include <map>
#include "esif_ccb_rc.h"
#include "DptfExport.h"
#include "CommandArguments.h"
#include "CommandHandler.h"
#include <memory>

class dptf_export ICommandDispatcher
{
public:
	virtual ~ICommandDispatcher(){};
	virtual void dispatch(const CommandArguments& arguments) = 0;
	virtual void registerHandler(const std::string commandName, std::shared_ptr<CommandHandler> handler) = 0;
	virtual void unregisterHandler(const std::string commandName) = 0;
	virtual std::string getLastSuccessfulCommandMessage() const = 0;
	virtual eEsifError getLastReturnCode() const = 0;
};

// TODO: rename files to CommandDispatcher as well
class CommandDispatcher : public ICommandDispatcher
{
public:
	CommandDispatcher();
	virtual ~CommandDispatcher();
	void dispatch(const CommandArguments& arguments) override;
	void registerHandler(const std::string commandName, std::shared_ptr<CommandHandler> handler) override;
	void unregisterHandler(const std::string commandName) override;
	virtual std::string getLastSuccessfulCommandMessage() const override;
	virtual eEsifError getLastReturnCode() const override;

private:
	void throwIfBadArguments(const CommandArguments& arguments);
	std::map<std::string, std::shared_ptr<CommandHandler>> m_registeredCommands;
	std::string m_lastSuccessfulCommandMessage;
	eEsifError m_lastCommandReturnCode;
};
