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

#include "WIDptfSwOemVariablesBroadcastReceived.h"
#include "PolicyManagerInterface.h"
#include "SystemModeManager.h"
#include "EsifServicesInterface.h"
#include "StatusFormat.h"
#include "DataManager.h"
#include "SwOemVariables.h"

WIDptfSwOemVariablesBroadcastReceived::WIDptfSwOemVariablesBroadcastReceived(
	DptfManagerInterface* dptfManager,
	DptfBuffer swOemVariablesData)
	: WorkItem(dptfManager, FrameworkEvent::DptfAppBroadcastPrivileged)
	, m_swOemVariablesData(swOemVariablesData)
{
}

WIDptfSwOemVariablesBroadcastReceived::~WIDptfSwOemVariablesBroadcastReceived(void)
{
}

void WIDptfSwOemVariablesBroadcastReceived::onExecute(void)
{
	writeWorkItemStartingInfoMessage();
	updateExistingSwOemVariables();
}

void WIDptfSwOemVariablesBroadcastReceived::updateExistingSwOemVariables()
{
	auto dptfManager = getDptfManager();
	auto dataManager = dptfManager->getDataManager();
	try 
	{
		const auto newVars = SwOemVariables::createFromAppBroadcastData(m_swOemVariablesData);
		const auto table = dataManager->getTableObject(TableObjectType::SwOemVariables, Constants::EmptyString).getData();
		auto persistedVars = SwOemVariables::createFromTableObjectData(table);
		persistedVars.add(newVars);
		dataManager->setTableObjectForNoPersist(persistedVars.toSwOemVariablesDvBinary(), TableObjectType::SwOemVariables);
	}
	catch (dptf_exception& ex) 
	{
		writeWorkItemErrorMessage(ex, "Failed to update existing sw-oem-variables table data");
	}
}