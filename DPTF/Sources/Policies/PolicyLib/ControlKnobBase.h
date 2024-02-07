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
#include "PolicyServicesInterfaceContainer.h"

// control knobs simplify the use of limiting controls by providing a uniform interface for limiting or unlimiting
// any control.  the details of each control are hidden from clients.
class dptf_export ControlKnobBase
{
public:
	ControlKnobBase(const PolicyServicesInterfaceContainer& policyServices, UIntN participantIndex, UIntN domainIndex);
	virtual ~ControlKnobBase(void);

	virtual void limit(UIntN target) = 0;
	virtual void unlimit(UIntN target) = 0;
	virtual Bool canLimit(UIntN target) = 0;
	virtual Bool canUnlimit(UIntN target) = 0;
	virtual Bool commitSetting() = 0;
	virtual void clearRequestForTarget(UIntN target) = 0;
	virtual void clearAllRequests() = 0;
	virtual void adjustRequestsToCapabilities() = 0;

protected:
	// accessors for subclasses
	PolicyServicesInterfaceContainer getPolicyServices() const;
	UIntN getParticipantIndex() const;
	UIntN getDomainIndex() const;

private:
	PolicyServicesInterfaceContainer m_policyServices;
	UIntN m_participantIndex;
	UIntN m_domainIndex;
};
