/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
#include "DisplayControlSetCachedProperty.h"
#include "DisplayControlCapabilitiesCachedProperty.h"

// this facade class provides a simpler interface on top of display controls as well as combines all of the display
// control properties and capabilities into a single class.  these properties also have the ability to be cached.
class dptf_export DisplayControlFacadeInterface
{
public:
	virtual ~DisplayControlFacadeInterface(){};

	virtual Bool supportsDisplayControls() = 0;
	virtual void setControlsToMax() = 0;
	virtual void setControl(UIntN displayControlIndex) = 0;
	virtual void setDisplayControlDynamicCaps(DisplayControlDynamicCaps newCapabilities) = 0;
	virtual void refreshCapabilities() = 0;
	virtual void invalidateControlSet() = 0;
	virtual DisplayControlStatus getStatus() = 0;
	virtual UIntN getUserPreferredDisplayIndex() = 0;
	virtual Bool isUserPreferredIndexModified() = 0;
	virtual const DisplayControlSet& getControls() = 0;
	virtual const DisplayControlDynamicCaps& getCapabilities() = 0;
	virtual void setValueWithinCapabilities() = 0;
	virtual void lockCapabilities() = 0;
	virtual void unlockCapabilities() = 0;
};
