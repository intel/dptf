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

#include "DomainSystemPowerControlBase.h"

DomainSystemPowerControlBase::DomainSystemPowerControlBase(
	UIntN participantIndex,
	UIntN domainIndex,
	std::shared_ptr<ParticipantServicesInterface> participantServicesInterface)
	: ControlBase(participantIndex, domainIndex, participantServicesInterface)
	, m_pl1Enabled(false)
	, m_pl2Enabled(false)
	, m_pl3Enabled(false)
{
}

DomainSystemPowerControlBase::~DomainSystemPowerControlBase()
{
}

void DomainSystemPowerControlBase::updateEnabled(PsysPowerLimitType::Type limitType)
{
	try
	{
		UInt32 plEnabled = getParticipantServices()->primitiveExecuteGetAsUInt32(
			GET_PLATFORM_POWER_LIMIT_ENABLE, getDomainIndex(), (UInt8)limitType);
		Bool enabled = plEnabled > 0 ? true : false;

		switch (limitType)
		{
		case PsysPowerLimitType::PSysPL1:
			m_pl1Enabled = enabled;
			break;
		case PsysPowerLimitType::PSysPL2:
			m_pl2Enabled = enabled;
			break;
		case PsysPowerLimitType::PSysPL3:
			m_pl3Enabled = enabled;
			break;
		default:
			break;
		}
	}
	catch (...)
	{
	}
}

void DomainSystemPowerControlBase::setEnabled(PsysPowerLimitType::Type limitType, Bool enable)
{
	try
	{
		getParticipantServices()->primitiveExecuteSetAsUInt32(
			SET_PLATFORM_POWER_LIMIT_ENABLE, enable ? (UInt32)1 : (UInt32)0, getDomainIndex(), (UInt8)limitType);
	}
	catch (...)
	{
	}
}

Bool DomainSystemPowerControlBase::isEnabled(PsysPowerLimitType::Type limitType) const
{
	switch (limitType)
	{
	case PsysPowerLimitType::PSysPL1:
		return m_pl1Enabled;
	case PsysPowerLimitType::PSysPL2:
		return m_pl2Enabled;
	case PsysPowerLimitType::PSysPL3:
		return m_pl3Enabled;
	default:
		return false;
	}
}
