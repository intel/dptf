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

#include "Dptf.h"

class PciInfo final
{
public:

    PciInfo(void);
    PciInfo(UInt16 pciVendor, UInt16 pciDevice, UInt8 pciBus, UInt8 pciBusDevice, UInt8 pciFunction,
        UInt8 pciRevision, UInt8 pciClass, UInt8 pciSubClass, UInt8 pciProgIf);

    UInt16 getPciVendor(void) const;
    UInt16 getPciDevice(void) const;
    UInt8 getPciBus(void) const;
    UInt8 getPciBusDevice(void) const;
    UInt8 getPciFunction(void) const;
    UInt8 getPciRevision(void) const;
    UInt8 getPciClass(void) const;
    UInt8 getPciSubClass(void) const;
    UInt8 getPciProgIf(void) const;

private:

    UInt16 m_pciVendor;
    UInt16 m_pciDevice;
    UInt8 m_pciBus;
    UInt8 m_pciBusDevice;
    UInt8 m_pciFunction;
    UInt8 m_pciRevision;
    UInt8 m_pciClass;
    UInt8 m_pciSubClass;
    UInt8 m_pciProgIf;
};