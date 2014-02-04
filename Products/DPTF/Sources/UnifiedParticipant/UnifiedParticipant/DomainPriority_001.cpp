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

#include "DomainPriority_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"

DomainPriority_001::DomainPriority_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface)
{
    clearCachedData();
}

DomainPriority DomainPriority_001::getDomainPriority(UIntN participantIndex, UIntN domainIndex)
{
    updateCacheIfCleared(domainIndex);
    return m_currentPriority;
}

void DomainPriority_001::clearCachedData(void)
{
    m_currentPriority = DomainPriority(0);                          // set priority to 0 (low)
    m_cacheDataCleared = true;
}

XmlNode* DomainPriority_001::getXml(UIntN domainIndex)
{
    XmlNode* root = XmlNode::createWrapperElement("domain_priority");

    root->addChild(getDomainPriority(Constants::Invalid, domainIndex).getXml());
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    return root;
}

void DomainPriority_001::updateCacheIfCleared(UIntN domainIndex)
{
    if (m_cacheDataCleared == true)
    {
        updateCache(domainIndex);
    }
}

void DomainPriority_001::updateCache(UIntN domainIndex)
{
    try
    {
        UInt32 priority = m_participantServicesInterface->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_DOMAIN_PRIORITY, domainIndex);

        m_currentPriority = DomainPriority(priority);
    }
    catch (...)
    {
        // if the primitive isn't available we keep the priority at 0
        m_currentPriority = DomainPriority(0);
    }

    m_cacheDataCleared = false;
}