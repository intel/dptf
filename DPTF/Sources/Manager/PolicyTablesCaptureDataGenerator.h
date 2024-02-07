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
#pragma once
#include "DptfManagerInterface.h"
#include "CaptureDataGenerator.h"
#include "XmlNode.h"
#include "DataManager.h"

class dptf_export PolicyTablesCaptureDataGenerator : public CaptureDataGenerator
{
public:
	PolicyTablesCaptureDataGenerator(DptfManagerInterface* dptfManager);
	std::shared_ptr<XmlNode> generate() const override;

private:
	void addDefaultPolicyTableDetails(TableObjectType::Type tableType, const std::shared_ptr<XmlNode>& root) const;
	void addDefaultTableResults(TableObjectType::Type tableType, const std::shared_ptr<XmlNode>& root) const;
	void addNamedPolicyTableDetails(TableObjectType::Type tableType, const std::shared_ptr<XmlNode>& root) const;
	void addNamedTableResults(
		std::string TableName,
		TableObjectType::Type tableType,
		const std::shared_ptr<XmlNode>& root) const;
	std::vector<std::string> getNamedTables(TableObjectType::Type tableType) const;
};
