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

#include "ParticipantStatusMap.h"
#include "ParticipantManagerInterface.h"
#include "XmlNode.h"

ParticipantStatusMap::ParticipantStatusMap(ParticipantManagerInterface* participantManager) :
    m_participantManager(participantManager)
{
}

void ParticipantStatusMap::clearCachedData()
{
    m_participantDomainsList.clear();
}

std::string ParticipantStatusMap::getGroupsString()
{
    if (m_participantDomainsList.size() == 0)
    {
        buildParticipantDomainsList();
    }

    auto modules = XmlNode::createWrapperElement("modules");

    for (UIntN i = 0; i < m_participantDomainsList.size(); i++)
    {
        try
        {
            Participant* participant = m_participantManager->getParticipantPtr(m_participantDomainsList[i].first);

            std::stringstream name;
            name << participant->getParticipantName();

            if (participant->getDomainCount() > 1)
            {
                name << '(' << m_participantDomainsList[i].second << ')';
            }

            auto module = XmlNode::createWrapperElement("module");
            modules->addChild(module);

            auto participantId = XmlNode::createDataElement("id", StlOverride::to_string(i));
            module->addChild(participantId);

            auto participantName = XmlNode::createDataElement("name", name.str());
            module->addChild(participantName);
        }
        catch (dptf_exception)
        {
            // Participant Not available.
        }
    }

    std::string s = modules->toString();

    return s;
}

void ParticipantStatusMap::buildParticipantDomainsList()
{
    UIntN participantCount = m_participantManager->getParticipantListCount();
    for (UIntN participantIndex = 0; participantIndex < participantCount; participantIndex++)
    {
        try
        {
            Participant* participant = m_participantManager->getParticipantPtr(participantIndex);
            UIntN domainCount = participant->getDomainCount();

            for (UIntN domainIndex = 0; domainIndex < domainCount; domainIndex++)
            {
                m_participantDomainsList.push_back(
                    std::make_pair(participantIndex, domainIndex));
            }
        }
        catch (...)
        {
            // participant not available, don't add it to the list
        }
    }
}

std::shared_ptr<XmlNode> ParticipantStatusMap::getStatusAsXml(UIntN mappedIndex)
{
    try
    {
        if (m_participantDomainsList.size() == 0)
        {
            buildParticipantDomainsList();
        }

        UIntN participantIndex = m_participantDomainsList[mappedIndex].first;
        UIntN domainIndex = m_participantDomainsList[mappedIndex].second;

        Participant* participant = m_participantManager->getParticipantPtr(participantIndex);
        return participant->getStatusAsXml(domainIndex);
    }
    catch (...)
    {
    	// Participant not available
        return XmlNode::createRoot();
    }
}