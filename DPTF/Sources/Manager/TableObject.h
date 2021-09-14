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
#include "TableObjectField.h"
#include "TableObjectType.h"
#include "DataVaultType.h"
#include "DptfBuffer.h"

class dptf_export TableObject
{
public:
	TableObject(
		TableObjectType::Type type,
		std::vector<TableObjectField> fields,
		std::vector<std::pair<DataVaultType::Type, std::string>> dataVaultPathForGet,
		std::vector<std::pair<DataVaultType::Type, std::string>> dataVaultPathForSet);
	~TableObject();

	TableObjectType::Type getType() const;
	std::vector<TableObjectField> getFields() const;
	std::vector<std::pair<DataVaultType::Type, std::string>> dataVaultPathForGet() const;
	std::vector<std::pair<DataVaultType::Type, std::string>> dataVaultPathForSet() const;

	const DptfBuffer& getData() const;
	void setData(const DptfBuffer& data);

	std::string getXmlString(UInt32 supportedRevision);

private:
	TableObjectType::Type m_type;
	std::vector<TableObjectField> m_fields;
	std::vector<std::pair<DataVaultType::Type, std::string>> m_dataVaultPathForGet;
	std::vector<std::pair<DataVaultType::Type, std::string>> m_dataVaultPathForSet;

	DptfBuffer m_data;
};
