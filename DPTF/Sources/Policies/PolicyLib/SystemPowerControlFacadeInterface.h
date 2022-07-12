/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "TimeSpan.h"

class dptf_export SystemPowerControlFacadeInterface
{
public:
	virtual ~SystemPowerControlFacadeInterface(){};

	virtual Bool isPl1PowerLimitEnabled(void) = 0;
	virtual Bool isPl2PowerLimitEnabled(void) = 0;
	virtual Bool isPl3PowerLimitEnabled(void) = 0;

	virtual Power getPl1PowerLimit(void) = 0;
	virtual Power getPl2PowerLimit(void) = 0;
	virtual Power getPl3PowerLimit(void) = 0;
	virtual TimeSpan getPl1TimeWindow(void) = 0;
	virtual TimeSpan getPl3TimeWindow(void) = 0;
	virtual Percentage getPl3DutyCycle(void) = 0;

	virtual void setPl1PowerLimit(const Power& powerLimit) = 0;
	virtual void setPl2PowerLimit(const Power& powerLimit) = 0;
	virtual void setPl3PowerLimit(const Power& powerLimit) = 0;
	virtual void setPl1TimeWindow(const TimeSpan& timeWindow) = 0;
	virtual void setPl3TimeWindow(const TimeSpan& timeWindow) = 0;
	virtual void setPl3DutyCycle(const Percentage& dutyCycle) = 0;
};
