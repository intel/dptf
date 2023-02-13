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
#include "CommandHandler.h"
#include "esif_ccb_string.h"
#include "EsifTime.h"
#include <time.h>
#include <fstream>

using namespace std;

string defaultSuccessMessage("The command completed successfully.");

CommandHandler::CommandHandler(DptfManagerInterface* dptfManager)
	: m_resultMessage(defaultSuccessMessage)
	, m_resultCode(ESIF_OK)
	, m_dptfManager(dptfManager)
{
}

CommandHandler::~CommandHandler()
{
}

string CommandHandler::getLastExecutionMessage() const
{
	return m_resultMessage;
}

void CommandHandler::setResultMessage(const string& message)
{
	m_resultMessage = message;
}

void CommandHandler::setDefaultResultMessage()
{
	m_resultMessage = defaultSuccessMessage;
}

eEsifError CommandHandler::getLastExecutionResultCode() const
{
	return m_resultCode;
}

void CommandHandler::setResultCode(const eEsifError resultCode)
{
	m_resultCode = resultCode;
}
