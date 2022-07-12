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

#pragma once

#include "Dptf.h"
#include "ControlBase.h"
#include "ParticipantServicesInterface.h"
#include "ParticipantActivityLoggingInterface.h"
#include "RequestHandlerInterface.h"
#include <functional>
#include "ActiveControlStaticCaps.h"
#include "ActiveControlStatus.h"
#include "ActiveControlSet.h"
#include "ActiveControlDynamicCaps.h"
#include "ArbitratorFanSpeed.h"
#include "ArbitratorActiveControlCapabilities.h"

class DomainActiveControlBase : public ControlBase, public ParticipantActivityLoggingInterface
{
public:
	DomainActiveControlBase(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainActiveControlBase();

protected:
	virtual DptfBuffer getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual DptfBuffer getActiveControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual DptfBuffer getActiveControlStatus(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual DptfBuffer getActiveControlSet(UIntN participantIndex, UIntN domainIndex) = 0;
	virtual void setActiveControl(UIntN participantIndex, UIntN domainIndex, const Percentage& fanSpeed) = 0;
	virtual void setActiveControlFanDirection(UInt32 fanDirection) = 0;
	virtual void setActiveControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		ActiveControlDynamicCaps newCapabilities) = 0;
	virtual void setFanCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) = 0;
	virtual std::shared_ptr<XmlNode> getArbitratorXml(UIntN policyIndex) const override;

private:
	void bindRequestHandlers();
	DptfRequestResult handleGetStaticCaps(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetDynamicCaps(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetStatus(const PolicyRequest& policyRequest);
	DptfRequestResult handleGetControlSet(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetFanSpeed(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetFanDirection(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetDynamicCaps(const PolicyRequest& policyRequest);
	DptfRequestResult handleSetFanCapsLock(const PolicyRequest& policyRequest);
	DptfRequestResult handleRemovePolicyRequests(const PolicyRequest& policyRequest);

	ArbitratorFanSpeed m_arbitratorFanSpeed;
	ArbitratorActiveControlCapabilities m_arbitratorDynamicCaps;
};
