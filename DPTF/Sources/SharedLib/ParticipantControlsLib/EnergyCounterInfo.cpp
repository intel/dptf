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

#include "EnergyCounterInfo.h"
#include "esif_sdk_data_misc.h"

enum{MICRO_JOULES_TO_JOULES = 1000000};

EnergyCounterInfo::EnergyCounterInfo()
	: m_energyCount(0.0)
	, m_timestamp(TimeSpan::createInvalid())
	, m_isValid(false)
{
}

EnergyCounterInfo::EnergyCounterInfo(double energyCountInJoules, UInt64 timestampInMicroseconds)
	: m_energyCount(energyCountInJoules)
	, m_timestamp(TimeSpan::createFromMicroseconds(timestampInMicroseconds))
	, m_isValid(true)
{
}

EnergyCounterInfo EnergyCounterInfo::fromDptfBuffer(const DptfBuffer& buffer)
{
	EnergyCounterInfo energyCounterInfo;

	if (buffer.size() < sizeof(RaplEnergyInfo))
	{
		throw dptf_exception("Incorrect buffer size for energyCounterInfo");
	}

	const auto currentRow = reinterpret_cast<struct RaplEnergyInfo_s*>(buffer.get());
	const auto revision = currentRow->revision;

	if (revision == RAPL_ENERGY_INFO_REVISION)
	{
		const auto energyCountInJoules = static_cast<double>(currentRow->energyCounter);
		const auto timestampInMicroseconds = currentRow->timestamp;
		energyCounterInfo = EnergyCounterInfo(energyCountInJoules, timestampInMicroseconds);
	}
	
	return energyCounterInfo;
}

double EnergyCounterInfo::getEnergyCounterInJoules(double energyUnit) const
{
	return m_energyCount * energyUnit;
}

double EnergyCounterInfo::getEnergyCounter() const
{
	return m_energyCount;
}

TimeSpan EnergyCounterInfo::getTimestamp() const
{
	return m_timestamp;
}

Bool EnergyCounterInfo::isValid() const
{
	return m_isValid;
}
