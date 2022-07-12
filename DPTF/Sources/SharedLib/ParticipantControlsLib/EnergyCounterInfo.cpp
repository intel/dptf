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

#include "EnergyCounterInfo.h"

#define MICROJOULESTOJOULES 1000000

EnergyCounterInfo::EnergyCounterInfo()
	: m_energyCount(0.0)
	, m_timestamp(TimeSpan::createInvalid())
	, m_isValid(false)
{
}

EnergyCounterInfo::EnergyCounterInfo(double m_energyCountInJoules, UInt64 timestampInMicroseconds)
	: m_energyCount(m_energyCountInJoules)
	, m_timestamp(TimeSpan::createFromMicroseconds(timestampInMicroseconds))
	, m_isValid(true)
{
}

EnergyCounterInfo EnergyCounterInfo::getEnergyCounterInfoFromBuffer(const DptfBuffer& buffer, UInt64 timestampInMicroseconds)
{
	if (buffer.size() == 0)
	{
		return EnergyCounterInfo();
	}

	UInt8* data = reinterpret_cast<UInt8*>(buffer.get());
	struct esif_data_energy_counter_info* currentRow = reinterpret_cast<struct esif_data_energy_counter_info*>(data);

	double energyCountInMicroJoules = static_cast<double>(currentRow->energyCount.integer.value);
	double energyCountInJoules = energyCountInMicroJoules / MICROJOULESTOJOULES;

	return EnergyCounterInfo(
		energyCountInJoules,
		timestampInMicroseconds);
}

const double EnergyCounterInfo::getEnergyCounter()
{
	return m_energyCount;
}

const TimeSpan EnergyCounterInfo::getTimestamp()
{
	return m_timestamp;
}

const Bool EnergyCounterInfo::isValid()
{
	return m_isValid;
}

EnergyCounterInfo::~EnergyCounterInfo()
{
}
