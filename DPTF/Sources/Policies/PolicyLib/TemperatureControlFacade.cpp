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

#include "TemperatureControlFacade.h"
#include "TemperatureStatus.h"
using namespace std;

TemperatureControlFacade::TemperatureControlFacade(
	UIntN participantIndex,
	UIntN domainIndex,
	const DomainProperties& domainProperties,
	const PolicyServicesInterfaceContainer& policyServices)
	: m_policyServices(policyServices)
	, m_participantIndex(participantIndex)
	, m_domainIndex(domainIndex)
	, m_domainProperties(domainProperties)
{
}

TemperatureControlFacade::~TemperatureControlFacade()
{
}

Temperature TemperatureControlFacade::getCurrentTemperature(void)
{
	if (supportsTemperatureControls())
	{
		DptfRequest request(DptfRequestType::TemperatureControlGetTemperatureStatus, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();

		return TemperatureStatus::createFromDptfBuffer(result.getData()).getCurrentTemperature();
	}
	else
	{
		throw dptf_exception("Domain does not support the temperature interface.");
	}
}

TemperatureThresholds TemperatureControlFacade::getTemperatureNotificationThresholds()
{
	if (supportsTemperatureThresholds())
	{
		if (m_temperatureThresholds.isInvalid())
		{
			DptfRequest request(DptfRequestType::TemperatureControlGetTemperatureThresholds, m_participantIndex, m_domainIndex);
			auto result = m_policyServices.serviceRequest->submitRequest(request);
			result.throwIfFailure();
			m_temperatureThresholds.set(TemperatureThresholds::createFromDptfBuffer(result.getData()));
		}

		return m_temperatureThresholds.get();
	}
	else
	{
		throw dptf_exception("Domain does not support the temperature threshold interface.");
	}
}

void TemperatureControlFacade::setTemperatureNotificationThresholds(
	const Temperature& lowerBound,
	const Temperature& upperBound)
{
	if (supportsTemperatureThresholds())
	{
		TemperatureThresholds thresholdsToSet(lowerBound, upperBound, getHysteresis());
		DptfRequest request(DptfRequestType::TemperatureControlSetTemperatureThresholds, m_participantIndex, m_domainIndex);
		request.setData(thresholdsToSet.toDptfBuffer());
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();
		m_temperatureThresholds.set(thresholdsToSet);
	}
}

Bool TemperatureControlFacade::supportsTemperatureControls()
{
	return m_domainProperties.implementsTemperatureInterface();
}

Bool TemperatureControlFacade::supportsTemperatureThresholds()
{
	return m_domainProperties.implementsTemperatureThresholdInterface();
}

Bool TemperatureControlFacade::isVirtualTemperatureControl()
{
	if (supportsTemperatureControls())
	{
		if (m_isVirtualSensor.isInvalid())
		{
			DptfRequest request(DptfRequestType::TemperatureControlIsVirtualTemperatureControl, m_participantIndex, m_domainIndex);
			auto result = m_policyServices.serviceRequest->submitRequest(request);
			result.throwIfFailure();
			m_isVirtualSensor.set(result.getDataAsBool());
		}

		return m_isVirtualSensor.get();
	}
	else
	{
		return false;
	}
}

DptfBuffer TemperatureControlFacade::getCalibrationTable()
{
	if (supportsTemperatureControls())
	{
		DptfRequest request(DptfRequestType::TemperatureControlGetCalibrationTable, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();

		return result.getData();
	}
	else
	{
		throw dptf_exception("Domain does not support the temperature interface.");
	}
}

DptfBuffer TemperatureControlFacade::getPollingTable()
{
	if (supportsTemperatureControls())
	{
		DptfRequest request(DptfRequestType::TemperatureControlGetPollingTable, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();

		return result.getData();
	}
	else
	{
		throw dptf_exception("Domain does not support the temperature interface.");
	}
}

void TemperatureControlFacade::setVirtualTemperature(const Temperature& temperature)
{
	if (supportsTemperatureControls())
	{
		DptfRequest request(DptfRequestType::TemperatureControlSetVirtualTemperature, m_participantIndex, m_domainIndex);
		request.setData(temperature.toDptfBuffer());
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();
	}
}

void TemperatureControlFacade::refreshHysteresis()
{
	if (supportsTemperatureThresholds())
	{
		DptfRequest request(DptfRequestType::TemperatureControlGetTemperatureThresholds, m_participantIndex, m_domainIndex);
		auto result = m_policyServices.serviceRequest->submitRequest(request);
		result.throwIfFailure();

		auto currentThresholds = TemperatureThresholds::createFromDptfBuffer(result.getData());
		if (m_temperatureThresholds.isInvalid())
		{
			m_temperatureThresholds.set(currentThresholds);
		}

		TemperatureThresholds thresholdsWithUpdatedHysteresis(
			m_temperatureThresholds.get().getAux0(),
			m_temperatureThresholds.get().getAux1(),
			currentThresholds.getHysteresis());
		m_temperatureThresholds.set(thresholdsWithUpdatedHysteresis);
	}
}

Temperature TemperatureControlFacade::getPowerShareTemperatureThreshold()
{
	DptfRequest request(DptfRequestType::TemperatureControlGetPowerShareTemperatureThreshold, m_participantIndex, m_domainIndex);
	auto result = m_policyServices.serviceRequest->submitRequest(request);
	result.throwIfFailure();

	return Temperature::createFromDptfBuffer(result.getData());
}

Temperature TemperatureControlFacade::getHysteresis()
{
	return getTemperatureNotificationThresholds().getHysteresis();
}
