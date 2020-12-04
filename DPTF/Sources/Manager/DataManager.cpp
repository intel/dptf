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

#include "DataManager.h"
#include "EsifServicesInterface.h"
using namespace TableObjectType;
using namespace std;

DataManager::DataManager(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
{
	loadTableObjectMap();
}

DataManager::~DataManager()
{
}

TableObject DataManager::getTableObject(TableObjectType::Type tableType)
{
	if (tableObjectExists(tableType))
	{
		auto table = m_tableObjectMap.find(tableType)->second;
		auto dataVaultPaths = table.getDataVaultPathMap().find("get")->second;
		for (auto path = dataVaultPaths.begin(); path != dataVaultPaths.end(); path++)
		{
			string nameSpace = path->first;
			string elementPath = path->second;
			try
			{
				DptfBuffer data =
					m_dptfManager->getEsifServices()->readConfigurationBinaryFromNameSpace(nameSpace, elementPath);
				table.setData(data);
				return table;
			}
			catch (...)
			{
			}
		}
	}

	throw dptf_exception("The table object specified was not found.");
}

void DataManager::setTableObject(string tableString)
{
}

Bool DataManager::tableObjectExists(TableObjectType::Type tableType)
{
	auto table = m_tableObjectMap.find(tableType);
	if (table != m_tableObjectMap.end())
	{
		return true;
	}
	return false;
}

void DataManager::loadTableObjectMap()
{
	m_tableObjectMap.insert(
	{TableObjectType::Apat, {TableObjectType::Apat, {
		{"fld1", "fld1", ESIF_DATA_UINT64},
		{"fld2", "fld2", ESIF_DATA_STRING},
		{"fld3", "fld3", ESIF_DATA_STRING},
		{"fld4", "fld4", ESIF_DATA_UINT64},
		{"fld5", "fld5", ESIF_DATA_STRING},
		{"fld6", "fld6", ESIF_DATA_STRING}}}});
}
