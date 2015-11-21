/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
#include "DomainProxyInterface.h"
#include "ControlStatus.h"

// contains the control status for a given domain inside a participant
class dptf_export PassiveDomainControlStatus
{
public:

    PassiveDomainControlStatus(DomainProxyInterface* domain);
    XmlNode* getXml();

private:

    UIntN m_participantIndex;
    UIntN m_domainIndex;
    std::string m_domainName;
    std::vector<ControlStatus> m_controlStatus;
    Temperature m_domainTemperature;
    DomainPriority m_domainPriority;
    UtilizationStatus m_domainUtilization;

    void aquireDomainStatus(DomainProxyInterface* domain);
    void addPowerStatus(DomainProxyInterface* domain);
    void addPstateStatus(DomainProxyInterface* domain);
    void addTstateStatus(DomainProxyInterface* domain);
    void addDisplayStatus(DomainProxyInterface* domain);
    void addCoreStatus(DomainProxyInterface* domain);

    UIntN indexOfFirstControlWithType(const PerformanceControlSet& controlSet, PerformanceControlType::Type type) const;
    PerformanceControlSet filterControlSet(
        const PerformanceControlSet& controlSet, 
        PerformanceControlDynamicCaps dynamicCapabilities, 
        PerformanceControlType::Type type) const;
};