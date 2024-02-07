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

#include "LogPolicyStateStartCommand.h"
#include "PolicyStateLogger.h"
#include "StringParser.h"

#define LOGGER_START_NAME "start"s
namespace ArgumentIndex
{
	enum ArgumentIndexEnum
	{
		CommandName = 0,
		FileName = 1,
		SamplePeriod = 2,
		Policies = 3,
		TotalArguments
	};
}

using namespace std;

LogPolicyStateStartCommand::LogPolicyStateStartCommand(DptfManagerInterface* dptfManager, const shared_ptr<IFileIo>& fileIo)
	: CommandHandler(dptfManager)
	, m_fileIo(fileIo)
{
}

string LogPolicyStateStartCommand::getCommandName() const
{
	return LOGGER_START_NAME;
}

void LogPolicyStateStartCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfInvalidArguments(arguments);
		startLogger(arguments);
		setResultCode(ESIF_OK);
		setResultMessage("Successfully started logger");
	}
	catch (command_failure& ex)
	{
		setResultCode(ex.getErrorCode());
		setResultMessage("Command Failed: " + ex.getDescription());
	}
}

void LogPolicyStateStartCommand::throwIfInvalidArguments(const CommandArguments& arguments) const
{
	throwIfInvalidArgumentCount(arguments);
	throwIfInvalidStartArgument(arguments);
	throwIfPolicyArgumentIsEmpty(arguments);
	throwIfInvalidSamplePeriodArgument(arguments);
}

void LogPolicyStateStartCommand::startLogger(const CommandArguments& arguments) const
{
	const auto policyArgument = StringParser::split(arguments[ArgumentIndex::Policies].getDataAsString(), ',');
	const auto fileName = arguments[ArgumentIndex::FileName].getDataAsString();
	const auto samplePeriod = TimeSpan::createFromMilliseconds(arguments[ArgumentIndex::SamplePeriod].getDataAsString());

	try
	{
		m_dptfManager->getPolicyStateLogger()->start(policyArgument, fileName, samplePeriod);
	}
	catch (const dptf_exception& ex)
	{
		throw command_failure(ESIF_E_INVALID_REQUEST_TYPE, ex.getDescription());
	}
}

void LogPolicyStateStartCommand::throwIfInvalidArgumentCount(const CommandArguments& arguments)
{
	if (arguments.size() != ArgumentIndex::TotalArguments)
	{
		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, "Invalid number of arguments for command: "s + LOGGER_START_NAME);
	}
}

void LogPolicyStateStartCommand::throwIfInvalidStartArgument(const CommandArguments& arguments)
{
	if (arguments[ArgumentIndex::CommandName].getDataAsString() != LOGGER_START_NAME)
	{
		throw command_failure(
			ESIF_E_INVALID_REQUEST_TYPE, "Invalid argument given: "s + arguments[ArgumentIndex::CommandName].getDataAsString());
	}
}

void LogPolicyStateStartCommand::throwIfPolicyArgumentIsEmpty(const CommandArguments& arguments)
{
	if (arguments[ArgumentIndex::Policies].getDataAsString().length() == 0)
	{
		throw command_failure(
			ESIF_E_INVALID_REQUEST_TYPE, "Empty Policy Argument."s);
	}
}

 void LogPolicyStateStartCommand::throwIfInvalidSamplePeriodArgument(const CommandArguments& arguments) const
{
	if (isSamplePeriodInvalid(arguments[ArgumentIndex::SamplePeriod].getDataAsString()))
	{
		 throw command_failure(
			 ESIF_E_INVALID_REQUEST_TYPE, "Invalid sample period: "s + arguments[ArgumentIndex::SamplePeriod].getDataAsString());
	}
}

 bool LogPolicyStateStartCommand::isSamplePeriodInvalid(const string& sampleTime) const
{
	if (sampleTime.empty() || all_of(sampleTime.begin(), sampleTime.end(), ::isdigit) == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}