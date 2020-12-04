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
#include "TableObjectType.h"
#include "TableObject.h"
#include "DptfManagerInterface.h"

class dptf_export DataManagerInterface
{
public:
	virtual ~DataManagerInterface(){};
	virtual TableObject getTableObject(TableObjectType::Type tableType) = 0;
	virtual void setTableObject(std::string tableString) = 0;
	virtual Bool tableObjectExists(TableObjectType::Type tableType) = 0;
};

class DataManager : public DataManagerInterface
{
public:
	DataManager(DptfManagerInterface* dptfManager);
	~DataManager(void);

	virtual TableObject getTableObject(TableObjectType::Type tableType) override;
	virtual void setTableObject(std::string tableString) override;
	virtual Bool tableObjectExists(TableObjectType::Type tableType) override;

private:
	DptfManagerInterface* m_dptfManager;
	std::map<TableObjectType::Type, TableObject> m_tableObjectMap;

	void loadTableObjectMap();
};
