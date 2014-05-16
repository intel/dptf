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

#include "ConfigTdpControlSet.h"
#include "XmlNode.h"

ConfigTdpControlSet::ConfigTdpControlSet(const std::vector<ConfigTdpControl>& configTdpControl) :
    m_configTdpControl(configTdpControl)
{
}

UIntN ConfigTdpControlSet::getCount(void) const
{
    return static_cast<UIntN>(m_configTdpControl.size());
}

const ConfigTdpControl& ConfigTdpControlSet::operator[](UIntN index) const
{
    return m_configTdpControl.at(index);
}

Bool ConfigTdpControlSet::operator==(const ConfigTdpControlSet& rhs) const
{
    return (this->m_configTdpControl == rhs.m_configTdpControl);
}

Bool ConfigTdpControlSet::operator!=(const ConfigTdpControlSet& rhs) const
{
    return !(*this == rhs);
}

std::vector<std::string> ConfigTdpControlSet::getAsNameList() const
{
    std::vector<std::string> nameList;
    for (auto control = m_configTdpControl.begin(); control != m_configTdpControl.end(); control++)
    {
        nameList.push_back(control->getNameListString());
    }
    return nameList;
}

XmlNode* ConfigTdpControlSet::getXml(void)
{
    XmlNode* root = XmlNode::createWrapperElement("config_tdp_control_set");
    for (UIntN i = 0; i < m_configTdpControl.size(); i++)
    {
        root->addChild(m_configTdpControl[i].getXml());
    }
    return root;
}