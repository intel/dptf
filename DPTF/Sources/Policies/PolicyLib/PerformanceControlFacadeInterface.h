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
#include "PerformanceControlStatus.h"
#include "PerformanceControlSetCachedProperty.h"
#include "PerformanceControlStatusCachedProperty.h"
#include "PerformanceControlCapabilitiesCachedProperty.h"

class dptf_export PerformanceControlFacadeInterface
{
public:

    virtual ~PerformanceControlFacadeInterface() {};

    // controls
    virtual Bool supportsPerformanceControls() = 0;
    virtual void initializeControlsIfNeeded() = 0;
    virtual void setControlsToMax() = 0;
    virtual void setControl(UIntN performanceControlIndex) = 0;
    virtual void setPerformanceControlDynamicCaps(PerformanceControlDynamicCaps newCapabilities) = 0;

    // properties
    virtual void refreshCapabilities() = 0;
    virtual void refreshControls() = 0;
    virtual PerformanceControlStatus getStatus() const = 0;
    virtual PerformanceControlStatus getLiveStatus() const = 0;
    virtual const PerformanceControlSet& getControls() = 0;
    virtual const PerformanceControlDynamicCaps& getDynamicCapabilities() = 0;

};