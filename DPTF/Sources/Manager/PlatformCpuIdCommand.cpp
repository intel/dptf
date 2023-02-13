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
#include "PlatformCpuIdCommand.h"

#include "DptfManagerInterface.h"
#include "EnvironmentProfiler.h"

using namespace std;

#define CPUID_STEPPING_BIT_OFFSET 4

PlatformCpuIdCommand::PlatformCpuIdCommand(DptfManagerInterface* dptfManager)
	: CommandHandler(dptfManager)
{
}

PlatformCpuIdCommand::~PlatformCpuIdCommand()
{
}

string PlatformCpuIdCommand::getCommandName() const
{
	return "getCpuId";
}

void PlatformCpuIdCommand::execute(const CommandArguments& arguments)
{
	try
	{
		throwIfInvalidArgumentCount(arguments);
		const auto environmentProfile = m_dptfManager->getEnvironmentProfile();
		setResultMessage(environmentProfile.cpuIdWithoutStepping);
		setResultCode(ESIF_OK);
	}
	catch (const command_failure& e)
	{
		setResultCode(e.getErrorCode());
		setResultMessage(e.getDescription());
	}
}

string PlatformCpuIdCommand::convertToHexString(UInt64 cpuId)
{
	stringstream stream;
	stream << hex << cpuId;
	string cpuIdHex = stream.str();

	for (auto& c : cpuIdHex)
	{
		c = (char)toupper(c);
	}
	return cpuIdHex;
}

void PlatformCpuIdCommand::throwIfInvalidArgumentCount(const CommandArguments& arguments)
{
	if (arguments.size() > 1)
	{
		string description = string(
			"No argument required for 'getCpuId' command. "
			"Run 'dptf help' command for more information.");

		throw command_failure(ESIF_E_INVALID_ARGUMENT_COUNT, description);
	}
}
