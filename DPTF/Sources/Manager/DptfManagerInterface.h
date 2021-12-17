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

#include "Dptf.h"
#include "esif_sdk_data.h"
#include "esif_sdk_iface_app.h"
#include "esif_sdk_iface_esif.h"
#include "IndexContainerInterface.h"
#include "EventCache.h"
#include "UserPreferredCache.h"
#include "CommandDispatcher.h"
#include "RequestDispatcher.h"

class EsifServicesInterface;
class WorkItemQueueManagerInterface;
class PolicyManagerInterface;
class ParticipantManagerInterface;
class DptfStatusInterface;
class DataManagerInterface;
class SystemModeManagerInterface;

class DptfManagerInterface
{
public:
	virtual ~DptfManagerInterface(){};

	virtual void createDptfManager(
		const esif_handle_t esifHandle,
		EsifInterfacePtr esifInterfacePtr,
		const std::string& dptfHomeDirectoryPath,
		eLogType currentLogVerbosityLevel,
		Bool dptfEnabled) = 0;
	virtual Bool isDptfManagerCreated(void) const = 0;
	virtual Bool isDptfShuttingDown(void) const = 0;
	virtual Bool isWorkItemQueueManagerCreated(void) const = 0;
	virtual EsifServicesInterface* getEsifServices(void) const = 0;
	virtual std::shared_ptr<EventCache> getEventCache(void) const = 0;
	virtual std::shared_ptr<UserPreferredCache> getUserPreferredCache(void) const = 0;
	virtual WorkItemQueueManagerInterface* getWorkItemQueueManager(void) const = 0;
	virtual PolicyManagerInterface* getPolicyManager(void) const = 0;
	virtual ParticipantManagerInterface* getParticipantManager(void) const = 0;
	virtual ICommandDispatcher* getCommandDispatcher() const = 0;
	virtual DptfStatusInterface* getDptfStatus(void) = 0;
	virtual IndexContainerInterface* getIndexContainer(void) const = 0;
	virtual DataManagerInterface* getDataManager(void) const = 0;
	virtual std::string getDptfHomeDirectoryPath(void) const = 0;
	virtual std::string getDptfPolicyDirectoryPath(void) const = 0;
	virtual std::string getDptfReportDirectoryPath(void) const = 0;
	virtual void bindDomainsToPolicies(UIntN participantIndex) const = 0;
	virtual void unbindDomainsFromPolicies(UIntN participantIndex) const = 0;
	virtual void bindParticipantToPolicies(UIntN participantIndex) const = 0;
	virtual void unbindParticipantFromPolicies(UIntN participantIndex) const = 0;
	virtual void bindAllParticipantsToPolicy(UIntN policyIndex) const = 0;
	virtual std::shared_ptr<RequestDispatcherInterface> getRequestDispatcher() const = 0;
	virtual std::shared_ptr<RequestHandlerInterface> getPlatformRequestHandler() const = 0;
	virtual SystemModeManagerInterface* getSystemModeManager() const = 0;
};
