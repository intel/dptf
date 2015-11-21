/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

#pragma once

#include "Dptf.h"
#include "DomainPerformanceControlInterface.h"
#include "DomainActiveControlInterface.h"
#include "ThermalRelationshipTableEntry.h"
#include "ActiveRelationshipTableEntry.h"
#include "DomainConfigTdpControlInterface.h"
#include "DomainPowerControlInterface.h"
#include "DomainCoreControlInterface.h"
#include "DomainDisplayControlInterface.h"
#include "ObjectsGeneric.h"
#include "ObjectsPassive.h"
#include "ObjectsActive.h"
#include "ObjectsProcessor.h"
#include "ObjectsCtdp.h"
#include "ObjectsDisplay.h"
#include "DptfBuffer.h"

class BinaryParse
{
public:

    static UInt64 extractBits(UInt16 startBit, UInt16 stopBit, UInt64 data);
    static std::vector<PerformanceControl> genericPpssObject(const DptfBuffer& buffer);
    static ActiveControlStaticCaps* fanFifObject(const DptfBuffer& buffer);
    static ActiveControlStatus* fanFstObject(const DptfBuffer& buffer);
    static std::vector<ActiveControl> fanFpsObject(const DptfBuffer& buffer);
    static std::vector<PerformanceControl> processorPssObject(const DptfBuffer& buffer);
    static std::vector<PerformanceControl> processorTssObject(PerformanceControl pN, const DptfBuffer& buffer);
    static std::vector<PerformanceControl> processorGfxPstates(const DptfBuffer& buffer);
    static std::vector<ConfigTdpControl> processorTdplObject(const DptfBuffer& buffer);
    static std::vector<PowerControlDynamicCaps> processorPpccObject(const DptfBuffer& buffer);
    static CoreControlLpoPreference* processorClpoObject(const DptfBuffer& buffer);
    static std::vector<DisplayControl> displayBclObject(const DptfBuffer& buffer);
    static std::string normalizeAcpiScope(const std::string& acpiScope);

private:

    static UIntN countPpssRows(UIntN size, UInt8* data);
    static UIntN countPssRows(UInt32 size, UInt8* data);
    static void validateData(UInt32 size);
    static UIntN countTssRows(UInt32 size, UInt8* data);
};
