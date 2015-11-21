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

#include "PlatformPowerStatusFacade.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

PlatformPowerStatusFacade::PlatformPowerStatusFacade(
    UIntN participantIndex, UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : m_policyServices(policyServices),
    m_domainProperties(domainProperties),
    m_participantIndex(participantIndex),
    m_domainIndex(domainIndex),
    m_batteryStatusValid(false)
{
}

PlatformPowerStatusFacade::~PlatformPowerStatusFacade(void)
{
}

Power PlatformPowerStatusFacade::getMaxBatteryPower(void)
{
    if (m_maxBatteryPower.isInvalid())
    {
        m_maxBatteryPower.set(m_policyServices.domainPlatformPowerStatus->getMaxBatteryPower(
            m_participantIndex, m_domainIndex));
    }
    return m_maxBatteryPower.get();
}

Power PlatformPowerStatusFacade::getAdapterPower(void)
{
    if (m_adapterPower.isInvalid())
    {
        m_adapterPower.set(m_policyServices.domainPlatformPowerStatus->getAdapterPower(
            m_participantIndex, m_domainIndex));
    }
    return m_adapterPower.get();
}

Power PlatformPowerStatusFacade::getPlatformPowerConsumption(void)
{
    if (m_platformPowerConsumption.isInvalid())
    {
        m_platformPowerConsumption.set(m_policyServices.domainPlatformPowerStatus->getPlatformPowerConsumption(
            m_participantIndex, m_domainIndex));
    }
    return m_platformPowerConsumption.get();
}

Power PlatformPowerStatusFacade::getPlatformRestOfPower(void)
{
    if (m_platformRestOfPower.isInvalid())
    {
        m_platformRestOfPower.set(m_policyServices.domainPlatformPowerStatus->getPlatformRestOfPower(
            m_participantIndex, m_domainIndex));
    }
    return m_platformRestOfPower.get();
}

Power PlatformPowerStatusFacade::getAdapterPowerRating(void)
{
    if (m_adapterPowerRating.isInvalid())
    {
        m_adapterPowerRating.set(m_policyServices.domainPlatformPowerStatus->getAdapterPowerRating(
            m_participantIndex, m_domainIndex));
    }
    return m_adapterPowerRating.get();
}

DptfBuffer PlatformPowerStatusFacade::getBatteryStatus(void)
{
    if (m_batteryStatusValid == false)
    {
        m_batteryStatus = m_policyServices.domainPlatformPowerStatus->getBatteryStatus(
            m_participantIndex, m_domainIndex);
        m_batteryStatusValid = true;
    }
    return m_batteryStatus;
}

PlatformPowerSource::Type PlatformPowerStatusFacade::getPlatformPowerSource(void)
{
    if (m_platformPowerSource.isInvalid())
    {
        m_platformPowerSource.set(m_policyServices.domainPlatformPowerStatus->getPlatformPowerSource(
            m_participantIndex, m_domainIndex));
    }
    return m_platformPowerSource.get();
}

ChargerType::Type PlatformPowerStatusFacade::getChargerType(void)
{
    if (m_chargerType.isInvalid())
    {
        m_chargerType.set(m_policyServices.domainPlatformPowerStatus->getChargerType(
            m_participantIndex, m_domainIndex));
    }
    return m_chargerType.get();
}

Percentage PlatformPowerStatusFacade::getPlatformStateOfCharge(void)
{
    if (m_platformStateOfCharge.isInvalid())
    {
        m_platformStateOfCharge.set(m_policyServices.domainPlatformPowerStatus->getPlatformStateOfCharge(
            m_participantIndex, m_domainIndex));
    }
    return m_platformStateOfCharge.get();
}

Power PlatformPowerStatusFacade::getACPeakPower(void)
{
    if (m_acPeakPower.isInvalid())
    {
        m_acPeakPower.set(m_policyServices.domainPlatformPowerStatus->getACPeakPower(
            m_participantIndex, m_domainIndex));
    }
    return m_acPeakPower.get();
}

TimeSpan PlatformPowerStatusFacade::getACPeakTimeWindow(void)
{
    if (m_acPeakTimeWindow.isInvalid())
    {
        m_acPeakTimeWindow.set(m_policyServices.domainPlatformPowerStatus->getACPeakTimeWindow(
            m_participantIndex, m_domainIndex));
    }
    return m_acPeakTimeWindow.get();
}

XmlNode* PlatformPowerStatusFacade::getXml() const
{
    XmlNode* control = XmlNode::createWrapperElement("platform_power_status");
    control->addChild(XmlNode::createDataElement("max_battery_power", m_maxBatteryPower.get().toString()));
    control->addChild(XmlNode::createDataElement("adapter_power", m_adapterPower.get().toString()));
    control->addChild(XmlNode::createDataElement("platform_power_consumption", m_platformPowerConsumption.get().toString()));
    control->addChild(XmlNode::createDataElement("platform_state_of_charge", m_platformStateOfCharge.get().toString()));
    control->addChild(XmlNode::createDataElement(
        "platform_power_source", PlatformPowerSource::ToString(m_platformPowerSource.get())));
    control->addChild(XmlNode::createDataElement("adapter_power_rating", m_adapterPowerRating.get().toString()));
    control->addChild(XmlNode::createDataElement("charger_type", ChargerType::ToString(m_chargerType.get())));
    control->addChild(XmlNode::createDataElement("platform_rest_of_power", m_platformRestOfPower.get().toString()));
    control->addChild(XmlNode::createDataElement("ac_peak_power", m_acPeakPower.get().toString()));
    control->addChild(XmlNode::createDataElement("ac_peak_time_window", m_acPeakTimeWindow.get().toStringMilliseconds()));
    return control;
}