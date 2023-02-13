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

#include "WIRunCommand.h"

WIRunCommand::WIRunCommand(DptfManagerInterface* dptfManager, const CommandArguments& arguments)
	: WorkItem(dptfManager, FrameworkEvent::DptfCommand)
	, m_arguments(arguments)
	, m_message(Constants::EmptyString)
	, m_errorCode(ESIF_OK)
{
}

WIRunCommand::~WIRunCommand(void)
{
}

void WIRunCommand::onExecute(void)
{
	writeWorkItemStartingInfoMessage();

	try
	{
		getDptfManager()->getCommandDispatcher()->dispatch(m_arguments);
		m_message = getDptfManager()->getCommandDispatcher()->getLastSuccessfulCommandMessage();
		m_errorCode = getDptfManager()->getCommandDispatcher()->getLastReturnCode();
	}
	catch (const command_failure& cf)
	{
		m_message = cf.getDescription();
		m_errorCode = cf.getErrorCode();
	}
	catch (const std::exception& ex)
	{
		m_message = ex.what();
		m_errorCode = ESIF_E_UNSPECIFIED;
	}
	catch (...)
	{
		m_message = "An unknown error occurred when processing app command.";
		m_errorCode = ESIF_E_UNSPECIFIED;
	}
}

const std::string& WIRunCommand::getLastMessage() const
{
	return m_message;
}

std::string WIRunCommand::getLastMessageWithNewline() const
{
	if (((getLastMessage().size() > 0) && (getLastMessage()[getLastMessage().size() - 1] != '\n'))
		|| (getLastMessage().size() == 0))
	{
		return getLastMessage() + std::string("\n");
	}
	else
	{
		return getLastMessage();
	}
}

esif_error_t WIRunCommand::getLastErrorCode() const
{
	return m_errorCode;
}
