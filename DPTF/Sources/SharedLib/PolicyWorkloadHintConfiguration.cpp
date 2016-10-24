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

#include "PolicyWorkloadHintConfiguration.h"
#include "StringConverter.h"
#include "StringParser.h"
#include "DataVaultPath.h"
using namespace std;

PolicyWorkloadHintConfiguration::PolicyWorkloadHintConfiguration(PlatformConfigurationDataInterface* platformConfigurationData)
    : m_platformConfigData(platformConfigurationData)
{
    generateWorkloadTable();
}

PolicyWorkloadHintConfiguration::~PolicyWorkloadHintConfiguration()
{

}

void PolicyWorkloadHintConfiguration::add(const PolicyWorkloadGroup& workloadGroup)
{
    m_workloadGroups.push_back(workloadGroup);
    updateWorkloadTable();
}

UInt32 PolicyWorkloadHintConfiguration::getHintForApplication(const std::string& application)
{
    std::string lowerCaseApplication = StringConverter::toLower(application);
    auto result = m_workloadLookupTable.find(lowerCaseApplication);
    if (result == m_workloadLookupTable.end())
    {
        throw dptf_exception("Application not found in workload hint configuration.");
    }
    return result->second;
}

std::shared_ptr<XmlNode> PolicyWorkloadHintConfiguration::getXml() const
{
    auto config = XmlNode::createWrapperElement("workload_hint_configuration");

    for (auto group = m_workloadGroups.begin(); group != m_workloadGroups.end(); group++)
    {
        try
        {
            config->addChild(group->getXml());
        }
        catch (...)
        {
            // skip group if getXml fails
        }
    }

    return config;
}

void PolicyWorkloadHintConfiguration::updateWorkloadTable()
{
    m_workloadLookupTable.clear();
    for (auto group = m_workloadGroups.begin(); group != m_workloadGroups.end(); group++)
    {
        try
        {
            vector<string> applications = group->getApplications();
            UInt32 hint = StringConverter::toUInt32(group->getHint());
            for (auto app = applications.begin(); app != applications.end(); app++)
            {
                m_workloadLookupTable[*app] = hint;
            }
        }
        catch (...)
        {
            // skip group if converting hint fails
        }
    }
}

void PolicyWorkloadHintConfiguration::generateWorkloadTable()
{
    try
    {
        std::string workloadHintsString = readWorkloadHintsConfigurationValue();
        if (!workloadHintsString.empty())
        {
            std::vector<std::string> workloadHintsPaths = StringParser::split(workloadHintsString, '|');
            for (auto path = workloadHintsPaths.begin(); path != workloadHintsPaths.end(); path++)
            {
                auto hint = StringParser::removeString(*path, DataVaultPath::Shared::Export::WorkloadHints);
                std::vector<std::string> applicationNames = StringParser::split(
                    readWorkloadHintConfigurationValues(*path), '|');
                add(PolicyWorkloadGroup(hint, applicationNames));
            }
        }
    }
    catch (...)
    {
        // Log failure to read workload hint
    }
}

std::string PolicyWorkloadHintConfiguration::readWorkloadHintsConfigurationValue()
{
    try
    {
        std::string workloadHints = m_platformConfigData->readConfigurationString(
            DataVaultPath::Shared::Export::WorkloadHints + "*");
        return workloadHints;
    }
    catch (...)
    {
        throw dptf_exception("Could not read \"workload_hints\" value from policy configuration.");
    }
}

std::string PolicyWorkloadHintConfiguration::readWorkloadHintConfigurationValues(const std::string& key)
{
    try
    {
        std::string hint = m_platformConfigData->readConfigurationString(key);
        hint = StringConverter::toLower(hint);
        return hint;
    }
    catch (...)
    {
        // Log failure to read specific key from configuration
        return "";
    }
}
