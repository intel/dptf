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
    m_maxBatteryPower(Power::createInvalid()),
    m_platformRestOfPower(Power::createInvalid()),
    m_adapterPowerRating(Power::createInvalid()),
    m_platformPowerSource(),
    m_chargerType(),
    m_acPeakPower(Power::createInvalid()),
    m_acPeakTimeWindow(TimeSpan::createInvalid()),
    m_batterySteadyState(Power::createInvalid())
{
}

PlatformPowerStatusFacade::~PlatformPowerStatusFacade(void)
{
}

Power PlatformPowerStatusFacade::getMaxBatteryPower(void)
{
    if (m_domainProperties.implementsPlatformPowerStatusInterface())
    {
        m_maxBatteryPower = m_policyServices.domainPlatformPowerStatus->getMaxBatteryPower(
            m_participantIndex, m_domainIndex);
    }
    return m_maxBatteryPower;
}

Power PlatformPowerStatusFacade::getPlatformRestOfPower(void)
{
    if (m_domainProperties.implementsPlatformPowerStatusInterface())
    {
        m_platformRestOfPower = m_policyServices.domainPlatformPowerStatus->getPlatformRestOfPower(
            m_participantIndex, m_domainIndex);
    }
    return m_platformRestOfPower;
}

Power PlatformPowerStatusFacade::getAdapterPowerRating(void)
{
    if (m_domainProperties.implementsPlatformPowerStatusInterface())
    {
        m_adapterPowerRating = m_policyServices.domainPlatformPowerStatus->getAdapterPowerRating(
            m_participantIndex, m_domainIndex);
    }
    return m_adapterPowerRating;
}

DptfBuffer PlatformPowerStatusFacade::getBatteryStatus(void)
{
    if (m_domainProperties.implementsPlatformPowerStatusInterface())
    {
        return m_policyServices.domainPlatformPowerStatus->getBatteryStatus(
            m_participantIndex, m_domainIndex);
    }

    throw dptf_exception("No support for platform power status interface");
}

DptfBuffer PlatformPowerStatusFacade::getBatteryInformation(void)
{
    if (m_domainProperties.implementsPlatformPowerStatusInterface())
    {
        return m_policyServices.domainPlatformPowerStatus->getBatteryInformation(
            m_participantIndex, m_domainIndex);
    }

    throw dptf_exception("No support for platform power status interface");
}

PlatformPowerSource::Type PlatformPowerStatusFacade::getPlatformPowerSource(void)
{
    if (m_domainProperties.implementsPlatformPowerStatusInterface())
    {
        m_platformPowerSource = m_policyServices.domainPlatformPowerStatus->getPlatformPowerSource(
            m_participantIndex, m_domainIndex);
    }
    return m_platformPowerSource;
}

ChargerType::Type PlatformPowerStatusFacade::getChargerType(void)
{
    if (m_domainProperties.implementsPlatformPowerStatusInterface())
    {
        m_chargerType = m_policyServices.domainPlatformPowerStatus->getChargerType(
            m_participantIndex, m_domainIndex);
    }
    return m_chargerType;
}

Power PlatformPowerStatusFacade::getACPeakPower(void)
{
    if (m_domainProperties.implementsPlatformPowerStatusInterface())
    {
        m_acPeakPower = m_policyServices.domainPlatformPowerStatus->getACPeakPower(
            m_participantIndex, m_domainIndex);
    }
    return m_acPeakPower;
}

TimeSpan PlatformPowerStatusFacade::getACPeakTimeWindow(void)
{
    if (m_domainProperties.implementsPlatformPowerStatusInterface())
    {
        m_acPeakTimeWindow = m_policyServices.domainPlatformPowerStatus->getACPeakTimeWindow(
            m_participantIndex, m_domainIndex);
    }
    return m_acPeakTimeWindow;
}

Power PlatformPowerStatusFacade::getPlatformBatterySteadyState(void)
{
    if (m_domainProperties.implementsPlatformPowerStatusInterface())
    {
        m_batterySteadyState = m_policyServices.domainPlatformPowerStatus->getPlatformBatterySteadyState(
            m_participantIndex, m_domainIndex);
    }
    return m_batterySteadyState;
}

std::shared_ptr<XmlNode> PlatformPowerStatusFacade::getXml() const
{
    auto control = XmlNode::createWrapperElement("platform_power_status");
    control->addChild(XmlNode::createDataElement("max_battery_power", m_maxBatteryPower.toString()));
    control->addChild(XmlNode::createDataElement(
        "platform_power_source", PlatformPowerSource::ToString(m_platformPowerSource)));
    control->addChild(XmlNode::createDataElement("adapter_power_rating", m_adapterPowerRating.toString()));
    control->addChild(XmlNode::createDataElement("charger_type", ChargerType::ToString(m_chargerType)));
    control->addChild(XmlNode::createDataElement("platform_rest_of_power", m_platformRestOfPower.toString()));
    control->addChild(XmlNode::createDataElement("ac_peak_power", m_acPeakPower.toString()));
    control->addChild(XmlNode::createDataElement("ac_peak_time_window", m_acPeakTimeWindow.toStringMilliseconds()));
    control->addChild(XmlNode::createDataElement("battery_steady_state", m_batterySteadyState.toString()));
    return control;
}
