/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

DptfManager::DptfManager(void) : m_dptfManagerCreateStarted(false), m_dptfManagerCreateFinished(false),
m_dptfShuttingDown(false), m_workItemQueueManagerCreated(false), m_dptfEnabled(false), m_esifAppServices(nullptr), 
m_esifServices(nullptr), m_workItemQueueManager(nullptr), m_policyManager(nullptr), m_participantManager(nullptr),
m_dptfStatus(nullptr), m_indexContainer(nullptr), m_dptfPolicyLoadNameOnly(false)
{
}

void DptfManager::createDptfManager(const void* esifHandle, EsifInterfacePtr esifInterfacePtr,
    const std::string& dptfHomeDirectoryPath, eLogType currentLogVerbosityLevel, Bool dptfEnabled)
{
    if (m_dptfManagerCreateStarted == true)
    {
        throw dptf_exception("DptfManager::createDptfManager() already executed.");
    }
    m_dptfManagerCreateStarted = true;

    try
    {
        // Parse DPTF Home Path. Possible formats:
        //   homepath           = Location of both XSL Files and Policy Libraries; specify full path when loading Libraries
        //   homepath|libpath   = (1) Location of XSL Files (2) Location of Policy Libraries [use full path when loading Libraries]
        //   homepath|#libpath  = (1) Location of XSL Files (2) Location of Policy Libraries [DO NOT use full path when loading Libraries]
        std::string homePath = dptfHomeDirectoryPath;
        std::string policyPath = dptfHomeDirectoryPath;
        std::vector<std::string> dptfPaths = StringParser::split(dptfHomeDirectoryPath, '|');
        EsifData eventData = { ESIF_DATA_VOID, NULL, 0, 0 };

        if (dptfPaths.size() > 1)
        {
            homePath = dptfPaths[0];
            policyPath = dptfPaths[1];
            if (policyPath[0] == '#')
            {
                policyPath.erase(0, 1);
                m_dptfPolicyLoadNameOnly = true;
            }
        }
        if (homePath.back() != *ESIF_PATH_SEP) {
            homePath += ESIF_PATH_SEP;
        }
        if (policyPath.back() != *ESIF_PATH_SEP) {
            policyPath += ESIF_PATH_SEP;
        }
        m_dptfHomeDirectoryPath = homePath;
        m_dptfPolicyDirectoryPath = policyPath;
        m_dptfEnabled = dptfEnabled;

        m_eventCache = std::make_shared<EventCache>();
        m_userPreferredCache = std::make_shared<UserPreferredCache>();
        m_indexContainer = new IndexContainer(Constants::Participants::MaxParticipantEstimate);
        m_esifAppServices = new EsifAppServices(esifInterfacePtr);
        m_esifServices = new EsifServices(this, esifHandle, m_esifAppServices, currentLogVerbosityLevel);
        m_participantManager = new ParticipantManager(this);
        m_policyManager = new PolicyManager(this);

        // Make sure to create these AFTER creating the ParticipantManager and PolicyManager
        m_workItemQueueManager = new WorkItemQueueManager(this);
        m_workItemQueueManagerCreated = true;

        m_dptfStatus = new DptfStatus(this);

        m_policyManager->createAllPolicies(m_dptfPolicyDirectoryPath);

        registerDptfFrameworkEvents();

        m_dptfManagerCreateFinished = true;

        m_esifServices->sendDptfEvent(FrameworkEvent::DptfAppLoaded, Constants::Invalid, Constants::Invalid, eventData);
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "The DPTF application has failed to start." << std::endl;
        message << ex.what() << std::endl;
        esifInterfacePtr->fWriteLogFuncPtr(esifHandle, this, nullptr, nullptr,
            EsifDataString(message.str()), eLogType::eLogTypeFatal);
    }

    if (m_dptfManagerCreateFinished == false)
    {
        shutDown();
        throw dptf_exception("Failed to start DPTF");
    }
}

DptfManager::~DptfManager(void)
{
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

DptfStatusInterface* DptfManager::getDptfStatus(void)
{
    return m_dptfStatus;
}

IndexContainerInterface* DptfManager::getIndexContainer(void) const
{
    return m_indexContainer;
}

std::string DptfManager::getDptfHomeDirectoryPath(void) const
{
    return m_dptfHomeDirectoryPath;
}

std::string DptfManager::getDptfPolicyDirectoryPath(void) const
{
    return m_dptfPolicyDirectoryPath;
}

Bool DptfManager::isDptfPolicyLoadNameOnly(void) const
{
    return m_dptfPolicyLoadNameOnly;
}

void DptfManager::shutDown(void)
{
    EsifData eventData = { ESIF_DATA_VOID, NULL, 0, 0 };
    m_dptfShuttingDown = true;
    m_dptfEnabled = false;

    m_esifServices->sendDptfEvent(FrameworkEvent::DptfAppUnloaded, Constants::Invalid, Constants::Invalid, eventData);
    unregisterDptfFrameworkEvents();

    disableAndEmptyAllQueues();
    destroyAllPolicies();
    destroyAllParticipants();
    deleteWorkItemQueueManager();
    deletePolicyManager();
    deleteParticipantManager();
    deleteEsifServices();
    deleteEsifAppServices();
    deleteIndexContainer();
    destroyUniqueIdGenerator();
    destroyFrameworkEventInfo();
    deleteDptfStatus();
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
    if (m_esifAppServices->getInterfaceVersion() == ESIF_INTERFACE_VERSION_2)
    {
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
    if (m_esifAppServices->getInterfaceVersion() == ESIF_INTERFACE_VERSION_2)
    {
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
    }
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
        for (UIntN policyIndex = 0; policyIndex < m_policyManager->getPolicyListCount(); policyIndex++)
        {
            try
            {
                Policy* policy = m_policyManager->getPolicyPtr(policyIndex);
                policy->bindDomain(participantIndex, domainIndex);
            }
            catch (dptf_exception ex)
            {
                ManagerMessage message = ManagerMessage(this, FLF, "DPTF was not able to bind domain to policies: " + ex.getDescription() + ".");
                message.addMessage("Participant Index", participantIndex);
                message.addMessage("Domain Index", domainIndex);
                message.addMessage("Policy Index", policyIndex);
                getEsifServices()->writeMessageWarning(message);
            }
            catch (...)
            {
                ManagerMessage message = ManagerMessage(this, FLF, "DptfManager::bindDomainsToPolicies Failed.");
                message.addMessage("Participant Index", participantIndex);
                message.addMessage("Domain Index", domainIndex);
                message.addMessage("Policy Index", policyIndex);
                getEsifServices()->writeMessageWarning(message);
            }
        }
    }
}

void DptfManager::unbindDomainsFromPolicies(UIntN participantIndex) const
{
    UIntN domainCount = m_participantManager->getParticipantPtr(participantIndex)->getDomainCount();

    for (UIntN domainIndex = 0; domainIndex < domainCount; domainIndex++)
    {
        for (UIntN policyIndex = 0; policyIndex < m_policyManager->getPolicyListCount(); policyIndex++)
        {
            try
            {
                Policy* policy = m_policyManager->getPolicyPtr(policyIndex);
                policy->unbindDomain(participantIndex, domainIndex);
            }
            catch (dptf_exception ex)
            {
                ManagerMessage message = ManagerMessage(this, FLF, "DPTF was not able to unbind domain from policies: " + ex.getDescription() + ".");
                message.addMessage("Participant Index", participantIndex);
                message.addMessage("Domain Index", domainIndex);
                message.addMessage("Policy Index", policyIndex);
                getEsifServices()->writeMessageWarning(message);
            }
            catch (...)
            {
                ManagerMessage message = ManagerMessage(this, FLF, "DptfManager::unbindDomainsFromPolicies Failed.");
                message.addMessage("Participant Index", participantIndex);
                message.addMessage("Domain Index", domainIndex);
                message.addMessage("Policy Index", policyIndex);
                getEsifServices()->writeMessageWarning(message);
            }
        }
    }
}

void DptfManager::bindParticipantToPolicies(UIntN participantIndex) const
{
    for (UIntN policyIndex = 0; policyIndex < m_policyManager->getPolicyListCount(); policyIndex++)
    {
        try
        {
            Policy* policy = m_policyManager->getPolicyPtr(policyIndex);
            policy->bindParticipant(participantIndex);
        }
        catch (dptf_exception ex)
        {
            ManagerMessage message = ManagerMessage(this, FLF, "DPTF was not able to bind participant to policies: " + ex.getDescription() + ".");
            message.addMessage("Participant Index", participantIndex);
            message.addMessage("Policy Index", policyIndex);
            getEsifServices()->writeMessageWarning(message);
        }
        catch (...)
        {
            ManagerMessage message = ManagerMessage(this, FLF, "DptfManager::bindParticipantToPolicies Failed.");
            message.addMessage("Participant Index", participantIndex);
            message.addMessage("Policy Index", policyIndex);
            getEsifServices()->writeMessageWarning(message);
        }
    }
}

void DptfManager::unbindParticipantFromPolicies(UIntN participantIndex) const
{
    for (UIntN policyIndex = 0; policyIndex < m_policyManager->getPolicyListCount(); policyIndex++)
    {
        try
        {
            Policy* policy = m_policyManager->getPolicyPtr(policyIndex);
            policy->unbindParticipant(participantIndex);
        }
        catch (dptf_exception ex)
        {
            ManagerMessage message = ManagerMessage(this, FLF, "DPTF was not able to unbind participant from policies: " + ex.getDescription() + ".");
            message.addMessage("Participant Index", participantIndex);
            message.addMessage("Policy Index", policyIndex);
            getEsifServices()->writeMessageWarning(message);
        }
        catch (...)
        {
            ManagerMessage message = ManagerMessage(this, FLF, "DptfManager::unbindParticipantFromPolicies Failed.");
            message.addMessage("Participant Index", participantIndex);
            message.addMessage("Policy Index", policyIndex);
            getEsifServices()->writeMessageWarning(message);
        }
    }
}
