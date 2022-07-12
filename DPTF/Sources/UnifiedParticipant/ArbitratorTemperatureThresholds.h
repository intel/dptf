/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#pragma once

#include "TemperatureThresholds.h"
#include "ParticipantServicesInterface.h"
#include <XmlNode.h>

//
// Arbitration Rule:
//
// For aux 0, we choose the temperature that is <= to the actual temperature plus hysteresis.
// for aux 1, we choose the temperature that is >= to the actual temperature.
//

class dptf_export ArbitratorTemperatureThresholds
{
public:
	ArbitratorTemperatureThresholds(std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~ArbitratorTemperatureThresholds(void);

	TemperatureThresholds arbitrateTemperatureThresholdsRequest(
		UIntN policyIndex,
		const TemperatureThresholds& temperatureThresholds,
		const Temperature& currentTemperature,
		const Temperature hysteresis);
	void commitTemperatureThresholdsRequest(
		UIntN policyIndex,
		const TemperatureThresholds& temperatureThresholds,
		const Temperature& currentTemperature,
		const Temperature hysteresis);
	void removeTemperatureThresholdsRequest(UIntN policyIndex, const Temperature& currentTemperature);
	Bool arbitratedTemperatureThresholdsChangedSinceLastSet() const;
	TemperatureThresholds getArbitratedTemperatureThresholds(void) const;
	std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex) const;

protected:
	std::shared_ptr<ParticipantServicesInterface> getParticipantServices() const;

private:
	std::shared_ptr<ParticipantServicesInterface> m_participantServices;

	Bool m_arbitratedValueChangedSinceLastSet;
	TemperatureThresholds m_arbitratedValue;
	Temperature m_lastKnownParticipantTemperature;
	std::map<UIntN, TemperatureThresholds> m_temperatureThresholdsPolicyRequests;

	void updateTemperatureThresholdsRequestAndArbitrate(
		UIntN policyIndex,
		const TemperatureThresholds& temperatureThresholds);
	TemperatureThresholds findNewTemperatureThresholds(
		std::map<UIntN, TemperatureThresholds> temperatureThresholdsPolicyRequests);

	void throwIfTemperatureThresholdsInvalid(
		UIntN policyIndex,
		const TemperatureThresholds& temperatureThresholds,
		const Temperature& currentTemperature,
		const Temperature hysteresis);

	std::string getArbitrationDataMessage(const std::string& title);
};
