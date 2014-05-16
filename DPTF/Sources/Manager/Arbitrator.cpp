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

#include "Arbitrator.h"
#include "DptfManager.h"

Arbitrator::Arbitrator(DptfManager* dptfManager) :
    m_dptfManager(dptfManager)
{
    m_activeControlArbitrator = new ActiveControlArbitrator(m_dptfManager);
    m_configTdpControlArbitrator = new ConfigTdpControlArbitrator(m_dptfManager);
    m_coreControlArbitrator = new CoreControlArbitrator(m_dptfManager);
    m_displayControlArbitrator = new DisplayControlArbitrator(m_dptfManager);
    m_performanceControlArbitrator = new PerformanceControlArbitrator(m_dptfManager);
    m_powerControlArbitrator = new PowerControlArbitrator(m_dptfManager);
    m_temperatureThresholdArbitrator = new TemperatureThresholdArbitrator(m_dptfManager);
}

Arbitrator::~Arbitrator(void)
{
    DELETE_MEMORY_TC(m_activeControlArbitrator);
    DELETE_MEMORY_TC(m_configTdpControlArbitrator);
    DELETE_MEMORY_TC(m_coreControlArbitrator);
    DELETE_MEMORY_TC(m_displayControlArbitrator);
    DELETE_MEMORY_TC(m_performanceControlArbitrator);
    DELETE_MEMORY_TC(m_powerControlArbitrator);
    DELETE_MEMORY_TC(m_temperatureThresholdArbitrator);
}

void Arbitrator::clearPolicyCachedData(UIntN policyIndex)
{
    // call each arbitrator class to remove the specified policy
    m_activeControlArbitrator->clearPolicyCachedData(policyIndex);
    m_configTdpControlArbitrator->clearPolicyCachedData(policyIndex);
    m_coreControlArbitrator->clearPolicyCachedData(policyIndex);
    m_displayControlArbitrator->clearPolicyCachedData(policyIndex);
    m_performanceControlArbitrator->clearPolicyCachedData(policyIndex);
    m_powerControlArbitrator->clearPolicyCachedData(policyIndex);
    m_temperatureThresholdArbitrator->clearPolicyCachedData(policyIndex);
}

ActiveControlArbitrator* Arbitrator::getActiveControlArbitrator(void) const
{
    return m_activeControlArbitrator;
}

ConfigTdpControlArbitrator* Arbitrator::getConfigTdpControlArbitrator(void) const
{
    return m_configTdpControlArbitrator;
}

CoreControlArbitrator* Arbitrator::getCoreControlArbitrator(void) const
{
    return m_coreControlArbitrator;
}

DisplayControlArbitrator* Arbitrator::getDisplayControlArbitrator(void) const
{
    return m_displayControlArbitrator;
}

PerformanceControlArbitrator* Arbitrator::getPerformanceControlArbitrator(void) const
{
    return m_performanceControlArbitrator;
}

PowerControlArbitrator* Arbitrator::getPowerControlArbitrator(void) const
{
    return m_powerControlArbitrator;
}

TemperatureThresholdArbitrator* Arbitrator::getTemperatureThresholdArbitrator(void) const
{
    return m_temperatureThresholdArbitrator;
}