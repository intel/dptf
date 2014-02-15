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

#include "DomainPowerControl_001.h"
#include "XmlNode.h"

DomainPowerControl_001::DomainPowerControl_001(ParticipantServicesInterface* participantServicesInterface) :
    m_participantServicesInterface(participantServicesInterface),
    m_powerControlDynamicCaps(nullptr), m_powerControlStatusSet(nullptr)
{
    m_canProgramPowerLimit[PowerControlType::pl1] = true;
    m_canProgramPowerLimit[PowerControlType::pl2] = true;
    m_canProgramTimeWindow[PowerControlType::pl1] = false;
    m_canProgramTimeWindow[PowerControlType::pl2] = false;
}

DomainPowerControl_001::~DomainPowerControl_001(void)
{
    DELETE_MEMORY_TC(m_powerControlStatusSet);
    DELETE_MEMORY_TC(m_powerControlDynamicCaps);
}

PowerControlDynamicCapsSet DomainPowerControl_001::getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex)
{
    initializePowerControlDynamicCapsSetIfNull(domainIndex);
    return *m_powerControlDynamicCaps;
}

PowerControlStatusSet DomainPowerControl_001::getPowerControlStatusSet(UIntN participantIndex, UIntN domainIndex)
{
    if (m_powerControlStatusSet == nullptr)
    {
        throw dptf_exception("No power limit has been set yet. Status unavailable.");
    }
    return *m_powerControlStatusSet;
}

void DomainPowerControl_001::setPowerControl(UIntN participantIndex, UIntN domainIndex, const PowerControlStatusSet& powerControlStatusSet)
{
    validatePowerControlStatus(powerControlStatusSet, domainIndex);
    programPowerControl(powerControlStatusSet, domainIndex);
    DELETE_MEMORY_TC(m_powerControlStatusSet);
    m_powerControlStatusSet = new PowerControlStatusSet(powerControlStatusSet);
}

void DomainPowerControl_001::validatePowerControlStatus(const PowerControlStatusSet& powerControlStatusSet, UIntN domainIndex)
{
    if (powerControlStatusSet.getCount() > PowerControlType::max)
    {
        throw dptf_exception("Too many power controls in the set!");
    }

    for (UIntN i = 0; i < powerControlStatusSet.getCount(); i++)
    {
        // Verify max was not passed in
        if (powerControlStatusSet[i].getPowerControlType() == PowerControlType::max)
        {
            throw dptf_exception("Invalid power control type.");
        }

        // Verify the desired power control is within the dynamic caps
        initializePowerControlDynamicCapsSetIfNull(domainIndex);
        if (powerControlStatusSet[i].getCurrentPowerLimit() >
            (*m_powerControlDynamicCaps)[powerControlStatusSet[i].getPowerControlType()].getMaxPowerLimit())
        {
            throw dptf_exception("Requested power limit too large.");
        }

        if (powerControlStatusSet[i].getCurrentPowerLimit() <
            (*m_powerControlDynamicCaps)[powerControlStatusSet[i].getPowerControlType()].getMinPowerLimit())
        {
            throw dptf_exception("Requested power limit too small.");
        }

        if (powerControlStatusSet[i].getCurrentTimeWindow() >
            (*m_powerControlDynamicCaps)[powerControlStatusSet[i].getPowerControlType()].getMaxTimeWindow())
        {
            throw dptf_exception("Requested time window is too large.");
        }

        if (powerControlStatusSet[i].getCurrentTimeWindow() <
            (*m_powerControlDynamicCaps)[powerControlStatusSet[i].getPowerControlType()].getMinTimeWindow())
        {
            throw dptf_exception("Requested time window is too small.");
        }
    }
}

void DomainPowerControl_001::programPowerControl(const PowerControlStatusSet& powerControlStatusSet, UIntN domainIndex)
{
    for (UIntN i = 0; i < powerControlStatusSet.getCount(); i++)
    {
        if (m_canProgramPowerLimit[powerControlStatusSet[i].getPowerControlType()] == true)
        {
            m_participantServicesInterface->primitiveExecuteSetAsPower(
                esif_primitive_type::SET_RAPL_POWER_LIMIT,
                powerControlStatusSet[i].getCurrentPowerLimit(),
                domainIndex,
                static_cast<UInt8>(powerControlStatusSet[i].getPowerControlType()));
            try
            {
                m_participantServicesInterface->primitiveExecuteSetAsUInt32(
                    esif_primitive_type::SET_RAPL_POWER_LIMIT_ENABLE,
                    1, //Enable the bit
                    domainIndex,
                    static_cast<UInt8>(powerControlStatusSet[i].getPowerControlType()));
            }
            catch (dptf_exception& ex)
            {
                // It is expected to hit this for the CPU's PL1 enable.  It is read only.
                ParticipantMessage message = ParticipantMessage(FLF, "Power limit enabled ignored.");
                message.setEsifPrimitive(esif_primitive_type::SET_RAPL_POWER_LIMIT_ENABLE, powerControlStatusSet[i].getPowerControlType());
                message.setExceptionCaught("primitiveExecuteSetAsUInt32", ex.what());
                m_participantServicesInterface->writeMessageWarning(message);
            }
        }
        else
        {
            std::stringstream msg;
            msg << PowerControlType::ToString(powerControlStatusSet[i].getPowerControlType()) << " power control is not programmable.  Ignoring.";
            m_participantServicesInterface->writeMessageDebug(ParticipantMessage(FLF, msg.str()));
        }

        if (m_canProgramTimeWindow[powerControlStatusSet[i].getPowerControlType()] == true)
        {
            m_participantServicesInterface->primitiveExecuteSetAsUInt32(
                esif_primitive_type::SET_RAPL_TIME_WINDOW,
                powerControlStatusSet[i].getCurrentTimeWindow(),
                domainIndex,
                static_cast<UInt8>(powerControlStatusSet[i].getPowerControlType()));
        }
        else
        {
            std::stringstream msg;
            msg << PowerControlType::ToString(powerControlStatusSet[i].getPowerControlType()) << " time window is not programmable.  Ignoring.";
            m_participantServicesInterface->writeMessageDebug(ParticipantMessage(FLF, msg.str()));
        }
    }
}

void DomainPowerControl_001::clearCachedData(void)
{
    DELETE_MEMORY_TC(m_powerControlDynamicCaps);
}

XmlNode* DomainPowerControl_001::getXml(UIntN domainIndex)
{
    initializePowerControlDynamicCapsSetIfNull(domainIndex);

    XmlNode* root = XmlNode::createWrapperElement("power_control");

    if (m_powerControlStatusSet != nullptr)
    {
        root->addChild(m_powerControlStatusSet->getXml());
    }

    root->addChild(m_powerControlDynamicCaps->getXml());
    root->addChild(XmlNode::createDataElement("control_knob_version", "001"));

    return root;
}

void DomainPowerControl_001::initializePowerControlDynamicCapsSetIfNull(UIntN domainIndex)
{
    if (m_powerControlDynamicCaps == nullptr)
    {
        UInt32 dataLength = 0;
        DptfMemory binaryData(Constants::DefaultBufferSize);
        m_participantServicesInterface->primitiveExecuteGet(
            esif_primitive_type::GET_RAPL_POWER_CONTROL_CAPABILITIES,
            ESIF_DATA_BINARY,
            binaryData,
            binaryData.getSize(),
            &dataLength,
            domainIndex);

        PowerControlDynamicCapsSet dynamicCapsSetFromControl(BinaryParse::processorPpccObject(dataLength, binaryData));
        m_powerControlDynamicCaps = new PowerControlDynamicCapsSet(getAdjustedDynamicCapsBasedOnConfigTdpMaxLimit(
            dynamicCapsSetFromControl));
        binaryData.deallocate();
        validatePowerControlDynamicCapsSet();
        determinePowerControlProgrammability();
    }
}

void DomainPowerControl_001::validatePowerControlDynamicCapsSet()
{
    if (m_powerControlDynamicCaps->getCount() == 0)
    {
        throw dptf_exception("Dynamic caps set is empty.  Impossible if we support power controls.");
    }

    for (UIntN i = 0; i < m_powerControlDynamicCaps->getCount(); i++)
    {
        std::stringstream msg;
        msg << "PL" << (*m_powerControlDynamicCaps)[i].getPowerControlType();
        if ((*m_powerControlDynamicCaps)[i].getMaxPowerLimit() < (*m_powerControlDynamicCaps)[i].getMinPowerLimit())
        {
            msg << " Bad power limit capabilities. Max is < min.";
            throw dptf_exception(msg.str());
        }

        if ((*m_powerControlDynamicCaps)[i].getMaxTimeWindow() < (*m_powerControlDynamicCaps)[i].getMinTimeWindow())
        {
            msg << " Bad time window capabilities. Max is < min.";
            throw dptf_exception(msg.str());
        }

        if ((*m_powerControlDynamicCaps)[i].getMaxDutyCycle() < (*m_powerControlDynamicCaps)[i].getMinDutyCycle())
        {
            msg << " Bad duty cycle capabilities. Max is < min.";
            throw dptf_exception(msg.str());
        }
    }
}

void DomainPowerControl_001::determinePowerControlProgrammability()
{
    for (UIntN i = 0; i < m_powerControlDynamicCaps->getCount(); i++)
    {
        if ((*m_powerControlDynamicCaps)[i].getMaxPowerLimit() == (*m_powerControlDynamicCaps)[i].getMinPowerLimit())
        {
            m_canProgramPowerLimit[(*m_powerControlDynamicCaps)[i].getPowerControlType()] = false;
        }

        if ((*m_powerControlDynamicCaps)[i].getMaxTimeWindow() == (*m_powerControlDynamicCaps)[i].getMinTimeWindow())
        {
            m_canProgramTimeWindow[(*m_powerControlDynamicCaps)[i].getPowerControlType()] = false;
        }
    }
}

PowerControlDynamicCapsSet DomainPowerControl_001::getAdjustedDynamicCapsBasedOnConfigTdpMaxLimit(
    const PowerControlDynamicCapsSet& capsSet)
{
    std::vector<PowerControlDynamicCaps> newCapsList;
    for (UIntN capsIndex = 0; capsIndex < capsSet.getCount(); capsIndex++)
    {
        Power maxPowerLimit(Power::createInvalid());
        if (capsSet[capsIndex].getPowerControlType() == PowerControlType::pl1)
        {
            maxPowerLimit = capsSet[capsIndex].getMaxPowerLimit();
        }
        else
        {
            maxPowerLimit = capsSet[capsIndex].getMaxPowerLimit();
        }
        newCapsList.push_back(PowerControlDynamicCaps(
            capsSet[capsIndex].getPowerControlType(), capsSet[capsIndex].getMinPowerLimit(), maxPowerLimit,
            capsSet[capsIndex].getPowerStepSize(), capsSet[capsIndex].getMinTimeWindow(),
            capsSet[capsIndex].getMaxTimeWindow(), capsSet[capsIndex].getMinDutyCycle(),
            capsSet[capsIndex].getMaxDutyCycle()));
    }
    return PowerControlDynamicCapsSet(newCapsList);
}
