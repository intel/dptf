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

#include "DataManager.h"
#include "EsifServicesInterface.h"
#include "PolicyManagerInterface.h"
#include "DataVaultPath.h"
#include "StringConverter.h"
#include "WorkItem.h"
#include "WorkItemQueueManagerInterface.h"
#include "WIPolicyTableObjectChanged.h"
#include "ParticipantManagerInterface.h"

using namespace TableObjectType;
using namespace std;

DataManager::DataManager(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
	, m_tableObjectMap()
{
	loadTableObjectMap();
}

DataManager::~DataManager()
{
}

TableObject DataManager::getTableObject(TableObjectType::Type tableType, string uuid, UIntN participantIndex)
{
	if (tableObjectExists(tableType))
	{
		auto table = m_tableObjectMap.find(tableType)->second;
		auto dataVaultPaths = table.dataVaultPathForGet();

		for (auto path = dataVaultPaths.begin(); path != dataVaultPaths.end(); path++)
		{
			string nameSpace = DataVaultType::ToString(path->first);
			string elementPath = path->second;

			if (uuid.empty())
			{
				elementPath = StringParser::replaceAll(elementPath, "/UUID", Constants::EmptyString);
			}
			else
			{
				elementPath = StringParser::replaceAll(elementPath, "UUID", StringConverter::toLower(uuid));
			}

			try
			{
				DptfBuffer data;
				if (table.getReadTablePrimitive() != (esif_primitive_type_t)0)
				{
					data = m_dptfManager->getEsifServices()->primitiveExecuteGet(
						table.getReadTablePrimitive(), ESIF_DATA_BINARY, participantIndex);
				}
				else
				{
					data = m_dptfManager->getEsifServices()->readConfigurationBinary(nameSpace, elementPath);
				}
				table.setData(data);
				return table;
			}
			catch (...)
			{
			}
		}

		return table;
	}
	else
	{
		throw dptf_exception("TableObject schema not found.");
	}
}

void DataManager::setTableObject(
	const DptfBuffer& tableData,
	TableObjectType::Type tableType,
	string uuid,
	UIntN participantIndex)
{
	if (!tableObjectExists(tableType))
	{
		throw dptf_exception("TableObject schema not found.");
	}

	auto table = m_dptfManager->getDataManager()->getTableObjectMap().find(tableType)->second;
	auto dataVaultPaths = table.dataVaultPathForSet();
	auto path = dataVaultPaths.begin();
	string nameSpace = DataVaultType::ToString(path->first);
	string elementPath = path->second;
	Bool isParticipantTable = m_dptfManager->getDataManager()->isParticipantTable(tableType);

	if (uuid.empty())
	{
		elementPath = StringParser::replaceAll(elementPath, "/UUID", Constants::EmptyString);
		deleteTableObjectKeyForNoPersist(tableType);
	}
	else
	{
		elementPath = StringParser::replaceAll(elementPath, "UUID", StringConverter::toLower(uuid));
	}

	if (isParticipantTable)
	{
		string participantName =
			m_dptfManager->getParticipantManager()->getParticipantPtr(participantIndex)->getParticipantName();
		elementPath = StringParser::replaceAll(elementPath, "%nm%", participantName + ".D0");
	}

	try
	{
		if (tableData.notEmpty())
		{
			m_dptfManager->getEsifServices()->writeConfigurationBinary(
				tableData.get(), tableData.size(), tableData.size(), nameSpace, elementPath, ESIF_SERVICE_CONFIG_PERSIST);
		}
		else
		{
			writeEmptyTable(nameSpace, elementPath);
		}

		sendTableChangedEvent(tableType, uuid, participantIndex);
	}
	catch (...)
	{
	}
}

void DataManager::deleteTableObject(TableObjectType::Type tableType, string uuid, UIntN participantIndex)
{
	if (!tableObjectExists(tableType))
	{
		throw dptf_exception("TableObject schema not found.");
	}

	auto table = m_dptfManager->getDataManager()->getTableObjectMap().find(tableType)->second;
	auto dataVaultPaths = table.dataVaultPathForSet();
	auto path = dataVaultPaths.begin();
	string nameSpace = DataVaultType::ToString(path->first);
	string elementPath = path->second;
	Bool isParticipantTable = m_dptfManager->getDataManager()->isParticipantTable(tableType);

	if (uuid.empty())
	{
		elementPath = StringParser::replaceAll(elementPath, "/UUID", Constants::EmptyString);
		deleteTableObjectKeyForNoPersist(tableType);
	}
	else
	{
		elementPath = StringParser::replaceAll(elementPath, "UUID", StringConverter::toLower(uuid));
	}

	if (isParticipantTable)
	{
		string participantName =
			m_dptfManager->getParticipantManager()->getParticipantPtr(participantIndex)->getParticipantName();
		elementPath = StringParser::replaceAll(elementPath, "%nm%", participantName + ".D0");
	}

	try
	{
		m_dptfManager->getEsifServices()->deleteConfigurationBinary(nameSpace, elementPath);
		sendTableChangedEvent(tableType, uuid, participantIndex);
	}
	catch (const dptf_exception&)
	{
		// nothing to do
	}
}

void DataManager::deleteAllTableObject(TableObjectType::Type tableType, string uuid, UIntN participantIndex)
{
	if (!tableObjectExists(tableType))
	{
		throw dptf_exception("TableObject schema not found.");
	}

	auto table = m_tableObjectMap.find(tableType)->second;
	auto dataVaultPaths = table.dataVaultPathForGet();
	Bool isParticipantTable = m_dptfManager->getDataManager()->isParticipantTable(tableType);

	for (auto path = dataVaultPaths.begin(); path != dataVaultPaths.end(); path++)
	{
		string nameSpace = DataVaultType::ToString(path->first);
		string elementPath = path->second;

		if (uuid.empty())
		{
			elementPath = StringParser::replaceAll(elementPath, "/UUID", Constants::EmptyString);
		}
		else
		{
			elementPath = StringParser::replaceAll(elementPath, "UUID", StringConverter::toLower(uuid));
		}

		if (isParticipantTable)
		{
			string participantName =
				m_dptfManager->getParticipantManager()->getParticipantPtr(participantIndex)->getParticipantName();
			elementPath = StringParser::replaceAll(elementPath, "%nm%", participantName + ".D0");
		}

		try
		{
			m_dptfManager->getEsifServices()->deleteConfigurationBinary(nameSpace, elementPath);
			sendTableChangedEvent(tableType, uuid, participantIndex);
		}
		catch (...)
		{
		}
	}
}

TableObject DataManager::getTableObjectBasedOnAlternativeDataSourceAndKey(
	TableObjectType::Type tableType,
	DataVaultType::Type dvType,
	string key)
{
	if (tableObjectExists(tableType))
	{
		auto table = m_tableObjectMap.find(tableType)->second;
		string nameSpace = DataVaultType::ToString(dvType);
		string elementPath = key;
		try
		{
			DptfBuffer data = m_dptfManager->getEsifServices()->readConfigurationBinary(nameSpace, elementPath);
			table.setData(data);
		}
		catch (...)
		{
		}

		return table;
	}
	else
	{
		throw dptf_exception("TableObject schema not found.");
	}
}

void DataManager::setTableObjectBasedOnAlternativeDataSourceAndKey(
	const DptfBuffer& tableData,
	TableObjectType::Type tableType,
	DataVaultType::Type dvType,
	string key)
{
	if (!tableObjectExists(tableType))
	{
		throw dptf_exception("TableObject schema not found.");
	}

	string nameSpace = DataVaultType::ToString(dvType);
	string elementPath = key;

	try
	{
		if (tableData.notEmpty())
		{
			m_dptfManager->getEsifServices()->writeConfigurationBinary(
				tableData.get(), tableData.size(), tableData.size(), nameSpace, elementPath, ESIF_SERVICE_CONFIG_PERSIST);
		}
		else
		{
			writeEmptyTable(nameSpace, elementPath);
		}
		
	}
	catch (...)
	{
	}
}

void DataManager::setTableObjectForNoPersist(DptfBuffer tableData, TableObjectType::Type tableType)
{
	if (!tableObjectExists(tableType))
	{
		throw dptf_exception("TableObject schema not found.");
	}

	// /nopersist/IETM.D0/test
	auto elementPath = "/nopersist/%nm%/" + TableObjectType::ToString(tableType);
	elementPath = StringParser::replaceAll(elementPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");

	auto dvType = DataVaultType::Override;

	m_dptfManager->getEsifServices()->writeConfigurationBinary(
		tableData.get(), tableData.size(), tableData.size(), DataVaultType::ToString(dvType), elementPath, 0);
	sendTableChangedEvent(tableType, Constants::EmptyString, Constants::Esif::NoParticipant);
}

void DataManager::deleteTableObjectKeyForNoPersist(TableObjectType::Type tableType)
{
	if (!tableObjectExists(tableType))
	{
		throw dptf_exception("TableObject schema not found.");
	}

	// /nopersist/IETM.D0/itmt
	auto elementPath = "/nopersist/%nm%/" + TableObjectType::ToString(tableType);
	elementPath = StringParser::replaceAll(elementPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");

	auto dvType = DataVaultType::Override;

	deleteConfigKey(dvType, elementPath);
}

void DataManager::deleteConfigKey(DataVaultType::Type dvType, string key)
{
	m_dptfManager->getEsifServices()->deleteConfigurationBinary(DataVaultType::ToString(dvType), key);
}

Bool DataManager::isParticipantTable(TableObjectType::Type tableType)
{
	if (!tableObjectExists(tableType))
	{
		throw dptf_exception("TableObject schema not found.");
	}

	auto table = m_tableObjectMap.find(tableType)->second;
	return table.isParticipantTable();
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

map<TableObjectType::Type, TableObject> DataManager::getTableObjectMap()
{
	return m_tableObjectMap;
}

void DataManager::writeEmptyTable(string nameSpace, string elementPath)
{
	u8 dummyBuffer = 0;

	m_dptfManager->getEsifServices()->writeConfigurationBinary(
		&dummyBuffer, sizeof(dummyBuffer), 0, nameSpace, elementPath, ESIF_SERVICE_CONFIG_PERSIST);
}

void DataManager::sendTableChangedEvent(TableObjectType::Type tableObjectType, string uuid, UIntN participantIndex)
{
	std::shared_ptr<WorkItem> wi =
		std::make_shared<WIPolicyTableObjectChanged>(m_dptfManager, tableObjectType, uuid, participantIndex);
	m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndReturn(wi);
}

void DataManager::loadTableObjectMap()
{
	loadAcprTableObject();
	loadApatTableObject();
	loadApctTableObject();
	loadArtTableObject();
	loadDdrfTableObject();
	loadDynamicIdspTableObject();
	loadEpotTableObject();
	loadItmtTableObject();
	loadOdvpTableObject();
	loadPbatTableObject();
	loadPbctTableObject();
	loadPbmtTableObject();
	loadPidaTableObject();
	loadPsh2TableObject();
	loadPshaTableObject();
	loadPsvtTableObject();
	loadSwOemVariablesTableObject();
	loadTpgaTableObject();
	loadTrtTableObject();
	loadVsctTableObject();
	loadVsptTableObject();
	loadVtmtTableObject();
}

void DataManager::loadApatTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Apat);
	set<UInt64> revisionsUsingEsifDataVariant = {2};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{2,
		 {{"fld1", "fld1", ESIF_DATA_UINT64},
		  {"fld2", "fld2", ESIF_DATA_STRING},
		  {"fld3", "fld3", ESIF_DATA_STRING},
		  {"fld4", "fld4", ESIF_DATA_UINT64},
		  {"fld5", "fld5", ESIF_DATA_STRING},
		  {"fld6", "fld6", ESIF_DATA_STRING}}});

	m_tableObjectMap.insert(
		{TableObjectType::Apat,
		 {TableObjectType::Apat,
		  fieldsMap,
		  {{DataVaultType::Override, dataVaultString}, {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadApctTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Apct);
	set<UInt64> revisionsUsingEsifDataVariant = {2};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert({2, {{"fld1", "fld1", ESIF_DATA_UINT64},	{"fld2", "fld2", ESIF_DATA_UINT64},
						  {"fld3", "fld3", ESIF_DATA_UINT64},	{"fld4", "fld4", ESIF_DATA_STRING},
						  {"fld5", "fld5", ESIF_DATA_UINT64},	{"fld6", "fld6", ESIF_DATA_UINT64},
						  {"fld7", "fld7", ESIF_DATA_UINT64},	{"fld8", "fld8", ESIF_DATA_UINT64},
						  {"fld9", "fld9", ESIF_DATA_UINT64},	{"fld10", "fld10", ESIF_DATA_STRING},
						  {"fld11", "fld11", ESIF_DATA_UINT64}, {"fld12", "fld12", ESIF_DATA_UINT64},
						  {"fld13", "fld13", ESIF_DATA_UINT64}, {"fld14", "fld14", ESIF_DATA_UINT64},
						  {"fld15", "fld15", ESIF_DATA_UINT64}, {"fld16", "fld16", ESIF_DATA_STRING},
						  {"fld17", "fld17", ESIF_DATA_UINT64}, {"fld18", "fld18", ESIF_DATA_UINT64},
						  {"fld19", "fld19", ESIF_DATA_UINT64}, {"fld20", "fld20", ESIF_DATA_UINT64},
						  {"fld21", "fld21", ESIF_DATA_UINT64}, {"fld22", "fld22", ESIF_DATA_STRING},
						  {"fld23", "fld23", ESIF_DATA_UINT64}, {"fld24", "fld24", ESIF_DATA_UINT64},
						  {"fld25", "fld25", ESIF_DATA_UINT64}, {"fld26", "fld26", ESIF_DATA_UINT64},
						  {"fld27", "fld27", ESIF_DATA_UINT64}, {"fld28", "fld28", ESIF_DATA_STRING},
						  {"fld29", "fld29", ESIF_DATA_UINT64}, {"fld30", "fld30", ESIF_DATA_UINT64},
						  {"fld31", "fld31", ESIF_DATA_UINT64}, {"fld32", "fld32", ESIF_DATA_UINT64},
						  {"fld33", "fld33", ESIF_DATA_UINT64}, {"fld34", "fld34", ESIF_DATA_STRING},
						  {"fld35", "fld35", ESIF_DATA_UINT64}, {"fld36", "fld36", ESIF_DATA_UINT64},
						  {"fld37", "fld37", ESIF_DATA_UINT64}, {"fld38", "fld38", ESIF_DATA_UINT64},
						  {"fld39", "fld39", ESIF_DATA_UINT64}, {"fld40", "fld40", ESIF_DATA_STRING},
						  {"fld41", "fld41", ESIF_DATA_UINT64}, {"fld42", "fld42", ESIF_DATA_UINT64},
						  {"fld43", "fld43", ESIF_DATA_UINT64}, {"fld44", "fld44", ESIF_DATA_UINT64},
						  {"fld45", "fld45", ESIF_DATA_UINT64}, {"fld46", "fld46", ESIF_DATA_STRING},
						  {"fld47", "fld47", ESIF_DATA_UINT64}, {"fld48", "fld48", ESIF_DATA_UINT64},
						  {"fld49", "fld49", ESIF_DATA_UINT64}, {"fld50", "fld50", ESIF_DATA_UINT64},
						  {"fld51", "fld51", ESIF_DATA_UINT64}, {"fld52", "fld52", ESIF_DATA_STRING},
						  {"fld53", "fld53", ESIF_DATA_UINT64}, {"fld54", "fld54", ESIF_DATA_UINT64},
						  {"fld55", "fld55", ESIF_DATA_UINT64}, {"fld56", "fld56", ESIF_DATA_UINT64},
						  {"fld57", "fld57", ESIF_DATA_UINT64}, {"fld58", "fld58", ESIF_DATA_STRING},
						  {"fld59", "fld59", ESIF_DATA_UINT64}, {"fld60", "fld60", ESIF_DATA_UINT64},
						  {"fld61", "fld61", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Apct,
		 {TableObjectType::Apct,
		  fieldsMap,
		  {{DataVaultType::Override, dataVaultString}, {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadDynamicIdspTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Dynamic_Idsp);
	set<UInt64> revisionsUsingEsifDataVariant = {1};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{1,
		 {{"uuid", "uuid", ESIF_DATA_STRING},
		  {"templateGuid", "templateGuid", ESIF_DATA_STRING},
		  {"policyName", "policyName", ESIF_DATA_STRING}}});

	m_tableObjectMap.insert(
		{TableObjectType::Dynamic_Idsp,
		 {TableObjectType::Dynamic_Idsp,
		  fieldsMap,
		  {{DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Dptf, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadDdrfTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Ddrf);
	set<UInt64> revisionsUsingEsifDataVariant = {1};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{1,
		 {{"fld1", "fld1", ESIF_DATA_UINT64},
		  {"fld2", "fld2", ESIF_DATA_UINT64},
		  {"fld3", "fld3", ESIF_DATA_UINT64},
		  {"fld4", "fld4", ESIF_DATA_UINT64},
		  {"fld5", "fld5", ESIF_DATA_UINT64},
		  {"fld6", "fld6", ESIF_DATA_UINT64},
		  {"fld7", "fld7", ESIF_DATA_UINT64},
		  {"fld8", "fld8", ESIF_DATA_UINT64},
		  {"fld9", "fld9", ESIF_DATA_UINT64},
		  {"fld10", "fld10", ESIF_DATA_UINT64},
		  {"fld11", "fld11", ESIF_DATA_UINT64},
		  {"fld12", "fld12", ESIF_DATA_UINT64},
		  {"fld13", "fld13", ESIF_DATA_UINT64},
		  {"fld14", "fld14", ESIF_DATA_UINT64},
		  {"fld15", "fld15", ESIF_DATA_UINT64},
		  {"fld16", "fld16", ESIF_DATA_UINT64},
		  {"fld17", "fld17", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Ddrf,
		 {TableObjectType::Ddrf,
		  fieldsMap,
		  {{DataVaultType::Override, dataVaultString}, {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadItmtTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Itmt);
	auto noPersistPath = "/nopersist/%nm%/" + TableObjectType::ToString(TableObjectType::Itmt);
	noPersistPath = StringParser::replaceAll(noPersistPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	set<UInt64> revisionsUsingEsifDataVariant = {1, 2};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{1,
		 {{"fld1", "fld1", ESIF_DATA_STRING},
		  {"fld2", "fld2", ESIF_DATA_UINT64},
		  {"fld3", "fld3", ESIF_DATA_STRING},
		  {"fld4", "fld4", ESIF_DATA_STRING},
		  {"fld5", "fld5", ESIF_DATA_STRING},
		  {"fld6", "fld6", ESIF_DATA_UINT64}}});

	fieldsMap.insert(
		{2,
		 {{"fld1", "fld1", ESIF_DATA_STRING},
		  {"fld2", "fld2", ESIF_DATA_UINT64},
		  {"fld3", "fld3", ESIF_DATA_STRING},
		  {"fld4", "fld4", ESIF_DATA_STRING},
		  {"fld5", "fld5", ESIF_DATA_STRING},
		  {"fld6", "fld6", ESIF_DATA_UINT64},
		  {"fld7", "fld7", ESIF_DATA_STRING},
		  {"fld8", "fld8", ESIF_DATA_UINT64},
		  {"fld9", "fld9", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Itmt,
		 {TableObjectType::Itmt,
		  fieldsMap,
		  {{DataVaultType::Override, noPersistPath},
		   {DataVaultType::Override, dataVaultString},
		   {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadEpotTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Epot);
	auto noPersistPath = "/nopersist/%nm%/" + TableObjectType::ToString(TableObjectType::Epot);
	noPersistPath = StringParser::replaceAll(noPersistPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	set<UInt64> revisionsUsingEsifDataVariant = {1, 2};
	map<UInt64, vector<TableObjectField>> fieldsMap;

	fieldsMap.insert(
		{1,
		 {{"fld1", "fld1", ESIF_DATA_STRING},
		  {"fld2", "fld2", ESIF_DATA_STRING},
		  {"fld3", "fld3", ESIF_DATA_UINT64},
		  {"fld4", "fld4", ESIF_DATA_UINT64}}});

	fieldsMap.insert(
		{2,
		 {{"fld1", "fld1", ESIF_DATA_STRING},
		  {"fld2", "fld2", ESIF_DATA_STRING},
		  {"fld3", "fld3", ESIF_DATA_UINT64},
		  {"fld4", "fld4", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Epot,
		 {TableObjectType::Epot,
		  fieldsMap,
		  {{DataVaultType::Override, noPersistPath},
		   {DataVaultType::Override, dataVaultString},
		   {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadTpgaTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Tpga);
	auto noPersistPath = "/nopersist/%nm%/" + TableObjectType::ToString(TableObjectType::Tpga);
	noPersistPath = StringParser::replaceAll(noPersistPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	set<UInt64> revisionsUsingEsifDataVariant = {1};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{1,
		 {{"fld1", "fld1", ESIF_DATA_STRING}, {"fld2", "fld2", ESIF_DATA_UINT64}, {"fld3", "fld3", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Tpga,
		 {TableObjectType::Tpga,
		  fieldsMap,
		  {{DataVaultType::Override, noPersistPath},
		   {DataVaultType::Override, dataVaultString},
		   {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadSwOemVariablesTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::SwOemVariables);
	auto noPersistPath = "/nopersist/%nm%/" + TableObjectType::ToString(TableObjectType::SwOemVariables);
	noPersistPath = StringParser::replaceAll(noPersistPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	set<UInt64> revisionsUsingEsifDataVariant = {1};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert({1, {{"fld1", "fld1", ESIF_DATA_UINT64}, {"fld2", "fld2", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::SwOemVariables,
		 {TableObjectType::SwOemVariables,
		  fieldsMap,
		  {{DataVaultType::Override, noPersistPath},
		   {DataVaultType::Override, dataVaultString},
		   {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadPsvtTableObject()
{
	auto dataVaultString = "/participants/%nm%/UUID/" + TableObjectType::ToString(TableObjectType::Psvt);
	dataVaultString = StringParser::replaceAll(dataVaultString, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	auto noPersistPath = "/nopersist/%nm%/" + TableObjectType::ToString(TableObjectType::Psvt);
	noPersistPath = StringParser::replaceAll(noPersistPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	set<UInt64> revisionsUsingEsifDataVariant = {1,2};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{1,
		 {{"fld1", "fld1", ESIF_DATA_STRING},
		  {"fld2", "fld2", ESIF_DATA_STRING},
		  {"fld3", "fld3", ESIF_DATA_UINT64},
		  {"fld4", "fld4", ESIF_DATA_UINT64},
		  {"fld5", "fld5", ESIF_DATA_UINT64},
		  {"fld6", "fld6", ESIF_DATA_UINT64},
		  {"fld7", "fld7", ESIF_DATA_UINT64},
		  {"fld8", "fld8", ESIF_DATA_STRING, true},
		  {"fld9", "fld9", ESIF_DATA_UINT64},
		  {"fld10", "fld10", ESIF_DATA_UINT64},
		  {"fld11", "fld11", ESIF_DATA_UINT64},
		  {"fld12", "fld12", ESIF_DATA_UINT64}}});

	fieldsMap.insert(
		{2,
		 {{"fld1", "fld1", ESIF_DATA_STRING},
		  {"fld2", "fld2", ESIF_DATA_STRING},
		  {"fld3", "fld3", ESIF_DATA_UINT64},
		  {"fld4", "fld4", ESIF_DATA_UINT64},
		  {"fld5", "fld5", ESIF_DATA_UINT64},
		  {"fld6", "fld6", ESIF_DATA_UINT64},
		  {"fld7", "fld7", ESIF_DATA_UINT64},
		  {"fld8", "fld8", ESIF_DATA_STRING, true},
		  {"fld9", "fld9", ESIF_DATA_UINT64},
		  {"fld10", "fld10", ESIF_DATA_UINT64},
		  {"fld11", "fld11", ESIF_DATA_UINT64},
		  {"fld12", "fld12", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Psvt,
		 {TableObjectType::Psvt,
		  fieldsMap,
		  {{DataVaultType::Override, noPersistPath},
		   {DataVaultType::Override, dataVaultString},
		   {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant,
		  0,
		  GET_PASSIVE_RELATIONSHIP_TABLE}});
}

void DataManager::loadVsctTableObject()
{
	auto dataVaultString = "/participants/%nm%/" + TableObjectType::ToString(TableObjectType::Vsct);
	set<UInt64> revisionsUsingEsifDataVariant = {1};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{1,
		 {{"fld1", "fld1", ESIF_DATA_STRING},
		  {"fld2", "fld2", ESIF_DATA_UINT64},
		  {"fld3", "fld3", ESIF_DATA_UINT64},
		  {"fld4", "fld4", ESIF_DATA_UINT64},
		  {"fld5", "fld5", ESIF_DATA_UINT64},
		  {"fld6", "fld6", ESIF_DATA_UINT64},
		  {"fld7", "fld7", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Vsct,
		 {TableObjectType::Vsct,
		  fieldsMap,
		  {{DataVaultType::Override, dataVaultString}, {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant,
		  0,
		  GET_VIRTUAL_SENSOR_CALIB_TABLE,
		  true}});
}

void DataManager::loadVsptTableObject()
{
	auto dataVaultString = "/participants/%nm%/" + TableObjectType::ToString(TableObjectType::Vspt);
	map<UInt64, vector<TableObjectField>> fieldsMap;
	set<UInt64> revisionsUsingEsifDataVariant = {1};
	fieldsMap.insert({1, {{"fld1", "fld1", ESIF_DATA_UINT64}, {"fld2", "fld2", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Vspt,
		 {TableObjectType::Vspt,
		  fieldsMap,
		  {{DataVaultType::Override, dataVaultString}, {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant,
		  0,
		  GET_VIRTUAL_SENSOR_POLLING_TABLE,
		  true}});
}

void DataManager::loadAcprTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Acpr);
	auto noPersistPath = "/nopersist/%nm%/" + TableObjectType::ToString(TableObjectType::Acpr);
	noPersistPath = StringParser::replaceAll(noPersistPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	UInt32 mode = 2;
	set<UInt64> revisionsUsingEsifDataVariant = {1};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{1,
		 {{"fld1", "fld1", ESIF_DATA_STRING},
		  {"fld2", "fld2", ESIF_DATA_UINT64},
		  {"fld3", "fld3", ESIF_DATA_STRING},
		  {"fld4", "fld4", ESIF_DATA_UINT64},
		  {"fld5", "fld5", ESIF_DATA_UINT64},
		  {"fld6", "fld6", ESIF_DATA_UINT64},
		  {"fld7", "fld7", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Acpr,
		 {TableObjectType::Acpr,
		  fieldsMap,
		  {{DataVaultType::Override, noPersistPath},
		   {DataVaultType::Override, dataVaultString},
		   {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant,
		  mode}});
}

void DataManager::loadPbatTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Pbat);
	set<UInt64> revisionsUsingEsifDataVariant = {2};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{2,
		 {{"fld1", "fld1", ESIF_DATA_UINT64},
		  {"fld2", "fld2", ESIF_DATA_STRING},
		  {"fld3", "fld3", ESIF_DATA_STRING},
		  {"fld4", "fld4", ESIF_DATA_UINT64},
		  {"fld5", "fld5", ESIF_DATA_STRING},
		  {"fld6", "fld6", ESIF_DATA_STRING}}});

	m_tableObjectMap.insert(
		{TableObjectType::Pbat,
		 {TableObjectType::Pbat,
		  fieldsMap,
		  {{DataVaultType::Override, dataVaultString}, {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadPbctTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Pbct);
	set<UInt64> revisionsUsingEsifDataVariant = {2};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert({2, {{"fld1", "fld1", ESIF_DATA_UINT64},	{"fld2", "fld2", ESIF_DATA_UINT64},
						  {"fld3", "fld3", ESIF_DATA_UINT64},	{"fld4", "fld4", ESIF_DATA_STRING},
						  {"fld5", "fld5", ESIF_DATA_UINT64},	{"fld6", "fld6", ESIF_DATA_UINT64},
						  {"fld7", "fld7", ESIF_DATA_UINT64},	{"fld8", "fld8", ESIF_DATA_UINT64},
						  {"fld9", "fld9", ESIF_DATA_UINT64},	{"fld10", "fld10", ESIF_DATA_STRING},
						  {"fld11", "fld11", ESIF_DATA_UINT64}, {"fld12", "fld12", ESIF_DATA_UINT64},
						  {"fld13", "fld13", ESIF_DATA_UINT64}, {"fld14", "fld14", ESIF_DATA_UINT64},
						  {"fld15", "fld15", ESIF_DATA_UINT64}, {"fld16", "fld16", ESIF_DATA_STRING},
						  {"fld17", "fld17", ESIF_DATA_UINT64}, {"fld18", "fld18", ESIF_DATA_UINT64},
						  {"fld19", "fld19", ESIF_DATA_UINT64}, {"fld20", "fld20", ESIF_DATA_UINT64},
						  {"fld21", "fld21", ESIF_DATA_UINT64}, {"fld22", "fld22", ESIF_DATA_STRING},
						  {"fld23", "fld23", ESIF_DATA_UINT64}, {"fld24", "fld24", ESIF_DATA_UINT64},
						  {"fld25", "fld25", ESIF_DATA_UINT64}, {"fld26", "fld26", ESIF_DATA_UINT64},
						  {"fld27", "fld27", ESIF_DATA_UINT64}, {"fld28", "fld28", ESIF_DATA_STRING},
						  {"fld29", "fld29", ESIF_DATA_UINT64}, {"fld30", "fld30", ESIF_DATA_UINT64},
						  {"fld31", "fld31", ESIF_DATA_UINT64}, {"fld32", "fld32", ESIF_DATA_UINT64},
						  {"fld33", "fld33", ESIF_DATA_UINT64}, {"fld34", "fld34", ESIF_DATA_STRING},
						  {"fld35", "fld35", ESIF_DATA_UINT64}, {"fld36", "fld36", ESIF_DATA_UINT64},
						  {"fld37", "fld37", ESIF_DATA_UINT64}, {"fld38", "fld38", ESIF_DATA_UINT64},
						  {"fld39", "fld39", ESIF_DATA_UINT64}, {"fld40", "fld40", ESIF_DATA_STRING},
						  {"fld41", "fld41", ESIF_DATA_UINT64}, {"fld42", "fld42", ESIF_DATA_UINT64},
						  {"fld43", "fld43", ESIF_DATA_UINT64}, {"fld44", "fld44", ESIF_DATA_UINT64},
						  {"fld45", "fld45", ESIF_DATA_UINT64}, {"fld46", "fld46", ESIF_DATA_STRING},
						  {"fld47", "fld47", ESIF_DATA_UINT64}, {"fld48", "fld48", ESIF_DATA_UINT64},
						  {"fld49", "fld49", ESIF_DATA_UINT64}, {"fld50", "fld50", ESIF_DATA_UINT64},
						  {"fld51", "fld51", ESIF_DATA_UINT64}, {"fld52", "fld52", ESIF_DATA_STRING},
						  {"fld53", "fld53", ESIF_DATA_UINT64}, {"fld54", "fld54", ESIF_DATA_UINT64},
						  {"fld55", "fld55", ESIF_DATA_UINT64}, {"fld56", "fld56", ESIF_DATA_UINT64},
						  {"fld57", "fld57", ESIF_DATA_UINT64}, {"fld58", "fld58", ESIF_DATA_STRING},
						  {"fld59", "fld59", ESIF_DATA_UINT64}, {"fld60", "fld60", ESIF_DATA_UINT64},
						  {"fld61", "fld61", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Pbct,
		 {TableObjectType::Pbct,
		  fieldsMap,
		  {{DataVaultType::Override, dataVaultString}, {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadPbmtTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Pbmt);
	set<UInt64> revisionsUsingEsifDataVariant = {2};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert({2, {{"fld1", "fld1", ESIF_DATA_STRING},	{"fld2", "fld2", ESIF_DATA_UINT64},
						  {"fld3", "fld3", ESIF_DATA_STRING},	{"fld4", "fld4", ESIF_DATA_STRING},
						  {"fld5", "fld5", ESIF_DATA_STRING},	{"fld6", "fld6", ESIF_DATA_STRING},
						  {"fld7", "fld7", ESIF_DATA_UINT64},	{"fld8", "fld8", ESIF_DATA_STRING},
						  {"fld9", "fld9", ESIF_DATA_UINT64},	{"fld10", "fld10", ESIF_DATA_UINT64},
						  {"fld11", "fld11", ESIF_DATA_UINT64}, {"fld12", "fld12", ESIF_DATA_STRING},
						  {"fld13", "fld13", ESIF_DATA_UINT64}, {"fld14", "fld14", ESIF_DATA_UINT64},
						  {"fld15", "fld15", ESIF_DATA_UINT64}, {"fld16", "fld16", ESIF_DATA_UINT64},
						  {"fld17", "fld17", ESIF_DATA_UINT64}, {"fld18", "fld18", ESIF_DATA_UINT64},
						  {"fld19", "fld19", ESIF_DATA_STRING}, {"fld20", "fld20", ESIF_DATA_STRING},
						  {"fld21", "fld21", ESIF_DATA_UINT64}, {"fld22", "fld22", ESIF_DATA_UINT64},
						  {"fld23", "fld23", ESIF_DATA_UINT64}, {"fld24", "fld24", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Pbmt,
		 {TableObjectType::Pbmt,
		  fieldsMap,
		  {{DataVaultType::Override, dataVaultString}, {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadVtmtTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Vtmt);
	set<UInt64> revisionsUsingEsifDataVariant = {1};
	UInt32 mode = 4;
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert({1, {{"fld1", "fld1", ESIF_DATA_STRING},	{"fld2", "fld2", ESIF_DATA_STRING},
						  {"fld3", "fld3", ESIF_DATA_UINT64},	{"fld4", "fld4", ESIF_DATA_UINT64},
						  {"fld5", "fld5", ESIF_DATA_UINT64},	{"fld6", "fld6", ESIF_DATA_UINT64},
						  {"fld7", "fld7", ESIF_DATA_UINT64},	{"fld8", "fld8", ESIF_DATA_UINT64},
						  {"fld9", "fld9", ESIF_DATA_UINT64},	{"fld10", "fld10", ESIF_DATA_UINT64},
						  {"fld11", "fld11", ESIF_DATA_UINT64}, {"fld12", "fld12", ESIF_DATA_STRING},
						  {"fld13", "fld13", ESIF_DATA_STRING}, {"fld14", "fld14", ESIF_DATA_STRING},
						  {"fld15", "fld15", ESIF_DATA_UINT64}, {"fld16", "fld16", ESIF_DATA_UINT64},
						  {"fld17", "fld17", ESIF_DATA_UINT64}, {"fld18", "fld18", ESIF_DATA_UINT64},
						  {"fld19", "fld19", ESIF_DATA_UINT64}, {"fld20", "fld20", ESIF_DATA_UINT64},
						  {"fld21", "fld21", ESIF_DATA_STRING}, {"fld22", "fld22", ESIF_DATA_STRING},
						  {"fld23", "fld23", ESIF_DATA_UINT64}, {"fld24", "fld24", ESIF_DATA_UINT64},
						  {"fld25", "fld25", ESIF_DATA_UINT64}, {"fld26", "fld26", ESIF_DATA_UINT64},
						  {"fld27", "fld27", ESIF_DATA_UINT64}, {"fld28", "fld28", ESIF_DATA_UINT64},
						  {"fld29", "fld29", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Vtmt,
		 {TableObjectType::Vtmt,
		  fieldsMap,
		  {{DataVaultType::Override, dataVaultString}, {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant,
		  mode}});
}

void DataManager::loadPshaTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Psha);
	auto noPersistPath = "/nopersist/%nm%/" + TableObjectType::ToString(TableObjectType::Psha);
	noPersistPath = StringParser::replaceAll(noPersistPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	set<UInt64> revisionsUsingEsifDataVariant = {1};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{1,
		 {{"fld1", "fld1", ESIF_DATA_STRING}, {"fld2", "fld2", ESIF_DATA_UINT64}, {"fld3", "fld3", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Psha,
		 {TableObjectType::Psha,
		  fieldsMap,
		  {{DataVaultType::Override, noPersistPath},
		   {DataVaultType::Override, dataVaultString},
		   {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadPsh2TableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Psh2);
	auto noPersistPath = "/nopersist/%nm%/" + TableObjectType::ToString(TableObjectType::Psh2);
	noPersistPath = StringParser::replaceAll(noPersistPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	set<UInt64> revisionsUsingEsifDataVariant = {1};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{1,
		 {{"fld1", "fld1", ESIF_DATA_STRING, false, 64},
		  {"fld2", "fld2", ESIF_DATA_UINT64, false, 8},
		  {"fld3", "fld3", ESIF_DATA_UINT64, false, 8}}});

	fieldsMap.insert(
		{2,
		 {{"fld1", "fld1", ESIF_DATA_STRING, false, 64},
		  {"fld2", "fld2", ESIF_DATA_UINT32, false, 4},
		  {"fld3", "fld3", ESIF_DATA_UINT32, false, 4},
		  {"fld4", "fld4", ESIF_DATA_UINT32, false, 4},
		  {"fld5", "fld5", ESIF_DATA_UINT32, false, 4},
		  {"fld6", "fld6", ESIF_DATA_UINT32, false, 4},
		  {"fld7", "fld7", ESIF_DATA_UINT32, false, 4}}});

	m_tableObjectMap.insert(
		{TableObjectType::Psh2,
		 {TableObjectType::Psh2,
		  fieldsMap,
		  {{DataVaultType::Override, noPersistPath},
		   {DataVaultType::Override, dataVaultString},
		   {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant,
		  0,
		  (esif_primitive_type_t)0,
		  false}});
}


void DataManager::loadTrtTableObject()
{
	auto dataVaultString = "/participants/%nm%/UUID/" + TableObjectType::ToString(TableObjectType::Trt);
	dataVaultString = StringParser::replaceAll(dataVaultString, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	auto noPersistPath = "/nopersist/%nm%/" + TableObjectType::ToString(TableObjectType::Trt);
	noPersistPath = StringParser::replaceAll(noPersistPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	set<UInt64> revisionsUsingEsifDataVariant = {0};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{0,
		 {{"fld1", "fld1", ESIF_DATA_STRING},
		  {"fld2", "fld2", ESIF_DATA_STRING},
		  {"fld3", "fld3", ESIF_DATA_UINT64},
		  {"fld4", "fld4", ESIF_DATA_UINT64},
		  {"fld5", "fld5", ESIF_DATA_UINT64},
		  {"fld6", "fld6", ESIF_DATA_UINT64},
		  {"fld7", "fld7", ESIF_DATA_UINT64},
		  {"fld8", "fld8", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Trt,
		 {TableObjectType::Trt,
		  fieldsMap,
		  {{DataVaultType::Override, noPersistPath},
		   {DataVaultType::Override, dataVaultString},
		   {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant,
		  0,
		  GET_THERMAL_RELATIONSHIP_TABLE,
		  false}});
}

void DataManager::loadOdvpTableObject()
{
	auto dataVaultString = "/participants/%nm%/UUID/" + TableObjectType::ToString(TableObjectType::Odvp);
	dataVaultString = StringParser::replaceAll(dataVaultString, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	set<UInt64> revisionsUsingEsifDataVariant = {0};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert({0, {{"field", "field", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Odvp,
		 {TableObjectType::Odvp,
		  fieldsMap,
		  {{DataVaultType::Override, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant,
		  0,
		  GET_OEM_VARS,
		  false}});
}

void DataManager::loadPidaTableObject()
{
	auto dataVaultString =
		DataVaultPathBasePaths::ExportRoot + "/UUID/" + TableObjectType::ToString(TableObjectType::Pida);
	auto noPersistPath = "/nopersist/%nm%/" + TableObjectType::ToString(TableObjectType::Pida);
	noPersistPath = StringParser::replaceAll(noPersistPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	set<UInt64> revisionsUsingEsifDataVariant = {2};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{2,
		 {{"fld1", "fld1", ESIF_DATA_STRING},
		  {"fld2", "fld2", ESIF_DATA_UINT64},
		  {"fld3", "fld3", ESIF_DATA_UINT64},
		  {"fld4", "fld4", ESIF_DATA_STRING},
		  {"fld5", "fld5", ESIF_DATA_UINT64},
		  {"fld6", "fld6", ESIF_DATA_UINT64},
		  {"fld7", "fld7", ESIF_DATA_UINT64},
		  {"fld8", "fld8", ESIF_DATA_UINT64},
		  {"fld9", "fld9", ESIF_DATA_UINT64},
		  {"fld10", "fld10", ESIF_DATA_UINT64},
		  {"fld11", "fld11", ESIF_DATA_UINT64},
		  {"fld12", "fld12", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Pida,
		 {TableObjectType::Pida,
		  fieldsMap,
		  {{DataVaultType::Override, noPersistPath},
		   {DataVaultType::Override, dataVaultString},
		   {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant}});
}

void DataManager::loadArtTableObject()
{
	auto dataVaultString = "/participants/%nm%/UUID/" + TableObjectType::ToString(TableObjectType::Art);
	dataVaultString = StringParser::replaceAll(dataVaultString, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	auto noPersistPath = "/nopersist/%nm%/" + TableObjectType::ToString(TableObjectType::Art);
	noPersistPath = StringParser::replaceAll(noPersistPath, "%nm%", DefaultScope::IETMParticipantScope + ".D0");
	set<UInt64> revisionsUsingEsifDataVariant = {1};
	map<UInt64, vector<TableObjectField>> fieldsMap;
	fieldsMap.insert(
		{1,
		 {{"fld1", "fld1", ESIF_DATA_STRING},
		  {"fld2", "fld2", ESIF_DATA_STRING},
		  {"fld3", "fld3", ESIF_DATA_UINT64},
		  {"fld4", "fld4", ESIF_DATA_UINT64},
		  {"fld5", "fld5", ESIF_DATA_UINT64},
		  {"fld6", "fld6", ESIF_DATA_UINT64},
		  {"fld7", "fld7", ESIF_DATA_UINT64},
		  {"fld8", "fld8", ESIF_DATA_UINT64},
		  {"fld9", "fld9", ESIF_DATA_UINT64},
		  {"fld10", "fld10", ESIF_DATA_UINT64},
		  {"fld11", "fld11", ESIF_DATA_UINT64},
		  {"fld12", "fld12", ESIF_DATA_UINT64},
		  {"fld13", "fld13", ESIF_DATA_UINT64}}});

	m_tableObjectMap.insert(
		{TableObjectType::Art,
		 {TableObjectType::Art,
		  fieldsMap,
		  {{DataVaultType::Override, noPersistPath},
		   {DataVaultType::Override, dataVaultString},
		   {DataVaultType::Dptf, dataVaultString}},
		  {{DataVaultType::Override, dataVaultString}},
		  revisionsUsingEsifDataVariant,
		  0,
		  GET_ACTIVE_RELATIONSHIP_TABLE,
		  false}});
}