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
#include "PolicyServices.h"
#include "DomainDisplayControlInterface.h"

class PolicyServicesDomainDisplayControl final : public PolicyServices, public DomainDisplayControlInterface
{
public:
	PolicyServicesDomainDisplayControl(DptfManagerInterface* dptfManager, UIntN policyIndex);

	virtual DisplayControlDynamicCaps getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
		override final;
	virtual DisplayControlStatus getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex) override final;
	virtual UIntN getUserPreferredDisplayIndex(UIntN participantIndex, UIntN domainIndex) override final;
	virtual UIntN getUserPreferredSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex) override final;
	virtual Bool isUserPreferredIndexModified(UIntN participantIndex, UIntN domainIndex) override final;
	virtual UIntN getSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex) override final;
	virtual DisplayControlSet getDisplayControlSet(UIntN participantIndex, UIntN domainIndex) override final;
	virtual void setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex) override final;
	virtual void setSoftBrightness(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex) override final;
	virtual void updateUserPreferredSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex) override final;
	virtual void restoreUserPreferredSoftBrightness(UIntN participantIndex, UIntN domainIndex) override final;
	virtual void setDisplayControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		DisplayControlDynamicCaps newCapabilities) override final;
	virtual void setDisplayCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override final;
};
