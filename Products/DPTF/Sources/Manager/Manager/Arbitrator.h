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

#include "ActiveControlArbitrator.h"
#include "ConfigTdpControlArbitrator.h"
#include "CoreControlArbitrator.h"
#include "DisplayControlArbitrator.h"
#include "PerformanceControlArbitrator.h"
#include "PowerControlArbitrator.h"
#include "TemperatureThresholdArbitrator.h"

class DptfManager;

class Arbitrator
{
public:

    Arbitrator(DptfManager* dptfManager);
    ~Arbitrator(void);

    void clearPolicyCachedData(UIntN policyIndex);

    ActiveControlArbitrator* getActiveControlArbitrator(void) const;
    ConfigTdpControlArbitrator* getConfigTdpControlArbitrator(void) const;
    CoreControlArbitrator* getCoreControlArbitrator(void) const;
    DisplayControlArbitrator* getDisplayControlArbitrator(void) const;
    PerformanceControlArbitrator* getPerformanceControlArbitrator(void) const;
    PowerControlArbitrator* getPowerControlArbitrator(void) const;
    TemperatureThresholdArbitrator* getTemperatureThresholdArbitrator(void) const;

    // toXml()

private:

    // hide the copy constructor and assignment operator.
    Arbitrator(const Arbitrator& rhs);
    Arbitrator& operator=(const Arbitrator& rhs);

    DptfManager* m_dptfManager;

    ActiveControlArbitrator* m_activeControlArbitrator;
    ConfigTdpControlArbitrator* m_configTdpControlArbitrator;
    CoreControlArbitrator* m_coreControlArbitrator;
    DisplayControlArbitrator* m_displayControlArbitrator;
    PerformanceControlArbitrator* m_performanceControlArbitrator;
    PowerControlArbitrator* m_powerControlArbitrator;
    TemperatureThresholdArbitrator* m_temperatureThresholdArbitrator;
};