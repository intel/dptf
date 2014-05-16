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

#include "PassiveParticipantControlStatus.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

PassiveParticipantControlStatus::PassiveParticipantControlStatus(ParticipantProxy& participant)
    : m_participantIndex(Constants::Invalid), m_name("")
{
    m_participantIndex = participant.getIndex();
    m_name = participant.getParticipantProperties().getName();

    auto domainIndexes = participant.getDomainIndexes();
    for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
    {
        m_domainStatus.push_back(PassiveDomainControlStatus(participant[*domainIndex]));
    }
}

XmlNode* PassiveParticipantControlStatus::getXml()
{
    XmlNode* participantControlStatus = XmlNode::createWrapperElement("participant_control_status");
    participantControlStatus->addChild(XmlNode::createDataElement("index", friendlyValue(m_participantIndex)));
    participantControlStatus->addChild(XmlNode::createDataElement("name", m_name));
    for (auto status = m_domainStatus.begin(); status != m_domainStatus.end(); status++)
    {
        participantControlStatus->addChild(status->getXml());
    }
    return participantControlStatus;
}