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

#include "AcpiInfo.h"
#include "BinaryParse.h" // TODO: Remove this eventually when we move the ACPI scope normalization

AcpiInfo::AcpiInfo(void) : m_acpiDevice(std::string()), m_acpiScope(std::string()), m_acpiUid(0), m_acpiType(0)
{
}

AcpiInfo::AcpiInfo(const std::string& acpiDevice, const std::string& acpiScope, UInt32 acpiUid, UInt32 acpiType) :
    m_acpiDevice(acpiDevice), m_acpiScope(acpiScope), m_acpiUid(acpiUid), m_acpiType(acpiType)
{
    m_acpiScope = BinaryParse::normalizeAcpiScope(m_acpiScope);
}

std::string AcpiInfo::getAcpiDevice(void) const
{
    return m_acpiDevice;
}

std::string AcpiInfo::getAcpiScope(void) const
{
    return m_acpiScope;
}

UInt32 AcpiInfo::getAcpiUid(void) const
{
    return m_acpiUid;
}

UInt32 AcpiInfo::getAcpiType(void) const
{
    return m_acpiType;
}