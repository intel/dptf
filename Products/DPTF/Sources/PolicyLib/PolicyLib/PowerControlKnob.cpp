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

#include "PowerControlKnob.h"
#include <algorithm>

using namespace std;

PowerControlKnob::PowerControlKnob(
    const PolicyServicesInterfaceContainer& policyServices,
    std::shared_ptr<PowerControlFacade> powerControl,
    UIntN participantIndex,
    UIntN domainIndex)
    : ControlKnobBase(policyServices, participantIndex, domainIndex),
    m_powerControl(powerControl)
{
}

PowerControlKnob::~PowerControlKnob(void)
{
}

void PowerControlKnob::limit()
{
    if (canLimit())
    {
        try
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Attempting to limit power.", getParticipantIndex(), getDomainIndex()));

            // get current power
            m_powerControl->refreshStatus();
            const PowerStatus& currentPowerStatus = m_powerControl->getStatus();
            Power currentPower = currentPowerStatus.getCurrentPower();
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
                FLF, "Current power is " + currentPower.toString() + ".", getParticipantIndex(), getDomainIndex()));

            // get current power limit
            UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
            const PowerControlStatus& currentControlStatus = m_powerControl->getControls()[pl1Index];
            Power currentPowerLimit = currentControlStatus.getCurrentPowerLimit();

            // get min power limit and step size
            const PowerControlDynamicCaps& pl1Capabilities = m_powerControl->getCapabilities()[pl1Index];
            Power minimumPowerLimit = pl1Capabilities.getMinPowerLimit();
            Power stepSize = pl1Capabilities.getPowerStepSize();

            // calculate next power limit
            Power nextPowerLimit = calculateNextLowerPowerLimit(
                currentPower, minimumPowerLimit, stepSize, currentPowerLimit);

            // set new power status
            PowerControlStatus newStatus(
                currentControlStatus.getPowerControlType(),
                nextPowerLimit,
                currentControlStatus.getCurrentTimeWindow(),
                currentControlStatus.getCurrentDutyCycle());
            m_powerControl->setControl(newStatus, pl1Index);

            stringstream message;
            message << "Limited power to " << newStatus.getCurrentPowerLimit().toString() << ".";
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, message.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

void PowerControlKnob::unlimit()
{
    if (canUnlimit())
    {
        try
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Attempting to unlimit power.", getParticipantIndex(), getDomainIndex()));

            UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
            const PowerControlDynamicCaps& pl1Capabilities = m_powerControl->getCapabilities()[pl1Index];
            const PowerControlStatus& currentStatus = m_powerControl->getControls()[pl1Index];
            Power nextPowerAfterStep = currentStatus.getCurrentPowerLimit() + pl1Capabilities.getPowerStepSize();
            Power nextPowerLimit(std::min(nextPowerAfterStep, pl1Capabilities.getMaxPowerLimit()));
            PowerControlStatus newStatus(
                currentStatus.getPowerControlType(),
                std::min(nextPowerLimit, pl1Capabilities.getMaxPowerLimit()),
                currentStatus.getCurrentTimeWindow(),
                currentStatus.getCurrentDutyCycle());
            m_powerControl->setControl(newStatus, pl1Index);

            stringstream message;
            message << "Unlimited power to " << newStatus.getCurrentPowerLimit().toString() << ".";
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, message.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

Bool PowerControlKnob::canLimit()
{
    try
    {
        if (m_powerControl->supportsPowerControls())
        {
            UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
            const PowerControlDynamicCaps& pl1Capabilities =
                m_powerControl->getCapabilities()[pl1Index];
            const PowerControlStatus& currentStatus = m_powerControl->getControls()[pl1Index];
            return (currentStatus.getCurrentPowerLimit() > pl1Capabilities.getMinPowerLimit());
        }
        else
        {
            return false;
        }
    }
    catch (...)
    {
        return false;
    }
}

Bool PowerControlKnob::canUnlimit()
{
    try
    {
        if (m_powerControl->supportsPowerControls())
        {
            UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
            const PowerControlDynamicCaps& pl1Capabilities =
                m_powerControl->getCapabilities()[pl1Index];
            const PowerControlStatus& currentStatus = m_powerControl->getControls()[pl1Index];
            return (currentStatus.getCurrentPowerLimit() < pl1Capabilities.getMaxPowerLimit());
        }
        else
        {
            return false;
        }
    }
    catch (...)
    {
        return false;
    }
}

Power PowerControlKnob::calculateNextLowerPowerLimit(
    Power currentPower, Power minimumPowerLimit, Power stepSize, Power currentPowerLimit)
{
    Power nextPowerLimit(Power::createInvalid());
    if (currentPower > minimumPowerLimit)
    {
        if (stepSize > currentPower)
        {
            nextPowerLimit = minimumPowerLimit;
        }
        else
        {
            Power leastPowerValue = std::min(currentPowerLimit - stepSize, currentPower - stepSize);
            nextPowerLimit = std::max(minimumPowerLimit, leastPowerValue);
        }
    }
    else
    {
        nextPowerLimit = minimumPowerLimit;
    }
    return nextPowerLimit;
}

XmlNode* PowerControlKnob::getXml() const
{
    XmlNode* knobStatus = XmlNode::createWrapperElement("power_control_status");
    if (m_powerControl->supportsPowerControls())
    {
        UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
        PowerControlDynamicCaps pl1Capabilities = m_powerControl->getCapabilities()[pl1Index];
        knobStatus->addChild(pl1Capabilities.getXml());
        PowerControlStatus currentControlStatus = m_powerControl->getControls()[pl1Index];
        knobStatus->addChild(currentControlStatus.getXml());
    }
    return knobStatus;
}