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

#include "DptfManager.h"
#include "EsifServices.h"
#include "WorkItemQueueManager.h"
#include "PolicyManager.h"
#include "ParticipantManager.h"
#include "UniqueIdGenerator.h"
#include "IndexContainer.h"
#include "esif_sdk_iface_app.h"
#include "EsifDataString.h"
#include "EsifAppServices.h"
#include <memory>
#include <string>
#include "HelpCommand.h"
#include "DiagCommand.h"
#include "ReloadCommand.h"
#include "TableObjectCommand.h"
#include "ConfigCommand.h"
#include "ManagerLogger.h"
#include "PlatformRequestHandler.h"
#include "DataManager.h"
#include "SystemModeManager.h"
#include "UiCommand.h"
#include "CaptureCommand.h"
#include "DttConfiguration.h"
#include "PlatformCpuIdCommand.h"
#include "RealEnvironmentProfileGenerator.h"
#include "PoliciesCommand.h"
#include "RealEnvironmentProfileUpdater.h"
#include "RealEventNotifier.h"

using namespace std;

const string DttConfigurationName{"dtt"s};
const DttConfigurationQuery DttConfigurationDefaultPoliciesQuery{"/DefaultPolicies/.*"s};

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
	, m_indexContainer(nullptr)
	, m_dataManager(nullptr)
	, m_systemModeManager(nullptr)
	, m_fileIo(nullptr)
{
}

void DptfManager::createDptfManager(
	const esif_handle_t esifHandle,
	EsifInterfacePtr esifInterfacePtr,
	const string& dptfHomeDirectoryPath,
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
		createBasicObjects(dptfHomeDirectoryPath, currentLogVerbosityLevel, dptfEnabled);
		createBasicServices(esifHandle, esifInterfacePtr, currentLogVerbosityLevel);
		createCommands();
		createPolicies();
		registerForEvents();
		notifyAppsThatDttHasLoaded();
		m_dptfManagerCreateFinished = true;
	}
	catch (exception& ex)
	{
		stringstream message;
		message << "The DPTF application has failed to start." << endl;
		message << ex.what() << endl;
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

void DptfManager::createBasicObjects(
	const string& dptfHomeDirectoryPath,
	eLogType currentLogVerbosityLevel,
	Bool dptfEnabled)
{
	m_dptfEnabled = dptfEnabled;
	m_filePathDirectory = make_shared<FilePathDirectory>(dptfHomeDirectoryPath);
	m_fileIo = make_shared<FileIo>();
	m_commandDispatcher = new CommandDispatcher();
	m_indexContainer = new IndexContainer();
	m_eventCache = make_shared<EventCache>();
	m_eventNotifier = make_shared<RealEventNotifier>();
	m_userPreferredCache = make_shared<UserPreferredCache>();
	m_messageLogFilter = make_shared<LogMessageFilter>();
	m_messageLogFilter->setLevel(currentLogVerbosityLevel);
	m_requestDispatcher = make_shared<RequestDispatcher>();
}

void DptfManager::createBasicServices(
	const esif_handle_t esifHandle,
	EsifInterfacePtr esifInterfacePtr,
	eLogType currentLogVerbosityLevel)
{
	m_esifAppServices = new EsifAppServices(esifInterfacePtr);
	m_esifServices = new EsifServices(this, esifHandle, m_esifAppServices, currentLogVerbosityLevel);
	m_messageLogger = make_shared<EsifMessageLogger>(m_messageLogFilter, m_esifServices);
	m_configurationManager = ConfigurationFileManager::makeDefault(
		m_messageLogger, m_fileIo, m_filePathDirectory->getConfigurationFilePaths());
	m_configurationManager->loadFiles();
	m_requestDispatcher->registerHandler(
		DptfRequestType::DataGetConfigurationFileContent, m_configurationManager.get());
	m_participantManager = new ParticipantManager(this); // required by work item queue manager
	m_workItemQueueManager = new WorkItemQueueManager(this);
	m_workItemQueueManagerCreated = true;
	m_platformRequestHandler = make_shared<PlatformRequestHandler>(this);
	m_dataManager = new DataManager(this);
	m_systemModeManager = new SystemModeManager(this);
	m_environmentProfileGenerator = make_shared<RealEnvironmentProfileGenerator>(m_messageLogger, m_esifServices);
	m_environmentProfileUpdater = make_shared<RealEnvironmentProfileUpdater>(this);
	m_eventNotifier->registerObserver(m_environmentProfileUpdater, {FrameworkEvent::DomainCreate});
	m_participantRequestHandlers = make_shared<ParticipantRequestHandler>(this);
}

void DptfManager::createPolicies()
{
	const auto defaultPolicies = readDefaultEnabledPolicies();
	m_policyManager = new PolicyManager(this, defaultPolicies);
	m_policyManager->createAllPolicies(m_filePathDirectory->getPath(FilePathDirectory::Path::InstallFolder));
}

set<Guid> DptfManager::readDefaultEnabledPolicies() const
{
	try
	{
		const auto configFileContent = m_configurationManager->getContent(DttConfigurationName);
		const DttConfiguration config(configFileContent);
		const auto environmentProfile = m_environmentProfileUpdater->getLastUpdatedProfile();
		const auto segments = config.getSegmentsWithValue(environmentProfile.cpuIdWithoutStepping);
		set<Guid> result;
		if (!segments.empty())
		{
			const auto keys = segments.front().getKeysThatMatch(DttConfigurationDefaultPoliciesQuery);
			for (const auto& key : keys)
			{
				const auto policyGuidString = segments.front().getValueAsString(key);
				const auto policyGuid = Guid::fromFormattedString(policyGuidString);
				result.emplace(policyGuid);
			}
		}
		return result;
	}
	catch (const exception& ex)
	{
		MANAGER_LOG_MESSAGE_WARNING_EX({return ManagerMessage(this,_file,_line,_function,
			"Failed to load DTT configuration: "s + string(ex.what()));});
		return {};
	}
}

void DptfManager::registerForEvents() const
{
	registerDptfFrameworkEvents();
	m_systemModeManager->registerFrameworkEvents();
}

void DptfManager::notifyAppsThatDttHasLoaded() const
{
	constexpr EsifData eventData = {ESIF_DATA_VOID, nullptr, 0, 0};
	m_esifServices->sendDptfEvent(FrameworkEvent::DptfAppLoaded, Constants::Invalid, Constants::Invalid, eventData);
}

DptfManager::~DptfManager()
{
	unregisterCommands();
	shutDown();
}

Bool DptfManager::isDptfManagerCreated() const
{
	return m_dptfManagerCreateFinished;
}

Bool DptfManager::isDptfShuttingDown() const
{
	return m_dptfShuttingDown;
}

Bool DptfManager::isWorkItemQueueManagerCreated() const
{
	return m_workItemQueueManagerCreated;
}

EsifServicesInterface* DptfManager::getEsifServices() const
{
	return m_esifServices;
}

WorkItemQueueManagerInterface* DptfManager::getWorkItemQueueManager() const
{
	return m_workItemQueueManager;
}

PolicyManagerInterface* DptfManager::getPolicyManager() const
{
	return m_policyManager;
}

ParticipantManagerInterface* DptfManager::getParticipantManager() const
{
	return m_participantManager;
}

ICommandDispatcher* DptfManager::getCommandDispatcher() const
{
	return m_commandDispatcher;
}

IndexContainerInterface* DptfManager::getIndexContainer() const
{
	return m_indexContainer;
}

DataManagerInterface* DptfManager::getDataManager() const
{
	return m_dataManager;
}

SystemModeManagerInterface* DptfManager::getSystemModeManager() const
{
	return m_systemModeManager;
}

shared_ptr<ConfigurationFileManagerInterface> DptfManager::getConfigurationManager() const
{
	return m_configurationManager;
}

EnvironmentProfile DptfManager::getEnvironmentProfile() const
{
	return m_environmentProfileUpdater->getLastUpdatedProfile();
}

std::shared_ptr<EnvironmentProfileGenerator> DptfManager::getEnvironmentProfileGenerator() const
{
	return m_environmentProfileGenerator;
}

string DptfManager::getDptfPolicyDirectoryPath() const
{
	return m_filePathDirectory->getPath(FilePathDirectory::Path::InstallFolder);
}

string DptfManager::getDptfReportDirectoryPath() const
{
	return m_filePathDirectory->getPath(FilePathDirectory::Path::LogFolder);
}

void DptfManager::shutDown()
{
	m_dptfShuttingDown = true;
	m_dptfEnabled = false;

	constexpr EsifData eventData = {ESIF_DATA_VOID, nullptr, 0, 0};
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
	DELETE_MEMORY_TC(m_commandDispatcher)
	DELETE_MEMORY_TC(m_dataManager)
}

void DptfManager::disableAndEmptyAllQueues(void) const
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

void DptfManager::destroyAllPolicies() const
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

void DptfManager::destroyAllParticipants() const
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

void DptfManager::deleteWorkItemQueueManager()
{
	DELETE_MEMORY_TC(m_workItemQueueManager)
}

void DptfManager::deletePolicyManager()
{
	DELETE_MEMORY_TC(m_policyManager)
}

void DptfManager::deleteParticipantManager()
{
	DELETE_MEMORY_TC(m_participantManager)
}

void DptfManager::deleteSystemModeManager()
{
	getSystemModeManager()->unregisterFrameworkEvents();
	DELETE_MEMORY_TC(m_systemModeManager)
}

void DptfManager::deleteEsifAppServices()
{
	DELETE_MEMORY_TC(m_esifAppServices)
}

void DptfManager::deleteEsifServices()
{
	DELETE_MEMORY_TC(m_esifServices)
}

void DptfManager::deleteIndexContainer()
{
	DELETE_MEMORY_TC(m_indexContainer)
}

void DptfManager::destroyUniqueIdGenerator()
{
	try
	{
		UniqueIdGenerator::destroy();
	}
	catch (...)
	{
	}
}

void DptfManager::destroyFrameworkEventInfo()
{
	try
	{
		FrameworkEventInfo::destroy();
	}
	catch (...)
	{
	}
}

void DptfManager::registerDptfFrameworkEvents() const
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
		m_esifServices->registerEvent(FrameworkEvent::DptfLowPowerModeEntry);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->registerEvent(FrameworkEvent::DptfLowPowerModeExit);
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

	try
	{
		m_esifServices->registerEvent(FrameworkEvent::DptfAppBroadcastPrivileged);
	}
	catch (...)
	{
	}
}

void DptfManager::unregisterDptfFrameworkEvents() const
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
		m_esifServices->unregisterEvent(FrameworkEvent::DptfLowPowerModeEntry);
	}
	catch (...)
	{
	}

	try
	{
		m_esifServices->unregisterEvent(FrameworkEvent::DptfLowPowerModeExit);
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

	try
	{
		m_esifServices->unregisterEvent(FrameworkEvent::DptfAppBroadcastPrivileged);
	}
	catch (...)
	{
	}
}

void DptfManager::registerCommands() const
{
	for (const auto& command : m_commands)
	{
		m_commandDispatcher->registerHandler(command->getCommandName(), command);
	}
}

void DptfManager::unregisterCommands() const
{
	for (const auto& command : m_commands)
	{
		m_commandDispatcher->unregisterHandler(command->getCommandName());
	}
}

void DptfManager::createCommands()
{
	m_commands.push_back(make_shared<HelpCommand>(this));
	m_commands.push_back(make_shared<DiagCommand>(this, m_fileIo));
	m_commands.push_back(make_shared<ReloadCommand>(this));
	m_commands.push_back(make_shared<TableObjectCommand>(this));
	m_commands.push_back(make_shared<ConfigCommand>(this));
	m_commands.push_back(make_shared<UiCommand>(this));
	m_commands.push_back(make_shared<CaptureCommand>(this, m_fileIo));
	m_commands.push_back(make_shared<PlatformCpuIdCommand>(this));
	m_commands.push_back(make_shared<PoliciesCommand>(this));
	registerCommands();
}

shared_ptr<EventCache> DptfManager::getEventCache() const
{
	return m_eventCache;
}

std::shared_ptr<EventNotifierInterface> DptfManager::getEventNotifier() const
{
	return m_eventNotifier;
}

shared_ptr<UserPreferredCache> DptfManager::getUserPreferredCache() const
{
	return m_userPreferredCache;
}

void DptfManager::bindDomainsToPolicies(UIntN participantIndex) const
{
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const UIntN domainCount = participant->getDomainCount();

	for (UIntN domainIndex = 0; domainIndex < domainCount; domainIndex++)
	{
		auto policyIndexes = m_policyManager->getPolicyIndexes();
		for (unsigned int policyIndex : policyIndexes)
		{
			try
			{
				const auto policy = m_policyManager->getPolicyPtr(policyIndex);
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
					message.addMessage("Participant Name", participant->getParticipantName());
					message.addMessage("Domain Index", domainIndex);
					message.addMessage("Domain Name", participant->getDomainName(domainIndex));
					message.addMessage("Policy Index", policyIndex);
					string policyName;
					try
					{
						const auto policy = m_policyManager->getPolicyPtr(policyIndex);
						policyName = policy->getName();
					}
					catch (...)
					{
						policyName = "Unknown"s;
					}
					message.addMessage("Policy Name"s, policyName);
					return message;
				});
			}
			catch (...)
			{
				MANAGER_LOG_MESSAGE_WARNING({
					ManagerMessage message =
						ManagerMessage(this, _file, _line, _function, "DptfManager::bindDomainsToPolicies Failed."s);
					message.addMessage("Participant Index", participantIndex);
					message.addMessage("Participant Name", participant->getParticipantName());
					message.addMessage("Domain Index", domainIndex);
					message.addMessage("Domain Name", participant->getDomainName(domainIndex));
					message.addMessage("Policy Index", policyIndex);
					string policyName;
					try
					{
						const auto policy = m_policyManager->getPolicyPtr(policyIndex);
						policyName = policy->getName();
					}
					catch (...)
					{
						policyName = "Unknown"s;
					}
					message.addMessage("Policy Name"s, policyName);
					return message;
				});
			}
		}
	}
}

void DptfManager::unbindDomainsFromPolicies(UIntN participantIndex) const
{
	const UIntN domainCount = m_participantManager->getParticipantPtr(participantIndex)->getDomainCount();

	for (UIntN domainIndex = 0; domainIndex < domainCount; domainIndex++)
	{
		auto policyIndexes = m_policyManager->getPolicyIndexes();
		for (unsigned int policyIndex : policyIndexes)
		{
			try
			{
				const auto policy = m_policyManager->getPolicyPtr(policyIndex);
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
					message.addMessage("Policy Index", policyIndex);
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
					message.addMessage("Policy Index", policyIndex);
					return message;
				});
			}
		}
	}
}

void DptfManager::bindParticipantToPolicies(UIntN participantIndex) const
{
	const auto policyIndexes = m_policyManager->getPolicyIndexes();
	for (unsigned int policyIndex : policyIndexes)
	{
		try
		{
			const auto policy = m_policyManager->getPolicyPtr(policyIndex);
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
				message.addMessage("Policy Index", policyIndex);
				return message;
			});
		}
		catch (...)
		{
			MANAGER_LOG_MESSAGE_WARNING({
				ManagerMessage message =
					ManagerMessage(this, _file, _line, _function, "DptfManager::bindParticipantToPolicies Failed.");
				message.addMessage("Participant Index", participantIndex);
				message.addMessage("Policy Index", policyIndex);
				return message;
			});
		}
	}
}

void DptfManager::unbindParticipantFromPolicies(UIntN participantIndex) const
{
	const auto policyIndexes = m_policyManager->getPolicyIndexes();
	for (const auto policyIndex : policyIndexes)
	{
		try
		{
			const auto policy = m_policyManager->getPolicyPtr(policyIndex);
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
				message.addMessage("Policy Index", policyIndex);
				return message;
			});
		}
		catch (...)
		{
			MANAGER_LOG_MESSAGE_WARNING({
				ManagerMessage message =
					ManagerMessage(this, _file, _line, _function, "DptfManager::unbindParticipantFromPolicies Failed.");
				message.addMessage("Participant Index", participantIndex);
				message.addMessage("Policy Index", policyIndex);
				return message;
			});
		}
	}
}

void DptfManager::bindAllParticipantsToPolicy(UIntN policyIndex) const
{
	const auto policy = m_policyManager->getPolicyPtr(policyIndex);
	const auto participantIndexes = m_participantManager->getParticipantIndexes();
	for (const auto participantIndex : participantIndexes)
	{
		try
		{
			policy->bindParticipant(participantIndex);
			const UIntN domainCount = m_participantManager->getParticipantPtr(participantIndex)->getDomainCount();
			for (UIntN domainIndex = 0; domainIndex < domainCount; domainIndex++)
			{
				policy->bindDomain(participantIndex, domainIndex);
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
				message.addMessage("Participant Index", participantIndex);
				message.addMessage("Policy Index", policyIndex);
				return message;
			});
		}
		catch (...)
		{
			MANAGER_LOG_MESSAGE_WARNING({
				ManagerMessage message =
					ManagerMessage(this, _file, _line, _function, "DptfManager::bindAllParticipantsToPolicy Failed.");
				message.addMessage("Participant Index", participantIndex);
				message.addMessage("Policy Index", policyIndex);

				return message;
			});
		}
	}
}

shared_ptr<RequestDispatcherInterface> DptfManager::getRequestDispatcher() const
{
	return m_requestDispatcher;
}

shared_ptr<RequestHandlerInterface> DptfManager::getPlatformRequestHandler() const
{
	return m_platformRequestHandler;
}

void DptfManager::setCurrentLogVerbosityLevel(eLogType level)
{
	m_esifServices->setCurrentLogVerbosityLevel(level);
	m_messageLogFilter->setLevel(level);
}