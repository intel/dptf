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
#include "DomainBatteryStatusInterface.h"
#include "ControlBase.h"
#include "ParticipantServicesInterface.h"
#include "ParticipantActivityLoggingInterface.h"

class DomainBatteryStatusBase : public ControlBase,
								public DomainBatteryStatusInterface,
								public ParticipantActivityLoggingInterface
{
public:
	DomainBatteryStatusBase(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainBatteryStatusBase();
	virtual void onClearCachedData(void) override;

private:
	void bindRequestHandlers();

	DptfRequestResult handleGetMaxBatteryPower(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetBatteryStatus(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetBatteryInformation(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetChargerType(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetBatterySteadyState(const PolicyRequest& policyRequest);
	DptfRequestResult handleClearCachedData(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetBatteryHighFrequencyImpedance(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetBatteryNoLoadVoltage(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetBatteryMaxPeakCurrent(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetBatteryPercentage(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetBatteryPercentage(const PolicyRequest& policyRequest);
};
