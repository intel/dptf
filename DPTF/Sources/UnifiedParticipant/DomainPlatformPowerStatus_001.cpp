/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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

DomainPlatformPowerStatus_001::DomainPlatformPowerStatus_001(UIntN participantIndex, UIntN domainIndex,
    std::shared_ptr<ParticipantServicesInterface> participantServicesInterface) :
    DomainPlatformPowerStatusBase(participantIndex, domainIndex, participantServicesInterface),
    m_platformPowerSource(),
    m_chargerType()
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

DptfBuffer DomainPlatformPowerStatus_001::getBatteryInformation(UIntN participantIndex, UIntN domainIndex)
{
    return getParticipantServices()->primitiveExecuteGet(
        esif_primitive_type::GET_BATTERY_INFORMATION, ESIF_DATA_BINARY, domainIndex);
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

Power DomainPlatformPowerStatus_001::getPlatformBatterySteadyState(UIntN participantIndex, UIntN domainIndex)
{
    m_batterySteadyState = getParticipantServices()->primitiveExecuteGetAsPower(
        esif_primitive_type::GET_PLATFORM_BATTERY_STEADY_STATE, domainIndex);
    return m_batterySteadyState;
}

void DomainPlatformPowerStatus_001::clearCachedData(void)
{
    initializeDataStructures();
}

std::shared_ptr<XmlNode> DomainPlatformPowerStatus_001::getXml(UIntN domainIndex)
{
    auto root = XmlNode::createWrapperElement("platform_power_status");
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    auto pmaxStatus = XmlNode::createWrapperElement("platform_power_status_object");
    pmaxStatus->addChild(XmlNode::createDataElement("name", "Max Battery Power (PMAX)"));
    try
    {
        pmaxStatus->addChild(XmlNode::createDataElement(
            "value", getMaxBatteryPower(getParticipantIndex(), domainIndex).toString() + "mW"));
    }
    catch (...)
    {
        pmaxStatus->addChild(XmlNode::createDataElement("value", "Error"));
    }
    root->addChild(pmaxStatus);
    
    auto psrcStatus = XmlNode::createWrapperElement("platform_power_status_object");
    psrcStatus->addChild(XmlNode::createDataElement("name", "Platform Power Source (PSRC)"));
    try
    {
        psrcStatus->addChild(XmlNode::createDataElement("value",
        PlatformPowerSource::ToString(getPlatformPowerSource(getParticipantIndex(), domainIndex))));
    }
    catch (...)
    {
        psrcStatus->addChild(XmlNode::createDataElement("value", "Error"));
    }
    root->addChild(psrcStatus);

    auto artgStatus = XmlNode::createWrapperElement("platform_power_status_object");
    artgStatus->addChild(XmlNode::createDataElement("name", "Adapter Power Rating (ARTG)"));
    try
    {
        artgStatus->addChild(XmlNode::createDataElement(
        "value", getAdapterPowerRating(getParticipantIndex(), domainIndex).toString() + "mW"));
    }
    catch (...)
    {
        artgStatus->addChild(XmlNode::createDataElement("value", "Error"));
    }
    root->addChild(artgStatus);

    auto ctypStatus = XmlNode::createWrapperElement("platform_power_status_object");
    ctypStatus->addChild(XmlNode::createDataElement("name", "Charger Type (CTYP)"));
    try
    {
        ctypStatus->addChild(XmlNode::createDataElement("value",
        ChargerType::ToString(getChargerType(getParticipantIndex(), domainIndex))));
    }
    catch (...)
    {
        ctypStatus->addChild(XmlNode::createDataElement("value", "Error"));
    }
    root->addChild(ctypStatus);

    auto propStatus = XmlNode::createWrapperElement("platform_power_status_object");
    propStatus->addChild(XmlNode::createDataElement("name", "Platform Rest Of Power (PROP)"));
    try
    {
        propStatus->addChild(XmlNode::createDataElement(
        "value", getPlatformRestOfPower(getParticipantIndex(), domainIndex).toString() + "mW"));
    }
    catch (...)
    {
        propStatus->addChild(XmlNode::createDataElement("value", "Error"));
    }
    root->addChild(propStatus);

    auto apkpStatus = XmlNode::createWrapperElement("platform_power_status_object");
    apkpStatus->addChild(XmlNode::createDataElement("name", "AC Peak Power (APKP)"));
    try
    {
        apkpStatus->addChild(XmlNode::createDataElement(
        "value", getACPeakPower(getParticipantIndex(), domainIndex).toString() + "mW"));
    }
    catch (...)
    {
        apkpStatus->addChild(XmlNode::createDataElement("value", "Error"));
    }
    root->addChild(apkpStatus);

    auto apktStatus = XmlNode::createWrapperElement("platform_power_status_object");
    apktStatus->addChild(XmlNode::createDataElement("name", "AC Peak Time Window (APKT)"));
    try
    {
        apktStatus->addChild(XmlNode::createDataElement(
        "value", getACPeakTimeWindow(getParticipantIndex(), domainIndex).toStringMilliseconds() + "msec"));
    }
    catch (...)
    {
        apktStatus->addChild(XmlNode::createDataElement("value", "Error"));
    }
    root->addChild(apktStatus);

    auto pbssStatus = XmlNode::createWrapperElement("platform_power_status_object");
    pbssStatus->addChild(XmlNode::createDataElement("name", "Platform Battery Steady State (PBSS)"));
    try
    {
        pbssStatus->addChild(XmlNode::createDataElement(
            "value", getPlatformBatterySteadyState(getParticipantIndex(), domainIndex).toString() + "mW"));
    }
    catch (...)
    {
        pbssStatus->addChild(XmlNode::createDataElement("value", "Error"));
    }
    root->addChild(pbssStatus);

    return root;
}

std::string DomainPlatformPowerStatus_001::getName(void)
{
    return "Platform Power Status (Version 1)";
}

void DomainPlatformPowerStatus_001::initializeDataStructures(void)
{
    m_maxBatteryPower = Power::createInvalid();
    m_platformRestOfPower = Power::createInvalid();
    m_adapterRating = Power::createInvalid();
    m_acPeakPower = Power::createInvalid();
    m_acPeakTimeWindow = TimeSpan::createInvalid();
    m_batterySteadyState = Power::createInvalid();
}