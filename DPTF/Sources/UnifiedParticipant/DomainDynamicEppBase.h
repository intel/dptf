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
#include "DomainDynamicEppInterface.h"
#include "ControlBase.h"
#include "ParticipantServicesInterface.h"
#include "ParticipantActivityLoggingInterface.h"

class DomainDynamicEppBase : public ControlBase,
	public DomainDynamicEppInterface,
	public ParticipantActivityLoggingInterface
{
public:
	DomainDynamicEppBase(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainDynamicEppBase();

	// ParticipantActivityLoggingInterface
	virtual void sendActivityLoggingDataIfEnabled(UIntN participantIndex, UIntN domainIndex) override;

	// ComponentExtendedInterface
	virtual void onClearCachedData() override;

protected:
	Bool m_isDynamicEppSupported;

private:
	void bindRequestHandlers();

	DptfRequestResult handleClearCachedResults(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetEppSensitivityHint(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetDynamicEppSupport(const PolicyRequest& policyRequest) const;
	DptfRequestResult handleSetDynamicEppSupport(const PolicyRequest& policyRequest);
};
