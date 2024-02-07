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
#include "DomainProcessorControlInterface.h"
#include "ControlBase.h"
#include "ParticipantServicesInterface.h"
#include "ParticipantActivityLoggingInterface.h"
#include "ArbitratorProcessorControl.h"

class DomainProcessorControlBase :	public ControlBase,
									public DomainProcessorControlInterface,
									public ParticipantActivityLoggingInterface
{
public:
	DomainProcessorControlBase(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainProcessorControlBase();
	virtual void updatePcieThrottleRequestState(UInt32 pcieThrottleRequestState) = 0;

protected:
	ArbitratorProcessorControl m_arbitrator;

private:
	void bindRequestHandlers();

	DptfRequestResult handleClearCachedData(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetUnderVoltageThreshold(const PolicyRequest& policyRequest);
	DptfRequestResult handleRemovePolicyRequests(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetTccOffsetTemperature(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetMaxTccOffsetTemperature(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetMinTccOffsetTemperature(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetPerfPreferenceMax(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetPerfPreferenceMin(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetPcieThrottleRequestState(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetSocGear(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetSocGear(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetSocSystemUsageMode(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetSocSystemUsageMode(const PolicyRequest& policyRequest);

	DptfRequestResult removePolicySetUnderVoltageThresholdRequest(UIntN policyIndex, const DptfRequest& request);
	DptfRequestResult removePolicySetTccOffsetTemperatureRequest(UIntN policyIndex, const DptfRequest& request);
};
