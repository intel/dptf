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

#include "TemperatureControlFacadeInterface.h"
#include "CachedValue.h"

// this facade class provides a simpler interface on top of temperature controls as well as combines all of the
// virtual temperature control properties into a single class.  These properties also have the ability to be cached.
class dptf_export TemperatureControlFacade : public TemperatureControlFacadeInterface
{
public:
	TemperatureControlFacade(
		UIntN participantIndex,
		UIntN domainIndex,
		const DomainProperties& domainProperties,
		const PolicyServicesInterfaceContainer& policyServices);
	~TemperatureControlFacade();

	virtual Temperature getCurrentTemperature() override;
	virtual TemperatureThresholds getTemperatureNotificationThresholds() override;
	virtual void setTemperatureNotificationThresholds(const Temperature& lowerBound, const Temperature& upperBound)
		override;
	virtual Bool supportsTemperatureControls() override;
	virtual Bool supportsTemperatureThresholds() override;
	virtual Temperature getPowerShareTemperatureThreshold() override;
	virtual Bool isVirtualTemperatureControl() override;
	virtual DptfBuffer getCalibrationTable() override;
	virtual DptfBuffer getPollingTable() override;
	virtual void setVirtualTemperature(const Temperature& temperature) override;
	virtual void refreshHysteresis() override;

private:
	// services
	PolicyServicesInterfaceContainer m_policyServices;

	// control properties
	UIntN m_participantIndex;
	UIntN m_domainIndex;
	DomainProperties m_domainProperties;

	// cached values
	CachedValue<TemperatureThresholds> m_temperatureThresholds;
	CachedValue<Bool> m_isVirtualSensor;

	Temperature getHysteresis();
};
