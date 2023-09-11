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

#include "DptfManagerInterface.h"
#include "EsifAppServicesInterface.h"
#include "EventCache.h"
#include "UserPreferredCache.h"
#include "CommandDispatcher.h"
#include "FileIo.h"
#include "ConfigurationFileManager.h"
#include "FilePathDirectory.h"
#include "EsifMessageLogger.h"
#include "ParticipantRequestHandler.h"
#include "EventNotifierInterface.h"

class EnvironmentProfileGenerator;
class EnvironmentProfileUpdater;
class EsifServicesInterface;
class WorkItemQueueManagerInterface;
class PolicyManagerInterface;
class ParticipantManagerInterface;
class DataManagerInterface;
class SystemModeManagerInterface;

//
// DPTF starts here!!!
//

class DptfManager : public DptfManagerInterface
{
public:
	// This is in place so we can create the handle for esif before it calls back to createDptfManager()
	DptfManager();

	// This will shut down DPTF and clean up resources.
	~DptfManager() override;

	// This will create all of the DPTF subsystems.
	void createDptfManager(
		const esif_handle_t esifHandle,
		EsifInterfacePtr esifInterfacePtr,
		const std::string& dptfHomeDirectoryPath,
		eLogType currentLogVerbosityLevel,
		Bool dptfEnabled) override;

	Bool isDptfManagerCreated() const override;
	Bool isDptfShuttingDown() const override;
	Bool isWorkItemQueueManagerCreated() const override;

	EsifServicesInterface* getEsifServices() const override;
	std::shared_ptr<EventCache> getEventCache() const override;
	std::shared_ptr<EventNotifierInterface> getEventNotifier() const override;
	std::shared_ptr<UserPreferredCache> getUserPreferredCache() const override;
	WorkItemQueueManagerInterface* getWorkItemQueueManager() const override;
	PolicyManagerInterface* getPolicyManager() const override;
	ParticipantManagerInterface* getParticipantManager() const override;
	ICommandDispatcher* getCommandDispatcher() const override;
	IndexContainerInterface* getIndexContainer() const override;
	DataManagerInterface* getDataManager() const override;
	std::shared_ptr<RequestDispatcherInterface> getRequestDispatcher() const override;
	std::shared_ptr<RequestHandlerInterface> getPlatformRequestHandler() const override;
	SystemModeManagerInterface* getSystemModeManager() const override;
	std::shared_ptr<ConfigurationFileManagerInterface> getConfigurationManager() const override;
	EnvironmentProfile getEnvironmentProfile() const override;
	std::shared_ptr<EnvironmentProfileGenerator> getEnvironmentProfileGenerator() const override;

	std::string getDptfPolicyDirectoryPath() const override;
	std::string getDptfReportDirectoryPath() const override;

	void bindDomainsToPolicies(UIntN participantIndex) const override;
	void unbindDomainsFromPolicies(UIntN participantIndex) const override;
	void bindParticipantToPolicies(UIntN participantIndex) const override;
	void unbindParticipantFromPolicies(UIntN participantIndex) const override;
	void bindAllParticipantsToPolicy(UIntN policyIndex) const override;

	void setCurrentLogVerbosityLevel(eLogType level) override;

private:
	// hide the copy constructor and assignment operator.
	DptfManager(const DptfManager& rhs);
	DptfManager& operator=(const DptfManager& rhs);

	Bool m_dptfManagerCreateStarted;
	Bool m_dptfManagerCreateFinished;
	Bool m_dptfShuttingDown;
	Bool m_workItemQueueManagerCreated;

	Bool m_dptfEnabled;

	// EsifServices is the only way to make calls back to ESIF.
	EsifAppServicesInterface* m_esifAppServices;
	EsifServicesInterface* m_esifServices;

	// All work item threads, enqueueing, dequeuing, and work item dispatch is handled by the WorkItemQueueManager.
	WorkItemQueueManagerInterface* m_workItemQueueManager;

	// Manages all of the polices and events that are registered for each policy.  When PolicyManager is instantiated
	// each policy is also created.
	PolicyManagerInterface* m_policyManager;

	// Manages all of the participants.  When ParticipantManager is first created there are no participants.  When
	// ESIF calls AppParticipantCreate() and AppDomainCreate() the participants will get created and the policies
	// will be notified as they come in to the DPTF framework.
	ParticipantManagerInterface* m_participantManager;

	// All commands register with the dispatcher at start up.
	// The dispatcher will direct commands to their proper handlers.
	ICommandDispatcher* m_commandDispatcher;
	std::list<std::shared_ptr<CommandHandler>> m_commands;
	std::shared_ptr<RequestDispatcherInterface> m_requestDispatcher;
	std::shared_ptr<RequestHandlerInterface> m_platformRequestHandler;

	std::shared_ptr<ConfigurationFileManagerInterface> m_configurationManager;
	std::shared_ptr<RequestHandlerInterface> m_participantRequestHandlers;

	std::shared_ptr<EventCache> m_eventCache;
	std::shared_ptr<EnvironmentProfileUpdater> m_environmentProfileUpdater;
	std::shared_ptr<EnvironmentProfileGenerator> m_environmentProfileGenerator;
	std::shared_ptr<EventNotifierInterface> m_eventNotifier;
	std::shared_ptr<UserPreferredCache> m_userPreferredCache;
	IndexContainerInterface* m_indexContainer;

	DataManagerInterface* m_dataManager;
	SystemModeManagerInterface* m_systemModeManager;

	std::shared_ptr<FilePathDirectory> m_filePathDirectory;
	std::shared_ptr<LogMessageFilter> m_messageLogFilter;
	std::shared_ptr<MessageLogger> m_messageLogger;
	std::shared_ptr<IFileIo> m_fileIo;

	// start up helpers
	void createBasicObjects(
		const std::string& dptfHomeDirectoryPath,
		eLogType currentLogVerbosityLevel,
		Bool dptfEnabled);
	void createBasicServices(
		esif_handle_t esifHandle,
		EsifInterfacePtr esifInterfacePtr,
		eLogType currentLogVerbosityLevel);
	std::set<Guid> readDefaultEnabledPolicies() const;
	void notifyAppsThatDttHasLoaded() const;
	void createPolicies();
	void registerForEvents() const;
	void registerCommands() const;
	void createCommands();
	void registerDptfFrameworkEvents() const;

	// shutdown helpers
	void shutDown();
	void disableAndEmptyAllQueues() const;
	void destroyAllPolicies() const;
	void destroyAllParticipants() const;
	void deleteWorkItemQueueManager();
	void deletePolicyManager();
	void deleteParticipantManager();
	void deleteSystemModeManager();
	void deleteEsifAppServices();
	void deleteEsifServices();
	void deleteIndexContainer();
	static void destroyUniqueIdGenerator();
	static void destroyFrameworkEventInfo();
	void unregisterDptfFrameworkEvents() const;
	void unregisterCommands() const;
};
