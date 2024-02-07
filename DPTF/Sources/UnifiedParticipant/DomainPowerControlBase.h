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
#include "DomainPowerControlInterface.h"
#include "ControlBase.h"
#include "ParticipantServicesInterface.h"
#include "ParticipantActivityLoggingInterface.h"

class DomainPowerControlBase : public ControlBase,
							   public DomainPowerControlInterface,
							   public ParticipantActivityLoggingInterface
{
	friend class PowerControlState;

public:
	DomainPowerControlBase(
		UIntN participantIndex,
		UIntN domainIndex,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface);
	virtual ~DomainPowerControlBase();

	// DomainPowerControl
	virtual void updateSocPowerFloorState(UInt32 socPowerFloorState) = 0;

protected:
	void updateEnabled(PowerControlType::Type controlType);
	void setEnabled(PowerControlType::Type controlType, Bool enable);
	Bool isEnabled(PowerControlType::Type controlType) const;

	Bool m_pl1Enabled;
	Bool m_pl2Enabled;
	Bool m_pl3Enabled;
	Bool m_pl4Enabled;
};
