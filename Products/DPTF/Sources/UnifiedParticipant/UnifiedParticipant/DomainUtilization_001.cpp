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

#include "DomainUtilization_001.h"
#include "XmlNode.h"

DomainUtilization_001::DomainUtilization_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface)
{
}

UtilizationStatus DomainUtilization_001::getUtilizationStatus(UIntN participantIndex, UIntN domainIndex)
{
    Percentage utilization = m_participantServicesInterface->primitiveExecuteGetAsPercentage(
        esif_primitive_type::GET_PARTICIPANT_UTILIZATION, domainIndex);

    return UtilizationStatus(utilization);
}

void DomainUtilization_001::clearCachedData(void)
{
    // Do nothing.  We don't cache domain utilization related data.
}

XmlNode* DomainUtilization_001::getXml(UIntN domainIndex)
{
    XmlNode* root = getUtilizationStatus(Constants::Invalid, domainIndex).getXml("utilization");
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));
    return root;
}