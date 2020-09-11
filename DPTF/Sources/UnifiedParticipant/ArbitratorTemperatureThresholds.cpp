/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "ArbitratorTemperatureThresholds.h"
#include <StatusFormat.h>
#include "ParticipantLogger.h"

ArbitratorTemperatureThresholds::ArbitratorTemperatureThresholds(
	std::shared_ptr<ParticipantServicesInterface> participantServices)
	: m_participantServices(participantServices)
	, m_arbitratedValueChangedSinceLastSet(false)
	, m_arbitratedValue(TemperatureThresholds::createInvalid())
	, m_lastKnownParticipantTemperature(Temperature::createInvalid())
	, m_temperatureThresholdsPolicyRequests()
{
}

ArbitratorTemperatureThresholds::~ArbitratorTemperatureThresholds(void)
{
}

std::shared_ptr<ParticipantServicesInterface> ArbitratorTemperatureThresholds::getParticipantServices() const
{
	return m_participantServices;
}

TemperatureThresholds ArbitratorTemperatureThresholds::arbitrateTemperatureThresholdsRequest(
	UIntN policyIndex,
	const TemperatureThresholds& temperatureThresholds,
	const Temperature& currentTemperature,
	const Temperature hysteresis)
{
	throwIfTemperatureThresholdsInvalid(policyIndex, temperatureThresholds, currentTemperature, hysteresis);
	auto tempPolicyRequests = m_temperatureThresholdsPolicyRequests;
	tempPolicyRequests[policyIndex] = temperatureThresholds;

	return findNewTemperatureThresholds(tempPolicyRequests);
}

void ArbitratorTemperatureThresholds::commitTemperatureThresholdsRequest(
	UIntN policyIndex,
	const TemperatureThresholds& temperatureThresholds,
	const Temperature& currentTemperature,
	const Temperature hysteresis)
{
#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
	std::string message;
	message += "Temperature arbitration data is being updated for a policy request.\n";
	message += "Policy Index = " + std::to_string(policyIndex) + "\n";
	message += "Current Temperature = " + currentTemperature.toString() + "\n";
	message += "Requested Aux0/Aux1 = " + temperatureThresholds.getAux0().toString() + "/"
			   + temperatureThresholds.getAux1().toString() + "\n";
	message += getArbitrationDataMessage("Arbitration data before applying update");
#endif

	throwIfTemperatureThresholdsInvalid(policyIndex, temperatureThresholds, currentTemperature, hysteresis);
	m_lastKnownParticipantTemperature = currentTemperature;
	updateTemperatureThresholdsRequestAndArbitrate(policyIndex, temperatureThresholds);

#ifdef ONLY_LOG_TEMPERATURE_THRESHOLDS
	message += getArbitrationDataMessage("Arbitration data after applying update");
	// TODO: wanted to use MessageCategory::TemperatureThresholds as a function parameter
	PARTICIPANT_LOG_MESSAGE_DEBUG({ return message; });
#endif
}

void ArbitratorTemperatureThresholds::removeTemperatureThresholdsRequest(
	UIntN policyIndex,
	const Temperature& currentTemperature)
{
	auto policyRequest = m_temperatureThresholdsPolicyRequests.find(policyIndex);
	if (policyRequest != m_temperatureThresholdsPolicyRequests.end())
	{
		commitTemperatureThresholdsRequest(
			policyIndex, TemperatureThresholds::createInvalid(), currentTemperature, Temperature::createInvalid());
	}
	else
	{
		m_arbitratedValueChangedSinceLastSet = false;
	}
}

Bool ArbitratorTemperatureThresholds::arbitratedTemperatureThresholdsChangedSinceLastSet() const
{
	return m_arbitratedValueChangedSinceLastSet;
}

TemperatureThresholds ArbitratorTemperatureThresholds::getArbitratedTemperatureThresholds(void) const
{
	return m_arbitratedValue;
}

std::shared_ptr<XmlNode> ArbitratorTemperatureThresholds::getArbitrationXmlForPolicy(UIntN policyIndex) const
{
	auto requestRoot = XmlNode::createWrapperElement("temperature_thresholds_arbitrator_status");
	TemperatureThresholds temperatureThresholds = TemperatureThresholds::createInvalid();
	auto policyRequest = m_temperatureThresholdsPolicyRequests.find(policyIndex);
	if (policyRequest != m_temperatureThresholdsPolicyRequests.end())
	{
		temperatureThresholds = policyRequest->second;
	}
	requestRoot->addChild(temperatureThresholds.getXml());

	return requestRoot;
}

void ArbitratorTemperatureThresholds::updateTemperatureThresholdsRequestAndArbitrate(
	UIntN policyIndex,
	const TemperatureThresholds& temperatureThresholds)
{
	m_temperatureThresholdsPolicyRequests[policyIndex] = temperatureThresholds;
	auto newArbitratedValue = findNewTemperatureThresholds(m_temperatureThresholdsPolicyRequests);

	// see if the aux trip points need to be updated
	auto newAux0 = newArbitratedValue.getAux0();
	auto newAux1 = newArbitratedValue.getAux1();
	if ((newAux0 != m_arbitratedValue.getAux0()) || (newAux1 != m_arbitratedValue.getAux1()))
	{
		m_arbitratedValue = newArbitratedValue;
		m_arbitratedValueChangedSinceLastSet = true;
	}
	else
	{
		m_arbitratedValueChangedSinceLastSet = false;
	}
}

TemperatureThresholds ArbitratorTemperatureThresholds::findNewTemperatureThresholds(
	std::map<UIntN, TemperatureThresholds> temperatureThresholdsPolicyRequests)
{
	auto newAux0 = Temperature::createInvalid();
	auto newAux1 = Temperature::createInvalid();

	for (auto thresholds = temperatureThresholdsPolicyRequests.begin();
		 thresholds != temperatureThresholdsPolicyRequests.end();
		 thresholds++)
	{
		auto currentTemperatureThresholds = thresholds->second;
		auto currentAux0 = currentTemperatureThresholds.getAux0();
		auto currentAux1 = currentTemperatureThresholds.getAux1();

		// check for a new aux0
		if ((currentAux0.isValid() == true) && ((newAux0.isValid() == false) || (currentAux0 > newAux0)))
		{
			newAux0 = currentAux0;
		}

		// check for a new aux1
		if ((currentAux1.isValid() == true) && ((newAux1.isValid() == false) || (currentAux1 < newAux1)))
		{
			newAux1 = currentAux1;
		}
	}

	if (newAux0.isValid())
	{
		newAux0 = Temperature::snapWithinAllowableTripPointRange(newAux0);
	}
	else
	{
		newAux0 = Temperature::fromCelsius(ESIF_SDK_MIN_AUX_TRIP);
	}

	if (newAux1.isValid())
	{
		newAux1 = Temperature::snapWithinAllowableTripPointRange(newAux1);
	}
	else
	{
		newAux1 = Temperature::fromCelsius(ESIF_SDK_MAX_AUX_TRIP);
	}

	return TemperatureThresholds(newAux0, newAux1, Temperature::createInvalid());
}

void ArbitratorTemperatureThresholds::throwIfTemperatureThresholdsInvalid(
	UIntN policyIndex,
	const TemperatureThresholds& temperatureThresholds,
	const Temperature& currentTemperature,
	const Temperature hysteresis)
{
	Temperature aux0 = temperatureThresholds.getAux0();
	Temperature aux1 = temperatureThresholds.getAux1();

	if ((aux0.isValid() == true && aux0 - hysteresis - Temperature(2742) > currentTemperature)
		|| (aux1.isValid() == true && aux1 < currentTemperature))
	{
		std::string message;
		message += "Received invalid temperature thresholds from policy.\n";
		message += "Policy Index = " + std::to_string(policyIndex) + "\n";
		message += "Current Temperature = " + currentTemperature.toString() + "\n";
		message += "Requested Aux0/Aux1 = " + aux0.toString() + "/" + aux1.toString() + "\n";
		message += "Current Hysteresis = " + hysteresis.toString() + "\n";
		// TODO: wanted to use MessageCategory::TemperatureThresholds as a function parameter
		PARTICIPANT_LOG_MESSAGE_ERROR({ return message; });
		throw dptf_exception(message);
	}
}

std::string ArbitratorTemperatureThresholds::getArbitrationDataMessage(const std::string& title)
{
	std::string message;
	message += title + "\n";
	message += "Last known participant temperature = " + m_lastKnownParticipantTemperature.toString() + "\n";
	message += "Arbitrated Aux0/Aux1 = " + m_arbitratedValue.getAux0().toString() + "/"
			   + m_arbitratedValue.getAux1().toString() + "\n";
	message += "--Requested temperature thresholds table contents--\n";

	for (auto thresholds = m_temperatureThresholdsPolicyRequests.begin();
		 thresholds != m_temperatureThresholdsPolicyRequests.end();
		 thresholds++)
	{
		message += "Policy " + std::to_string(thresholds->first) + " = " + thresholds->second.getAux0().toString() + "/"
				   + thresholds->second.getAux1().toString() + "\n";
	}

	return message;
}
