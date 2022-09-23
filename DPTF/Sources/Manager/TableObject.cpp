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

#include "TableObject.h"
#include "StatusFormat.h"
using namespace std;

TableObject::TableObject(
	TableObjectType::Type type,
	map<UInt64, vector<TableObjectField>> fieldsMap,
	vector<pair<DataVaultType::Type, string>> dataVaultPathForGet,
	vector<pair<DataVaultType::Type, string>> dataVaultPathForSet,
	std::set<UInt64> revisionsUsingEsifDataVariant,
	UInt32 supportedMode,
	esif_primitive_type_t readPrimitive,
	Bool isParticipantTable)
	: m_type(type)
	, m_fieldsMap(fieldsMap)
	, m_dataVaultPathForGet(dataVaultPathForGet)
	, m_dataVaultPathForSet(dataVaultPathForSet)
	, m_revisionsUsingEsifDataVariant(revisionsUsingEsifDataVariant)
	, m_supportedMode(supportedMode)
	, m_readPrimitive(readPrimitive)
	, m_isParticipantTable(isParticipantTable)
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
	if (m_data.size())
	{
		union esif_data_variant* obj = (union esif_data_variant*)m_data.get();

		auto remain_bytes = m_data.size();
		auto resultRoot = XmlNode::createWrapperElement("result");

		auto revision = (u32)obj->integer.value;

		if (m_fieldsMap.find(0) != m_fieldsMap.end())
		{
			revision = 0;
		}

		vector<TableObjectField> fields;

		if (m_fieldsMap.find(revision) != m_fieldsMap.end())
		{
			addRevisionFields(remain_bytes, obj, fields, resultRoot, revision);
			addModeFields(remain_bytes, obj, fields, resultRoot);

			if (addValueFields(remain_bytes, obj, fields, resultRoot, revision) == false)
			{
				return "TableObject field datatype not supported.";
			}
		
			return resultRoot->toString();
		}
		else
		{
			return "TableObject revision not supported.";
		}
	}
	else
	{
		return "TableObject is empty.";
	}
}

void TableObject::addRevisionFields(
	UInt32& remain_bytes,
	esif_data_variant*& obj,
	vector<TableObjectField>& fields,
	shared_ptr<XmlNode>& resultRoot,
	UInt32 revision)
{
	if (m_fieldsMap.find(revision) != m_fieldsMap.end() && revision != 0)
	{
		obj = (union esif_data_variant*)((u8*)obj + sizeof(*obj));
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
	vector<TableObjectField>& fields,
	shared_ptr<XmlNode>& resultRoot)
{
	if (m_supportedMode)
	{
		auto mode = (u64)obj->integer.value;
		obj = (union esif_data_variant*)((u8*)obj + sizeof(*obj));
		remain_bytes -= sizeof(*obj);
		resultRoot->addChild(XmlNode::createDataElement("mode", StatusFormat::friendlyValue(mode)));
	}
}

Bool TableObject::addValueFields(
	UInt32& remain_bytes,
	esif_data_variant*& obj,
	vector<TableObjectField>& fields,
	shared_ptr<XmlNode>& resultRoot,
	UInt32 revision)
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
	vector<TableObjectField>& fields,
	shared_ptr<XmlNode>& resultRoot)
{
	while (remain_bytes >= sizeof(*obj))
	{
		auto rowRoot = XmlNode::createWrapperElement("row");
		for (auto field = fields.begin(); field != fields.end(); field++)
		{
			remain_bytes -= sizeof(*obj);
			if (obj->type == ESIF_DATA_UINT64)
			{
				UInt64 int64FieldValue = (u64)obj->integer.value;
				obj = (union esif_data_variant*)((u8*)obj + sizeof(*obj));
				rowRoot->addChild(
					XmlNode::createDataElement(field->m_fieldLabel, StatusFormat::friendlyValue(int64FieldValue)));
			}
			else if (obj->type == ESIF_DATA_UINT32)
			{
				UInt32 int32FieldValue = (u32)obj->integer.value;
				obj = (union esif_data_variant*)((u8*)obj + sizeof(*obj));
				rowRoot->addChild(
					XmlNode::createDataElement(field->m_fieldLabel, StatusFormat::friendlyValue(int32FieldValue)));
			}
			else if (obj->type == ESIF_DATA_STRING)
			{
				char* strFieldValue = (char*)((u8*)obj + sizeof(*obj));
				remain_bytes -= obj->string.length;
				obj = (union esif_data_variant*)((u8*)obj + (sizeof(*obj) + obj->string.length));
				rowRoot->addChild(XmlNode::createDataElement(field->m_fieldLabel, strFieldValue));
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
	vector<TableObjectField>& fields,
	shared_ptr<XmlNode>& resultRoot)
{
	char* data = (char*)obj;

	while (remain_bytes > 0)
	{
		auto rowRoot = XmlNode::createWrapperElement("row");
		for (auto field = fields.begin(); field != fields.end(); field++)
		{
			if (field->m_fieldDataType == ESIF_DATA_UINT64)
			{
				UInt64* int64FieldValue = reinterpret_cast<UInt64*>(data);
				remain_bytes -= sizeof(*int64FieldValue);
				data = data + sizeof(*int64FieldValue);
				rowRoot->addChild(
					XmlNode::createDataElement(field->m_fieldLabel, StatusFormat::friendlyValue(*int64FieldValue)));
			}
			else if (field->m_fieldDataType == ESIF_DATA_UINT32)
			{
				UInt32* int32FieldValue = reinterpret_cast<UInt32*>(data);
				remain_bytes -= sizeof(*int32FieldValue);
				data = data + sizeof(*int32FieldValue);
				rowRoot->addChild(
					XmlNode::createDataElement(field->m_fieldLabel, StatusFormat::friendlyValue(*int32FieldValue)));
			}
			else if (field->m_fieldDataType == ESIF_DATA_STRING)
			{
				char* strFieldValue = data;
				remain_bytes -= field->m_fieldLength;
				data = data + field->m_fieldLength;
				rowRoot->addChild(XmlNode::createDataElement(field->m_fieldLabel, strFieldValue));
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

const Bool TableObject::isParticipantTable() const
{
	return m_isParticipantTable;
}

const Bool TableObject::hasRevisionField() const
{
	return m_fieldsMap.find(0) == m_fieldsMap.end();
}

const Bool TableObject::hasModeField() const
{
	return (m_supportedMode > 0);
}

const Bool TableObject::isUsingEsifDataVariant(UInt64 revision) const
{
	return m_revisionsUsingEsifDataVariant.find(revision) != m_revisionsUsingEsifDataVariant.end();
}