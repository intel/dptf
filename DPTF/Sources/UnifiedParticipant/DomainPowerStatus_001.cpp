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

#include "DomainPowerStatus_001.h"
#include "XmlNode.h"
#include "esif_ccb.h"

DomainPowerStatus_001::DomainPowerStatus_001(UIntN participantIndex, UIntN domainIndex, 
    ParticipantServicesInterface* participantServicesInterface) :
    DomainPowerStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
}

PowerStatus DomainPowerStatus_001::getPowerStatus(UIntN participantIndex, UIntN domainIndex)
{
    getPower(participantIndex, domainIndex);
    esif_ccb_sleep_msec(250);
    return PowerStatus(getPower(participantIndex, domainIndex));
}

Power DomainPowerStatus_001::getPower(UIntN participantIndex, UIntN domainIndex)
{
    Power power = Power::createInvalid();

    try
    {
        power = getParticipantServices()->primitiveExecuteGetAsPower(
            esif_primitive_type::GET_RAPL_POWER, domainIndex);
    }
    catch (primitive_try_again)
    {
        power = getParticipantServices()->primitiveExecuteGetAsPower(
            esif_primitive_type::GET_RAPL_POWER, domainIndex);
    }

    return power;
}

void DomainPowerStatus_001::clearCachedData(void)
{
    // Do nothing.  We don't cache the power status.
}

std::shared_ptr<XmlNode> DomainPowerStatus_001::getXml(UIntN domainIndex)
{
    std::shared_ptr<XmlNode> root = getPowerStatus(Constants::Invalid, domainIndex).getXml();

    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    return root;
}

std::string DomainPowerStatus_001::getName(void)
{
    return "Power Status (Version 1)";
}
