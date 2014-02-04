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
#include "StatusFormat.h"

DomainTemperature_001::DomainTemperature_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface)
{
}

TemperatureStatus DomainTemperature_001::getTemperatureStatus(UIntN participantIndex, UIntN domainIndex)
{
    Temperature temperature = m_participantServicesInterface->primitiveExecuteGetAsTemperatureC(
        esif_primitive_type::GET_TEMPERATURE, domainIndex);

    return TemperatureStatus(temperature);
}

TemperatureThresholds DomainTemperature_001::getTemperatureThresholds(UIntN participantIndex, UIntN domainIndex)
{
    Temperature aux0 = m_participantServicesInterface->primitiveExecuteGetAsTemperatureC(
        esif_primitive_type::GET_TEMPERATURE_THRESHOLDS, domainIndex, 0);

    Temperature aux1 = m_participantServicesInterface->primitiveExecuteGetAsTemperatureC(
        esif_primitive_type::GET_TEMPERATURE_THRESHOLDS, domainIndex, 1);

    UIntN hysteresis =
        static_cast<UIntN>(
            m_participantServicesInterface->primitiveExecuteGetAsUInt32(
                esif_primitive_type::GET_TEMPERATURE_THRESHOLD_HYSTERESIS,
                domainIndex));

    return TemperatureThresholds(aux0, aux1, hysteresis);
}

void DomainTemperature_001::setTemperatureThresholds(UIntN participantIndex, UIntN domainIndex,
    const TemperatureThresholds& temperatureThresholds)
{
    Temperature aux0(temperatureThresholds.getAux0());
    if (aux0.isValid() == false)
    {
        aux0 = 5;
    }
    m_participantServicesInterface->primitiveExecuteSetAsTemperatureC(
        esif_primitive_type::SET_TEMPERATURE_THRESHOLDS, aux0, domainIndex, 0);

    Temperature aux1(temperatureThresholds.getAux1());
    if (aux1.isValid() == false)
    {
        aux1 = 199;
    }
    m_participantServicesInterface->primitiveExecuteSetAsTemperatureC(
        esif_primitive_type::SET_TEMPERATURE_THRESHOLDS, aux1, domainIndex, 1);
}

void DomainTemperature_001::clearCachedData(void)
{
    // Do nothing.  We don't cache temperature related data.
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