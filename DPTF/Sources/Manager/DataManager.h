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
#include "TableObjectType.h"
#include "TableObject.h"
#include "DptfManagerInterface.h"
#include "StringParser.h"
#include "DataVaultType.h"

class dptf_export DataManagerInterface
{
public:
	virtual ~DataManagerInterface(){};
	virtual TableObject getTableObject(TableObjectType::Type tableType, std::string uuid) = 0;
	virtual void setTableObject(
		UInt32 tableDataLength,
		UInt8* tableData,
		TableObjectType::Type tableType,
		std::string uuid) = 0;
	virtual void deleteTableObject(TableObjectType::Type tableType, std::string uuid) = 0;
	virtual void deleteAllTableObject(TableObjectType::Type tableType, std::string uuid) = 0;
	virtual Bool tableObjectExists(TableObjectType::Type tableType) = 0;
	virtual std::map<TableObjectType::Type, TableObject> getTableObjectMap() = 0;
	virtual UInt32 getLatestSupportedTableRevision(TableObjectType::Type) = 0;

	virtual TableObject getTableObjectBasedOnAlternativeDataSourceAndKey(
		TableObjectType::Type tableType,
		DataVaultType::Type dvType,
		std::string key) = 0;
	virtual void setTableObjectBasedOnAlternativeDataSourceAndKey(
		UInt32 tableDataLength,
		UInt8* tableData,
		TableObjectType::Type tableType,
		DataVaultType::Type dvType,
		std::string key) = 0;

	virtual void setTableObjectForNoPersist(DptfBuffer tableData, TableObjectType::Type tableType) = 0;
	virtual void deleteTableObjectKeyForNoPersist(TableObjectType::Type tableType) = 0;

	virtual void deleteConfigKey(DataVaultType::Type dvType, std::string key) = 0;
};

class DataManager : public DataManagerInterface
{
public:
	DataManager(DptfManagerInterface* dptfManager);
	~DataManager(void);

	virtual TableObject getTableObject(TableObjectType::Type tableType, std::string uuid) override;
	virtual void setTableObject(
		UInt32 tableDataLength,
		UInt8* tableData,
		TableObjectType::Type tableType,
		std::string uuid) override;
	virtual void deleteTableObject(TableObjectType::Type tableType, std::string) override;
	virtual void deleteAllTableObject(TableObjectType::Type tableType, std::string) override;
	virtual Bool tableObjectExists(TableObjectType::Type tableType) override;
	virtual std::map<TableObjectType::Type, TableObject> getTableObjectMap() override;
	virtual UInt32 getLatestSupportedTableRevision(TableObjectType::Type tableType) override;

	virtual TableObject getTableObjectBasedOnAlternativeDataSourceAndKey(
		TableObjectType::Type tableType,
		DataVaultType::Type dvType,
		std::string key) override;
	virtual void setTableObjectBasedOnAlternativeDataSourceAndKey(
		UInt32 tableDataLength,
		UInt8* tableData,
		TableObjectType::Type tableType,
		DataVaultType::Type dvType,
		std::string key) override;

	virtual void setTableObjectForNoPersist(DptfBuffer tableData, TableObjectType::Type tableType) override;
	virtual void deleteTableObjectKeyForNoPersist(TableObjectType::Type tableType) override;

	virtual void deleteConfigKey(DataVaultType::Type dvType, std::string key) override;

private:
	DptfManagerInterface* m_dptfManager;
	std::map<TableObjectType::Type, TableObject> m_tableObjectMap;
	std::map<TableObjectType::Type, UInt32> m_tableRevisions;

	void sendTableChangedEvent(TableObjectType::Type tableObjectType, std::string uuid);
	void loadTableRevisions();
	void loadTableObjectMap();
};
