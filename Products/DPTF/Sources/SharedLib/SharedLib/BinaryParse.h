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

#pragma once

#include "DomainPerformanceControlInterface.h"
#include "DomainActiveControlInterface.h"
#include "ThermalRelationshipTableEntry.h"
#include "ActiveRelationshipTableEntry.h"
#include "LpmTable.h"
#include "DomainConfigTdpControlInterface.h"
#include "DomainPowerControlInterface.h"
#include "DomainCoreControlInterface.h"
#include "DomainDisplayControlInterface.h"
#include "AcpiObjectsGeneric.h"
#include "AcpiObjectsPassive.h"
#include "AcpiObjectsActive.h"
#include "AcpiObjectsFan.h"
#include "AcpiObjectsProcessor.h"
#include "AcpiObjectsCtdp.h"
#include "AcpiObjectsDisplay.h"
#include "AcpiObjectsLpm.h"

class BinaryParse
{
public:

    static UInt64 extractBits(UInt16 startBit, UInt16 stopBit, UInt64 data);
    static std::vector<PerformanceControl> genericPpssObject(UInt32 dataLength, void* esifData);
    static std::vector<ThermalRelationshipTableEntry> passiveTrtObject(UInt32 dataLength, void* esifData);
    static std::vector<ActiveRelationshipTableEntry> activeArtObject(UInt32 dataLength, void* binaryData);
    static LpmTable lpmTableObject(UInt32 dataLength, void* binaryData);
    static ActiveControlStaticCaps* fanFifObject(UInt32 dataLength, void* esifData);
    static ActiveControlStatus* fanFstObject(UInt32 dataLength, void* esifData);
    static std::vector<ActiveControl> fanFpsObject(UInt32 dataLength, void* esifData);
    static std::vector<PerformanceControl> processorPssObject(UInt32 dataLength, void* esifData);
    static std::vector<PerformanceControl> processorTssObject(PerformanceControl pN, UInt32 dataLength, void* esifData);
    static std::vector<PerformanceControl> processorGfxPstates(UInt32 dataLength, void* binaryData);
    static std::vector<ConfigTdpControl> processorTdplObject(UInt32 dataLength, void* binaryData);
    static std::vector<PowerControlDynamicCaps> processorPpccObject(UInt32 dataLength, void* binaryData);
    static CoreControlLpoPreference* processorClpoObject(UInt32 dataLength, void* binaryData);
    static std::vector<DisplayControl> displayBclObject(UInt32 dataLength, void* binaryData);
    static std::string normalizeAcpiScope(const std::string& acpiScope);

private:

    static UIntN countPpssRows(UIntN size, UInt8* data);
    static UIntN countTrtRows(UInt32 size, UInt8* data);
    static UIntN countArtRows(UInt32 size, UInt8* data);
    static UIntN countLpmtRows(UInt32 size, UInt8* data);
    static UIntN countPssRows(UInt32 size, UInt8* data);
    static void validateData(UInt32 size);
    static UIntN countTssRows(UInt32 size, UInt8* data);
};