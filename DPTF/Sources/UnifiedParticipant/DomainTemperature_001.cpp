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

#include "DomainTemperature_001.h"
#include "XmlNode.h"

DomainTemperature_001::DomainTemperature_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface)
{
}

TemperatureStatus DomainTemperature_001::getTemperatureStatus(UIntN participantIndex, UIntN domainIndex)
{
    try
    {
        Temperature temperature = m_participantServicesInterface->primitiveExecuteGetAsTemperatureC(
            esif_primitive_type::GET_TEMPERATURE, domainIndex);
        return TemperatureStatus(temperature);
    }
    catch (...)
    {
        return TemperatureStatus(Temperature(0));
    }
}

TemperatureThresholds DomainTemperature_001::getTemperatureThresholds(UIntN participantIndex, UIntN domainIndex)
{
    Temperature aux0 = getAuxTemperatureThreshold(domainIndex, 0);
    Temperature aux1 = getAuxTemperatureThreshold(domainIndex, 1);
    UIntN hysteresis = getHysteresis(domainIndex);
    return TemperatureThresholds(aux0, aux1, hysteresis);
}

void DomainTemperature_001::setTemperatureThresholds(UIntN participantIndex, UIntN domainIndex,
    const TemperatureThresholds& temperatureThresholds)
{
    try
    {
        Temperature aux0(temperatureThresholds.getAux0());
        if (aux0.isValid() == false)
        {
            aux0 = 5;
        }
        m_participantServicesInterface->primitiveExecuteSetAsTemperatureC(
            esif_primitive_type::SET_TEMPERATURE_THRESHOLDS, aux0, domainIndex, 0);
    }
    catch (...)
    {
        // eat any errors here
    }

    try
    {
        Temperature aux1(temperatureThresholds.getAux1());
        if (aux1.isValid() == false)
        {
            aux1 = 199;
        }
        m_participantServicesInterface->primitiveExecuteSetAsTemperatureC(
            esif_primitive_type::SET_TEMPERATURE_THRESHOLDS, aux1, domainIndex, 1);
    }
    catch (...)
    {
        // eat any errors here
    }
}

void DomainTemperature_001::clearCachedData(void)
{
    // Do nothing.  We don't cache temperature related data.
}

Temperature DomainTemperature_001::getAuxTemperatureThreshold(UIntN domainIndex, UInt8 auxNumber)
{
    try
    {
        return m_participantServicesInterface->primitiveExecuteGetAsTemperatureC(
            esif_primitive_type::GET_TEMPERATURE_THRESHOLDS, domainIndex, auxNumber);
    }
    catch (...)
    {
        return Temperature(0);
    }
}

UIntN DomainTemperature_001::getHysteresis(UIntN domainIndex)
{
    try
    {
        return static_cast<UIntN>(
            m_participantServicesInterface->primitiveExecuteGetAsUInt32(
            esif_primitive_type::GET_TEMPERATURE_THRESHOLD_HYSTERESIS,
            domainIndex));
    }
    catch (...)
    {
        return 0;
    }
}

XmlNode* DomainTemperature_001::getXml(UIntN domainIndex)
{
    XmlNode* root = XmlNode::createWrapperElement("temperature_control");

    root->addChild(getTemperatureStatus(Constants::Invalid, domainIndex).getXml());

    try
    {
        root->addChild(getTemperatureThresholds(Constants::Invalid, domainIndex).getXml());
    }
    catch (...)
    {
        //FIXME : There needs to be an interface change to decouple thresholds from temperature status.
    }

    return root;
}