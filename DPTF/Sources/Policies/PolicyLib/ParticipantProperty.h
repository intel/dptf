/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

// base class for properties associated with a participant
class dptf_export ParticipantProperty
{
public:
	ParticipantProperty(UIntN participantIndex, const PolicyServicesInterfaceContainer& policyServices);
	virtual ~ParticipantProperty() = default;

	ParticipantProperty(const ParticipantProperty& other) = default;
	ParticipantProperty& operator=(const ParticipantProperty& other) = default;
	ParticipantProperty(ParticipantProperty&& other) = default;
	ParticipantProperty& operator=(ParticipantProperty&& other) = default;

	virtual Bool supportsProperty() = 0;

protected:
	PolicyServicesInterfaceContainer getPolicyServices() const;
	UIntN getParticipantIndex() const;

private:
	UIntN m_participantIndex;
	PolicyServicesInterfaceContainer m_policyServices;
};
