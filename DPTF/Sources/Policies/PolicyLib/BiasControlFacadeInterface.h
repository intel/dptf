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
#include "OpportunisticBoostMode.h"

class dptf_export BiasControlFacadeInterface
{
public:
	virtual ~BiasControlFacadeInterface() = default;
	virtual Bool supportsBiasControl() const = 0;
	virtual void setCpuOpboostEnableAC(Bool enabled) = 0;
	virtual void setCpuOpboostEnableDC(Bool enabled) = 0;
	virtual void setGpuOpboostEnableAC(Bool enabled) = 0;
	virtual void setGpuOpboostEnableDC(Bool enabled) = 0;
	virtual void setSplitRatio(const Percentage& splitRatio) = 0;
	virtual void setSplitRatioMax(const Percentage& splitRatio) = 0;
	virtual Bool getCpuOpboostEnableAC() = 0;
	virtual Bool getCpuOpboostEnableDC() = 0;
	virtual Bool getGpuOpboostEnableAC() = 0;
	virtual Bool getGpuOpboostEnableDC() = 0;
	virtual Percentage getSplitRatio() = 0;
	virtual Percentage getSplitRatioActive() = 0;
	virtual Percentage getSplitRatioMax() = 0;
	virtual Power getReservedTgp() = 0;
	virtual OpportunisticBoostMode::Type getOppBoostMode() = 0;
};
