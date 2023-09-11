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
#include "TableObjectField.h"
#include "TableObjectType.h"
#include "DataVaultType.h"
#include "DptfBuffer.h"
#include "esif_sdk_primitive_type.h"
#include "XmlNode.h"

class dptf_export TableObject
{
public:
	TableObject(
		TableObjectType::Type type,
		const std::map<UInt64, std::vector<TableObjectField>>& fieldsMap,
		const std::vector<std::pair<DataVaultType::Type, std::string>>& dataVaultPathForGet,
		const std::vector<std::pair<DataVaultType::Type, std::string>>& dataVaultPathForSet,
		const std::set<UInt64>& revisionsUsingEsifDataVariant,
		UInt32 supportedMode = 0,
		esif_primitive_type_t readPrimitive = (esif_primitive_type_t)0,
		Bool isParticipantTable = false,
		Bool hasRevisionField = true);
	~TableObject();

	TableObjectType::Type getType() const;
	const std::map<UInt64, std::vector<TableObjectField>>& getFieldsMap() const;
	std::vector<std::pair<DataVaultType::Type, std::string>> dataVaultPathForGet() const;
	std::vector<std::pair<DataVaultType::Type, std::string>> dataVaultPathForSet() const;
	esif_primitive_type_t getReadTablePrimitive() const;

	const DptfBuffer& getData() const;
	void setData(const DptfBuffer& data);

	std::string getXmlString();
	std::shared_ptr<XmlNode> getXml();

	Bool isParticipantTable() const;
	Bool hasRevisionField() const;
	Bool hasModeField() const;
	Bool isUsingEsifDataVariant(UInt64 revision) const;

private:

	void addRevisionFields(
		UInt32& remain_bytes,
		esif_data_variant*& obj,
		std::vector<TableObjectField>& fields,
		const std::shared_ptr<XmlNode>& resultRoot,
		UInt32 revision) const;

	void addModeFields(
		UInt32& remain_bytes,
		esif_data_variant*& obj,
		const std::shared_ptr<XmlNode>& resultRoot) const;

	Bool addValueFields(
		UInt32& remain_bytes,
		esif_data_variant*& obj,
		const std::vector<TableObjectField>& fields,
		const std::shared_ptr<XmlNode>& resultRoot,
		UInt32 revision) const;

	Bool addValueFieldsWithEsifDataVariant(
		UInt32& remain_bytes,
		esif_data_variant*& obj,
		const std::vector<TableObjectField>& fields,
		const std::shared_ptr<XmlNode>& resultRoot) const;

	Bool addValueFieldsWithOutEsifDataVariant(
		UInt32& remain_bytes,
		esif_data_variant*& obj,
		const std::vector<TableObjectField>& fields,
		const std::shared_ptr<XmlNode>& resultRoot) const;

	static void addMessage(
		const std::shared_ptr<XmlNode>& resultRoot,
		const std::string& xmlWrapperTag,
		const std::string& messages);

	TableObjectType::Type m_type;
	std::map<UInt64, std::vector<TableObjectField>> m_fieldsMap;
	std::vector<std::pair<DataVaultType::Type, std::string>> m_dataVaultPathForGet;
	std::vector<std::pair<DataVaultType::Type, std::string>> m_dataVaultPathForSet;
	std::set<UInt64> m_revisionsUsingEsifDataVariant;
	UInt32 m_supportedMode;
	esif_primitive_type m_readPrimitive;
	DptfBuffer m_data;
	Bool m_isParticipantTable;
	Bool m_hasRevisionField;
};
