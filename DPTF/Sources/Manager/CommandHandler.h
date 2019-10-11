/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#include "CommandArguments.h"
class DptfManagerInterface;

class dptf_export CommandHandler
{
public:
	CommandHandler(DptfManagerInterface* dptfManager);
	virtual ~CommandHandler();
	virtual void execute(const CommandArguments& arguments) = 0;
	virtual std::string getCommandName() const = 0;
	virtual std::string getLastExecutionMessage();
	virtual eEsifError getLastExecutionResultCode();

protected:
	void setResultMessage(const std::string& message);
	void setResultCode(const eEsifError resultCode);
	std::string m_resultMessage;
	eEsifError m_resultCode;
	DptfManagerInterface* m_dptfManager;
};
