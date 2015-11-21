/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#include "PolicyManager.h"
#include "DptfManager.h"
#include "WorkItemQueueManager.h"
#include "WIPolicyCreateAll.h"
#include "WIPolicyDestroy.h"
#include "EsifFileEnumerator.h"
#include "EsifLibrary.h"
#include "EsifServices.h"
#include "Utility.h"
#include "StatusFormat.h"

using namespace StatusFormat;

PolicyManager::PolicyManager(DptfManager* dptfManager) : m_dptfManager(dptfManager),
    m_supportedPolicyList(dptfManager)
{
}

PolicyManager::~PolicyManager(void)
{
    destroyAllPolicies();
}

void PolicyManager::reloadAllPolicies(const std::string& dptfPolicyDirectoryPath)
{
    m_supportedPolicyList.update();
    createAllPolicies(dptfPolicyDirectoryPath);
}

void PolicyManager::createAllPolicies(const std::string& dptfPolicyDirectoryPath)
{
    try
    {
        // Queue up a work item and wait for the return.
        WorkItem* workItem = new WIPolicyCreateAll(m_dptfManager, dptfPolicyDirectoryPath);
        m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
    }
    catch (...)
    {
        ManagerMessage message = ManagerMessage(
            m_dptfManager, FLF, "Failed while trying to enqueue and wait for WIPolicyCreateAll.");
        m_dptfManager->getEsifServices()->writeMessageError(message);
    }

    if (getPolicyCount() == 0)
    {
        throw dptf_exception("DPTF was not able to load any policies.");
    }
}

void PolicyManager::createPolicy(const std::string& policyFileName)
{
    UIntN firstAvailableIndex = Constants::Invalid;
    Policy* policy = nullptr;

    try
    {
        policy = new Policy(m_dptfManager);
        firstAvailableIndex = getFirstNonNullIndex(m_policy);
        m_policy[firstAvailableIndex] = policy;
    }
    catch (...)
    {
        DELETE_MEMORY_TC(policy);
        throw;
    }

    try
    {
        // Create the policy.  This will end up calling the functions in the .dll/.so and will throw an
        // exception if it doesn't find a valid policy to load.
        policy->createPolicy(policyFileName, firstAvailableIndex, m_supportedPolicyList);

        ManagerMessage message = ManagerMessage(m_dptfManager, FLF, "Policy has been created.");
        message.setPolicyIndex(firstAvailableIndex);
        message.addMessage("Policy Index", firstAvailableIndex);
        message.addMessage("Policy File Name", policyFileName);
        m_dptfManager->getEsifServices()->writeMessageInfo(message);
    }
    catch (policy_not_in_idsp_list ex)
    {
        destroyPolicy(firstAvailableIndex);
    }
    catch (...)
    {
        destroyPolicy(firstAvailableIndex);
        throw;
    }
}

void PolicyManager::destroyAllPolicies(void)
{
    for (UIntN i = 0; i < m_policy.size(); i++)
    {
        if (m_policy[i] != nullptr)
        {
            try
            {
                // Queue up a work item and wait for the return.
                WorkItem* workItem = new WIPolicyDestroy(m_dptfManager, i);
                m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
            }
            catch (...)
            {
                ManagerMessage message = ManagerMessage(m_dptfManager, FLF, "Failed while trying to enqueue and wait for WIPolicyDestroy.");
                message.addMessage("Policy Index", i);
                m_dptfManager->getEsifServices()->writeMessageError(message);
            }
        }
    }
}

void PolicyManager::destroyPolicy(UIntN policyIndex)
{
    if ((policyIndex < m_policy.size()) &&
        (m_policy[policyIndex] != nullptr))
    {
        try
        {
            m_policy[policyIndex]->destroyPolicy();
        }
        catch (...)
        {
            ManagerMessage message = ManagerMessage(m_dptfManager, FLF, "Failed while trying to destroy policy.");
            message.addMessage("Policy Index", policyIndex);
            m_dptfManager->getEsifServices()->writeMessageError(message);
        }

        DELETE_MEMORY_TC(m_policy[policyIndex]);
    }
}

UIntN PolicyManager::getPolicyListCount(void) const
{
    return static_cast<UIntN>(m_policy.size());
}

Policy* PolicyManager::getPolicyPtr(UIntN policyIndex)
{
    if ((policyIndex >= m_policy.size()) ||
        (m_policy[policyIndex] == nullptr))
    {
        throw policy_index_invalid();
    }

    return m_policy[policyIndex];
}

void PolicyManager::registerEvent(UIntN policyIndex, PolicyEvent::Type policyEvent)
{
    if ((m_registeredEvents.test(policyEvent) == false) &&
        (PolicyEvent::RequiresEsifEventRegistration(policyEvent)))
    {
        // Let ESIF know since the first policy is registering
        FrameworkEvent::Type frameworkEvent = PolicyEvent::ToFrameworkEvent(policyEvent);
        m_dptfManager->getEsifServices()->registerEvent(frameworkEvent);
    }

    m_registeredEvents.set(policyEvent);
    getPolicyPtr(policyIndex)->registerEvent(policyEvent);
}

void PolicyManager::unregisterEvent(UIntN policyIndex, PolicyEvent::Type policyEvent)
{
    if (isAnyPolicyRegisteredForEvent(policyEvent) == true)
    {
        getPolicyPtr(policyIndex)->unregisterEvent(policyEvent);
        m_registeredEvents.set(policyEvent, isAnyPolicyRegisteredForEvent(policyEvent));

        if ((m_registeredEvents.test(policyEvent) == false) &&
            (PolicyEvent::RequiresEsifEventRegistration(policyEvent)))
        {
            FrameworkEvent::Type frameworkEvent = PolicyEvent::ToFrameworkEvent(policyEvent);
            m_dptfManager->getEsifServices()->unregisterEvent(frameworkEvent);
        }
    }
}

XmlNode* PolicyManager::getStatusAsXml(void)
{
    XmlNode* status = XmlNode::createWrapperElement("policy_manager");
    status->addChild(getEventsInXml());

    auto policyCount = getPolicyListCount();
    for (UIntN index = 0; index < policyCount; index++)
    {
        try
        {
            auto policy = getPolicyPtr(index);
            std::string name = policy->getName();
            XmlNode* policyStatus = XmlNode::createWrapperElement("policy_status");
            XmlNode* policyName = XmlNode::createDataElement("policy_name", name);
            policyStatus->addChild(policyName);
            policyStatus->addChild(getEventsXmlForPolicy(index));
            status->addChild(policyStatus);
        }
        catch (...)
        {
            // Policy not available, do not add.
        }
    }
    return status;
}

Bool PolicyManager::isAnyPolicyRegisteredForEvent(PolicyEvent::Type policyEvent)
{
    Bool policyRegistered = false;

    for (UIntN i = 0; i < m_policy.size(); i++)
    {
        if ((m_policy[i] != nullptr) && (m_policy[i]->isEventRegistered(policyEvent) == true))
        {
            policyRegistered = true;
            break;
        }
    }

    return policyRegistered;
}

UIntN PolicyManager::getPolicyCount(void)
{
    UIntN policyCount = 0;

    for (UIntN i = 0; i < m_policy.size(); i++)
    {
        if (m_policy[i] != nullptr)
        {
            policyCount++;
        }
    }

    return policyCount;
}

XmlNode* PolicyManager::getEventsXmlForPolicy(UIntN policyIndex)
{
    XmlNode* status = XmlNode::createWrapperElement("event_values");
    auto eventCount = PolicyEvent::Max;
    auto policy = getPolicyPtr(policyIndex);
    for (auto eventIndex = 1; eventIndex < eventCount; eventIndex++)   // Skip the "Invalid" event
    {
        XmlNode* event = XmlNode::createWrapperElement("event");
        XmlNode* eventName = XmlNode::createDataElement("event_name", PolicyEvent::toString((PolicyEvent::Type)eventIndex));
        event->addChild(eventName);
        XmlNode* eventStatus = XmlNode::createDataElement("event_status",
            friendlyValue(policy->isEventRegistered((PolicyEvent::Type)eventIndex)));
        event->addChild(eventStatus);
        status->addChild(event);
    }
    return status;
}

XmlNode* PolicyManager::getEventsInXml()
{
    XmlNode* status = XmlNode::createWrapperElement("events");
    auto eventCount = PolicyEvent::Max;
    for (auto eventIndex = 1; eventIndex < eventCount; eventIndex++)   // Skip the "Invalid" event
    {
        XmlNode* event = XmlNode::createWrapperElement("event");
        XmlNode* eventName = XmlNode::createDataElement("event_name", PolicyEvent::toString((PolicyEvent::Type)eventIndex));
        event->addChild(eventName);
        status->addChild(event);
    }
    return status;
}
