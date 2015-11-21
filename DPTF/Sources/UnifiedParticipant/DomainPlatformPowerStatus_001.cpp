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

#include "DomainPlatformPowerStatus_001.h"
#include "XmlNode.h"
#include "BinaryParse.h"

DomainPlatformPowerStatus_001::DomainPlatformPowerStatus_001(UIntN participantIndex, UIntN domainIndex,
    ParticipantServicesInterface* participantServicesInterface) :
    DomainPlatformPowerStatusBase(participantIndex, domainIndex, participantServicesInterface)
{
    initializeDataStructures();
}

DomainPlatformPowerStatus_001::~DomainPlatformPowerStatus_001(void)
{
}

Power DomainPlatformPowerStatus_001::getMaxBatteryPower(UIntN participantIndex, UIntN domainIndex)
{
    m_maxBatteryPower = getParticipantServices()->primitiveExecuteGetAsPower(
            esif_primitive_type::GET_PLATFORM_MAX_BATTERY_POWER, domainIndex);
    return m_maxBatteryPower;
}

Power DomainPlatformPowerStatus_001::getAdapterPower(UIntN participantIndex, UIntN domainIndex)
{
    m_adapterPower = getParticipantServices()->primitiveExecuteGetAsPower(
        esif_primitive_type::GET_ADAPTER_POWER, domainIndex);
    return m_adapterPower;
}

Power DomainPlatformPowerStatus_001::getPlatformPowerConsumption(UIntN participantIndex, UIntN domainIndex)
{
    m_platformPower = getParticipantServices()->primitiveExecuteGetAsPower(
        esif_primitive_type::GET_PLATFORM_POWER_CONSUMPTION, domainIndex);
    return m_platformPower;
}

Power DomainPlatformPowerStatus_001::getPlatformRestOfPower(UIntN participantIndex, UIntN domainIndex)
{
    m_platformRestOfPower = getParticipantServices()->primitiveExecuteGetAsPower(
        esif_primitive_type::GET_PLATFORM_REST_OF_POWER, domainIndex);
    return m_platformRestOfPower;
}

Power DomainPlatformPowerStatus_001::getAdapterPowerRating(UIntN participantIndex, UIntN domainIndex)
{
    m_adapterRating = getParticipantServices()->primitiveExecuteGetAsPower(
        esif_primitive_type::GET_ADAPTER_POWER_RATING, domainIndex);
    return m_adapterRating;
}

DptfBuffer DomainPlatformPowerStatus_001::getBatteryStatus(UIntN participantIndex, UIntN domainIndex)
{
    return getParticipantServices()->primitiveExecuteGet(
        esif_primitive_type::GET_BATTERY_STATUS, ESIF_DATA_BINARY, domainIndex);
}

PlatformPowerSource::Type DomainPlatformPowerStatus_001::getPlatformPowerSource(UIntN participantIndex, UIntN domainIndex)
{
    auto powerSource = getParticipantServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_PLATFORM_POWER_SOURCE, domainIndex);
    m_platformPowerSource = PlatformPowerSource::Type(powerSource);
    return m_platformPowerSource;
}

ChargerType::Type DomainPlatformPowerStatus_001::getChargerType(UIntN participantIndex, UIntN domainIndex)
{
    auto chargerType = getParticipantServices()->primitiveExecuteGetAsUInt32(
        esif_primitive_type::GET_CHARGER_TYPE, domainIndex);
    m_chargerType = ChargerType::Type(chargerType);
    return m_chargerType;
}

Percentage DomainPlatformPowerStatus_001::getPlatformStateOfCharge(UIntN participantIndex, UIntN domainIndex)
{
    m_stateOfCharge = getParticipantServices()->primitiveExecuteGetAsPercentage(
        esif_primitive_type::GET_PLATFORM_STATE_OF_CHARGE, domainIndex);
    return m_stateOfCharge;
}

Power DomainPlatformPowerStatus_001::getACPeakPower(UIntN participantIndex, UIntN domainIndex)
{
    m_acPeakPower = getParticipantServices()->primitiveExecuteGetAsPower(
        esif_primitive_type::GET_AC_PEAK_POWER, domainIndex);
    return m_acPeakPower;
}

TimeSpan DomainPlatformPowerStatus_001::getACPeakTimeWindow(UIntN participantIndex, UIntN domainIndex)
{
    m_acPeakTimeWindow = getParticipantServices()->primitiveExecuteGetAsTimeInMilliseconds(
        esif_primitive_type::GET_AC_PEAK_TIME_WINDOW, domainIndex);
    return m_acPeakTimeWindow;
}

void DomainPlatformPowerStatus_001::clearCachedData(void)
{
    initializeDataStructures();
}

XmlNode* DomainPlatformPowerStatus_001::getXml(UIntN domainIndex)
{
    XmlNode* root = XmlNode::createWrapperElement("platform_power_status");
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));
    try
    {
        root->addChild(XmlNode::createDataElement(
            "max_battery_power", getMaxBatteryPower(Constants::Invalid, domainIndex).toString() + "mW"));
    }
    catch (...)
    {
        root->addChild(XmlNode::createDataElement("max_battery_power", "Error"));
    }
    try
    {
    root->addChild(XmlNode::createDataElement(
        "adapter_power", getAdapterPower(Constants::Invalid, domainIndex).toString() + "mW"));
    }
    catch (...)
    {
        root->addChild(XmlNode::createDataElement("adapter_power", "Error"));
    }
    try
    {
    root->addChild(XmlNode::createDataElement(
        "platform_power_consumption", getPlatformPowerConsumption(Constants::Invalid, domainIndex).toString() + "mW"));
    }
    catch (...)
    {
        root->addChild(XmlNode::createDataElement("platform_power_consumption", "Error"));
    }
    try
    {
    root->addChild(XmlNode::createDataElement(
        "platform_state_of_charge", getPlatformStateOfCharge(Constants::Invalid, domainIndex).toString() + "%"));
    }
    catch (...)
    {
        root->addChild(XmlNode::createDataElement("platform_state_of_charge", "Error"));
    }
    try
    {
    root->addChild(XmlNode::createDataElement("platform_power_source", 
        PlatformPowerSource::ToString(getPlatformPowerSource(Constants::Invalid, domainIndex))));
    }
    catch (...)
    {
        root->addChild(XmlNode::createDataElement("platform_power_source", "Error"));
    }
    try
    {
    root->addChild(XmlNode::createDataElement(
        "adapter_power_rating", getAdapterPowerRating(Constants::Invalid, domainIndex).toString() + "mW"));
    }
    catch (...)
    {
        root->addChild(XmlNode::createDataElement("adapter_power_rating", "Error"));
    }
    try
    {
    root->addChild(XmlNode::createDataElement("charger_type", 
        ChargerType::ToString(getChargerType(Constants::Invalid, domainIndex))));
    }
    catch (...)
    {
        root->addChild(XmlNode::createDataElement("charger_type", "Error"));
    }
    try
    {
    root->addChild(XmlNode::createDataElement(
        "platform_rest_of_power", getPlatformRestOfPower(Constants::Invalid, domainIndex).toString() + "mW"));
    }
    catch (...)
    {
        root->addChild(XmlNode::createDataElement("platform_rest_of_power", "Error"));
    }
    try
    {
    root->addChild(XmlNode::createDataElement(
        "ac_peak_power", getACPeakPower(Constants::Invalid, domainIndex).toString() + "mW"));
    }
    catch (...)
    {
        root->addChild(XmlNode::createDataElement("ac_peak_power", "Error"));
    }
    try
    {
    root->addChild(XmlNode::createDataElement(
        "ac_peak_time_window", getACPeakTimeWindow(Constants::Invalid, domainIndex).toStringMilliseconds() + "msec"));
    }
    catch (...)
    {
        root->addChild(XmlNode::createDataElement("ac_peak_time_window", "Error"));
    }

    return root;
}

std::string DomainPlatformPowerStatus_001::getName(void)
{
    return "Platform Power Status (Version 1)";
}

void DomainPlatformPowerStatus_001::initializeDataStructures(void)
{
    m_maxBatteryPower = Power::createInvalid();
    m_adapterPower = Power::createInvalid();
    m_platformPower = Power::createInvalid();
    m_platformRestOfPower = Power::createInvalid();
    m_adapterRating = Power::createInvalid();
    m_stateOfCharge = Percentage::createInvalid();
    m_acPeakPower = Power::createInvalid();
    m_acPeakTimeWindow = TimeSpan::createInvalid();
}