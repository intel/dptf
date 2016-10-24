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

#include "DomainRfProfileControl_001.h"

DomainRfProfileControl_001::DomainRfProfileControl_001(UIntN participantIndex, UIntN domainIndex, 
    ParticipantServicesInterface* participantServicesInterface) :
    DomainRfProfileControlBase(participantIndex, domainIndex, participantServicesInterface)
{
    // TODO: does this need to capture or restore anything?
}

DomainRfProfileControl_001::~DomainRfProfileControl_001(void)
{
}

RfProfileCapabilities DomainRfProfileControl_001::getRfProfileCapabilities(UIntN participantIndex, UIntN domainIndex)
{
    Frequency defaultCenterFrequency = getParticipantServices()->primitiveExecuteGetAsFrequency(
        esif_primitive_type::GET_RFPROFILE_DEFAULT_CENTER_FREQUENCY, domainIndex);

    Percentage leftClipPercent = getParticipantServices()->primitiveExecuteGetAsPercentage(
        esif_primitive_type::GET_RFPROFILE_CLIP_PERCENT_LEFT, domainIndex);

    Percentage rightClipPercent = getParticipantServices()->primitiveExecuteGetAsPercentage(
        esif_primitive_type::GET_RFPROFILE_CLIP_PERCENT_RIGHT, domainIndex);

    Frequency frequencyAdjustResolution = getParticipantServices()->primitiveExecuteGetAsFrequency(
        esif_primitive_type::GET_RFPROFILE_FREQUENCY_ADJUST_RESOLUTION, domainIndex);

    RfProfileCapabilities rfProfileCapabilities(defaultCenterFrequency, leftClipPercent, rightClipPercent,
        frequencyAdjustResolution);

    return rfProfileCapabilities;
}

void DomainRfProfileControl_001::setRfProfileCenterFrequency(UIntN participantIndex, UIntN domainIndex,
    const Frequency& centerFrequency)
{
    getParticipantServices()->primitiveExecuteSetAsFrequency(
        esif_primitive_type::SET_RFPROFILE_CENTER_FREQUENCY, centerFrequency, domainIndex, Constants::Esif::NoInstance);
}

void DomainRfProfileControl_001::clearCachedData(void)
{
    // FIXME: do we clear the cache for this control?
}

std::shared_ptr<XmlNode> DomainRfProfileControl_001::getXml(UIntN domainIndex)
{
    // FIXME
    throw implement_me();
}

std::string DomainRfProfileControl_001::getName(void)
{
    return "RF Profile Control (Version 1)";
}
