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

#include "DomainRfProfileStatus_001.h"

//
// version 001 is for fivr
//

DomainRfProfileStatus_001::DomainRfProfileStatus_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface)
{
}

DomainRfProfileStatus_001::~DomainRfProfileStatus_001(void)
{
}

RfProfileData DomainRfProfileStatus_001::getRfProfileData(UIntN participantIndex, UIntN domainIndex)
{
    // FIXME:  can this be cached?  If so how do we know when the data changes?

    // if center frequency isn't available the error will get thrown back to the policy

    Frequency centerFrequency = m_participantServicesInterface->primitiveExecuteGetAsFrequency(
        esif_primitive_type::GET_RFPROFILE_CENTER_FREQUENCY, domainIndex);

    RfProfileSupplementalData rfProfileSupplementalData(0, 0, 0, 0, RadioConnectionStatus::NotConnected, 0);
    RfProfileData rfProfileData(centerFrequency, Frequency(0), Frequency(0), rfProfileSupplementalData);

    return rfProfileData;
}

void DomainRfProfileStatus_001::clearCachedData(void)
{
    // For now nothing is cached
}

XmlNode* DomainRfProfileStatus_001::getXml(UIntN domainIndex)
{
    // FIXME
    throw implement_me();
}