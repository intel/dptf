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
#include "ControlFactoryType.h"
#include "UiSubCommand.h"
class XmlNode;

class dptf_export UiGetModuleDataCommand : public UiSubCommand
{
public:
	UiGetModuleDataCommand(DptfManagerInterface* dptfManager);
	virtual ~UiGetModuleDataCommand();
	virtual std::string getCommandName() const override;
	virtual void execute(const CommandArguments& arguments) override;

private:
	static void throwIfBadArgumentCount(const CommandArguments& arguments);
	static void throwIfBadArgumentData(const CommandArguments& arguments);
	static void throwIfBadGroupNumber(const CommandArguments& arguments);
	void throwIfBadModuleNumber(const CommandArguments& arguments) const;
	void throwIfBadPolicyModule(const CommandArguments& arguments) const;
	void throwIfBadParticipantModule(const CommandArguments& arguments) const;
	static void throwIfBadFrameworkModule(const CommandArguments& arguments);
	static void throwIfBadArbitratorModule(const CommandArguments& arguments);
	static void throwIfBadSystemModule(const CommandArguments& arguments);

	std::string getModuleData(const UInt32 group, const UInt32 module) const;
	std::string getModuleDataForPolicy(const UInt32 module) const;
	std::string getModuleDataForParticipant(const UInt32 module) const;
	std::string getModuleDataForFramework(const UInt32 module) const;
	std::string getModuleDataForFrameworkPolicies() const;
	void addXmlForFrameworkPolicy(const std::shared_ptr<XmlNode>& root) const;
	std::shared_ptr<XmlNode> getXmlForFrameworkLoadedPolicies() const;
	std::shared_ptr<XmlNode> getXmlForPlatformRequests() const;
	std::string getModuleDataForFrameworkParticipants() const;
	std::shared_ptr<XmlNode> getXmlForFrameworkLoadedParticipants() const;
	std::string getModuleDataForFrameworkEvents() const;
	std::string getModuleDataForFrameworkStatistics() const;
	void addXmlForArbitratorPolicy(ControlFactoryType::Type type, const std::shared_ptr<XmlNode>& root) const;
	std::string getModuleDataForArbitrator(const UInt32 module) const;
	std::string getModuleDataForSystem(const UInt32 module) const;
};
