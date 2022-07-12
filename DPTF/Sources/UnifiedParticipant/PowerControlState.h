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
#include "DomainPowerControlBase.h"
#include "CachedValue.h"
#include "XmlNode.h"

class PowerControlState
{
public:
	PowerControlState(DomainPowerControlBase* control);
	~PowerControlState();

	void capture();
	void restore();
	std::shared_ptr<XmlNode> toXml() const;

private:
	void captureEnables();
	void captureLimit(PowerControlType::Type controlType, CachedValue<Power>& limit);
	void captureTimeWindow(PowerControlType::Type controlType, CachedValue<TimeSpan>& timeWindow);
	void captureDutyCycle(PowerControlType::Type controlType, CachedValue<Percentage>& dutyCycle);

	void restoreEnables();
	void restoreLimit(PowerControlType::Type controlType, const CachedValue<Power>& limit);
	void restoreTimeWindow(PowerControlType::Type controlType, const CachedValue<TimeSpan>& timeWindow);
	void restoreDutyCycle(PowerControlType::Type controlType, const CachedValue<Percentage>& dutyCycle);

	DomainPowerControlBase* m_control;

	CachedValue<Bool> m_pl1Enabled;
	CachedValue<Bool> m_pl2Enabled;
	CachedValue<Bool> m_pl3Enabled;
	CachedValue<Bool> m_pl4Enabled;

	CachedValue<Power> m_pl1Limit;
	CachedValue<Power> m_pl2Limit;
	CachedValue<Power> m_pl3Limit;
	CachedValue<Power> m_pl4Limit;

	CachedValue<TimeSpan> m_pl1TimeWindow;
	CachedValue<TimeSpan> m_pl3TimeWindow;

	CachedValue<Percentage> m_pl3DutyCycle;
};
