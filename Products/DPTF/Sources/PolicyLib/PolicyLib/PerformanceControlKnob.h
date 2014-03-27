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
#include "ControlKnobBase.h"
#include "PerformanceControlFacade.h"
#include "PerformanceControl.h"
#include "XmlNode.h"
#include <memory>
#include <map>

// control knob for performance controls.  the instantiation of this knob can be either for p-states or t-states.
class dptf_export PerformanceControlKnob : public ControlKnobBase
{
public:

    PerformanceControlKnob(
        const PolicyServicesInterfaceContainer& policyServices,
        UIntN participantIndex,
        UIntN domainIndex,
        std::shared_ptr<PerformanceControlFacade> performanceControl,
        std::shared_ptr<std::map<UIntN, UIntN>> perfControlRequests, 
        PerformanceControlType::Type controlType);
    ~PerformanceControlKnob(void);

    virtual void limit(UIntN target) override;
    virtual void unlimit(UIntN target) override;
    virtual Bool canLimit(UIntN target) override;
    virtual Bool canUnlimit(UIntN target) override;
    virtual Bool commitSetting() override;
    virtual void clearRequestForTarget(UIntN target) override;
    virtual void clearAllRequests() override;

    Bool supportsPerformanceControls() const;
    UIntN getTargetRequest(UIntN target) const;
    void setTstateUtilizationThreshold(UtilizationStatus tstateUtilizationThreshold);

    XmlNode* getXml();

private:

    std::shared_ptr<PerformanceControlFacade> m_performanceControl;
    PerformanceControlType::Type m_controlType;
    std::shared_ptr<std::map<UIntN, UIntN>> m_requests;
    UtilizationStatus m_tstateUtilizationThreshold;

    std::string controlTypeToString(PerformanceControlType::Type controlType);
    UIntN findHighestPerformanceIndexRequest() const;
    Bool canCommit(UIntN currentIndex, UIntN nextIndex) const;
    Bool checkUtilizationIsLessThanThreshold() const;
    UIntN snapToCapabilitiesBounds(UIntN controlIndex);
};