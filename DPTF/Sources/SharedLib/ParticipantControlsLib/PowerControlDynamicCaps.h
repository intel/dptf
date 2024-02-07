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
#include "PowerControlType.h"
#include "TimeSpan.h"

class XmlNode;

// Stores the min/max values for PL1/PL2/PL3.  The current values are stored in PowerControlStatus.

class PowerControlDynamicCaps
{
public:
	PowerControlDynamicCaps();
	PowerControlDynamicCaps(
		PowerControlType::Type powerControlType,
		const Power& minPowerLimit,
		const Power& maxPowerLimit,
		const Power& powerStepSize,
		const TimeSpan& minTimeWindow,
		const TimeSpan& maxTimeWindow,
		const Percentage& minDutyCycle,
		const Percentage& maxDutyCycle);
	virtual ~PowerControlDynamicCaps() = default;

	PowerControlDynamicCaps(const PowerControlDynamicCaps& other) = default;
	PowerControlDynamicCaps& operator=(const PowerControlDynamicCaps& other) = default;
	PowerControlDynamicCaps(PowerControlDynamicCaps&& other) = default;
	PowerControlDynamicCaps& operator=(PowerControlDynamicCaps&& other) = default;

	PowerControlType::Type getPowerControlType() const;

	Bool arePowerLimitCapsValid() const;
	Power getMinPowerLimit() const;
	Power getMaxPowerLimit() const;
	Power getPowerStepSize() const;
	void setMinPowerLimit(const Power& minPower);
	void setMaxPowerLimit(const Power& maxPower);
	void setPowerStepSize(const Power& stepSize);
	void setMinTimeWindow(const TimeSpan& minTimeWindow);
	void setMaxTimeWindow(const TimeSpan& maxTimeWindow);

	Bool areTimeWindowCapsValid() const;
	TimeSpan getMinTimeWindow() const;
	TimeSpan getMaxTimeWindow() const;

	Bool areDutyCycleCapsValid() const;
	Percentage getMinDutyCycle() const; // TODO: remove as this is not even in the PPCC
	Percentage getMaxDutyCycle() const;

	Bool operator==(const PowerControlDynamicCaps& rhs) const;
	Bool operator!=(const PowerControlDynamicCaps& rhs) const;
	std::shared_ptr<XmlNode> getXml() const;
	PowerControlDynamicCaps getDefaultPpccRowValues(PowerControlType::Type PowerControlType) const;
	PowerControlDynamicCaps getDefaultPpccPl4RowValues(const Power& pl4PowerLimit) const;
	PowerControlDynamicCaps getPpccPlRowValues(const struct EsifDataBinaryPpccPackage* currentRow) const;

private:
	Bool m_valid;
	PowerControlType::Type m_powerControlType;

	Power m_minPowerLimit; // in mW
	Power m_maxPowerLimit; // in mW
	Power m_powerStepSize; // in mW.

	TimeSpan m_minTimeWindow; // in ms
	TimeSpan m_maxTimeWindow; // in ms. Min = Max if time window is not programmable.

	Percentage m_minDutyCycle; // percentage
	Percentage m_maxDutyCycle; // percentage

	void throwIfNotValid() const;
};
