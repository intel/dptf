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

#include "DomainPlatformPowerControl_001.h"
#include "XmlNode.h"
#include "StatusFormat.h"
using namespace StatusFormat;
using namespace std;

DomainPlatformPowerControl_001::DomainPlatformPowerControl_001(UIntN participantIndex, UIntN domainIndex,
    ParticipantServicesInterface* participantServicesInterface) :
    DomainPlatformPowerControlBase(participantIndex, domainIndex, participantServicesInterface),
    m_pl1Enabled(false),
    m_pl2Enabled(false),
    m_pl3Enabled(false),
    m_initialState(this)
{
    m_initialState.capture();

    m_pl1Enabled = checkEnabled(PlatformPowerLimitType::PSysPL1);
    m_pl2Enabled = checkEnabled(PlatformPowerLimitType::PSysPL2);
    m_pl3Enabled = checkEnabled(PlatformPowerLimitType::PSysPL3);
}

DomainPlatformPowerControl_001::~DomainPlatformPowerControl_001(void)
{
    m_initialState.restore();
}

Bool DomainPlatformPowerControl_001::isPlatformPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType)
{
    return isEnabled(limitType);
}

Power DomainPlatformPowerControl_001::getPlatformPowerLimit(UIntN participantIndex, UIntN domainIndex,
    PlatformPowerLimitType::Type limitType)
{
    throwIfLimitNotEnabled(limitType);
    throwIfTypeInvalidForPowerLimit(limitType);
    return getParticipantServices()->primitiveExecuteGetAsPower(
        esif_primitive_type::GET_PLATFORM_POWER_LIMIT, domainIndex, (UInt8)limitType);
}

void DomainPlatformPowerControl_001::setPlatformPowerLimit(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType, const Power& powerLimit)
{
    throwIfLimitNotEnabled(limitType);
    throwIfTypeInvalidForPowerLimit(limitType);
    getParticipantServices()->primitiveExecuteSetAsPower(
        esif_primitive_type::SET_PLATFORM_POWER_LIMIT, powerLimit, domainIndex, (UInt8)limitType);
}

TimeSpan DomainPlatformPowerControl_001::getPlatformPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType)
{
    throwIfLimitNotEnabled(limitType);
    throwIfTypeInvalidForTimeWindow(limitType);
    return getParticipantServices()->primitiveExecuteGetAsTimeInMilliseconds(
        esif_primitive_type::GET_PLATFORM_POWER_LIMIT_TIME_WINDOW, domainIndex, (UInt8)limitType);
}

void DomainPlatformPowerControl_001::setPlatformPowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType, const TimeSpan& timeWindow)
{
    throwIfLimitNotEnabled(limitType);
    throwIfTypeInvalidForTimeWindow(limitType);
    getParticipantServices()->primitiveExecuteSetAsTimeInMilliseconds(
        esif_primitive_type::SET_PLATFORM_POWER_LIMIT_TIME_WINDOW, timeWindow,
        domainIndex, (UInt8)limitType);
}

Percentage DomainPlatformPowerControl_001::getPlatformPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType)
{
    throwIfLimitNotEnabled(limitType);
    throwIfTypeInvalidForDutyCycle(limitType);
    return getParticipantServices()->primitiveExecuteGetAsPercentage(
        esif_primitive_type::GET_PLATFORM_POWER_LIMIT_DUTY_CYCLE, domainIndex, (UInt8)limitType);
}

void DomainPlatformPowerControl_001::setPlatformPowerLimitDutyCycle(UIntN participantIndex, UIntN domainIndex, 
    PlatformPowerLimitType::Type limitType, const Percentage& dutyCycle)
{
    throwIfLimitNotEnabled(limitType);
    throwIfTypeInvalidForDutyCycle(limitType);
    getParticipantServices()->primitiveExecuteSetAsPercentage(
        esif_primitive_type::SET_PLATFORM_POWER_LIMIT_DUTY_CYCLE, dutyCycle,
        domainIndex, (UInt8)limitType);
}

void DomainPlatformPowerControl_001::clearCachedData(void)
{
}

XmlNode* DomainPlatformPowerControl_001::getXml(UIntN domainIndex)
{
    XmlNode* root = XmlNode::createWrapperElement("platform_power_control");
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    XmlNode* set = XmlNode::createWrapperElement("platform_power_limit_set");
    set->addChild(createStatusNode(PlatformPowerLimitType::PSysPL1));
    set->addChild(createStatusNode(PlatformPowerLimitType::PSysPL2));
    set->addChild(createStatusNode(PlatformPowerLimitType::PSysPL3));
    root->addChild(set);
    return root;
}

XmlNode* DomainPlatformPowerControl_001::createStatusNode(PlatformPowerLimitType::Type limitType)
{
    XmlNode* pl = XmlNode::createWrapperElement("platform_power_limit");
    pl->addChild(XmlNode::createDataElement("type", PlatformPowerLimitType::ToString(limitType)));
    pl->addChild(XmlNode::createDataElement("enabled", createStatusStringForEnabled(limitType)));
    pl->addChild(XmlNode::createDataElement("limit_value", createStatusStringForLimitValue(limitType)));
    pl->addChild(XmlNode::createDataElement("time_window", createStatusStringForTimeWindow(limitType)));
    pl->addChild(XmlNode::createDataElement("duty_cycle", createStatusStringForDutyCycle(limitType)));
    return pl;
}

std::string DomainPlatformPowerControl_001::createStatusStringForEnabled(PlatformPowerLimitType::Type limitType)
{
    switch (limitType)
    {
    case PlatformPowerLimitType::PSysPL1:
        return friendlyValue(m_pl1Enabled);
    case PlatformPowerLimitType::PSysPL2:
        return friendlyValue(m_pl2Enabled);
    case PlatformPowerLimitType::PSysPL3:
        return friendlyValue(m_pl3Enabled);
    default:
        return "ERROR";
    }
}

std::string DomainPlatformPowerControl_001::createStatusStringForLimitValue(PlatformPowerLimitType::Type limitType)
{
    try
    {
        if (isEnabled(limitType))
        {
            Power powerLimit = getPlatformPowerLimit(getParticipantIndex(), getDomainIndex(),
                limitType);
            return powerLimit.toString();
        }
        else
        {
            return "DISABLED";
        }
    }
    catch (...)
    {
        return "ERROR";
    }
}

std::string DomainPlatformPowerControl_001::createStatusStringForTimeWindow(PlatformPowerLimitType::Type limitType)
{
    try
    {
        if (isEnabled(limitType) && 
            ((limitType == PlatformPowerLimitType::PSysPL1) || 
            (limitType == PlatformPowerLimitType::PSysPL3)))
        {
            TimeSpan timeWindow = getPlatformPowerLimitTimeWindow(getParticipantIndex(), getDomainIndex(),
                limitType);
            return timeWindow.toStringMilliseconds();
        }
        else
        {
            return "DISABLED";
        }
    }
    catch (...)
    {
        return "ERROR";
    }
}

std::string DomainPlatformPowerControl_001::createStatusStringForDutyCycle(PlatformPowerLimitType::Type limitType)
{
    try
    {
        if (isEnabled(limitType) &&
            (limitType == PlatformPowerLimitType::PSysPL3))
        {
            Percentage dutyCycle = getPlatformPowerLimitDutyCycle(getParticipantIndex(), getDomainIndex(),
                limitType);
            return dutyCycle.toString();
        }
        else
        {
            return "DISABLED";
        }
    }
    catch (...)
    {
        return "ERROR";
    }
}

Bool DomainPlatformPowerControl_001::isEnabled(PlatformPowerLimitType::Type limitType) const
{
    switch (limitType)
    {
    case PlatformPowerLimitType::PSysPL1:
        return m_pl1Enabled;
    case PlatformPowerLimitType::PSysPL2:
        return m_pl2Enabled;
    case PlatformPowerLimitType::PSysPL3:
        return m_pl3Enabled;
    default:
        return false;
    }
}

std::string DomainPlatformPowerControl_001::getName(void)
{
    return "Platform Power Control (Version 1)";
}

void DomainPlatformPowerControl_001::throwIfLimitNotEnabled(PlatformPowerLimitType::Type limitType) 
{
    if (isEnabled(limitType) == false)
    {
        string message = "Platform " + PlatformPowerLimitType::ToString(limitType) + " is disabled.";
        throw dptf_exception(message);
    }
}

void DomainPlatformPowerControl_001::throwIfTypeInvalidForPowerLimit(PlatformPowerLimitType::Type limitType)
{
    switch (limitType)
    {
    case PlatformPowerLimitType::PSysPL1:
    case PlatformPowerLimitType::PSysPL2:
    case PlatformPowerLimitType::PSysPL3:
        return;
    default:
        throw dptf_exception("Invalid power limit type selected for Platform Power Limit.");
    }
}

void DomainPlatformPowerControl_001::throwIfTypeInvalidForTimeWindow(PlatformPowerLimitType::Type limitType)
{
    switch (limitType)
    {
    case PlatformPowerLimitType::PSysPL1:
    case PlatformPowerLimitType::PSysPL3:
        return;
    case PlatformPowerLimitType::PSysPL2:
        throw dptf_exception("Platform power limit time window not supported for " +
            PlatformPowerLimitType::ToString(limitType) + ".");
    default:
        throw dptf_exception("Invalid power limit type selected for Platform Power Time Window.");
    }
}

void DomainPlatformPowerControl_001::throwIfTypeInvalidForDutyCycle(PlatformPowerLimitType::Type limitType)
{
    switch (limitType)
    {
    case PlatformPowerLimitType::PSysPL1:
    case PlatformPowerLimitType::PSysPL2:
        throw dptf_exception("Platform power limit duty cycle not supported for " +
            PlatformPowerLimitType::ToString(limitType) + ".");
    case PlatformPowerLimitType::PSysPL3:
        return;
    default:
        throw dptf_exception("Invalid power limit type selected for Platform Power Duty Cycle.");
    }
}
