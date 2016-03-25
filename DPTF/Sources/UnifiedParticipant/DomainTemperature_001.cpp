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

#include "DomainTemperature_001.h"
#include "XmlNode.h"

DomainTemperature_001::DomainTemperature_001(UIntN participantIndex, UIntN domainIndex, 
    ParticipantServicesInterface* participantServicesInterface) :
    DomainTemperatureBase(participantIndex, domainIndex, participantServicesInterface)
{
}

TemperatureStatus DomainTemperature_001::getTemperatureStatus(UIntN participantIndex, UIntN domainIndex)
{
    try
    {
        Temperature temperature = getParticipantServices()->primitiveExecuteGetAsTemperatureC(
            esif_primitive_type::GET_TEMPERATURE, domainIndex);
        return TemperatureStatus(temperature);
    }
    catch (primitive_destination_unavailable)
    {
        return TemperatureStatus(Temperature(0));
    }
    catch (dptf_exception& ex)
    {
        getParticipantServices()->writeMessageWarning(ParticipantMessage(FLF, ex.what()));

        // TODO: Let the policies handle the exceptions themselves and don't return a value.
        return TemperatureStatus(Temperature(0));
    }
}

TemperatureThresholds DomainTemperature_001::getTemperatureThresholds(UIntN participantIndex, UIntN domainIndex)
{
    Temperature aux0 = getAuxTemperatureThreshold(domainIndex, 0);
    Temperature aux1 = getAuxTemperatureThreshold(domainIndex, 1);
    Temperature hysteresis = getHysteresis(domainIndex);
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
        getParticipantServices()->primitiveExecuteSetAsTemperatureC(
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
        getParticipantServices()->primitiveExecuteSetAsTemperatureC(
            esif_primitive_type::SET_TEMPERATURE_THRESHOLDS, aux1, domainIndex, 1);
    }
    catch (...)
    {
        // eat any errors here
    }
}

DptfBuffer DomainTemperature_001::getCalibrationTable(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

DptfBuffer DomainTemperature_001::getPollingTable(UIntN participantIndex, UIntN domainIndex)
{
    throw not_implemented();
}

Bool DomainTemperature_001::isVirtualTemperature(UIntN participantIndex, UIntN domainIndex)
{
    return false;
}

void DomainTemperature_001::setVirtualTemperature(UIntN participantIndex, UIntN domainIndex, 
    const Temperature& temperature)
{
    throw not_implemented();
}

void DomainTemperature_001::clearCachedData(void)
{
    // Do nothing.  We don't cache temperature related data.
}

Temperature DomainTemperature_001::getAuxTemperatureThreshold(UIntN domainIndex, UInt8 auxNumber)
{
    try
    {
        return getParticipantServices()->primitiveExecuteGetAsTemperatureC(
            esif_primitive_type::GET_TEMPERATURE_THRESHOLDS, domainIndex, auxNumber);
    }
    catch (...)
    {
        return Temperature(0);
    }
}

Temperature DomainTemperature_001::getHysteresis(UIntN domainIndex)
{
    try
    {
        return getParticipantServices()->primitiveExecuteGetAsTemperatureC(
            esif_primitive_type::GET_TEMPERATURE_THRESHOLD_HYSTERESIS,
            domainIndex);
    }
    catch (...)
    {
        return Temperature(0);
    }
}

void DomainTemperature_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
    try
    {
        if (isActivityLoggingEnabled() == true)
        {
            TemperatureThresholds tempthreshold = getTemperatureThresholds(participantIndex, domainIndex);

            EsifCapabilityData capability;
            capability.type = Capability::TemperatureThreshold;
            capability.size = sizeof(capability);
            capability.data.temperatureControl.aux0 = tempthreshold.getAux0();
            capability.data.temperatureControl.aux1 = tempthreshold.getAux1();
            capability.data.temperatureControl.hysteresis = tempthreshold.getHysteresis();

            getParticipantServices()->sendDptfEvent(ParticipantEvent::DptfParticipantControlAction,
                domainIndex, Capability::getEsifDataFromCapabilityData(&capability));
        }
    }
    catch (...)
    {
        // skip if there are any issue in sending log data
    }
}

std::shared_ptr<XmlNode> DomainTemperature_001::getXml(UIntN domainIndex)
{
    auto root = XmlNode::createWrapperElement("temperature_control");
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    root->addChild(getTemperatureStatus(Constants::Invalid, domainIndex).getXml());
    root->addChild(getTemperatureThresholds(Constants::Invalid, domainIndex).getXml());

    return root;
}

std::string DomainTemperature_001::getName(void)
{
    return "Temperature Control (Version 1)";
}