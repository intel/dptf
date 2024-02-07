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

#include "TableObject.h"
#include "StatusFormat.h"
using namespace std;

TableObject::TableObject(
	TableObjectType::Type type,
	const map<UInt64, vector<TableObjectField>>& fieldsMap,
	const vector<pair<DataVaultType::Type, string>>& dataVaultPathForGet,
	const vector<pair<DataVaultType::Type, string>>& dataVaultPathForSet,
	const set<UInt64>& revisionsUsingEsifDataVariant,
	UInt32 supportedMode,
	esif_primitive_type_t readPrimitive,
	Bool isParticipantTable,
	Bool hasRevisionField)
	: m_type(type)
	, m_fieldsMap(fieldsMap)
	, m_dataVaultPathForGet(dataVaultPathForGet)
	, m_dataVaultPathForSet(dataVaultPathForSet)
	, m_revisionsUsingEsifDataVariant(revisionsUsingEsifDataVariant)
	, m_supportedMode(supportedMode)
	, m_readPrimitive(readPrimitive)
	, m_isParticipantTable(isParticipantTable)
	, m_hasRevisionField(hasRevisionField)
{
}

TableObject::~TableObject()
{
}

const DptfBuffer& TableObject::getData() const
{
	return m_data;
}

void TableObject::setData(const DptfBuffer& data)
{
	m_data = data;
}

TableObjectType::Type TableObject::getType() const
{
	return m_type;
}

const map<UInt64, vector<TableObjectField>>& TableObject::getFieldsMap() const
{
	return m_fieldsMap;
}

vector<pair<DataVaultType::Type, string>> TableObject::dataVaultPathForGet() const
{
	return m_dataVaultPathForGet;
}

vector<pair<DataVaultType::Type, string>> TableObject::dataVaultPathForSet() const
{
	return m_dataVaultPathForSet;
}

esif_primitive_type_t TableObject::getReadTablePrimitive() const
{
	return m_readPrimitive;
}

string TableObject::getXmlString()
{
	return getXml()->toString(); 
}

std::shared_ptr<XmlNode> TableObject::getXml()
{
	std::shared_ptr<XmlNode> resultRoot = XmlNode::createWrapperElement("result"); 	
	if (m_data.size())
	{
		auto obj = reinterpret_cast<union esif_data_variant*>(m_data.get());
		auto remain_bytes = m_data.size();
		auto revision = static_cast<u32>(obj->integer.value);

		if (m_fieldsMap.find(0) != m_fieldsMap.end())
		{
			revision = 0;
		}

		vector<TableObjectField> fields;

		if (m_fieldsMap.find(revision) != m_fieldsMap.end())
		{
			addRevisionFields(remain_bytes, obj, fields, resultRoot, revision);
			addModeFields(remain_bytes, obj, resultRoot);
			if (addValueFields(remain_bytes, obj, fields, resultRoot, revision) == false)
			{
				addMessage(resultRoot, "row", "TableObject field datatype not supported.");
			}
		}
		else
		{
			addMessage(resultRoot, "revision", "TableObject revision not supported.");
		}
	}
	else
	{
		addMessage(resultRoot, "message", "TableObject is empty.");
	}
	return resultRoot;
}

void TableObject::addMessage(
	const shared_ptr<XmlNode>& resultRoot,
	const string& xmlWrapperTag,
	const string& messages)
{
	resultRoot->addChild(XmlNode::createDataElement(xmlWrapperTag, messages));
}

void TableObject::addRevisionFields(
	UInt32& remain_bytes,
	esif_data_variant*& obj,
	vector<TableObjectField>& fields,
	const shared_ptr<XmlNode>& resultRoot,
	UInt32 revision) const
{
	if (hasRevisionField())
	{
		obj = reinterpret_cast<union esif_data_variant*>(reinterpret_cast<u8*>(obj) + sizeof(*obj));
		remain_bytes -= sizeof(*obj);
		resultRoot->addChild(XmlNode::createDataElement("revision", StatusFormat::friendlyValue(revision)));
		fields = m_fieldsMap.find(revision)->second;
	}
	else
	{
		const auto result = m_fieldsMap.find(0);
		if (result != m_fieldsMap.end())
		{
			fields = result->second;
		}
		else
		{
			throw dptf_exception("Failed to find field mapping for revision 0"s);
		}
	}
}

void TableObject::addModeFields(
	UInt32& remain_bytes,
	esif_data_variant*& obj,
	const shared_ptr<XmlNode>& resultRoot) const
{
	if (m_supportedMode)
	{
		const auto mode = (u64)obj->integer.value;
		obj = reinterpret_cast<union esif_data_variant*>(reinterpret_cast<u8*>(obj) + sizeof(*obj));
		remain_bytes -= sizeof(*obj);
		resultRoot->addChild(XmlNode::createDataElement("mode", StatusFormat::friendlyValue(mode)));
	}
}

Bool TableObject::addValueFields(
	UInt32& remain_bytes,
	esif_data_variant*& obj,
	const vector<TableObjectField>& fields,
	const shared_ptr<XmlNode>& resultRoot,
	UInt32 revision) const
{
	if (isUsingEsifDataVariant(revision))
	{
		return addValueFieldsWithEsifDataVariant(remain_bytes, obj, fields, resultRoot);
	}
	else
	{
		return addValueFieldsWithOutEsifDataVariant(remain_bytes, obj, fields, resultRoot);
	}
}

Bool TableObject::addValueFieldsWithEsifDataVariant(
	UInt32& remain_bytes,
	esif_data_variant*& obj,
	const vector<TableObjectField>& fields,
	const shared_ptr<XmlNode>& resultRoot) const
{
	while (remain_bytes >= sizeof(*obj))
	{
		const auto rowRoot = XmlNode::createWrapperElement("row");
		for (auto& field : fields)
		{
			remain_bytes -= sizeof(*obj);
			if (obj->type == ESIF_DATA_UINT64)
			{
				const auto int64FieldValue = (u64)obj->integer.value;
				obj = reinterpret_cast<union esif_data_variant*>(reinterpret_cast<u8*>(obj) + sizeof(*obj));
				rowRoot->addChild(
					XmlNode::createDataElement(field.m_fieldLabel, StatusFormat::friendlyValue(int64FieldValue)));
			}
			else if (obj->type == ESIF_DATA_UINT32)
			{
				const auto int32FieldValue = static_cast<u32>(obj->integer.value);
				obj = reinterpret_cast<union esif_data_variant*>(reinterpret_cast<u8*>(obj) + sizeof(*obj));
				rowRoot->addChild(
					XmlNode::createDataElement(field.m_fieldLabel, StatusFormat::friendlyValue(int32FieldValue)));
			}
			else if (obj->type == ESIF_DATA_STRING)
			{
				const char* strFieldValue = reinterpret_cast<char*>(reinterpret_cast<u8*>(obj) + sizeof(*obj));
				remain_bytes -= obj->string.length;
				obj = reinterpret_cast<union esif_data_variant*>(reinterpret_cast<u8*>(obj) + (sizeof(*obj) + obj->string.length));
				rowRoot->addChild(XmlNode::createDataElement(field.m_fieldLabel, strFieldValue));
			}
			else
			{
				return false;
			}
		}
		resultRoot->addChild(rowRoot);
	}

	return true;
}

Bool TableObject::addValueFieldsWithOutEsifDataVariant(
	UInt32& remain_bytes,
	esif_data_variant*& obj,
	const vector<TableObjectField>& fields,
	const shared_ptr<XmlNode>& resultRoot) const
{
	auto data = reinterpret_cast<char*>(obj);

	while (remain_bytes > 0)
	{
		const auto rowRoot = XmlNode::createWrapperElement("row");
		for (auto& field : fields)
		{
			if (field.m_fieldDataType == ESIF_DATA_UINT64)
			{
				const auto int64FieldValue = reinterpret_cast<UInt64*>(data);
				remain_bytes -= sizeof(*int64FieldValue);
				data = data + sizeof(*int64FieldValue);
				rowRoot->addChild(
					XmlNode::createDataElement(field.m_fieldLabel, StatusFormat::friendlyValue(*int64FieldValue)));
			}
			else if (field.m_fieldDataType == ESIF_DATA_UINT32)
			{
				const auto int32FieldValue = reinterpret_cast<UInt32*>(data);
				remain_bytes -= sizeof(*int32FieldValue);
				data = data + sizeof(*int32FieldValue);
				rowRoot->addChild(
					XmlNode::createDataElement(field.m_fieldLabel, StatusFormat::friendlyValue(*int32FieldValue)));
			}
			else if (field.m_fieldDataType == ESIF_DATA_STRING)
			{
				const auto strFieldValue = data;
				remain_bytes -= field.m_fieldLength;
				data = data + field.m_fieldLength;
				rowRoot->addChild(XmlNode::createDataElement(field.m_fieldLabel, strFieldValue));
			}
			else
			{
				return false;
			}
		}
		resultRoot->addChild(rowRoot);
	}

	return true;
}

Bool TableObject::isParticipantTable() const
{
	return m_isParticipantTable;
}

Bool TableObject::hasRevisionField() const
{
	return m_hasRevisionField;
}

Bool TableObject::hasModeField() const
{
	return (m_supportedMode > 0);
}

Bool TableObject::isUsingEsifDataVariant(UInt64 revision) const
{
	return m_revisionsUsingEsifDataVariant.find(revision) != m_revisionsUsingEsifDataVariant.end();
}