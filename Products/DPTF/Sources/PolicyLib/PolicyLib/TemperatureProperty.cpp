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

#include "TemperatureProperty.h"
using namespace std;

TemperatureProperty::TemperatureProperty(
    UIntN participantIndex,
    UIntN domainIndex,
    const DomainProperties& domainProperties,
    const PolicyServicesInterfaceContainer& policyServices)
    : DomainProperty(participantIndex, domainIndex, domainProperties, policyServices),
    m_lastThresholdsSet(Temperature::createInvalid(), Temperature::createInvalid(), Constants::Invalid),
    m_lastThresholdsSetValid(false)
{
}

TemperatureProperty::~TemperatureProperty()
{
}

void TemperatureProperty::setTemperatureNotificationThresholds(
    const Temperature& lowerBound,
    const Temperature& upperBound)
{
    updateThresholdsIfRequired();
    TemperatureThresholds thresholds(lowerBound, upperBound, m_lastThresholdsSet.getHysteresis());
    getPolicyServices().domainTemperature->setTemperatureThresholds(
        getParticipantIndex(), getDomainIndex(), thresholds);
    m_lastThresholdsSet = thresholds;
}

Temperature TemperatureProperty::getCurrentTemperature(void)
{
    if (implementsTemperatureInterface())
    {
        auto temperatureStatus = getPolicyServices().domainTemperature->getTemperatureStatus(
            getParticipantIndex(),getDomainIndex());
        return temperatureStatus.getCurrentTemperature();
    }
    else
    {
        throw dptf_exception("Domain does not support the temperature interface.");
    }
}

Bool TemperatureProperty::implementsTemperatureInterface(void)
{
    return getDomainProperties().implementsTemperatureInterface();
}

TemperatureThresholds TemperatureProperty::getTemperatureNotificationThresholds()
{
    updateThresholdsIfRequired();
    return m_lastThresholdsSet;
}

void TemperatureProperty::updateThresholdsIfRequired()
{
    if (m_lastThresholdsSetValid == false)
    {
        TemperatureThresholds thresholds =
            getPolicyServices().domainTemperature->getTemperatureThresholds(getParticipantIndex(), getDomainIndex());
        m_lastThresholdsSet = TemperatureThresholds(m_lastThresholdsSet.getAux0(), m_lastThresholdsSet.getAux1(), thresholds.getHysteresis());
        m_lastThresholdsSetValid = true;
    }
}

Bool TemperatureProperty::supportsProperty()
{
    return getDomainProperties().implementsTemperatureInterface();
}
