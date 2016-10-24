/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "ActiveControlStatus.h"

class dptf_export ActiveCoolingControlFacadeInterface
{
public:

    virtual ~ActiveCoolingControlFacadeInterface() {};

    // controls
    virtual Bool supportsActiveCoolingControls() = 0;
    virtual Bool supportsFineGrainControl() = 0;

    //properties
    virtual const ActiveControlStaticCaps& getCapabilities() = 0;
    virtual void refreshCapabilities() = 0;
    virtual ActiveControlStatus getStatus() = 0;

    //status
    virtual std::shared_ptr<XmlNode> getXml() = 0;

    // fan speed requests
    virtual void requestFanSpeedPercentage(UIntN requestorIndex, const Percentage& fanSpeed) = 0;
    virtual void requestActiveControlIndex(UIntN requestorIndex, UIntN activeControlIndex) = 0;
    virtual void forceFanOff(void) = 0;

    virtual void setControl(Percentage activeCoolingControlFanSpeed) = 0;
};