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

DomainTemperature_001::~DomainTemperature_001()
{
    clearCachedData();
}

TemperatureStatus DomainTemperature_001::getTemperatureStatus(UIntN participantIndex, UIntN domainIndex)
{
    try
    {
        Temperature temperature = getParticipantServices()->primitiveExecuteGetAsTemperatureTenthK(
            esif_primitive_type::GET_TEMPERATURE, domainIndex);
        return TemperatureStatus(temperature);
    }
    catch (primitive_destination_unavailable)
    {
        return TemperatureStatus(Temperature::minValidTemperature);
    }
    catch (dptf_exception& ex)
    {
        getParticipantServices()->writeMessageWarning(ParticipantMessage(FLF, ex.what()));

        // TODO: Let the policies handle the exceptions themselves and don't return a value.
        return TemperatureStatus(Temperature::minValidTemperature);
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
}

std::shared_ptr<XmlNode> DomainTemperature_001::getXml(UIntN domainIndex)
{
    auto root = XmlNode::createWrapperElement("temperature_control");
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    root->addChild(getTemperatureStatus(getParticipantIndex(), domainIndex).getXml());
    root->addChild(getTemperatureThresholds(getParticipantIndex(), domainIndex).getXml());

    return root;
}

std::string DomainTemperature_001::getName(void)
{
    return "Temperature Control (Version 1)";
}