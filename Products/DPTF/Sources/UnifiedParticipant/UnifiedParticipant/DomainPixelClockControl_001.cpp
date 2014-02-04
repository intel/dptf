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

#include "DomainPixelClockControl_001.h"

DomainPixelClockControl_001::DomainPixelClockControl_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface)
{
}

DomainPixelClockControl_001::~DomainPixelClockControl_001(void)
{
}

void DomainPixelClockControl_001::setPixelClockControl(UIntN participantIndex, UIntN domainIndex,
    const PixelClockDataSet& pixelClockDataSet)
{
    UInt32 clockCount = m_participantServicesInterface->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_CLOCK_COUNT, domainIndex);

    if (clockCount != pixelClockDataSet.getCount())
    {
        throw dptf_exception("DomainPixelClockControl_001::setPixelClockControl:  Clock count doesn't match.");
    }

    for (UIntN i = 0; i < pixelClockDataSet.getCount(); i++)
    {
        m_participantServicesInterface->primitiveExecuteSetAsUInt64(
            esif_primitive_type::SET_CLOCK_SSC_ENABLED, pixelClockDataSet[i].getSscEnabledNudgeFrequency(),
            domainIndex, static_cast<UInt8>(i));
        m_participantServicesInterface->primitiveExecuteSetAsUInt64(
            esif_primitive_type::SET_CLOCK_SSC_DISABLED, pixelClockDataSet[i].getSscDisabledNudgeFrequency(),
            domainIndex, static_cast<UInt8>(i));
    }
}

void DomainPixelClockControl_001::clearCachedData(void)
{
    // FIXME:  do we clear the cache for DomainPixelClockStatus?
    throw implement_me();
}

XmlNode* DomainPixelClockControl_001::getXml(UIntN domainIndex)
{
    // FIXME;
    throw implement_me();
}