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

#include "CriticalPolicyStatistics.h"
using namespace std;

CriticalPolicyStatistics::CriticalPolicyStatistics()
    : m_numberOfTimesSleepSignalled(0), m_numberOfTimesHibernateSignalled(0), m_numberOfTimesShutdownSignalled(0)
{
}

void CriticalPolicyStatistics::sleepSignalled()
{
    m_numberOfTimesSleepSignalled++;
}

void CriticalPolicyStatistics::hibernateSignalled()
{
    m_numberOfTimesHibernateSignalled++;
}

void CriticalPolicyStatistics::shutdownSignalled()
{
    m_numberOfTimesShutdownSignalled++;
}

XmlNode* CriticalPolicyStatistics::getXml()
{
    XmlNode* node = XmlNode::createWrapperElement("critical_policy_statistics");
    node->addChild(XmlNode::createDataElement("sleep_signaled", std::to_string(m_numberOfTimesSleepSignalled)));
    node->addChild(XmlNode::createDataElement("hibernate_signaled", std::to_string(m_numberOfTimesHibernateSignalled)));
    node->addChild(XmlNode::createDataElement("shutdown_signaled", std::to_string(m_numberOfTimesShutdownSignalled)));
    return node;
}