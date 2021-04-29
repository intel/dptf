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

#include "DptfManagerInterface.h"
#include "EsifAppServicesInterface.h"
#include "EventCache.h"
#include "UserPreferredCache.h"
#include "CommandDispatcher.h"
#include "FileIO.h"

class EsifServicesInterface;
class WorkItemQueueManagerInterface;
class PolicyManagerInterface;
class ParticipantManagerInterface;
class DptfStatusInterface;
class DataManagerInterface;

//
// DPTF starts here!!!
//

class DptfManager : public DptfManagerInterface
{
public:
	// This is in place so we can create the handle for esif before it calls back to createDptfManager()
	DptfManager(void);

	// This will shut down DPTF and clean up resources.
	virtual ~DptfManager(void);

	// This will create all of the DPTF subsystems.
	virtual void createDptfManager(
		const esif_handle_t esifHandle,
		EsifInterfacePtr esifInterfacePtr,
		const std::string& dptfHomeDirectoryPath,
		eLogType currentLogVerbosityLevel,
		Bool dptfEnabled) override;

	virtual Bool isDptfManagerCreated(void) const override;
	virtual Bool isDptfShuttingDown(void) const override;
	virtual Bool isWorkItemQueueManagerCreated(void) const override;

	virtual EsifServicesInterface* getEsifServices(void) const override;
	virtual std::shared_ptr<EventCache> getEventCache(void) const override;
	virtual std::shared_ptr<UserPreferredCache> getUserPreferredCache(void) const override;
	virtual WorkItemQueueManagerInterface* getWorkItemQueueManager(void) const override;
	virtual PolicyManagerInterface* getPolicyManager(void) const override;
	virtual ParticipantManagerInterface* getParticipantManager(void) const override;
	virtual ICommandDispatcher* getCommandDispatcher() const override;
	virtual DptfStatusInterface* getDptfStatus(void) override;
	virtual IndexContainerInterface* getIndexContainer(void) const override;
	virtual DataManagerInterface* getDataManager(void) const override;
	virtual std::shared_ptr<RequestDispatcherInterface> getRequestDispatcher() const override;
	virtual std::shared_ptr<RequestHandlerInterface> getPlatformRequestHandler() const override;

	virtual std::string getDptfHomeDirectoryPath(void) const override;
	virtual std::string getDptfPolicyDirectoryPath(void) const override;
	virtual std::string getDptfReportDirectoryPath(void) const override;
	virtual Bool isDptfPolicyLoadNameOnly(void) const override;

	void bindDomainsToPolicies(UIntN participantIndex) const override;
	void unbindDomainsFromPolicies(UIntN participantIndex) const override;
	void bindParticipantToPolicies(UIntN participantIndex) const override;
	void unbindParticipantFromPolicies(UIntN participantIndex) const override;
	void bindAllParticipantsToPolicy(UIntN policyIndex) const override;

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
	std::shared_ptr<IFileIO> m_fileIo;
	std::shared_ptr<RequestDispatcherInterface> m_requestDispatcher;

	std::shared_ptr<RequestHandlerInterface> m_platformRequestHandler;

	std::shared_ptr<EventCache> m_eventCache;
	std::shared_ptr<UserPreferredCache> m_userPreferredCache;

	// Creates XML needed for requests from the UI
	DptfStatusInterface* m_dptfStatus;

	IndexContainerInterface* m_indexContainer;

	DataManagerInterface* m_dataManager;

	std::string m_dptfHomeDirectoryPath;
	std::string m_dptfPolicyDirectoryPath;
	std::string m_dptfReportDirectoryPath;
	Bool m_dptfPolicyLoadNameOnly;

	void shutDown(void);
	void disableAndEmptyAllQueues(void);
	void deleteDptfStatus(void);
	void destroyAllPolicies(void);
	void destroyAllParticipants(void);
	void deleteWorkItemQueueManager(void);
	void deletePolicyManager(void);
	void deleteParticipantManager(void);
	void deleteEsifAppServices(void);
	void deleteEsifServices(void);
	void deleteIndexContainer(void);
	void destroyUniqueIdGenerator(void);
	void destroyFrameworkEventInfo(void);

	void registerDptfFrameworkEvents(void);
	void unregisterDptfFrameworkEvents(void);
	void registerCommands();
	void createCommands();
	void unregisterCommands();
};
