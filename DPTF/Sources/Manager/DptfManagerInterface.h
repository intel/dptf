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

#pragma once

#include "Dptf.h"
#include "esif_sdk_data.h"
#include "esif_sdk_iface_app.h"
#include "esif_sdk_iface_esif.h"
#include "IndexContainerInterface.h"
#include "EventCache.h"
#include "UserPreferredCache.h"
#include "CommandDispatcher.h"
#include "ConfigurationFileManager.h"
#include "RequestDispatcher.h"

class EnvironmentProfileGenerator;
class EventNotifierInterface;
class EsifServicesInterface;
class WorkItemQueueManagerInterface;
class PolicyManagerInterface;
class ParticipantManagerInterface;
class DataManagerInterface;
class SystemModeManagerInterface;

class DptfManagerInterface
{
public:
	virtual ~DptfManagerInterface(){}
	virtual void createDptfManager(
		const esif_handle_t esifHandle,
		EsifInterfacePtr esifInterfacePtr,
		const std::string& dptfHomeDirectoryPath,
		eLogType currentLogVerbosityLevel,
		Bool dptfEnabled) = 0;
	virtual Bool isDptfManagerCreated() const = 0;
	virtual Bool isDptfShuttingDown() const = 0;
	virtual Bool isWorkItemQueueManagerCreated() const = 0;
	virtual EsifServicesInterface* getEsifServices() const = 0;
	virtual std::shared_ptr<EventCache> getEventCache() const = 0;
	virtual std::shared_ptr<EventNotifierInterface> getEventNotifier() const = 0;
	virtual std::shared_ptr<UserPreferredCache> getUserPreferredCache() const = 0;
	virtual WorkItemQueueManagerInterface* getWorkItemQueueManager() const = 0;
	virtual PolicyManagerInterface* getPolicyManager() const = 0;
	virtual ParticipantManagerInterface* getParticipantManager() const = 0;
	virtual ICommandDispatcher* getCommandDispatcher() const = 0;
	virtual IndexContainerInterface* getIndexContainer() const = 0;
	virtual DataManagerInterface* getDataManager() const = 0;
	virtual std::string getDptfPolicyDirectoryPath() const = 0;
	virtual std::string getDptfReportDirectoryPath() const = 0;
	virtual void bindDomainsToPolicies(UIntN participantIndex) const = 0;
	virtual void unbindDomainsFromPolicies(UIntN participantIndex) const = 0;
	virtual void bindParticipantToPolicies(UIntN participantIndex) const = 0;
	virtual void unbindParticipantFromPolicies(UIntN participantIndex) const = 0;
	virtual void bindAllParticipantsToPolicy(UIntN policyIndex) const = 0;
	virtual std::shared_ptr<RequestDispatcherInterface> getRequestDispatcher() const = 0;
	virtual std::shared_ptr<RequestHandlerInterface> getPlatformRequestHandler() const = 0;
	virtual SystemModeManagerInterface* getSystemModeManager() const = 0;
	virtual std::shared_ptr<ConfigurationFileManagerInterface> getConfigurationManager() const = 0;
	virtual void setCurrentLogVerbosityLevel(eLogType level) = 0;
	virtual EnvironmentProfile getEnvironmentProfile() const = 0;
	virtual std::shared_ptr<EnvironmentProfileGenerator> getEnvironmentProfileGenerator() const = 0;
};
