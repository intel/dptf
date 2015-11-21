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
#include "CoreControlStatusCachedProperty.h"
#include "CoreControlCapabilitiesCachedProperty.h"
#include "CoreControlPreferencesCachedProperty.h"

class dptf_export CoreControlFacadeInterface
{
public:

    virtual ~CoreControlFacadeInterface() {};

    // controls
    virtual Bool supportsCoreControls() = 0;
    virtual void initializeControlsIfNeeded() = 0;
    virtual void setControlsToMax() = 0;
    virtual void setControl(CoreControlStatus coreControl) = 0;

    // properties
    virtual CoreControlStatus getStatus() = 0;
    virtual CoreControlDynamicCaps getDynamicCapabilities() = 0;
    virtual CoreControlStaticCaps getStaticCapabilities() = 0;
    virtual CoreControlLpoPreference getPreferences() = 0;
    virtual void refreshCapabilities() = 0;
    virtual void refreshPreferences() = 0;
    virtual void setValueWithinCapabilities() = 0;
};