/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "TableObjectType.h"

class dptf_export TableObjectSetCommand : public CommandHandler
{
public:
	TableObjectSetCommand(DptfManagerInterface* dptfManager);
	virtual ~TableObjectSetCommand();
	virtual std::string getCommandName() const override;
	virtual void execute(const CommandArguments& arguments) override;

private:
	void throwIfBadArguments(const CommandArguments& arguments);
	void throwIfTableObjectNotExist(const CommandArguments& arguments);
	void throwIfFailedToAllocateMemory(char* tableValue);
	void throwIfFailedToAllocateMemory(UInt8* tableValue);
	void setTableObjectXmlString(const CommandArguments& arguments);
	void convertToBinaryAndSet(
		TableObjectType::Type tableType,
		const char* textInput,
		std::string uuid,
		std::string dvName,
		std::string key);
	UInt32 extractInteger(const esif_string str);
	void setTableData(
		UInt32 totalBytesNeeded,
		UInt8* tableMem,
		TableObjectType::Type tableType,
		std::string uuid,
		std::string dvName,
		std::string key);
};
