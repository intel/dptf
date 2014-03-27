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

void PowerControlKnob::limit(UIntN target)
{
    if (canLimit(target))
    {
        try
        {
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, "Calculating request to limit power controls.", 
                getParticipantIndex(), getDomainIndex()));

            UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
            const PowerControlDynamicCaps& pl1Capabilities = m_powerControl->getCapabilities()[pl1Index];
            Power minimumPowerLimit = pl1Capabilities.getMinPowerLimit();
            Power stepSize = pl1Capabilities.getPowerStepSize();

            Power nextPowerLimit(pl1Capabilities.getMaxPowerLimit());
            Power targetRequest = getTargetRequest(target);
            if (targetRequest >= pl1Capabilities.getMaxPowerLimit())
            {
                // limit one step from current power
                const PowerStatus& currentPowerStatus = m_powerControl->getCurrentPower();
                Power currentPower = currentPowerStatus.getCurrentPower();
                getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
                    FLF, "Current power is " + currentPower.toString() + ".", 
                    getParticipantIndex(), getDomainIndex()));
                nextPowerLimit = calculateNextLowerPowerLimit(
                    currentPower, minimumPowerLimit, stepSize, targetRequest);
            }
            else
            {
                // limit one step size down
                nextPowerLimit = std::max((int)targetRequest - (int)stepSize, (int)minimumPowerLimit);
            }
            m_requests[target] = nextPowerLimit;

            stringstream message;
            message << "Requesting to limit power to " << nextPowerLimit.toString() << ".";
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, message.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

void PowerControlKnob::unlimit(UIntN target)
{
    if (canUnlimit(target))
    {
        try
        {
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, "Calculating request to unlimit power controls.", 
                getParticipantIndex(), getDomainIndex()));

            UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
            const PowerControlDynamicCaps& pl1Capabilities = m_powerControl->getCapabilities()[pl1Index];

            Power lastRequest = getTargetRequest(target);
            Power nextPowerAfterStep = lastRequest + pl1Capabilities.getPowerStepSize();
            Power nextPowerLimit(std::min(nextPowerAfterStep, pl1Capabilities.getMaxPowerLimit()));
            m_requests[target] = nextPowerLimit;

            stringstream message;
            message << "Requesting to unlimit power to " << nextPowerLimit.toString() << ".";
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, message.str(), getParticipantIndex(), getDomainIndex()));
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
            throw ex;
        }
    }
}

Bool PowerControlKnob::canLimit(UIntN target)
{
    try
    {
        if (m_powerControl->supportsPowerControls())
        {
            UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
            const PowerControlDynamicCaps& pl1Capabilities =
                m_powerControl->getCapabilities()[pl1Index];
            Power currentStatus = getTargetRequest(target);
            return (currentStatus > pl1Capabilities.getMinPowerLimit());
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

Bool PowerControlKnob::canUnlimit(UIntN target)
{
    try
    {
        if (m_powerControl->supportsPowerControls())
        {
            UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
            const PowerControlDynamicCaps& pl1Capabilities =
                m_powerControl->getCapabilities()[pl1Index];
            Power currentStatus = getTargetRequest(target);
            return (currentStatus < pl1Capabilities.getMaxPowerLimit());
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

Bool PowerControlKnob::commitSetting()
{
    try
    {
        if (m_powerControl->supportsPowerControls())
        {
            // find lowest power limit in request
            Power lowestPowerLimit = snapToCapabilitiesBounds(findLowestPowerLimitRequest(m_requests));

            // set new power status
            UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
            PowerControlStatus currentControlStatus = m_powerControl->getLastIssuedPowerLimit();
            if (currentControlStatus.getCurrentPowerLimit() != lowestPowerLimit)
            {
                stringstream messageBefore;
                messageBefore << "Attempting to change power limit to " << lowestPowerLimit.toString() << ".";
                getPolicyServices().messageLogging->writeMessageDebug(
                    PolicyMessage(FLF, messageBefore.str(), getParticipantIndex(), getDomainIndex()));

                PowerControlStatus newStatus(
                    currentControlStatus.getPowerControlType(),
                    lowestPowerLimit,
                    currentControlStatus.getCurrentTimeWindow(),
                    currentControlStatus.getCurrentDutyCycle());
                m_powerControl->setControl(newStatus, pl1Index);

                stringstream messageAfter;
                messageAfter << "Changed power limit to " << newStatus.getCurrentPowerLimit().toString() << ".";
                getPolicyServices().messageLogging->writeMessageDebug(
                    PolicyMessage(FLF, messageAfter.str(), getParticipantIndex(), getDomainIndex()));
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    catch (std::exception& ex)
    {
        getPolicyServices().messageLogging->writeMessageDebug(
            PolicyMessage(FLF, ex.what(), getParticipantIndex(), getDomainIndex()));
        throw ex;
    }
}

Power PowerControlKnob::findLowestPowerLimitRequest(const std::map<UIntN, Power>& m_requests)
{
    if (m_requests.size() == 0)
    {
        return m_powerControl->getLastIssuedPowerLimit().getCurrentPowerLimit();
    }
    else
    {
        Power lowestPower;
        for (auto request = m_requests.begin(); request != m_requests.end(); request++)
        {
            if (lowestPower.isValid() == false)
            {
                lowestPower = request->second;
            }
            else
            {
                if (request->second < lowestPower)
                {
                    lowestPower = request->second;
                }
            }
        }
        return lowestPower;
    }
}

Power PowerControlKnob::getTargetRequest(UIntN target)
{
    UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
    const PowerControlDynamicCaps& pl1Capabilities = m_powerControl->getCapabilities()[pl1Index];
    auto targetRequest = m_requests.find(target);
    if (targetRequest == m_requests.end())
    {
        return pl1Capabilities.getMaxPowerLimit();
    }
    else
    {
        return targetRequest->second;
    }
}

UIntN PowerControlKnob::snapToCapabilitiesBounds(Power powerLimit)
{
    UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
    const PowerControlDynamicCaps& pl1Capabilities = m_powerControl->getCapabilities()[pl1Index];
    Power maxPowerLimit = pl1Capabilities.getMaxPowerLimit();
    Power minPowerLimit = pl1Capabilities.getMinPowerLimit();
    if (powerLimit > maxPowerLimit)
    {
        powerLimit = maxPowerLimit;
    }
    if (powerLimit < minPowerLimit)
    {
        powerLimit = minPowerLimit;
    }
    return powerLimit;
}

void PowerControlKnob::clearRequestForTarget(UIntN target)
{
    auto targetRequest = m_requests.find(target);
    if (targetRequest != m_requests.end())
    {
        m_requests.erase(targetRequest);
    }
}

void PowerControlKnob::clearAllRequests()
{
    m_requests.clear();
}

XmlNode* PowerControlKnob::getXml() const
{
    XmlNode* knobStatus = XmlNode::createWrapperElement("power_control_status");
    if (m_powerControl->supportsPowerControls())
    {
        UIntN pl1Index = m_powerControl->getPl1ControlSetIndex();
        PowerControlDynamicCaps pl1Capabilities = m_powerControl->getCapabilities()[pl1Index];
        knobStatus->addChild(pl1Capabilities.getXml());
        PowerControlStatus currentControlStatus = m_powerControl->getLastIssuedPowerLimit();
        knobStatus->addChild(currentControlStatus.getXml());
    }
    return knobStatus;
}