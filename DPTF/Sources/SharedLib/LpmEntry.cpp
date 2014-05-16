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

#include "LpmEntry.h"
#include "StatusFormat.h"

using namespace StatusFormat;

LpmEntry::LpmEntry(
    std::string targetDeviceAcpiScope,
    DomainType::Type domainType,
    ControlKnobType::Type controlKnob,
    UInt32 controlValue)
    : m_domainIndex(Constants::Invalid),
    m_targetDeviceIndex(Constants::Invalid),
    m_targetDeviceAcpiScope(targetDeviceAcpiScope),
    m_domainType(domainType),
    m_controlKnob(controlKnob),
    m_controlValue(controlValue),
    m_appliedControl(Constants::Invalid),
    m_appliedControlUnits("X")
{
}

LpmEntry::~LpmEntry()
{
}

UIntN LpmEntry::domainIndex() const
{
    return m_domainIndex;
}

const std::string& LpmEntry::targetDeviceAcpiScope() const
{
    return m_targetDeviceAcpiScope;
}

UIntN LpmEntry::targetDeviceIndex() const
{
    return m_targetDeviceIndex;
}

void LpmEntry::associateParticipantWithAcpiScope(std::string deviceAcpiScope, UIntN deviceIndex)
{
    if (m_targetDeviceAcpiScope == deviceAcpiScope)
    {
        m_targetDeviceIndex = deviceIndex;
    }
}

void LpmEntry::associateParticipantWithDomainType(DomainType::Type domainType, UIntN target)
{
    if (m_domainType == domainType)
    {
        m_targetDeviceIndex = target;
    }
}

void LpmEntry::disassociateParticipant(UIntN deviceIndex)
{
    if (m_targetDeviceIndex == deviceIndex)
    {
        m_targetDeviceIndex = Constants::Invalid;
    }
}

DomainType::Type LpmEntry::domainType() const
{
    return m_domainType;
}

void LpmEntry::associateDomain(UIntN target, UIntN domainIndex, DomainType::Type domainType)
{
    if ((target == targetDeviceIndex()) && (domainType == m_domainType))
    {
        m_domainIndex = domainIndex;
    }
}

void LpmEntry::disassociateDomain(UIntN targetIndex, UIntN domainIndex)
{
    if ((m_targetDeviceIndex == targetIndex) && (m_domainIndex == domainIndex))
    {
        m_domainIndex = Constants::Invalid;
    }
}

ControlKnobType::Type LpmEntry::controlKnob() const
{
    return m_controlKnob;
}

UInt32 LpmEntry::controlValue() const
{
    return m_controlValue;
}

Bool LpmEntry::operator==(const LpmEntry& rhs) const
{
    return ((this->targetDeviceIndex() == rhs.targetDeviceIndex()) &&
            (this->domainIndex() == rhs.domainIndex()) &&
            (this->domainType() == rhs.domainType()) &&
            (this->controlKnob() == rhs.controlKnob()));
}

Bool LpmEntry::operator!=(const LpmEntry& rhs) const
{
    return !(*this == rhs);
}

XmlNode* LpmEntry::getXml(void) const
{
    XmlNode* lpmEntryRoot = XmlNode::createWrapperElement("lpm_entry");
    lpmEntryRoot->addChild(XmlNode::createDataElement("target_acpi_scope", targetDeviceAcpiScope()));
    lpmEntryRoot->addChild(XmlNode::createDataElement("target_index", friendlyValue(m_targetDeviceIndex)));
    lpmEntryRoot->addChild(XmlNode::createDataElement("domain_index", friendlyValue(m_domainIndex)));
    lpmEntryRoot->addChild(XmlNode::createDataElement("domain_type", DomainType::ToString(domainType())));
    lpmEntryRoot->addChild(XmlNode::createDataElement("control_knob", ControlKnobType::ToString(controlKnob())));
    lpmEntryRoot->addChild(XmlNode::createDataElement("control_value", friendlyValue(controlValue())));
    lpmEntryRoot->addChild(XmlNode::createDataElement("applied_control", friendlyValue(appliedControl())));
    lpmEntryRoot->addChild(XmlNode::createDataElement("applied_control_units", appliedControlUnits()));
    return lpmEntryRoot;
}

UInt32 LpmEntry::appliedControl() const
{
    return m_appliedControl;
}

void LpmEntry::setAppliedControl(UInt32 appliedControl)
{
    m_appliedControl = appliedControl;
}

std::string LpmEntry::appliedControlUnits() const
{
    return m_appliedControlUnits;
}

void LpmEntry::setAppliedControlUnits(std::string units)
{
    m_appliedControlUnits = units;
}