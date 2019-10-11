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
#include "CommandHandler.h"
#include "FileIO.h"

class dptf_export DiagPolicyCommand : public CommandHandler
{
public:
	DiagPolicyCommand(DptfManagerInterface* dptfManager, std::shared_ptr<IFileIO> fileIo);
	virtual ~DiagPolicyCommand();
	virtual std::string getCommandName() const override;
	virtual void execute(const CommandArguments& arguments) override;

private:
	void throwIfBadArguments(const CommandArguments& arguments);
	void throwIfPolicyNotExist(const CommandArguments& arguments);
	void throwIfReportNameIsInvalid(const CommandArguments& arguments);
	Bool reportNameProvided(const CommandArguments& arguments);
	std::string getPolicyDiagnosticReport(const CommandArguments& arguments);
	std::string generateReportPath(const CommandArguments& arguments);
	std::shared_ptr<IFileIO> m_fileIo;
};
