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
#include "PolicyTablesCaptureDataGenerator.h"
#include "DataManager.h"
#include "EsifServicesInterface.h"
#include "DataVaultPath.h"

using namespace std;

PolicyTablesCaptureDataGenerator::PolicyTablesCaptureDataGenerator(DptfManagerInterface* dptfManager)
	: CaptureDataGenerator(dptfManager)
{
}

/*
Example:
<default_psvt>
	<result>
	<revision>
		2
	</revision>
	<row>
		<fld1>\_SB_.PC00.TCPU</fld1>
		<fld2>\_SB_.PC00.TCPU</fld2>
		<fld3>1</fld3>
		<fld4>10</fld4>
		<fld5>3722</fld5>
		<fld6>9</fld6>
		<fld7>65536</fld7>
		<fld8>MAX</fld8>
		<fld9>1000</fld9>
		<fld10>10</fld10>
		<fld11>10</fld11>
		<fld12>0</fld12>
	</row>
	<row>
		<fld1>\_SB_.PC00.TCPU</fld1>
		<fld2>\_SB_.PC00.LPCB.EC0_.SEN1</fld2>
		<fld3>7</fld3>
		<fld4>100</fld4>
		<fld5>3192</fld5>
		<fld6>9</fld6>
		<fld7>65536</fld7>
		<fld8>MAX</fld8>
		<fld9>250</fld9>
		<fld10>10</fld10>
		<fld11>20</fld11>
		<fld12>0</fld12>
	</row>
	<row>
		<fld1>\_SB_.PC00.TCPU</fld1>
		<fld2>\_SB_.PC00.LPCB.EC0_.SEN2</fld2>
		<fld3>7</fld3>
		<fld4>100</fld4>
		<fld5>3232</fld5>
		<fld6>9</fld6>
		<fld7>65536</fld7>
		<fld8>MAX</fld8>
		<fld9>250</fld9>
		<fld10>10</fld10>
		<fld11>20</fld11>
		<fld12>0</fld12>
	</row>
	</result>
</default_psvt>
*/

shared_ptr<XmlNode> PolicyTablesCaptureDataGenerator::generate() const
{
	auto root = XmlNode::createRoot();	
	for (TableObjectType::Type tableType = TableObjectType::FIRST; tableType != TableObjectType::LAST;
			 tableType = (TableObjectType::Type)(tableType + 1))
	{
		addDefaultPolicyTableDetails(tableType, root);
		addNamedPolicyTableDetails(tableType, root);
	}	
	return root;
}

void PolicyTablesCaptureDataGenerator::addDefaultPolicyTableDetails(
	TableObjectType::Type tableType, const shared_ptr<XmlNode>& root) const
{
	try
	{
		const auto tableTypeRoot = XmlNode::createWrapperElement("default_" + TableObjectType::ToString(tableType));
		addDefaultTableResults(tableType, tableTypeRoot); 
		root->addChild(tableTypeRoot);
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

void PolicyTablesCaptureDataGenerator::addDefaultTableResults(
	TableObjectType::Type tableType, const shared_ptr<XmlNode>& root) const
{
	try
	{
		string uuid = Constants::EmptyString;
		auto resultRoot = m_dptfManager->getDataManager()->getTableObject(tableType, uuid).getXml();

		if (resultRoot != nullptr)
		{
			root->addChild(resultRoot);
		}
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

void PolicyTablesCaptureDataGenerator::addNamedPolicyTableDetails(
	TableObjectType::Type tableType,
	const shared_ptr<XmlNode>& root) const
{
	try
	{
		vector<string> tableNames = getNamedTables(tableType);

		for (auto tableName : tableNames)
		{
			const auto tableTypeRoot =
				XmlNode::createWrapperElement(tableName + "_" + TableObjectType::ToString(tableType));
			addNamedTableResults(tableName, tableType, tableTypeRoot);
			root->addChild(tableTypeRoot);
		}
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

vector<string> PolicyTablesCaptureDataGenerator::getNamedTables(TableObjectType::Type tableType) const
{
	vector<string> tableNames;
	string namedTableSearch = DataVaultPathBasePaths::TablesRoot + "/" + TableObjectType::ToString(tableType) + "/*";
	const string esifResponse = m_dptfManager->getEsifServices()->readConfigurationString(
		DataVaultType::ToString(DataVaultType::Dptf), namedTableSearch);

	// Example EsifResponse: /shared/tables/itmt/test1|/shared/tables/itmt/test2
	
	const vector<string> tableKeys = StringParser::split(esifResponse, '|'); 

	for (UIntN tableIndex = 0; tableIndex < tableKeys.size(); tableIndex++)
	{
		// Example: /shared/tables/itmt/test1

		vector<string> singleTableKey = StringParser::split(tableKeys.at(tableIndex), '/');

		for (UIntN index = 0; index < singleTableKey.size() - 1; index++)
		{
			if (singleTableKey[index] == TableObjectType::ToString(tableType))
			{
				tableNames.push_back(singleTableKey[index + 1]);
			}
		}
	}

	return tableNames;
}

void PolicyTablesCaptureDataGenerator::addNamedTableResults(
	string tableName,
	TableObjectType::Type tableType,
	const shared_ptr<XmlNode>& root) const
{
	try
	{
		string tableKey =
			DataVaultPathBasePaths::TablesRoot + "/" +TableObjectType::ToString(tableType) + "/" + tableName;

		auto resultRoot =
			m_dptfManager->getDataManager()
				->getTableObjectBasedOnAlternativeDataSourceAndKey(tableType, DataVaultType::Dptf, tableKey)
				.getXml();

		if (resultRoot != nullptr)
		{
			root->addChild(resultRoot);
		}
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}