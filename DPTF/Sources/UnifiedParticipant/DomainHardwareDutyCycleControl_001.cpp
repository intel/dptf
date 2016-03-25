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

#include "DomainHardwareDutyCycleControl_001.h"
#include "StatusFormat.h"
#include "DptfBuffer.h"
#include "XmlNode.h"

DomainHardwareDutyCycleControl_001::DomainHardwareDutyCycleControl_001(UIntN participantIndex, UIntN domainIndex,
    ParticipantServicesInterface* participantServicesInterface) : 
    DomainHardwareDutyCycleControlBase(participantIndex, domainIndex, participantServicesInterface)
{
    
}

void DomainHardwareDutyCycleControl_001::clearCachedData(void)
{
    
}

std::shared_ptr<XmlNode> DomainHardwareDutyCycleControl_001::getXml(UIntN domainIndex)
{
    auto root = XmlNode::createWrapperElement("hardware_duty_cycle_control");
    root->addChild(XmlNode::createDataElement("hardware_duty_cycle", 
        getHardwareDutyCycle(Constants::Invalid, domainIndex).toString()));
    root->addChild(XmlNode::createDataElement("is_supported_by_platform", 
        StatusFormat::friendlyValue(isSupportedByPlatform(Constants::Invalid, domainIndex))));
    root->addChild(XmlNode::createDataElement("is_supported_by_operating_system",
        StatusFormat::friendlyValue(isSupportedByOperatingSystem(Constants::Invalid, domainIndex))));
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));
    return root;
}

DptfBuffer DomainHardwareDutyCycleControl_001::getHardwareDutyCycleUtilizationSet(
    UIntN participantIndex, UIntN domainIndex) const
{
    DptfBuffer buffer = getParticipantServices()->primitiveExecuteGet(
        GET_PROC_PER_CORE_UTILIZATION, ESIF_DATA_BINARY, domainIndex);
    return buffer;
}

Bool DomainHardwareDutyCycleControl_001::isEnabledByPlatform(UIntN participantIndex, UIntN domainIndex) const
{
    UInt64 enabledByPlatform = getParticipantServices()->primitiveExecuteGetAsUInt64(
        esif_primitive_type::GET_PROC_HDC_ENABLE, domainIndex);
    return enabledByPlatform == 0 ? false : true;
}

Bool DomainHardwareDutyCycleControl_001::isSupportedByPlatform(UIntN participantIndex, UIntN domainIndex) const
{
    UInt64 supportedByPlatform = getParticipantServices()->primitiveExecuteGetAsUInt64(
        esif_primitive_type::GET_PROC_HDC_SUPPORT_CHECK, domainIndex);
    return supportedByPlatform == 0 ? false : true;
}

Bool DomainHardwareDutyCycleControl_001::isEnabledByOperatingSystem(UIntN participantIndex, UIntN domainIndex) const
{
    UInt64 enabledByOperatingSystem = getParticipantServices()->primitiveExecuteGetAsUInt64(
        esif_primitive_type::GET_PROC_HWP_ENABLE, domainIndex);
    return enabledByOperatingSystem == 0 ? false : true;
}

Bool DomainHardwareDutyCycleControl_001::isSupportedByOperatingSystem(UIntN participantIndex, UIntN domainIndex) const
{
    UInt64 supportedByOperatingSystem = getParticipantServices()->primitiveExecuteGetAsUInt64(
        esif_primitive_type::GET_PROC_HWP_SUPPORT_CHECK, domainIndex);
    return supportedByOperatingSystem == 0 ? false : true;
}

Bool DomainHardwareDutyCycleControl_001::isHdcOobEnabled(UIntN participantIndex, UIntN domainIndex) const
{
    UInt64 hdcOobEnabled = getParticipantServices()->primitiveExecuteGetAsUInt64(
        esif_primitive_type::GET_PROC_HDC_OOB_ENABLE, domainIndex);
    return hdcOobEnabled == 0 ? false : true;
}

void DomainHardwareDutyCycleControl_001::setHdcOobEnable(
    UIntN participantIndex, UIntN domainIndex, const UInt8& hdcOobEnable)
{
    getParticipantServices()->primitiveExecuteSetAsUInt8(
        esif_primitive_type::SET_PROC_HDC_OOB_ENABLE,
        hdcOobEnable,
        domainIndex);
}

void DomainHardwareDutyCycleControl_001::setHardwareDutyCycle(
    UIntN participantIndex, UIntN domainIndex, const Percentage& dutyCycle)
{
    getParticipantServices()->primitiveExecuteSetAsPercentage(
        esif_primitive_type::SET_PROC_HDC_DUTY_CYCLE,
        dutyCycle,
        domainIndex);
}

Percentage DomainHardwareDutyCycleControl_001::getHardwareDutyCycle(UIntN participantIndex, UIntN domainIndex) const
{
    Percentage dutyCycle = getParticipantServices()->primitiveExecuteGetAsPercentage(
        esif_primitive_type::GET_PROC_HDC_DUTY_CYCLE, domainIndex);
    return dutyCycle;
}

void DomainHardwareDutyCycleControl_001::sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex)
{
    try
    {
        if (isActivityLoggingEnabled() == true) 
        {
            EsifCapabilityData capability;
            capability.type = Capability::HdcControl;
            capability.size = sizeof(capability);

            getParticipantServices()->sendDptfEvent(ParticipantEvent::DptfParticipantControlAction,
                domainIndex, Capability::getEsifDataFromCapabilityData(&capability));
        }
    }
    catch (...)
    {
        // skip if there are any issue in sending log data
    }
}

std::string DomainHardwareDutyCycleControl_001::getName(void)
{
    return "Hardware Duty Cycle Control (Version 1)";
}