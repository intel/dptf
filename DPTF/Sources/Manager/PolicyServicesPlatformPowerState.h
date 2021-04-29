/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include "PolicyServices.h"
#include "PlatformPowerStateInterface.h"
#include "esif_sdk_data_misc.h"
#include "esif_ccb_thread.h"

class PolicyServicesPlatformPowerState final : public PolicyServices, public PlatformPowerStateInterface
{
public:
	PolicyServicesPlatformPowerState(DptfManagerInterface* dptfManager, UIntN policyIndex);
	virtual ~PolicyServicesPlatformPowerState();
	esif_data_complex_thermal_event* getThermalEventPtr(void);
	void setThermalEvent(
		const Temperature currentTemperature,
		const Temperature tripPointTemperature,
		const std::string participantName);

	virtual void sleep(void) override final;
	virtual void hibernate(
		const Temperature& currentTemperature,
		const Temperature& tripPointTemperature,
		const std::string& participantName) override final;
	virtual void shutDown(
		const Temperature& currentTemperature,
		const Temperature& tripPointTemperature,
		const std::string& participantName) override final;

private:
	esif_data_complex_thermal_event m_thermalEvent;
	esif_thread_t m_thread;
};
