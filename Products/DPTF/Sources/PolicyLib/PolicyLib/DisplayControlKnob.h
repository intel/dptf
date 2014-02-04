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
#include "DisplayControlFacade.h"
#include <memory>

// control knob for display controls
class dptf_export DisplayControlKnob : public ControlKnobBase
{
public:

    DisplayControlKnob(
        const PolicyServicesInterfaceContainer& policyServices,
        std::shared_ptr<DisplayControlFacade> displayControl,
        UIntN participantIndex,
        UIntN domainIndex);
    ~DisplayControlKnob(void);

    virtual void limit() override;
    virtual void unlimit() override;
    virtual Bool canLimit() override;
    virtual Bool canUnlimit() override;

    XmlNode* getXml();

private:

    std::shared_ptr<DisplayControlFacade> m_displayControl;
    Bool m_hasBeenLimited; // only unlimit if the control has been limited in the past due to thermal condition
};