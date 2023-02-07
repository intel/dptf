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

#include "DptfManager.h"
#include "EsifServices.h"
#include "WorkItemQueueManager.h"
#include "PolicyManager.h"
#include "ParticipantManager.h"
#include "UniqueIdGenerator.h"
#include "IndexContainer.h"
#include "esif_sdk_iface_app.h"
#include "DptfStatus.h"
#include "EsifDataString.h"
#include "EsifAppServices.h"
#include "StringParser.h"
#include <memory>
#include "HelpCommand.h"
#include "DiagCommand.h"
#include "ReloadCommand.h"
#include "StatusCommand.h"
#include "TableObjectCommand.h"
#include "ConfigCommand.h"
#include "ManagerLogger.h"
#include "PlatformRequestHandler.h"
#include "EsifLibrary.h"
#include "DataManager.h"
#include "SystemModeManager.h"

DptfManager::DptfManager(void)
	: m_dptfManagerCreateStarted(false)
	, m_dptfManagerCreateFinished(false)
	, m_dptfShuttingDown(false)
	, m_workItemQueueManagerCreated(false)
	, m_dptfEnabled(false)
	, m_esifAppServices(nullptr)
	, m_esifServices(nullptr)
	, m_workItemQueueManager(nullptr)
	, m_policyManager(nullptr)
	, m_participantManager(nullptr)
	, m_commandDispatcher(nullptr)
	, m_commands()
	, m_fileIo(nullptr)
	, m_dptfStatus(nullptr)
	, m_indexContainer(nullptr)
	, m_dataManager(nullptr)
	, m_systemModeManager(nullptr)
	, m_dptfHomeDirectoryPath(Constants::EmptyString)
	, m_dptfPolicyDirectoryPath(Constants::EmptyString)
	, m_dptfReportDirectoryPath(Constants::EmptyString)
{
}

void DptfManager::createDptfManager(
	const esif_handle_t esifHandle,
	EsifInterfacePtr esifInterfacePtr,
	const std::string& dptfHomeDirectoryPath,
	eLogType currentLogVerbosityLevel,
	Bool dptfEnabled)
{
	if (m_dptfManagerCreateStarted == true)
	{
		throw dptf_exception("DptfManager::createDptfManager() already executed.");
	}
	m_dptfManagerCreateStarted = true;

	try
	{
		// HomePath is a Directory writable by this process, so use for ReportPath too
		std::string homePath = dptfHomeDirectoryPath;
		std::string reportPath = dptfHomeDirectoryPath;

		// Determine Policy Path by getting this Library's Full Pathname
		EsifLibrary dptfLib;
		dptfLib.load();
		std::string policyPath = dptfLib.getLibDirectory();
		dptfLib.unload();
		if (!policyPath.empty())
		{
			m_dptfPolicyDirectoryPath = policyPath;
		}

		if (homePath.back() != *ESIF_PATH_SEP)
		{
			homePath += ESIF_PATH_SEP;
		}
		if (policyPath.back() != *ESIF_PATH_SEP)
		{
			policyPath += ESIF_PATH_SEP;
		}
		if (reportPath.back() != *ESIF_PATH_SEP)
		{
			reportPath += ESIF_PATH_SEP;
		}
		m_dptfHomeDirectoryPath = homePath;
		m_dptfPolicyDirectoryPath = policyPath;
		m_dptfReportDirectoryPath = reportPath;
		m_dptfEnabled = dptfEnabled;

		EsifData eventData = {ESIF_DATA_VOID, NULL, 0, 0};
		m_eventCache = std::make_shared<EventCache>();
		m_userPreferredCache = std::make_shared<UserPreferredCache>();
		m_indexContainer = new IndexContainer();
		m_esifAppServices = new EsifAppServices(esifInterfacePtr);
		m_esifServices = new EsifServices(this, esifHandle, m_esifAppServices, currentLogVerbosityLevel);
		m_participantManager = new ParticipantManager(this);
		m_commandDispatcher = new CommandDispatcher();
		m_fileIo = std::make_shared<FileIO>();
		createCommands();
		registerCommands();

		m_dataManager = new DataManager(this);

		m_policyManager = new PolicyManager(this);

		m_systemModeManager = new SystemModeManager(this);

		// Make sure to create these AFTER creating the ParticipantManager and PolicyManager
		m_workItemQueueManager = new WorkItemQueueManager(this);
		m_workItemQueueManagerCreated = true;

		m_requestDispatcher = std::make_shared<RequestDispatcher>();
		m_platformRequestHandler = std::make_shared<PlatformRequestHandler>(this);

		m_dptfStatus = new DptfStatus(this);

		m_policyManager->createAllPolicies(m_dptfPolicyDirectoryPath);

		registerDptfFrameworkEvents();
		m_systemModeManager->registerFrameworkEvents();

		m_dptfManagerCreateFinished = true;

		m_esifServices->sendDptfEvent(FrameworkEvent::DptfAppLoaded, Constants::Invalid, Constants::Invalid, eventData);
	}
	catch (std::exception& ex)
	{
		std::stringstream message;
		message << "The DPTF application has failed to start." << std::endl;
		message << ex.what() << std::endl;
		esifInterfacePtr->fWriteLogFuncPtr(
			esifHandle,
			ESIF_INVALID_HANDLE,
			ESIF_INVALID_HANDLE,
			EsifDataString(message.str()),
			eLogType::eLogTypeFatal);
	}

	if (m_dptfManagerCreateFinished == false)
	{
		shutDown();
		throw dptf_exception("Failed to start DPTF");
	}
}

DptfManager::~DptfManager(void)
{
	unregisterCommands();
	shutDown();
}

Bool DptfManager::isDptfManagerCreated(void) const
{
	return m_dptfManagerCreateFinished;
}

Bool DptfManager::isDptfShuttingDown(void) const
{
	return m_dptfShuttingDown;
}

Bool DptfManager::isWorkItemQueueManagerCreated(void) const
{
	return m_workItemQueueManagerCreated;
}

EsifServicesInterface* DptfManager::getEsifServices(void) const
{
	return m_esifServices;
}

WorkItemQueueManagerInterface* DptfManager::getWorkItemQueueManager(void) const
{
	return m_workItemQueueManager;
}

PolicyManagerInterface* DptfManager::getPolicyManager(void) const
{
	return m_policyManager;
}

ParticipantManagerInterface* DptfManager::getParticipantManager(void) const
{
	return m_participantManager;
}

ICommandDispatcher* DptfManager::getCommandDispatcher() const
{
	return m_commandDispatcher;
}

DptfStatusInterface* DptfManager::getDptfStatus(void)
{
	return m_dptfStatus;
}

IndexContainerInterface* DptfManager::getIndexContainer(void) const
{
	return m_indexContainer;
}

DataManagerInterface* DptfManager::getDataManager(void) const
{
	return m_dataManager;
}

SystemModeManagerInterface* DptfManager::getSystemModeManager(void) const
{
	return m_systemModeManager;
}

std::string DptfManager::getDptfHomeDirectoryPath(void) const
{
	return m_dptfHomeDirectoryPath;
}

std::string DptfManager::getDptfPolicyDirectoryPath(void) const
{
	return m_dptfPolicyDirectoryPath;
}

std::string DptfManager::getDptfReportDirectoryPath(void) const
{
	return m_dptfReportDirectoryPath;
}

void DptfManager::shutDown(void)
{
	EsifData eventData = {ESIF_DATA_VOID, NULL, 0, 0};
	m_dptfShuttingDown = true;
	m_dptfEnabled = false;

	m_esifServices->sendDptfEvent(FrameworkEvent::DptfAppUnloaded, Constants::Invalid, Constants::Invalid, eventData);
	unregisterDptfFrameworkEvents();

	disableAndEmptyAllQueues();
	destroyAllPolicies();
	destroyAllParticipants();
	deleteWorkItemQueueManager();
	deleteSystemModeManager();
	deletePolicyManager();
	deleteParticipantManager();
	deleteEsifServices();
	deleteEsifAppServices();
	deleteIndexContainer();
	destroyUniqueIdGenerator();
	destroyFrameworkEventInfo();
	deleteDptfStatus();
	DELETE_MEMORY_TC(m_commandDispatcher);
	DELETE_MEMORY_TC(m_dataManager);
}

void DptfManager::disableAndEmptyAllQueues(void)
{
	try
	{
		// Disable enqueueing of new work items and destroy the items already in the queue.  Once this executes
		// the only work items that can get added to the queue are WIPolicyDestroy and WIParticipantDestroy.  These
		// are coming up next.
		if (m_workItemQueueManager != nullptr)
		{
			m_workItemQueueManager->disableAndEmptyAllQueues();
		}
	}
	catch (...)
	{
	}
}

void DptfManager::destroyAllPolicies(void)
{
	try
	{
		// Destroy all policies.  The policy manager will enqueue a work item for each of these and return
		// once all work items have executed.
		if (m_policyManager != nullptr)
		{
			m_policyManager->destroyAllPolicies();
		}
	}
	catch (...)
	{
	}
}

void DptfManager::destroyAllParticipants(void)
{
	try
	{
		// Destroy all participants.  The participant manager will enqueue a work item for each of these and
		// return once all work items have executed.
		if (m_participantManager != nullptr)
		{
			m_participantManager->destroyAllParticipants();
		}
	}
	catch (...)
	{
	}
}

void DptfManager::deleteDptfStatus()
{
	DELETE_MEMORY_TC(m_dptfStatus);
}

void DptfManager::deleteWorkItemQueueManager(void)
{
	DELETE_MEMORY_TC(m_workItemQueueManager);
}

void DptfManager::deletePolicyManager(void)
{
	DELETE_MEMORY_TC(m_policyManager);
}

void DptfManager::deleteParticipantManager(void)
{
	DELETE_MEMORY_TC(m_participantManager);
}

void DptfManager::deleteSystemModeManager(void)
{
	getSystemModeManager()->unregisterFrameworkEvents();
	DELETE_MEMORY_TC(m_systemModeManager);
}

void DptfManager::deleteEsifAppServices(void)
{
	DELETE_MEMORY_TC(m_esifAppServices);
}

void DptfManager::deleteEsifServices(void)
{
	DELETE_MEMORY_TC(m_esifServices);
}

void DptfManager::deleteIndexContainer(void)
{
	DELETE_MEMORY_TC(m_indexContainer);
}

void DptfManager::destroyUniqueIdGenerator(void)
{
	try
	{
		UniqueIdGenerator::destroy();
	}
	catch (...)
	{
	}
}

void DptfManager::destroyFrameworkEventInfo(void)
{
	try
	{
		FrameworkEventInfo::destroy();
	}
	catch (...)
	{
	}
}

void DptfManager::registerDptfFrameworkEvents(void)
{
	// FIXME:  Do these belong here?
	//  DptfConnectedStandbyEntry
	//  DptfConnectedStandbyExit

	try
	{
		m_esifServices->registerEvent(FrameworkEvent::DptfConnectedStandbyEntry);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->registerEvent(FrameworkEvent::DptfConnectedStandbyExit);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->registerEvent(FrameworkEvent::DptfLogVerbosityChanged);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->registerEvent(FrameworkEvent::DptfSupportedPoliciesChanged);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->registerEvent(FrameworkEvent::DptfParticipantActivityLoggingEnabled);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->registerEvent(FrameworkEvent::DptfParticipantActivityLoggingDisabled);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->registerEvent(FrameworkEvent::DptfPolicyActivityLoggingEnabled);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->registerEvent(FrameworkEvent::DptfPolicyActivityLoggingDisabled);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->registerEvent(FrameworkEvent::DptfAppAliveRequest);
	}
	catch (...)
	{
	}
}

void DptfManager::unregisterDptfFrameworkEvents(void)
{
	try
	{
		m_esifServices->unregisterEvent(FrameworkEvent::DptfConnectedStandbyEntry);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->unregisterEvent(FrameworkEvent::DptfConnectedStandbyExit);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->unregisterEvent(FrameworkEvent::DptfLogVerbosityChanged);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->unregisterEvent(FrameworkEvent::DptfSupportedPoliciesChanged);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->unregisterEvent(FrameworkEvent::DptfParticipantActivityLoggingEnabled);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->unregisterEvent(FrameworkEvent::DptfParticipantActivityLoggingDisabled);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->unregisterEvent(FrameworkEvent::DptfPolicyActivityLoggingEnabled);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->unregisterEvent(FrameworkEvent::DptfPolicyActivityLoggingDisabled);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->unregisterEvent(FrameworkEvent::DptfAppAliveRequest);
	}
	catch (...)
	{
	}
}

void DptfManager::registerCommands()
{
	for (auto command = m_commands.begin(); command != m_commands.end(); ++command)
	{
		m_commandDispatcher->registerHandler((*command)->getCommandName(), *command);
	}
}

void DptfManager::unregisterCommands()
{
	for (auto command = m_commands.begin(); command != m_commands.end(); ++command)
	{
		m_commandDispatcher->unregisterHandler((*command)->getCommandName());
	}
}

void DptfManager::createCommands()
{
	m_commands.push_back(std::make_shared<HelpCommand>(this));
	m_commands.push_back(std::make_shared<DiagCommand>(this, m_fileIo));
	m_commands.push_back(std::make_shared<ReloadCommand>(this));
	m_commands.push_back(std::make_shared<StatusCommand>(this));
	m_commands.push_back(std::make_shared<TableObjectCommand>(this));
	m_commands.push_back(std::make_shared<ConfigCommand>(this));
}

std::shared_ptr<EventCache> DptfManager::getEventCache(void) const
{
	return m_eventCache;
}

std::shared_ptr<UserPreferredCache> DptfManager::getUserPreferredCache(void) const
{
	return m_userPreferredCache;
}

void DptfManager::bindDomainsToPolicies(UIntN participantIndex) const
{
	UIntN domainCount = m_participantManager->getParticipantPtr(participantIndex)->getDomainCount();

	for (UIntN domainIndex = 0; domainIndex < domainCount; domainIndex++)
	{
		auto policyIndexes = m_policyManager->getPolicyIndexes();
		for (auto policyIndex = policyIndexes.begin(); policyIndex != policyIndexes.end(); ++policyIndex)
		{
			try
			{
				auto policy = m_policyManager->getPolicyPtr(*policyIndex);
				policy->bindDomain(participantIndex, domainIndex);
			}
			catch (dptf_exception& ex)
			{
				MANAGER_LOG_MESSAGE_WARNING_EX({
					ManagerMessage message = ManagerMessage(
						this,
						_file,
						_line,
						_function,
						"DPTF was not able to bind domain to policies: " + ex.getDescription() + ".");
					message.addMessage("Participant Index", participantIndex);
					message.addMessage("Domain Index", domainIndex);
					message.addMessage("Policy Index", *policyIndex);
					return message;
				});
			}
			catch (...)
			{
				MANAGER_LOG_MESSAGE_WARNING({
					ManagerMessage message =
						ManagerMessage(this, _file, _line, _function, "DptfManager::bindDomainsToPolicies Failed.");
					message.addMessage("Participant Index", participantIndex);
					message.addMessage("Domain Index", domainIndex);
					message.addMessage("Policy Index", *policyIndex);
					return message;
				});
			}
		}
	}
}

void DptfManager::unbindDomainsFromPolicies(UIntN participantIndex) const
{
	UIntN domainCount = m_participantManager->getParticipantPtr(participantIndex)->getDomainCount();

	for (UIntN domainIndex = 0; domainIndex < domainCount; domainIndex++)
	{
		auto policyIndexes = m_policyManager->getPolicyIndexes();
		for (auto policyIndex = policyIndexes.begin(); policyIndex != policyIndexes.end(); ++policyIndex)
		{
			try
			{
				auto policy = m_policyManager->getPolicyPtr(*policyIndex);
				policy->unbindDomain(participantIndex, domainIndex);
			}
			catch (dptf_exception& ex)
			{
				MANAGER_LOG_MESSAGE_WARNING_EX({
					ManagerMessage message = ManagerMessage(
						this,
						_file,
						_line,
						_function,
						"DPTF was not able to unbind domain from policies: " + ex.getDescription() + ".");
					message.addMessage("Participant Index", participantIndex);
					message.addMessage("Domain Index", domainIndex);
					message.addMessage("Policy Index", *policyIndex);
					return message;
				});
			}
			catch (...)
			{
				MANAGER_LOG_MESSAGE_WARNING({
					ManagerMessage message =
						ManagerMessage(this, _file, _line, _function, "DptfManager::unbindDomainsFromPolicies Failed.");
					message.addMessage("Participant Index", participantIndex);
					message.addMessage("Domain Index", domainIndex);
					message.addMessage("Policy Index", *policyIndex);
					return message;
				});
			}
		}
	}
}

void DptfManager::bindParticipantToPolicies(UIntN participantIndex) const
{
	auto policyIndexes = m_policyManager->getPolicyIndexes();
	for (auto policyIndex = policyIndexes.begin(); policyIndex != policyIndexes.end(); ++policyIndex)
	{
		try
		{
			auto policy = m_policyManager->getPolicyPtr(*policyIndex);
			policy->bindParticipant(participantIndex);
		}
		catch (dptf_exception& ex)
		{
			MANAGER_LOG_MESSAGE_WARNING_EX({
				ManagerMessage message = ManagerMessage(
					this,
					_file,
					_line,
					_function,
					"DPTF was not able to bind participant to policies: " + ex.getDescription() + ".");
				message.addMessage("Participant Index", participantIndex);
				message.addMessage("Policy Index", *policyIndex);
				return message;
			});
		}
		catch (...)
		{
			MANAGER_LOG_MESSAGE_WARNING({
				ManagerMessage message =
					ManagerMessage(this, _file, _line, _function, "DptfManager::bindParticipantToPolicies Failed.");
				message.addMessage("Participant Index", participantIndex);
				message.addMessage("Policy Index", *policyIndex);
				return message;
			});
		}
	}
}

void DptfManager::unbindParticipantFromPolicies(UIntN participantIndex) const
{
	auto policyIndexes = m_policyManager->getPolicyIndexes();
	for (auto policyIndex = policyIndexes.begin(); policyIndex != policyIndexes.end(); ++policyIndex)
	{
		try
		{
			auto policy = m_policyManager->getPolicyPtr(*policyIndex);
			policy->unbindParticipant(participantIndex);
		}
		catch (dptf_exception& ex)
		{
			MANAGER_LOG_MESSAGE_WARNING_EX({
				ManagerMessage message = ManagerMessage(
					this,
					_file,
					_line,
					_function,
					"DPTF was not able to unbind participant from policies: " + ex.getDescription() + ".");
				message.addMessage("Participant Index", participantIndex);
				message.addMessage("Policy Index", *policyIndex);
				return message;
			});
		}
		catch (...)
		{
			MANAGER_LOG_MESSAGE_WARNING({
				ManagerMessage message =
					ManagerMessage(this, _file, _line, _function, "DptfManager::unbindParticipantFromPolicies Failed.");
				message.addMessage("Participant Index", participantIndex);
				message.addMessage("Policy Index", *policyIndex);
				return message;
			});
		}
	}
}

void DptfManager::bindAllParticipantsToPolicy(UIntN policyIndex) const
{
	auto policy = m_policyManager->getPolicyPtr(policyIndex);
	auto participantIndexes = m_participantManager->getParticipantIndexes();
	for (auto participantIndex = participantIndexes.begin(); participantIndex != participantIndexes.end();
		 ++participantIndex)
	{
		try
		{
			policy->bindParticipant(*participantIndex);

			UIntN domainCount = m_participantManager->getParticipantPtr(*participantIndex)->getDomainCount();

			for (UIntN domainIndex = 0; domainIndex < domainCount; domainIndex++)
			{
				policy->bindDomain(*participantIndex, domainIndex);
			}
		}
		catch (dptf_exception& ex)
		{
			MANAGER_LOG_MESSAGE_WARNING_EX({
				ManagerMessage message = ManagerMessage(
					this,
					_file,
					_line,
					_function,
					"DPTF was not able to bind participant and domains to policy: " + ex.getDescription() + ".");
				message.addMessage("Participant Index", *participantIndex);
				message.addMessage("Policy Index", policyIndex);
				return message;
			});
		}
		catch (...)
		{
			MANAGER_LOG_MESSAGE_WARNING({
				ManagerMessage message =
					ManagerMessage(this, _file, _line, _function, "DptfManager::bindAllParticipantsToPolicy Failed.");
				message.addMessage("Participant Index", *participantIndex);
				message.addMessage("Policy Index", policyIndex);

				return message;
			});
		}
	}
}

std::shared_ptr<RequestDispatcherInterface> DptfManager::getRequestDispatcher() const
{
	return m_requestDispatcher;
}

std::shared_ptr<RequestHandlerInterface> DptfManager::getPlatformRequestHandler() const
{
	return m_platformRequestHandler;
}
