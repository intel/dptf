/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "SocGear.h"
#include "SystemUsageMode.h"

class DomainProcessorControlInterface
{
public:
	virtual ~DomainProcessorControlInterface() {};

	virtual Temperature getTccOffsetTemperature() = 0;
	virtual void setTccOffsetTemperature(const Temperature& tccOffset) = 0;
	virtual Temperature getMaxTccOffsetTemperature() = 0;
	virtual Temperature getMinTccOffsetTemperature() = 0;
	virtual void setUnderVoltageThreshold(const UInt32 voltageThreshold) = 0;
	virtual void setPerfPreferenceMax(const Percentage& cpuMaxRatio) = 0;
	virtual void setPerfPreferenceMin(const Percentage& cpuMinRatio) = 0;
	virtual UInt32 getPcieThrottleRequestState() = 0;
	virtual SocGear::Type getSocGear() const = 0;
	virtual void setSocGear(SocGear::Type socGear) = 0;
	virtual SystemUsageMode::Type getSocSystemUsageMode() = 0;
	virtual void setSocSystemUsageMode(SystemUsageMode::Type mode) = 0;
};