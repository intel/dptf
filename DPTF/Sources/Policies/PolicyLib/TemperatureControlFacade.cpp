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

#include "TemperatureControlFacade.h"
using namespace std;

TemperatureControlFacade::TemperatureControlFacade(
    UIntN participantIndex, 
    UIntN domainIndex, 
    const DomainProperties& domainProperties, 
    const PolicyServicesInterfaceContainer& policyServices)
    : m_policyServices(policyServices),
    m_participantIndex(participantIndex),
    m_domainIndex(domainIndex),
    m_domainProperties(domainProperties),
    m_temperatureThresholds(
    TemperatureThresholds(Temperature::createInvalid(), Temperature::createInvalid(), Temperature::createInvalid()))
{
    m_temperatureThresholds.invalidate();
    m_calibrationTable.invalidate();
    m_pollingTable.invalidate();
}

TemperatureControlFacade::~TemperatureControlFacade()
{
}

Temperature TemperatureControlFacade::getCurrentTemperature(void)
{
    if (supportsTemperatureControls())
    {
        auto temperatureStatus = m_policyServices.domainTemperature->getTemperatureStatus(
            m_participantIndex, m_domainIndex);
        return temperatureStatus.getCurrentTemperature();
    }
    else
    {
        throw dptf_exception("Domain does not support the temperature interface.");
    }
}

TemperatureThresholds TemperatureControlFacade::getTemperatureNotificationThresholds()
{
    if (supportsTemperatureThresholds())
    {
        if (m_temperatureThresholds.isInvalid())
        {
            m_temperatureThresholds.set(m_policyServices.domainTemperature->getTemperatureThresholds(
                m_participantIndex, m_domainIndex));
        }

        return m_temperatureThresholds.get();
    }
    else
    {
        throw dptf_exception("Domain does not support the temperature threshold interface.");
    }
}

void TemperatureControlFacade::setTemperatureNotificationThresholds(
    const Temperature& lowerBound, const Temperature& upperBound)
{
    if (supportsTemperatureThresholds())
    {
        TemperatureThresholds thresholdsToSet(lowerBound, upperBound, getHysteresis());
        m_policyServices.domainTemperature->setTemperatureThresholds(
            m_participantIndex, m_domainIndex, thresholdsToSet);
        m_temperatureThresholds.set(thresholdsToSet);
    }
}

Bool TemperatureControlFacade::supportsTemperatureControls()
{
    return m_domainProperties.implementsTemperatureInterface();
}

Bool TemperatureControlFacade::supportsTemperatureThresholds()
{
    return m_domainProperties.implementsTemperatureThresholdInterface();
}

Bool TemperatureControlFacade::isVirtualTemperatureControl()
{
    if (supportsTemperatureControls())
    {
        if (m_isVirtualSensor.isInvalid())
        {
            m_isVirtualSensor.set(m_policyServices.domainTemperature->isVirtualTemperature(
                m_participantIndex, m_domainIndex));
        }

        return m_isVirtualSensor.get();
    }
    else
    {
        return false;
    }
}

DptfBuffer TemperatureControlFacade::getCalibrationTable()
{
    if (supportsTemperatureControls())
    {
        if (m_calibrationTable.isInvalid())
        {
            m_calibrationTable.set(m_policyServices.domainTemperature->getCalibrationTable(m_participantIndex, m_domainIndex));
        }

        return m_calibrationTable.get();
    }
    else
    {
        throw dptf_exception("Domain does not support the temperature interface.");
    }
}

DptfBuffer TemperatureControlFacade::getPollingTable()
{
    if (supportsTemperatureControls())
    {
        if (m_pollingTable.isInvalid())
        {
            m_pollingTable.set(m_policyServices.domainTemperature->getPollingTable(m_participantIndex, m_domainIndex));
        }

        return m_pollingTable.get();
    }
    else
    {
        throw dptf_exception("Domain does not support the temperature interface.");
    }
}

void TemperatureControlFacade::setVirtualTemperature(const Temperature& temperature)
{
    if (supportsTemperatureControls())
    {
        m_policyServices.domainTemperature->setVirtualTemperature(m_participantIndex, m_domainIndex, temperature);
    }
}

void TemperatureControlFacade::refreshHysteresis()
{
    if (supportsTemperatureThresholds())
    {
        if (m_temperatureThresholds.isInvalid())
        {
            m_temperatureThresholds.set(m_policyServices.domainTemperature->getTemperatureThresholds(
                m_participantIndex, m_domainIndex));
        }

        auto currentThresholds = m_policyServices.domainTemperature->getTemperatureThresholds(
            m_participantIndex, m_domainIndex);
        TemperatureThresholds thresholdsWithUpdatedHysteresis(
            m_temperatureThresholds.get().getAux0(), 
            m_temperatureThresholds.get().getAux1(), 
            currentThresholds.getHysteresis());
        m_temperatureThresholds.set(thresholdsWithUpdatedHysteresis);
    }
}

void TemperatureControlFacade::refreshVirtualSensorTables()
{
    m_calibrationTable.invalidate();
    m_pollingTable.invalidate();
}

Temperature TemperatureControlFacade::getHysteresis()
{
    if (supportsTemperatureThresholds())
    {
        if (m_temperatureThresholds.isInvalid())
        {
            m_temperatureThresholds.set(m_policyServices.domainTemperature->getTemperatureThresholds(
                m_participantIndex, m_domainIndex));
        }

        return m_temperatureThresholds.get().getHysteresis();
    }
    else
    {
        throw dptf_exception("Domain does not support the temperature threshold interface.");
    }
}
