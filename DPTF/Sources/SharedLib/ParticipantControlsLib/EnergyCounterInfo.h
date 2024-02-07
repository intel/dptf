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

class XmlNode;

class EnergyCounterInfo
{
public:
	EnergyCounterInfo();
	EnergyCounterInfo(double energyCountInJoules, UInt64 timestampInMicroseconds);
	virtual ~EnergyCounterInfo() = default;
	EnergyCounterInfo(const EnergyCounterInfo& other) = default;
	EnergyCounterInfo& operator=(const EnergyCounterInfo& other) = default;
	EnergyCounterInfo(EnergyCounterInfo&& other) = default;
	EnergyCounterInfo& operator=(EnergyCounterInfo&& other) = default;

	double getEnergyCounterInJoules(double energyUnit) const;
	double getEnergyCounter() const;
	TimeSpan getTimestamp() const;
	Bool isValid() const;
	static EnergyCounterInfo fromDptfBuffer(const DptfBuffer& buffer);

private:
	double m_energyCount;
	TimeSpan m_timestamp;
	Bool m_isValid;
};
