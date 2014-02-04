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

#include "PciInfo.h"

PciInfo::PciInfo(void) :
    m_pciVendor(0),
    m_pciDevice(0),
    m_pciBus(0),
    m_pciBusDevice(0),
    m_pciFunction(0),
    m_pciRevision(0),
    m_pciClass(0),
    m_pciSubClass(0),
    m_pciProgIf(0)
{
}

PciInfo::PciInfo(UInt16 pciVendor, UInt16 pciDevice, UInt8 pciBus, UInt8 pciBusDevice,
    UInt8 pciFunction, UInt8 pciRevision, UInt8 pciClass, UInt8 pciSubClass, UInt8 pciProgIf) :
    m_pciVendor(pciVendor),
    m_pciDevice(pciDevice),
    m_pciBus(pciBus),
    m_pciBusDevice(pciBusDevice),
    m_pciFunction(pciFunction),
    m_pciRevision(pciRevision),
    m_pciClass(pciClass),
    m_pciSubClass(pciSubClass),
    m_pciProgIf(pciProgIf)
{
}

UInt16 PciInfo::getPciVendor(void) const
{
    return m_pciVendor;
}

UInt16 PciInfo::getPciDevice(void) const
{
    return m_pciDevice;
}

UInt8 PciInfo::getPciBus(void) const
{
    return m_pciBus;
}

UInt8 PciInfo::getPciBusDevice(void) const
{
    return m_pciBusDevice;
}

UInt8 PciInfo::getPciFunction(void) const
{
    return m_pciFunction;
}

UInt8 PciInfo::getPciRevision(void) const
{
    return m_pciRevision;
}

UInt8 PciInfo::getPciClass(void) const
{
    return m_pciClass;
}

UInt8 PciInfo::getPciSubClass(void) const
{
    return m_pciSubClass;
}

UInt8 PciInfo::getPciProgIf(void) const
{
    return m_pciProgIf;
}