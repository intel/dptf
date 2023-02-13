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

#include "SystemPowerControlState.h"

SystemPowerControlState::SystemPowerControlState(DomainSystemPowerControlBase* control)
	: m_control(control)
	, m_pl1Enabled(false)
	, m_pl2Enabled(false)
	, m_pl3Enabled(false)
	, m_enablesValid(false)
	, m_pl1LimitValid(false)
	, m_pl2LimitValid(false)
	, m_pl3LimitValid(false)
	, m_pl1TimeWindowValid(false)
	, m_pl3TimeWindowValid(false)
	, m_pl3DutyCycleValid(false)
{
}

SystemPowerControlState::~SystemPowerControlState()
{
}

void SystemPowerControlState::capture()
{
	captureEnables();
	captureLimit(PsysPowerLimitType::PSysPL1, m_pl1Limit, m_pl1LimitValid);
	captureLimit(PsysPowerLimitType::PSysPL2, m_pl2Limit, m_pl2LimitValid);
	captureLimit(PsysPowerLimitType::PSysPL3, m_pl3Limit, m_pl3LimitValid);
	captureTimeWindow(PsysPowerLimitType::PSysPL1, m_pl1TimeWindow, m_pl1TimeWindowValid);
	captureTimeWindow(PsysPowerLimitType::PSysPL3, m_pl3TimeWindow, m_pl3TimeWindowValid);
	captureDutyCycle(PsysPowerLimitType::PSysPL3, m_pl3DutyCycle, m_pl3DutyCycleValid);
}

void SystemPowerControlState::restore()
{
	restoreLimit(PsysPowerLimitType::PSysPL1, m_pl1Limit, m_pl1LimitValid);
	restoreLimit(PsysPowerLimitType::PSysPL2, m_pl2Limit, m_pl2LimitValid);
	restoreLimit(PsysPowerLimitType::PSysPL3, m_pl3Limit, m_pl3LimitValid);
	restoreTimeWindow(PsysPowerLimitType::PSysPL1, m_pl1TimeWindow, m_pl1TimeWindowValid);
	restoreTimeWindow(PsysPowerLimitType::PSysPL3, m_pl3TimeWindow, m_pl3TimeWindowValid);
	restoreDutyCycle(PsysPowerLimitType::PSysPL3, m_pl3DutyCycle, m_pl3DutyCycleValid);
	restoreEnables();
}

void SystemPowerControlState::restoreEnables()
{
	if (m_enablesValid)
	{
		m_control->setEnabled(PsysPowerLimitType::PSysPL1, m_pl1Enabled);
		m_control->setEnabled(PsysPowerLimitType::PSysPL2, m_pl2Enabled);
		m_control->setEnabled(PsysPowerLimitType::PSysPL3, m_pl3Enabled);
	}
}

void SystemPowerControlState::captureEnables()
{
	m_control->updateEnabled(PsysPowerLimitType::PSysPL1);
	m_pl1Enabled = m_control->isEnabled(PsysPowerLimitType::PSysPL1);
	m_control->updateEnabled(PsysPowerLimitType::PSysPL2);
	m_pl2Enabled = m_control->isEnabled(PsysPowerLimitType::PSysPL2);
	m_control->updateEnabled(PsysPowerLimitType::PSysPL3);
	m_pl3Enabled = m_control->isEnabled(PsysPowerLimitType::PSysPL3);
	m_enablesValid = true;
}

void SystemPowerControlState::captureLimit(PsysPowerLimitType::Type limitType, Power& limit, Bool& limitValid)
{
	try
	{
		limit =
			m_control->getSystemPowerLimit(m_control->getParticipantIndex(), m_control->getDomainIndex(), limitType);
		limitValid = true;
	}
	catch (...)
	{
		limitValid = false;
	}
}

void SystemPowerControlState::captureTimeWindow(
	PsysPowerLimitType::Type limitType,
	TimeSpan& timeWindow,
	Bool& timeWindowValid)
{
	try
	{
		timeWindow = m_control->getSystemPowerLimitTimeWindow(
			m_control->getParticipantIndex(), m_control->getDomainIndex(), limitType);
		timeWindowValid = true;
	}
	catch (...)
	{
		timeWindowValid = false;
	}
}

void SystemPowerControlState::captureDutyCycle(
	PsysPowerLimitType::Type limitType,
	Percentage& dutyCycle,
	Bool& dutyCycleValid)
{
	try
	{
		dutyCycle = m_control->getSystemPowerLimitDutyCycle(
			m_control->getParticipantIndex(), m_control->getDomainIndex(), limitType);
		dutyCycleValid = true;
	}
	catch (...)
	{
		dutyCycleValid = false;
	}
}

void SystemPowerControlState::restoreLimit(
	PsysPowerLimitType::Type limitType,
	const Power& limit,
	const Bool& limitValid)
{
	if (limitValid)
	{
		try
		{
			m_control->setSystemPowerLimit(
				m_control->getParticipantIndex(), m_control->getDomainIndex(), limitType, limit);
		}
		catch (...)
		{
		}
	}
}

void SystemPowerControlState::restoreTimeWindow(
	PsysPowerLimitType::Type limitType,
	const TimeSpan& timeWindow,
	const Bool& timeWindowValid)
{
	if (timeWindowValid)
	{
		try
		{
			m_control->setSystemPowerLimitTimeWindow(
				m_control->getParticipantIndex(), m_control->getDomainIndex(), limitType, timeWindow);
		}
		catch (...)
		{
		}
	}
}

void SystemPowerControlState::restoreDutyCycle(
	PsysPowerLimitType::Type limitType,
	const Percentage& dutyCycle,
	const Bool& dutyCycleValid)
{
	if (dutyCycleValid)
	{
		try
		{
			m_control->setSystemPowerLimitDutyCycle(
				m_control->getParticipantIndex(), m_control->getDomainIndex(), limitType, dutyCycle);
		}
		catch (...)
		{
		}
	}
}
