/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
#include "DptfBuffer.h"

class dptf_export TableObjectInterface
{
public:
	virtual ~TableObjectInterface(){};
	virtual TableObjectType::Type getType() const = 0;
	virtual const std::map<std::string, std::vector<std::pair<std::string, std::string>>> getDataVaultPathMap() const = 0;
	virtual const DptfBuffer& getData() const = 0;
	virtual void setData(const DptfBuffer& data) = 0;
	virtual std::string getXmlString() = 0;
};

class dptf_export TableObject : public TableObjectInterface
{
public:
	TableObject(TableObjectType::Type type, std::vector<TableObjectField> fields);
	~TableObject();

	TableObjectType::Type getType() const override;
	const std::map<std::string, std::vector<std::pair<std::string, std::string>>> getDataVaultPathMap() const override;

	const DptfBuffer& getData() const override;
	void setData(const DptfBuffer& data) override;

	std::string getXmlString() override;

private:
	TableObjectType::Type m_type;
	std::vector<TableObjectField> m_fields;

	DptfBuffer m_data;
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> m_dataVaultPathMap;
};
