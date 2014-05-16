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

#include "DomainPowerStatus_001.h"
#include "XmlNode.h"

DomainPowerStatus_001::DomainPowerStatus_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface)
{
}

PowerStatus DomainPowerStatus_001::getPowerStatus(UIntN participantIndex, UIntN domainIndex)
{
    Power power = m_participantServicesInterface->primitiveExecuteGetAsPower(
        esif_primitive_type::GET_RAPL_POWER, domainIndex);

    return PowerStatus(power);
}

void DomainPowerStatus_001::clearCachedData(void)
{
    // Do nothing.  We don't cache the power status.
}

XmlNode* DomainPowerStatus_001::getXml(UIntN domainIndex)
{
    XmlNode* root = getPowerStatus(Constants::Invalid, domainIndex).getXml();

    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    return root;
}