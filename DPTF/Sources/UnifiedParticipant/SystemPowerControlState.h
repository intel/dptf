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
#include "DomainSystemPowerControlBase.h"

class SystemPowerControlState
{
public:
	SystemPowerControlState(DomainSystemPowerControlBase* control);
	~SystemPowerControlState();

	void capture();
	void restore();

private:
	void captureEnables();
	void captureLimit(PsysPowerLimitType::Type limitType, Power& limit, Bool& limitValid);
	void captureTimeWindow(PsysPowerLimitType::Type limitType, TimeSpan& timeWindow, Bool& timeWindowValid);
	void captureDutyCycle(PsysPowerLimitType::Type limitType, Percentage& dutyCycle, Bool& dutyCycleValid);

	void restoreEnables();
	void restoreLimit(PsysPowerLimitType::Type limitType, const Power& limit, const Bool& limitValid);
	void restoreTimeWindow(PsysPowerLimitType::Type limitType, const TimeSpan& timeWindow, const Bool& timeWindowValid);
	void restoreDutyCycle(PsysPowerLimitType::Type limitType, const Percentage& dutyCycle, const Bool& dutyCycleValid);

	DomainSystemPowerControlBase* m_control;

	Bool m_pl1Enabled;
	Bool m_pl2Enabled;
	Bool m_pl3Enabled;
	Bool m_enablesValid;

	Power m_pl1Limit;
	Power m_pl2Limit;
	Power m_pl3Limit;
	Bool m_pl1LimitValid;
	Bool m_pl2LimitValid;
	Bool m_pl3LimitValid;

	TimeSpan m_pl1TimeWindow;
	TimeSpan m_pl3TimeWindow;
	Bool m_pl1TimeWindowValid;
	Bool m_pl3TimeWindowValid;

	Percentage m_pl3DutyCycle;
	Bool m_pl3DutyCycleValid;
};
