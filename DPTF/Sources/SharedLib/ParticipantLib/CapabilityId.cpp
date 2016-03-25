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

#include "Dptf.h"
#include "CapabilityId.h"

#define CASE(eventType) \
    case eventType: return ((UInt32)(0x1 << eventType));

namespace Capability
{
    UInt32 ToCapabilityId(Capability::Type capabilityType)
    {
        switch (capabilityType)
        {
            CASE(ActiveControl)
            CASE(CtdpControl)
            CASE(CoreControl)
            CASE(DisplayControl)
            CASE(DomainPriority)
            CASE(PerformanceControl)
            CASE(PowerControl)
            CASE(PowerStatus)
            CASE(TemperatureStatus)
            CASE(UtilizationStatus)
            CASE(PixelClockStatus)
            CASE(PixelClockControl)
            CASE(PlatformPowerStatus)
            CASE(TemperatureThreshold)
            CASE(RfProfileStatus)
            CASE(RfProfileControl)
            CASE(NetworkControl)
            CASE(TransmitPowerControl)
            CASE(CurrentControl)
            CASE(HdcControl)
            CASE(PSysControl)            
            default:
                throw dptf_exception("Capability::Type is invalid.");
        }
    }

    esif_data getEsifDataFromCapabilityData(EsifCapabilityData* capability)
    {
        esif_data eventData = { esif_data_type::ESIF_DATA_STRUCTURE, capability, sizeof(*capability), sizeof(*capability) };
        return eventData;
    }
}