/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

struct esif_data_energy_counter_info
{
	union esif_data_variant energyCount;    // ULONG
	union esif_data_variant timestamp;      // ULONG
};

class EnergyCounterInfo final
{
public:
	EnergyCounterInfo();
	EnergyCounterInfo(double m_energyCountInJoules, UInt64 timestampInMicroseconds);
	const double getEnergyCounter();
	const TimeSpan getTimestamp();
	const Bool isValid();
	static EnergyCounterInfo getEnergyCounterInfoFromBuffer(const DptfBuffer& buffer, UInt64 timestampInMicroseconds);

	~EnergyCounterInfo();

private:
	double m_energyCount;
	TimeSpan m_timestamp;
	Bool m_isValid;
};
