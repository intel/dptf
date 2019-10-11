/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#include "TemperatureStatus.h"
#include "TemperatureThresholds.h"

class DomainTemperatureInterface
{
public:
	virtual ~DomainTemperatureInterface() {};

	virtual TemperatureStatus getTemperatureStatus() = 0;
	virtual TemperatureThresholds getTemperatureThresholds() = 0;
	virtual void setTemperatureThresholds(const TemperatureThresholds& temperatureThresholds,
		const TemperatureThresholds& lastSetTemperatureThresholds) = 0;
	virtual Temperature getPowerShareTemperatureThreshold() = 0;
	virtual DptfBuffer getCalibrationTable() = 0;
	virtual DptfBuffer getPollingTable() = 0;
	virtual Bool isVirtualTemperature() = 0;
	virtual void setVirtualTemperature(const Temperature& temperature) = 0;
};
