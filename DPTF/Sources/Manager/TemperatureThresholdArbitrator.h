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

#pragma once

#include "Dptf.h"
#include "TemperatureThresholds.h"
#include "ManagerMessage.h"

class DptfManager;

//
// Arbitration Rule:
//
// For aux 0, we choose the temperature that is <= to the actual temperature.
// for aux 1, we choose the temperature that is >= to the actual temperature.
//

class TemperatureThresholdArbitrator
{
public:

    TemperatureThresholdArbitrator(DptfManager* dptfManager);
    ~TemperatureThresholdArbitrator(void);

    // arbitrate() returns true if the arbitrated value has changed
    Bool arbitrate(UIntN policyIndex, const TemperatureThresholds& temperatureThresholds, const Temperature& currentTemperature);

    TemperatureThresholds getArbitratedTemperatureThresholds(void) const;
    void clearPolicyCachedData(UIntN policyIndex);

private:

    // hide the copy constructor and assignment operator.
    TemperatureThresholdArbitrator(const TemperatureThresholdArbitrator& rhs);
    TemperatureThresholdArbitrator& operator=(const TemperatureThresholdArbitrator& rhs);

    DptfManager* m_dptfManager;

    Temperature m_lastKnownParticipantTemperature;
    TemperatureThresholds m_arbitratedTemperatureThresholds;

    std::vector<TemperatureThresholds*> m_requestedTemperatureThresholds;

    void throwIfTemperatureThresholdsInvalid(UIntN policyIndex, const TemperatureThresholds& temperatureThresholds,
        const Temperature& currentTemperature);
    void updateTemperatureDataForPolicy(UIntN policyIndex, const TemperatureThresholds& temperatureThresholds);
    Bool findNewTemperatureThresholds(const Temperature& currentTemperature);

    void addArbitrationDataToMessage(ManagerMessage& message, const std::string& title);
};