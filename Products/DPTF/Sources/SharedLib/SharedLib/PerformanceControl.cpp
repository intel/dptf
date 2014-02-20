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

#include "PerformanceControl.h"
#include "StatusFormat.h"
#include "XmlNode.h"

PerformanceControl::PerformanceControl(UIntN controlId, PerformanceControlType::Type performanceControlType,
    UIntN tdpPower, Percentage performancePercentage, UIntN transitionLatency, UIntN controlAbsoluteValue,
    std::string valueUnits) : m_controlId(controlId), m_performanceControlType(performanceControlType),
    m_tdpPower(tdpPower), m_performancePercentage(performancePercentage), m_transitionLatency(transitionLatency),
    m_controlAbsoluteValue(controlAbsoluteValue), m_valueUnits(valueUnits)
{
}

PerformanceControl PerformanceControl::createInvalid()
{
    return PerformanceControl(0, PerformanceControlType::Unknown, 0, Percentage(0.0), 0, 0, "");
}

UIntN PerformanceControl::getControlId(void) const
{
    return m_controlId;
}

PerformanceControlType::Type PerformanceControl::getPerformanceControlType(void) const
{
    return m_performanceControlType;
}

UIntN PerformanceControl::getTdpPower(void) const
{
    return m_tdpPower;
}

Percentage PerformanceControl::getPerformancePercentage(void) const
{
    return m_performancePercentage;
}

UIntN PerformanceControl::getTransitionLatency(void) const
{
    return m_transitionLatency;
}

UIntN PerformanceControl::getControlAbsoluteValue(void) const
{
    return m_controlAbsoluteValue;
}

std::string PerformanceControl::getValueUnits(void) const
{
    return m_valueUnits;
}

Bool PerformanceControl::operator==(const PerformanceControl& rhs) const
{
    return
        ((this->getControlId() == rhs.getControlId()) &&
         (this->getPerformanceControlType() == rhs.getPerformanceControlType()) &&
         (this->getTdpPower() == rhs.getTdpPower()) &&
         (this->getPerformancePercentage() == rhs.getPerformancePercentage()) &&
         (this->getTransitionLatency() == rhs.getTransitionLatency()) &&
         (this->getControlAbsoluteValue() == rhs.getControlAbsoluteValue()) &&
         (this->getValueUnits() == rhs.getValueUnits()));
}

Bool PerformanceControl::operator!=(const PerformanceControl& rhs) const
{
    return !(*this == rhs);
}

std::string PerformanceControl::PerformanceControlTypeToString(PerformanceControlType::Type type)
{
    switch (type)
    {
        case PerformanceControlType::Unknown:
            return "Unknown";
        case PerformanceControlType::PerformanceState:
            return "P-State";
        case PerformanceControlType::ThrottleState:
            return "T-State";
        default:
            throw dptf_exception("Bad performance control type!");
    }
}

XmlNode* PerformanceControl::getXml()
{
    XmlNode* root = XmlNode::createWrapperElement("performance_control");

    root->addChild(XmlNode::createDataElement("control_id", StatusFormat::friendlyValue(m_controlId)));
    root->addChild(XmlNode::createDataElement("control_type", PerformanceControlTypeToString(m_performanceControlType)));
    root->addChild(XmlNode::createDataElement("tdp_power", StatusFormat::friendlyValue(m_tdpPower)));
    root->addChild(XmlNode::createDataElement("performance_percentage", m_performancePercentage.toString()));
    root->addChild(XmlNode::createDataElement("transition_latency", StatusFormat::friendlyValue(m_transitionLatency)));
    root->addChild(XmlNode::createDataElement("control_absolute_value", StatusFormat::friendlyValue(m_controlAbsoluteValue)));
    root->addChild(XmlNode::createDataElement("value_units", m_valueUnits));

    return root;
}