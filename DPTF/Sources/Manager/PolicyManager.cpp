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

#include "PolicyManager.h"
#include "DptfManager.h"
#include "WorkItemQueueManagerInterface.h"
#include "WIPolicyCreateAll.h"
#include "WIPolicyDestroy.h"
#include "EsifFileEnumerator.h"
#include "EsifServicesInterface.h"
#include "StatusFormat.h"
#include "MapOps.h"
#include "Utility.h"
#include "ManagerLogger.h"

using namespace StatusFormat;
using namespace std;

PolicyManager::PolicyManager(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
	, m_policies()
	, m_supportedPolicyList(make_shared<SupportedPolicyList>(dptfManager))
	, m_supportedDynamicPolicyList(make_shared<SupportedDynamicPolicyList>(dptfManager))
	, m_registeredEvents()
{
}

PolicyManager::~PolicyManager(void)
{
	destroyAllPolicies();
}

void PolicyManager::createAllPolicies(const string& dptfPolicyDirectoryPath)
{
	try
	{
		// Queue up a work item and wait for the return.
		auto workItem = make_shared<WIPolicyCreateAll>(m_dptfManager, dptfPolicyDirectoryPath);
		m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
	}
	catch (...)
	{
		MANAGER_LOG_MESSAGE_ERROR({
			ManagerMessage message = ManagerMessage(
				m_dptfManager,
				_file,
				_line,
				_function,
				"Failed while trying to enqueue and wait for WIPolicyCreateAll.");
			return message;
		});
	}
}

UIntN PolicyManager::createPolicy(const string& policyFileName)
{
	UIntN firstAvailableIndex = Constants::Invalid;

	try
	{
		// First check if we have an instance of the policy already
		throwIfPolicyAlreadyExists(policyFileName);

		auto indexesInUse = MapOps<UIntN, shared_ptr<IPolicy>>::getKeys(m_policies);
		firstAvailableIndex = getFirstAvailableIndex(indexesInUse);
		m_policies[firstAvailableIndex] = make_shared<Policy>(m_dptfManager);

		// Create the policy.  This will end up calling the functions in the .dll/.so and will throw an
		// exception if it doesn't find a valid policy to load.
		m_policies[firstAvailableIndex]->createPolicy(
			policyFileName,
			firstAvailableIndex,
			m_supportedPolicyList,
			Guid::createInvalid(),
			Guid::createInvalid(),
			Constants::EmptyString,
			Constants::EmptyString);

		MANAGER_LOG_MESSAGE_INFO({
			ManagerMessage message = ManagerMessage(m_dptfManager, _file, _line, _function, "Policy has been created.");
			message.setPolicyIndex(firstAvailableIndex);
			message.addMessage("Policy Index", firstAvailableIndex);
			message.addMessage("Policy File Name", policyFileName);
			return message;
		});
	}
	catch (policy_not_in_idsp_list&)
	{
		destroyPolicy(firstAvailableIndex);
		firstAvailableIndex = Constants::Invalid;
	}
	catch (...)
	{
		destroyPolicy(firstAvailableIndex);
		throw;
	}

	return firstAvailableIndex;
}

UIntN PolicyManager::createDynamicPolicy(
	const string& policyFileName,
	Guid dynamicPolicyUuid,
	Guid dynamicPolicyTemplateGuid,
	const string& dynamicPolicyName,
	const string& dynamicPolicyUuidString)
{
	UIntN firstAvailableIndex = Constants::Invalid;

	try
	{
		// First check if we have an instance of the dynamic policy already
		throwIfDynamicPolicyAlreadyExists(policyFileName, dynamicPolicyName);

		auto indexesInUse = MapOps<UIntN, shared_ptr<IPolicy>>::getKeys(m_policies);
		firstAvailableIndex = getFirstAvailableIndex(indexesInUse);
		m_policies[firstAvailableIndex] = make_shared<Policy>(m_dptfManager);

		// Create the dynamic policy.  This will end up calling the functions in the .dll/.so and will throw an
		// exception if it doesn't find a valid policy to load.
		m_policies[firstAvailableIndex]->createPolicy(
			policyFileName,
			firstAvailableIndex,
			m_supportedPolicyList,
			dynamicPolicyUuid,
			dynamicPolicyTemplateGuid,
			dynamicPolicyName,
			dynamicPolicyUuidString);

		MANAGER_LOG_MESSAGE_INFO({
			ManagerMessage message =
				ManagerMessage(m_dptfManager, _file, _line, _function, "Dynamic Policy has been created.");
			message.setPolicyIndex(firstAvailableIndex);
			message.addMessage("Policy Index", firstAvailableIndex);
			message.addMessage("Policy File Name", policyFileName);
			message.addMessage("Policy Name", dynamicPolicyName);
			return message;
		});
	}
	catch (policy_already_exists&)
	{
		// Ignore duplicate dynamic policy instance
	}
	catch (policy_not_in_idsp_list&)
	{
		destroyPolicy(firstAvailableIndex);
		firstAvailableIndex = Constants::Invalid;
	}
	catch (dynamic_policy_template_guid_invalid&)
	{
		destroyPolicy(firstAvailableIndex);
		firstAvailableIndex = Constants::Invalid;
	}
	catch (...)
	{
		destroyPolicy(firstAvailableIndex);
		throw;
	}

	return firstAvailableIndex;
}

void PolicyManager::destroyAllPolicies(void)
{
	auto policyIndexes = MapOps<UIntN, shared_ptr<IPolicy>>::getKeys(m_policies);
	for (auto index = policyIndexes.begin(); index != policyIndexes.end(); ++index)
	{
		if (m_policies[*index] != nullptr)
		{
			try
			{
				// Queue up a work item and wait for the return.
				auto workItem = make_shared<WIPolicyDestroy>(m_dptfManager, *index);
				m_dptfManager->getWorkItemQueueManager()->enqueueImmediateWorkItemAndWait(workItem);
			}
			catch (...)
			{
				MANAGER_LOG_MESSAGE_ERROR({
					ManagerMessage message = ManagerMessage(
						m_dptfManager,
						_file,
						_line,
						_function,
						"Failed while trying to enqueue and wait for WIPolicyDestroy.");
					message.addMessage("Policy Index", *index);
					return message;
				});
			}
		}
	}
}

void PolicyManager::destroyPolicy(UIntN policyIndex)
{
	auto matchedPolicy = m_policies.find(policyIndex);
	if ((matchedPolicy != m_policies.end()) && (matchedPolicy->second != nullptr))
	{
		try
		{
			matchedPolicy->second->destroyPolicy();
		}
		catch (...)
		{
			MANAGER_LOG_MESSAGE_ERROR({
				ManagerMessage message =
					ManagerMessage(m_dptfManager, _file, _line, _function, "Failed while trying to destroy policy.");
				message.addMessage("Policy Index", policyIndex);
				return message;
			});
		}

		m_policies.erase(matchedPolicy);
	}
}

set<UIntN> PolicyManager::getPolicyIndexes(void) const
{
	return MapOps<UIntN, shared_ptr<IPolicy>>::getKeys(m_policies);
}

shared_ptr<ISupportedPolicyList> PolicyManager::getSupportedPolicyList(void) const
{
	return m_supportedPolicyList;
}

shared_ptr<ISupportedDynamicPolicyList> PolicyManager::getSupportedDynamicPolicyList(void) const
{
	return m_supportedDynamicPolicyList;
}

IPolicy* PolicyManager::getPolicyPtr(UIntN policyIndex)
{
	auto matchedPolicy = m_policies.find(policyIndex);
	if ((matchedPolicy == m_policies.end()) || (matchedPolicy->second == nullptr))
	{
		throw policy_index_invalid();
	}

	return matchedPolicy->second.get();
}

void PolicyManager::registerEvent(UIntN policyIndex, PolicyEvent::Type policyEvent)
{
	if ((m_registeredEvents.test(policyEvent) == false) && (PolicyEvent::RequiresEsifEventRegistration(policyEvent)))
	{
		// Let ESIF know since the first policy is registering
		FrameworkEvent::Type frameworkEvent = PolicyEvent::ToFrameworkEvent(policyEvent);
		m_dptfManager->getEsifServices()->registerEvent(frameworkEvent);
	}

	m_registeredEvents.set(policyEvent);
	dynamic_cast<Policy*>(getPolicyPtr(policyIndex))->registerEvent(policyEvent);
}

void PolicyManager::unregisterEvent(UIntN policyIndex, PolicyEvent::Type policyEvent)
{
	if (isAnyPolicyRegisteredForEvent(policyEvent) == true)
	{
		dynamic_cast<Policy*>(getPolicyPtr(policyIndex))->unregisterEvent(policyEvent);
		m_registeredEvents.set(policyEvent, isAnyPolicyRegisteredForEvent(policyEvent));

		if ((m_registeredEvents.test(policyEvent) == false)
			&& (PolicyEvent::RequiresEsifEventRegistration(policyEvent)))
		{
			FrameworkEvent::Type frameworkEvent = PolicyEvent::ToFrameworkEvent(policyEvent);
			m_dptfManager->getEsifServices()->unregisterEvent(frameworkEvent);
		}
	}
}

shared_ptr<XmlNode> PolicyManager::getStatusAsXml(void)
{
	auto root = XmlNode::createRoot();
	root->addChild(XmlNode::createComment("format_id=10-E0-F6-61-4B-7D-F7-40-AE-90-CF-DA-99-0F-F9-1A"));

	auto eventStatus = XmlNode::createWrapperElement("policy_manager_event_status");
	eventStatus->addChild(getEventsInXml());

	for (auto policy = m_policies.begin(); policy != m_policies.end(); ++policy)
	{
		try
		{
			if (policy->second != nullptr)
			{
				string name = policy->second->getName();
				auto policyStatus = XmlNode::createWrapperElement("policy_event_status");
				auto policyName = XmlNode::createDataElement("policy_name", name);
				policyStatus->addChild(policyName);
				policyStatus->addChild(getEventsXmlForPolicy(policy->first));
				eventStatus->addChild(policyStatus);
			}
		}
		catch (...)
		{
			// Policy not available, do not add.
		}
	}
	root->addChild(eventStatus);
	return root;
}

string PolicyManager::getDiagnosticsAsXml(void)
{
	auto root = XmlNode::createRoot();
	return root->toString();
}

shared_ptr<IPolicy> PolicyManager::getPolicy(const string& policyName) const
{
	for (auto policy = m_policies.begin(); policy != m_policies.end(); ++policy)
	{
		if (policyName == policy->second->getName())
		{
			return policy->second;
		}
	}
	throw dptf_exception(string("Policy \"") + policyName + string("\" not found."));
}

Bool PolicyManager::policyExists(const string& policyName) const
{
	for (auto policy = m_policies.begin(); policy != m_policies.end(); ++policy)
	{
		if (policyName == policy->second->getName())
		{
			return true;
		}
	}
	return false;
}

Bool PolicyManager::IsDynamicPolicyTemplateFileName(const std::string& policyName) const
{
	// TODO : Make this as list if there are more policy templates
	const std::string policyTemplateFileName = "DptfPolicyAdaptivePerformance" ESIF_LIB_EXT;

	if ( policyName == policyTemplateFileName)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void PolicyManager::throwIfPolicyAlreadyExists(string policyFileName)
{
	for (auto policy = m_policies.begin(); policy != m_policies.end(); ++policy)
	{
		if (policy->second->getPolicyFileName() == policyFileName && policy->second->isDynamicPolicy() == false)
		{
			MANAGER_LOG_MESSAGE_DEBUG({
				ManagerMessage debugMessage =
					ManagerMessage(m_dptfManager, _file, _line, _function, "Policy instance already exists.");
				debugMessage.setPolicyIndex(policy->first);
				debugMessage.addMessage("Policy Index", policy->first);
				debugMessage.addMessage("Policy File Name", policyFileName);
				return debugMessage;
			});
			throw policy_already_exists();
		}
	}
}

void PolicyManager::throwIfDynamicPolicyAlreadyExists(string policyFileName, string policyName)
{
	for (auto policy = m_policies.begin(); policy != m_policies.end(); ++policy)
	{
		if (policy->second->getPolicyFileName() == policyFileName && policy->second->getName() == policyName
			&& policy->second->isDynamicPolicy() == true)
		{
			MANAGER_LOG_MESSAGE_DEBUG({
				ManagerMessage debugMessage =
					ManagerMessage(m_dptfManager, _file, _line, _function, "Dynamic Policy instance already exists.");
				debugMessage.setPolicyIndex(policy->first);
				debugMessage.addMessage("Policy Index", policy->first);
				debugMessage.addMessage("Policy File Name", policyFileName);
				debugMessage.addMessage("Policy Name", policyName);
				return debugMessage;
			});
			throw policy_already_exists();
		}
	}
}

Bool PolicyManager::isAnyPolicyRegisteredForEvent(PolicyEvent::Type policyEvent)
{
	Bool policyRegistered = false;

	for (auto policy = m_policies.begin(); policy != m_policies.end(); ++policy)
	{
		if ((policy->second != nullptr)
			&& (dynamic_cast<Policy*>(policy->second.get())->isEventRegistered(policyEvent) == true))
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

	for (auto policy = m_policies.begin(); policy != m_policies.end(); ++policy)
	{
		if (policy->second != nullptr)
		{
			policyCount++;
		}
	}

	return policyCount;
}

shared_ptr<XmlNode> PolicyManager::getEventsXmlForPolicy(UIntN policyIndex)
{
	auto status = XmlNode::createWrapperElement("event_values");
	auto eventCount = PolicyEvent::Max;
	auto policy = dynamic_cast<Policy*>(getPolicyPtr(policyIndex));
	for (auto eventIndex = 1; eventIndex < eventCount; eventIndex++) // Skip the "Invalid" event
	{
		auto event = XmlNode::createWrapperElement("event");
		auto eventName = XmlNode::createDataElement("event_name", PolicyEvent::toString((PolicyEvent::Type)eventIndex));
		event->addChild(eventName);
		auto eventStatus = XmlNode::createDataElement(
			"event_status", friendlyValue(policy->isEventRegistered((PolicyEvent::Type)eventIndex)));
		event->addChild(eventStatus);
		status->addChild(event);
	}
	return status;
}

shared_ptr<XmlNode> PolicyManager::getEventsInXml()
{
	auto status = XmlNode::createWrapperElement("events");
	auto eventCount = PolicyEvent::Max;
	for (auto eventIndex = 1; eventIndex < eventCount; eventIndex++) // Skip the "Invalid" event
	{
		auto event = XmlNode::createWrapperElement("event");
		auto eventName = XmlNode::createDataElement("event_name", PolicyEvent::toString((PolicyEvent::Type)eventIndex));
		event->addChild(eventName);
		status->addChild(event);
	}
	return status;
}

EsifServicesInterface* PolicyManager::getEsifServices()
{
	return m_dptfManager->getEsifServices();
}
