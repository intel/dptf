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

#include "Temperature.h"
#include <cmath>
#include "StatusFormat.h"
#include "DptfBufferStream.h"
using namespace StatusFormat;

Temperature::Temperature(void)
	: m_valid(false)
	, m_temperature(0)
{
}

Temperature::Temperature(UInt32 temperatureInTenthKelvin)
	: m_valid(true)
	, m_temperature(temperatureInTenthKelvin)
{
	if ((temperatureInTenthKelvin > maxValidTemperature || temperatureInTenthKelvin < minValidTemperature)
		&& temperatureInTenthKelvin != Constants::MaxUInt32)
	{
		throw temperature_out_of_range("Temperature out of valid range");
	}
}

Temperature Temperature::createInvalid()
{
	return Temperature();
}

Bool Temperature::operator==(const Temperature& rhs) const
{
	// Do not throw an exception if temperature is not valid.

	if (this->isValid() == true && rhs.isValid() == true)
	{
		return (this->m_temperature == rhs.m_temperature);
	}
	else if (this->isValid() == false && rhs.isValid() == false)
	{
		return true;
	}
	else
	{
		return false;
	}
}

Bool Temperature::operator!=(const Temperature& rhs) const
{
	// Do not throw an exception if temperature is not valid.
	return !(*this == rhs);
}

Bool Temperature::operator>(const Temperature& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);
	return (this->m_temperature > rhs.m_temperature);
}

Bool Temperature::operator>=(const Temperature& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);
	return (this->m_temperature >= rhs.m_temperature);
}

Bool Temperature::operator<(const Temperature& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);
	return (this->m_temperature < rhs.m_temperature);
}

Bool Temperature::operator<=(const Temperature& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);
	return (this->m_temperature <= rhs.m_temperature);
}

Temperature Temperature::operator+(const Temperature& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);
	auto thisTemperature = this->m_temperature;
	auto rhsTemperature = rhs.m_temperature;
	if (thisTemperature == Constants::MaxUInt32)
	{
		thisTemperature = 0;
	}

	if (rhsTemperature == Constants::MaxUInt32)
	{
		rhsTemperature = 0;
	}

	if (thisTemperature == 0 || rhsTemperature == 0)
	{
		return Temperature(thisTemperature + rhsTemperature);
	}
	else
	{
		return Temperature(thisTemperature + rhsTemperature - CELSIUS_TO_TENTH_KELVIN);
	}
}

Temperature Temperature::operator-(const Temperature& rhs) const
{
	throwIfInvalid(*this);
	throwIfInvalid(rhs);
	auto thisTemperature = this->m_temperature;
	auto rhsTemperature = rhs.m_temperature;
	if (thisTemperature == Constants::MaxUInt32)
	{
		thisTemperature = 0;
	}

	if (rhsTemperature == Constants::MaxUInt32)
	{
		rhsTemperature = 0;
	}

	if (thisTemperature == 0 || rhsTemperature == 0)
	{
		return Temperature(thisTemperature - rhsTemperature);
	}
	else
	{
		return Temperature(CELSIUS_TO_TENTH_KELVIN + thisTemperature - rhsTemperature);
	}
}

std::ostream& operator<<(std::ostream& os, const Temperature& temperature)
{
	os << temperature.toString();
	return os;
}

Temperature::operator UInt32(void) const
{
	throwIfInvalid(*this);
	return m_temperature;
}

Bool Temperature::isValid() const
{
	return m_valid;
}

std::string Temperature::toString() const
{
	if (isValid() && m_temperature != Constants::MaxUInt32)
	{
		return friendlyValueWithPrecision(getTemperatureInCelsius(), 1);
	}
	else
	{
		return std::string(Constants::InvalidString);
	}
}

void Temperature::throwIfInvalid(const Temperature& temperature) const
{
	if (temperature.isValid() == false)
	{
		throw temperature_out_of_range("Temperature is not valid.");
	}
}

Temperature Temperature::fromCelsius(double temperatureInCelsius)
{
	UInt32 temperatureInTenthKelvin = UInt32(round(temperatureInCelsius * 10.0 + CELSIUS_TO_TENTH_KELVIN));
	return Temperature(temperatureInTenthKelvin);
}

double Temperature::getTemperatureInCelsius() const
{
	double temperatureInTenthCelsius = (double)m_temperature - (double)CELSIUS_TO_TENTH_KELVIN;
	double temperatureInCelsius = (double)temperatureInTenthCelsius / 10.0;
	return temperatureInCelsius;
}

Temperature Temperature::snapWithinAllowableTripPointRange(Temperature aux)
{
	if ((UInt32)aux != Constants::MaxUInt32)
	{
		Temperature minAux = fromCelsius(ESIF_SDK_MIN_AUX_TRIP);
		if (aux.isValid() && aux < minAux)
		{
			aux = minAux;
		}

		Temperature maxAux = fromCelsius(ESIF_SDK_MAX_AUX_TRIP);
		if (aux.isValid() && aux > maxAux)
		{
			aux = maxAux;
		}
	}

	return aux;
}

DptfBuffer Temperature::toDptfBuffer() const
{
	DptfBuffer buffer;
	buffer.append((UInt8*)&m_valid, sizeof(m_valid));
	buffer.append((UInt8*)&m_temperature, sizeof(m_temperature));
	return buffer;
}

Temperature Temperature::createFromDptfBuffer(const DptfBuffer& buffer)
{
	if (buffer.size() != (Temperature().toDptfBuffer().size()))
	{
		throw dptf_exception("Buffer given to Temperature class has invalid length.");
	}

	DptfBuffer bufferCopy = buffer;
	DptfBufferStream stream(bufferCopy);
	Temperature newTemperature;
	newTemperature.m_valid = stream.readNextBool();
	newTemperature.m_temperature = stream.readNextUint32();
	return newTemperature;
}
