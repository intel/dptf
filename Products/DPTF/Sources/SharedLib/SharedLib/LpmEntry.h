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
#include "DomainType.h"
#include "ControlKnobType.h"
#include <string>
#include "XmlNode.h"

namespace LpmConfigurationVersion
{
    enum Type
    {
        v0,
        v1,
        vInvalid
    };
}

class dptf_export LpmEntry final
{
public:

    LpmEntry(
        std::string targetDeviceAcpiScope,
        DomainType::Type domainType,
        ControlKnobType::Type controlKnob,
        UInt32 controlValue
        );
    ~LpmEntry();

    void associateDomain(UIntN target, UIntN domainIndex, DomainType::Type domainType);
    void disassociateDomain(UIntN targetIndex, UIntN domainIndex);
    void associateParticipantWithAcpiScope(std::string deviceAcpiScope, UIntN deviceIndex);
    void associateParticipantWithDomainType(DomainType::Type domainType, UIntN target);
    void disassociateParticipant(UIntN deviceIndex);

    const std::string& targetDeviceAcpiScope() const;
    UIntN targetDeviceIndex() const;
    UIntN domainIndex() const;
    DomainType::Type domainType() const;
    ControlKnobType::Type controlKnob() const;
    UInt32 controlValue() const;
    UInt32 appliedControl() const;
    std::string appliedControlUnits() const;

    Bool operator==(const LpmEntry& rhs) const;
    Bool operator!=(const LpmEntry& rhs) const;


    void setAppliedControl(UInt32 appliedControl);
    void setAppliedControlUnits(std::string units);

    XmlNode* getXml(void) const;

private:

    UIntN m_domainIndex;
    UIntN m_targetDeviceIndex;

    std::string m_targetDeviceAcpiScope;
    DomainType::Type m_domainType;
    ControlKnobType::Type m_controlKnob;
    UInt32 m_controlValue;

    // Used for status only.
    UInt32 m_appliedControl;
    std::string m_appliedControlUnits;
};