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

#pragma once
#include "TableObjectType.h"
#include "TableObject.h"
#include "DptfManagerInterface.h"
#include "StringParser.h"
#include "DataVaultType.h"

class dptf_export DataManagerInterface
{
public:
	virtual ~DataManagerInterface(){};
	virtual TableObject getTableObject(
		TableObjectType::Type tableType,
		std::string uuid,
		UIntN participantIndex = Constants::Esif::NoParticipant) = 0;
	virtual void setTableObject(
		const DptfBuffer& tableData,
		TableObjectType::Type tableType,
		std::string uuid,
		UIntN participantIndex = Constants::Esif::NoParticipant) = 0;
	virtual void deleteTableObject(
		TableObjectType::Type tableType,
		std::string uuid,
		UIntN participantIndex = Constants::Esif::NoParticipant) = 0;
	virtual void deleteAllTableObject(
		TableObjectType::Type tableType,
		std::string uuid,
		UIntN participantIndex = Constants::Esif::NoParticipant) = 0;
	virtual Bool tableObjectExists(TableObjectType::Type tableType) = 0;
	virtual std::map<TableObjectType::Type, TableObject> getTableObjectMap() = 0;

	virtual TableObject getTableObjectBasedOnAlternativeDataSourceAndKey(
		TableObjectType::Type tableType,
		DataVaultType::Type dvType,
		std::string key) = 0;
	virtual void setTableObjectBasedOnAlternativeDataSourceAndKey(
		const DptfBuffer& tableData,
		TableObjectType::Type tableType,
		DataVaultType::Type dvType,
		std::string key) = 0;

	virtual void setTableObjectForNoPersist(DptfBuffer tableData, TableObjectType::Type tableType) = 0;
	virtual void deleteTableObjectKeyForNoPersist(TableObjectType::Type tableType) = 0;

	virtual void deleteConfigKey(DataVaultType::Type dvType, std::string key) = 0;

	virtual Bool isParticipantTable(TableObjectType::Type tableType) = 0;
};

class DataManager : public DataManagerInterface
{
public:
	DataManager(DptfManagerInterface* dptfManager);
	~DataManager(void);

	virtual TableObject getTableObject(TableObjectType::Type tableType, std::string uuid, UIntN participantIndex)
		override;
	virtual void setTableObject(
		const DptfBuffer& tableData,
		TableObjectType::Type tableType,
		std::string uuid,
		UIntN participantIndex) override;
	virtual void deleteTableObject(TableObjectType::Type tableType, std::string, UIntN participantIndex) override;
	virtual void deleteAllTableObject(TableObjectType::Type tableType, std::string, UIntN participantIndex) override;
	virtual Bool tableObjectExists(TableObjectType::Type tableType) override;
	virtual std::map<TableObjectType::Type, TableObject> getTableObjectMap() override;

	virtual TableObject getTableObjectBasedOnAlternativeDataSourceAndKey(
		TableObjectType::Type tableType,
		DataVaultType::Type dvType,
		std::string key) override;
	virtual void setTableObjectBasedOnAlternativeDataSourceAndKey(
		const DptfBuffer& tableData,
		TableObjectType::Type tableType,
		DataVaultType::Type dvType,
		std::string key) override;

	virtual void setTableObjectForNoPersist(DptfBuffer tableData, TableObjectType::Type tableType) override;
	virtual void deleteTableObjectKeyForNoPersist(TableObjectType::Type tableType) override;

	virtual void deleteConfigKey(DataVaultType::Type dvType, std::string key) override;

	virtual Bool isParticipantTable(TableObjectType::Type tableType) override;

private:
	DptfManagerInterface* m_dptfManager;
	std::map<TableObjectType::Type, TableObject> m_tableObjectMap;

	void sendTableChangedEvent(TableObjectType::Type tableObjectType, std::string uuid, UIntN participantIndex);
	void loadTableObjectMap();
	void loadAcprTableObject();
	void loadApatTableObject();
	void loadApctTableObject();
	void loadArtTableObject();
	void loadDynamicIdspTableObject();
	void loadDdrfTableObject();
	void loadEpotTableObject();
	void loadItmtTableObject();
	void loadOdvpTableObject();
	void loadPbatTableObject();
	void loadPbctTableObject();
	void loadPbmtTableObject();
	void loadPidaTableObject();
	void loadPsh2TableObject();
	void loadPshaTableObject();
	void loadPsvtTableObject();
	void loadSwOemVariablesTableObject();
	void loadTpgaTableObject();
	void loadTrtTableObject();
	void loadVsctTableObject();
	void loadVsptTableObject();
	void loadVtmtTableObject();
};
