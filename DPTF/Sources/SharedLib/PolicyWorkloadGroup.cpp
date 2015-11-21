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

#include "PolicyWorkloadGroup.h"
using namespace std;

PolicyWorkloadGroup::PolicyWorkloadGroup(string hint, vector<string> applications)
    : m_hint(hint), m_applications(applications)
{

}

PolicyWorkloadGroup::~PolicyWorkloadGroup()
{

}

vector<string> PolicyWorkloadGroup::getApplications() const
{
    return m_applications;
}

std::string PolicyWorkloadGroup::getHint() const
{
    return m_hint;
}

XmlNode* PolicyWorkloadGroup::getXml() const
{
    XmlNode* workloadGroup = XmlNode::createWrapperElement("workload_group");
    workloadGroup->addChild(XmlNode::createDataElement("id", m_hint));
    XmlNode* applications = XmlNode::createWrapperElement("applications");
    for (auto application = m_applications.begin(); application != m_applications.end(); application++)
    {
        applications->addChild(XmlNode::createDataElement("application", *application));
    }
    workloadGroup->addChild(applications);
    return workloadGroup;
}

