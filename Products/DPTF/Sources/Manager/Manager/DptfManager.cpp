/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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
#include "esif_uf_app_iface.h"
#include "esif_uf_iface.h"
#include "DptfStatus.h"
#include "EsifDataString.h"

DptfManager::DptfManager(void) : m_dptfManagerCreateStarted(false), m_dptfManagerCreateFinished(false),
    m_dptfShuttingDown(false), m_workItemQueueManagerCreated(false), m_dptfEnabled(false), m_esifServices(nullptr),
    m_workItemQueueManager(nullptr), m_policyManager(nullptr), m_participantManager(nullptr),
    m_dptfStatus(nullptr), m_indexContainer(nullptr)
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
#ifdef ESIF_ATTR_OS_WINDOWS
        m_dptfHomeDirectoryPath = dptfHomeDirectoryPath + "\\";
#else
        m_dptfHomeDirectoryPath = dptfHomeDirectoryPath + "/";
#endif

        m_dptfEnabled = dptfEnabled;

        m_indexContainer = new IndexContainer(Constants::Participants::MaxParticipantEstimate);
        m_esifServices = new EsifServices(this, esifHandle, esifInterfacePtr, currentLogVerbosityLevel);
        m_participantManager = new ParticipantManager(this);
        m_policyManager = new PolicyManager(this);

        // Make sure to create these AFTER creating the ParticipantManager and PolicyManager
        m_workItemQueueManager = new WorkItemQueueManager(this);
        m_workItemQueueManagerCreated = true;

        m_dptfStatus = new DptfStatus(this);

        m_policyManager->createAllPolicies(m_dptfHomeDirectoryPath);

        registerDptfFrameworkEvents();

        m_dptfManagerCreateFinished = true;
    }
    catch (std::exception ex)
    {
        if ((esifInterfacePtr != nullptr) && (esifInterfacePtr->fWriteLogFuncPtr != nullptr))
        {
            std::stringstream message;
            message << "The DPTF application has failed to start." << std::endl;
            message << ex.what() << std::endl;
            esifInterfacePtr->fWriteLogFuncPtr(esifHandle, this, nullptr, nullptr,
                EsifDataString(message.str()), eLogType::eLogTypeFatal);
        }
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

EsifServices* DptfManager::getEsifServices(void) const
{
    return m_esifServices;
}

WorkItemQueueManager* DptfManager::getWorkItemQueueManager(void) const
{
    return m_workItemQueueManager;
}

PolicyManager* DptfManager::getPolicyManager(void) const
{
    return m_policyManager;
}

ParticipantManager* DptfManager::getParticipantManager(void) const
{
    return m_participantManager;
}

DptfStatus* DptfManager::getDptfStatus(void) const
{
    return m_dptfStatus;
}

IndexContainer* DptfManager::getIndexContainer(void) const
{
    return m_indexContainer;
}

std::string DptfManager::getDptfHomeDirectoryPath(void) const
{
    return m_dptfHomeDirectoryPath;
}

void DptfManager::shutDown(void)
{
    m_dptfShuttingDown = true;
    m_dptfEnabled = false;

    unregisterDptfFrameworkEvents();

    disableAndEmptyAllQueues();
    destroyAllPolicies();
    destroyAllParticipants();
    deleteDptfStatus();
    deleteWorkItemQueueManager();
    deletePolicyManager();
    deleteParticipantManager();
    deleteEsifServices();
    deleteIndexContainer();
    destroyUniqueIdGenerator();
    destroyFrameworkEventInfo();
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
}