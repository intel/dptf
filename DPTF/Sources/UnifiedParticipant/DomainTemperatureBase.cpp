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

#include "DomainTemperatureBase.h"

DomainTemperatureBase::DomainTemperatureBase(UIntN participantIndex, UIntN domainIndex,
    ParticipantServicesInterface* participantServicesInterface)
    : ControlBase(participantIndex, domainIndex, participantServicesInterface)
{

}

DomainTemperatureBase::~DomainTemperatureBase()
{

}

TemperatureThresholds DomainTemperatureBase::getTemperatureThresholds(UIntN participantIndex, UIntN domainIndex)
{
    Temperature aux0 = getAuxTemperatureThreshold(domainIndex, 0);
    Temperature aux1 = getAuxTemperatureThreshold(domainIndex, 1);
    Temperature hysteresis = getHysteresis(domainIndex);
    return TemperatureThresholds(aux0, aux1, hysteresis);
}

void DomainTemperatureBase::setTemperatureThresholds(UIntN participantIndex,
    UIntN domainIndex, const TemperatureThresholds& temperatureThresholds)
{
    Temperature aux0;
    Temperature minAux = Temperature::fromCelsius(ESIF_SDK_MIN_AUX_TRIP);

    if (temperatureThresholds.getAux0().isValid())
    {
        aux0 = temperatureThresholds.getAux0();
        aux0 = Temperature::snapWithinAllowableTripPointRange(aux0);
    }
    else
    {
        aux0 = minAux;
    }

    Temperature aux1;
    Temperature maxAux = Temperature::fromCelsius(ESIF_SDK_MAX_AUX_TRIP);

    if (temperatureThresholds.getAux1().isValid())
    {
        aux1 = temperatureThresholds.getAux1();
        aux1 = Temperature::snapWithinAllowableTripPointRange(aux1);
    }
    else
    {
        aux1 = maxAux;
    }

    if ((m_lastSetAux0.isInvalid() && m_lastSetAux1.isInvalid()) ||
        (m_lastSetAux0.isValid() && aux0 < m_lastSetAux0.get()) ||
        (m_lastSetAux1.isValid() && aux1 < m_lastSetAux1.get()))
    {
        setAux0(aux0, domainIndex);
        setAux1(aux1, domainIndex);
    }
    else
    {
        setAux1(aux1, domainIndex);
        setAux0(aux0, domainIndex);
    }
}

void DomainTemperatureBase::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
    try
    {
        if (isActivityLoggingEnabled() == true)
        {
            TemperatureThresholds tempthreshold = getTemperatureThresholds(participantIndex, domainIndex);

            EsifCapabilityData capability;
            capability.type = ESIF_CAPABILITY_TYPE_TEMP_THRESHOLD;
            capability.size = sizeof(capability);
            capability.data.temperatureThresholdControl.aux0 = tempthreshold.getAux0();
            capability.data.temperatureThresholdControl.aux1 = tempthreshold.getAux1();
            capability.data.temperatureThresholdControl.hysteresis = tempthreshold.getHysteresis();

            getParticipantServices()->sendDptfEvent(ParticipantEvent::DptfParticipantControlAction,
                domainIndex, Capability::getEsifDataFromCapabilityData(&capability));
        }
    }
    catch (...)
    {
        // skip if there are any issues in sending log data
    }
}

Temperature DomainTemperatureBase::getAuxTemperatureThreshold(UIntN domainIndex, UInt8 auxNumber)
{
    try
    {
        auto aux = getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
            esif_primitive_type::GET_TEMPERATURE_THRESHOLDS, domainIndex, auxNumber);
        aux = Temperature::snapWithinAllowableTripPointRange(aux);
        return aux;
    }
    catch (...)
    {
        return Temperature::fromCelsius(0);
    }
}

Temperature DomainTemperatureBase::getHysteresis(UIntN domainIndex) const
{
    try
    {
        return getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
            esif_primitive_type::GET_TEMPERATURE_THRESHOLD_HYSTERESIS,
            domainIndex);
    }
    catch (...)
    {
        return Temperature::fromCelsius(0);
    }
}

void DomainTemperatureBase::setAux0(Temperature &aux0, UIntN domainIndex)
{
    try
    {
        getParticipantServices()->primitiveExecuteSetAsTemperatureTenthK(
            esif_primitive_type::SET_TEMPERATURE_THRESHOLDS, aux0, domainIndex, 0);
        m_lastSetAux0.set(aux0);
    }
    catch (...)
    {
        // eat any errors here
    }
}

void DomainTemperatureBase::setAux1(Temperature &aux1, UIntN domainIndex)
{
    try
    {
        getParticipantServices()->primitiveExecuteSetAsTemperatureTenthK(
            esif_primitive_type::SET_TEMPERATURE_THRESHOLDS, aux1, domainIndex, 1);
        m_lastSetAux1.set(aux1);
    }
    catch (...)
    {
        // eat any errors here
    }
}
