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

#include "TemperatureThresholdArbitrator.h"
#include "ManagerMessage.h"
#include "DptfManager.h"
#include "EsifServices.h"
#include "Utility.h"

TemperatureThresholdArbitrator::TemperatureThresholdArbitrator(DptfManager* dptfManager) :
    m_dptfManager(dptfManager),
    m_lastKnownParticipantTemperature(Temperature::createInvalid()),
    m_arbitratedTemperatureThresholds(Temperature::createInvalid(), Temperature::createInvalid(), 0)
{
}

TemperatureThresholdArbitrator::~TemperatureThresholdArbitrator(void)
{
    for (UIntN i = 0; i < m_requestedTemperatureThresholds.size(); i++)
    {
        DELETE_MEMORY_TC(m_requestedTemperatureThresholds[i]);
    }
}

Bool TemperatureThresholdArbitrator::arbitrate(UIntN policyIndex, const TemperatureThresholds& temperatureThresholds,
    const Temperature& currentTemperature)
{
#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
    ManagerMessage message = ManagerMessage(m_dptfManager, FLF,
        "Temperature arbitration data is being updated for a policy request.");
    message.setPolicyIndex(policyIndex);
    message.addMessage("Current Temperature", currentTemperature);
    message.addMessage("Requested Aux0/Aux1", temperatureThresholds.getAux0().toString() + "/" + temperatureThresholds.getAux1().toString());
    addArbitrationDataToMessage(message, "Arbitration data before applying update");
#endif

    increaseVectorSizeIfNeeded(m_requestedTemperatureThresholds, policyIndex);
    throwIfTemperatureThresholdsInvalid(policyIndex, temperatureThresholds, currentTemperature);
    updateTemperatureDataForPolicy(policyIndex, temperatureThresholds);
    Bool result = findNewTemperatureThresholds(currentTemperature);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
    addArbitrationDataToMessage(message, "Arbitration data after applying update");
    m_dptfManager->getEsifServices()->writeMessageDebug(message, MessageCategory::TemperatureThresholds);
#endif

    return result;
}

TemperatureThresholds TemperatureThresholdArbitrator::getArbitratedTemperatureThresholds(void) const
{
    return m_arbitratedTemperatureThresholds;
}

void TemperatureThresholdArbitrator::clearPolicyCachedData(UIntN policyIndex)
{
    if (policyIndex < m_requestedTemperatureThresholds.size())
    {
        DELETE_MEMORY_TC(m_requestedTemperatureThresholds[policyIndex]);
    }
}

void TemperatureThresholdArbitrator::throwIfTemperatureThresholdsInvalid(UIntN policyIndex,
    const TemperatureThresholds& temperatureThresholds, const Temperature& currentTemperature)
{
    Temperature aux0 = temperatureThresholds.getAux0();
    Temperature aux1 = temperatureThresholds.getAux1();

    if ((aux0.isValid() == true && aux0 > currentTemperature) ||
        (aux1.isValid() == true && aux1 < currentTemperature))
    {
        ManagerMessage message = ManagerMessage(m_dptfManager, FLF,
            "Received invalid temperature thresholds from policy.");
        message.setPolicyIndex(policyIndex);
        message.addMessage("Current Temperature", currentTemperature);
        message.addMessage("Requested Aux0/Aux1", aux0.toString() + "/" + aux1.toString());
        m_dptfManager->getEsifServices()->writeMessageError(message, MessageCategory::TemperatureThresholds);
        throw dptf_exception(message);
    }
}

void TemperatureThresholdArbitrator::updateTemperatureDataForPolicy(UIntN policyIndex,
    const TemperatureThresholds& temperatureThresholds)
{
    if (m_requestedTemperatureThresholds[policyIndex] == nullptr)
    {
        m_requestedTemperatureThresholds[policyIndex] = new TemperatureThresholds(temperatureThresholds);
    }
    else
    {
        *(m_requestedTemperatureThresholds[policyIndex]) = temperatureThresholds;
    }
}

Bool TemperatureThresholdArbitrator::findNewTemperatureThresholds(const Temperature& currentTemperature)
{
    m_lastKnownParticipantTemperature = currentTemperature;

    Temperature newAux0 = Temperature::createInvalid();
    Temperature newAux1 = Temperature::createInvalid();

    for (UIntN i = 0; i < m_requestedTemperatureThresholds.size(); i++)
    {
        TemperatureThresholds* currentTemperatureThresholds = m_requestedTemperatureThresholds[i];

        if (currentTemperatureThresholds != nullptr)
        {
            Temperature currentAux0 = currentTemperatureThresholds->getAux0();
            Temperature currentAux1 = currentTemperatureThresholds->getAux1();

            // check for a new aux0
            if ((currentAux0.isValid() == true) &&
                (currentAux0 <= m_lastKnownParticipantTemperature) &&
                ((newAux0.isValid() == false) || (currentAux0 > newAux0)))
            {
                newAux0 = currentAux0;
            }

            // check for a new aux1
            if ((currentAux1.isValid() == true) &&
                (currentAux1 >= m_lastKnownParticipantTemperature) &&
                ((newAux1.isValid() == false) || (currentAux1 < newAux1)))
            {
                newAux1 = currentAux1;
            }
        }
    }

    // see if the aux trip points need to be updated
    if ((newAux0 != m_arbitratedTemperatureThresholds.getAux0()) ||
        (newAux1 != m_arbitratedTemperatureThresholds.getAux1()))
    {
        m_arbitratedTemperatureThresholds = TemperatureThresholds(newAux0, newAux1, 0);
        return true;
    }
    else
    {
        return false;
    }
}

void TemperatureThresholdArbitrator::addArbitrationDataToMessage(ManagerMessage& message, const std::string& title)
{
    message.addMessage(" ");
    message.addMessage(title);
    message.addMessage("Last known participant temperature", m_lastKnownParticipantTemperature);
    message.addMessage("Arbitrated Aux0/Aux1", m_arbitratedTemperatureThresholds.getAux0().toString() +
        "/" + m_arbitratedTemperatureThresholds.getAux1().toString());

    message.addMessage("--Requested temperature thresholds table contents--");
    for (UIntN i = 0; i < m_requestedTemperatureThresholds.size(); i++)
    {
        TemperatureThresholds* threshold = m_requestedTemperatureThresholds[i];
        if (threshold != nullptr)
        {
            message.addMessage("Policy " + std::to_string(i), threshold->getAux0().toString() +
                "/" + threshold->getAux1().toString());
        }
    }
}