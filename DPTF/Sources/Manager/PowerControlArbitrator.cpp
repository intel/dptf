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

#include "PowerControlArbitrator.h"
#include "Utility.h"

PowerControlArbitrator::PowerControlArbitrator(DptfManager* dptfManager) :
    m_dptfManager(dptfManager),
    m_arbitratedPowerControlStatusSet(nullptr)
{
    std::vector<PowerControlStatus> emptyPowerControlStatusVector;
    m_arbitratedPowerControlStatusSet = new PowerControlStatusSet(emptyPowerControlStatusVector);
}

PowerControlArbitrator::~PowerControlArbitrator(void)
{
    DELETE_MEMORY_TC(m_arbitratedPowerControlStatusSet);

    for (UIntN i = 0; i < m_requestedPowerControlStatusSet.size(); i++)
    {
        DELETE_MEMORY_TC(m_requestedPowerControlStatusSet[i]);
    }
}

Bool PowerControlArbitrator::arbitrate(UIntN policyIndex, const PowerControlStatusSet& powerControlStatusSet)
{
    increaseVectorSizeIfNeeded(m_requestedPowerControlStatusSet, policyIndex);
    savePolicyRequest(policyIndex, powerControlStatusSet);
    std::vector<PowerControlStatus> arbitratedPowerControlStatusVector = createInitialArbitratedPowerControlStatusVector();
    arbitrate(arbitratedPowerControlStatusVector);
    PowerControlStatusSet arbitratedPowerControlStatusSet = getArbitratedPowerControlStatusSet(arbitratedPowerControlStatusVector);

    // see if the arbitrated PowerControlStatusSet is changing
    Bool arbitratedValueChanged = false;
    if (arbitratedPowerControlStatusSet != *m_arbitratedPowerControlStatusSet)
    {
        arbitratedValueChanged = true;
        DELETE_MEMORY_TC(m_arbitratedPowerControlStatusSet);
        m_arbitratedPowerControlStatusSet = new PowerControlStatusSet(arbitratedPowerControlStatusSet);
    }

    return arbitratedValueChanged;
}

PowerControlStatusSet PowerControlArbitrator::getArbitratedPowerControlStatusSet(void) const
{
    return *m_arbitratedPowerControlStatusSet;
}

void PowerControlArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
    if (policyIndex < m_requestedPowerControlStatusSet.size())
    {
        DELETE_MEMORY_TC(m_requestedPowerControlStatusSet[policyIndex]);
    }
}

void PowerControlArbitrator::savePolicyRequest(UIntN policyIndex, const PowerControlStatusSet& powerControlStatusSet)
{
    // save data for the specified policy
    DELETE_MEMORY_TC(m_requestedPowerControlStatusSet[policyIndex]);
    m_requestedPowerControlStatusSet[policyIndex] = new PowerControlStatusSet(powerControlStatusSet);
}

std::vector<PowerControlStatus> PowerControlArbitrator::createInitialArbitratedPowerControlStatusVector()
{
    // create a vector to temporarily store the arbitrated data.  initialize to invalid.
    std::vector<PowerControlStatus> arbitratedPowerControlStatusVector;
    for (UIntN i = 0; i < PowerControlType::max; i++)
    {
        PowerControlStatus pcs = PowerControlStatus((PowerControlType::Type)i, Power::createInvalid(), Constants::Invalid, Percentage::createInvalid());
        arbitratedPowerControlStatusVector.push_back(pcs);
    }

    return arbitratedPowerControlStatusVector;
}

void PowerControlArbitrator::arbitrate(std::vector<PowerControlStatus> &arbitratedPowerControlStatusVector)
{
    // find the new value for PL1, PL2, and PL3.  We choose the lowest power limit and lowest time window.
    for (UIntN i = 0; i < PowerControlType::max; i++)
    {
        PowerControlType::Type currentPowerControlType = (PowerControlType::Type)i;

        // loop through the request from each policy
        for (UIntN currentPolicy = 0; currentPolicy < m_requestedPowerControlStatusSet.size(); currentPolicy++)
        {
            if (m_requestedPowerControlStatusSet[currentPolicy] == nullptr)
            {
                continue;
            }

            PowerControlStatusSet& currentPowerControlStatusSet = *(m_requestedPowerControlStatusSet[currentPolicy]);

            // loop through the PowerControlStatusSet for this policy and locate the PowerControlType we are processing in this loop
            for (UIntN k = 0; k < currentPowerControlStatusSet.getCount(); k++)
            {
                PowerControlStatus currentPowerControlStatus = currentPowerControlStatusSet[k];

                if (currentPowerControlStatus.getPowerControlType() == currentPowerControlType)
                {
                    // Power
                    if ((currentPowerControlStatus.getCurrentPowerLimit().isValid() == true) &&
                        ((arbitratedPowerControlStatusVector[currentPowerControlType].getCurrentPowerLimit().isValid() == false) ||
                         (currentPowerControlStatus.getCurrentPowerLimit() < arbitratedPowerControlStatusVector[currentPowerControlType].getCurrentPowerLimit())))
                    {
                        arbitratedPowerControlStatusVector[currentPowerControlType].m_currentPowerLimit =
                            currentPowerControlStatus.getCurrentPowerLimit();
                        arbitratedPowerControlStatusVector[currentPowerControlType].m_currentDutyCycle =
                            currentPowerControlStatus.getCurrentDutyCycle();
                    }

                    // Time Window
                    if ((currentPowerControlStatus.getCurrentTimeWindow() != Constants::Invalid) &&
                        ((arbitratedPowerControlStatusVector[currentPowerControlType].getCurrentTimeWindow() == Constants::Invalid) ||
                         (currentPowerControlStatus.getCurrentTimeWindow() < arbitratedPowerControlStatusVector[currentPowerControlType].getCurrentTimeWindow())))
                    {
                        arbitratedPowerControlStatusVector[currentPowerControlType].m_currentTimeWindow =
                            currentPowerControlStatus.getCurrentTimeWindow();
                    }

                    break;
                }
            }
        }
    }
}

PowerControlStatusSet PowerControlArbitrator::getArbitratedPowerControlStatusSet(
    std::vector<PowerControlStatus>& arbitratedPowerControlStatusVector)
{
    // create a new vector that only contains the power control type's that have values
    std::vector<PowerControlStatus> reducedPowerControlStatusVector;
    for (UIntN i = 0; i < PowerControlType::max; i++)
    {
        PowerControlStatus pcs = arbitratedPowerControlStatusVector[i];
        if ((pcs.getCurrentPowerLimit().isValid() == true) ||
            (pcs.getCurrentTimeWindow() != Constants::Invalid))
        {
            reducedPowerControlStatusVector.push_back(pcs);
        }
    }

    PowerControlStatusSet arbitratedPowerControlStatusSet(reducedPowerControlStatusVector);

    return arbitratedPowerControlStatusSet;
}