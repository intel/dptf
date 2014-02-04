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
#include "PolicyServicesInterfaceContainer.h"
#include "DomainProperties.h"
#include "ActiveControlStaticCapsCachedProperty.h"
#include "XmlNode.h"
#include <map>

// TODO: rename as a Facade
// this class provides an easy-to-use interface and arbitration for fan speed requests
class dptf_export ActiveCoolingControl
{
public:

    ActiveCoolingControl(
        UIntN participantIndex,
        UIntN domainIndex,
        const DomainProperties& domainProperties,
        const ParticipantProperties& participantProperties,
        const PolicyServicesInterfaceContainer& policyServices);
    ~ActiveCoolingControl(void);

    // control capabilities
    Bool supportsActiveCoolingControls();
    Bool supportsFineGrainControl();

    // fan speed requests
    void requestFanSpeedPercentage(UIntN requestorIndex, const Percentage& fanSpeed);
    void requestActiveControlIndex(UIntN requestorIndex, UIntN activeControlIndex);
    void forceFanOff(void);
    
    // status
    XmlNode* getXml();

private:

    // services
    PolicyServicesInterfaceContainer m_policyServices;
    
    // control properties
    DomainProperties m_domainProperties;
    ParticipantProperties m_participantProperties;
    UIntN m_participantIndex;
    UIntN m_domainIndex;
    ActiveControlStaticCapsCachedProperty m_staticCaps;

    // fan speed request arbitration
    std::map<UIntN, Percentage> m_fanSpeedRequestTable;
    std::map<UIntN, UIntN> m_activeControlRequestTable;
    Percentage m_lastFanSpeedRequest;
    UIntN m_lastFanSpeedRequestIndex;
    void updateFanSpeedRequestTable(UIntN requestorIndex, const Percentage& fanSpeed);
    Percentage chooseHighestFanSpeedRequest();
    void updateActiveControlRequestTable(UIntN requestorIndex, UIntN activeControlIndex);
    UIntN chooseHighestActiveControlIndex();
};