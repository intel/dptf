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
#include "DomainSystemPowerControlInterface.h"
#include "ControlBase.h"
#include "ParticipantServicesInterface.h"
#include "ParticipantActivityLoggingInterface.h"

class DomainSystemPowerControlBase : public ControlBase,
									 public DomainSystemPowerControlInterface,
									 public ParticipantActivityLoggingInterface
{
	friend class SystemPowerControlState;

public:
	DomainSystemPowerControlBase(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainSystemPowerControlBase();

protected:
	void updateEnabled(PsysPowerLimitType::Type limitType);
	void setEnabled(PsysPowerLimitType::Type limitType, Bool enable);
	Bool isEnabled(PsysPowerLimitType::Type limitType) const;

	Bool m_pl1Enabled;
	Bool m_pl2Enabled;
	Bool m_pl3Enabled;
};
