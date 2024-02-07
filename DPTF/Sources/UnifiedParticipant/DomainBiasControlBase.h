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
#include "ControlBase.h"
#include "DomainBiasControlInterface.h"
#include "ParticipantServicesInterface.h"
#include "ParticipantActivityLoggingInterface.h"

class DomainBiasControlBase : public ControlBase,
	public DomainBiasControlInterface,
	public ParticipantActivityLoggingInterface
{
public:
	DomainBiasControlBase(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainBiasControlBase() override = default;

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData() override;

protected:
	Bool m_isBiasControlSupported;

private:
	void bindRequestHandlers();

	DptfRequestResult handleClearCachedResults(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetCpuOpboostEnableAC(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetCpuOpboostEnableDC(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetGpuOpboostEnableAC(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetGpuOpboostEnableDC(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetSplitRatio(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetSplitRatioMax(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetCpuOpboostEnableAC(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetCpuOpboostEnableDC(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetGpuOpboostEnableAC(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetGpuOpboostEnableDC(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetSplitRatio(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetSplitRatioActive(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetSplitRatioMax(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetReservedTgp(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetOppBoostMode(const PolicyRequest& policyRequest);
};
