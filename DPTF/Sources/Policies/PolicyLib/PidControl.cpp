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

#include "PidControl.h"
#include "StatusFormat.h"
#include "PolicyLogger.h"

using namespace std;
using namespace StatusFormat;

PidControl::PidControl(Int32 pidTarget, double alpha, double kp, double ki)
	: m_pidTarget(pidTarget)
	, m_alpha(alpha)
	, m_kp(kp)
	, m_ki(ki)
	, m_pidBudget(Constants::Invalid)
	, m_prevPidBudget(Constants::Invalid)
	, m_iterm(0.0)
	, m_prevIterm(0.0)
	, m_prevItermCalculationStartTime(TimeSpan::createInvalid())
	, m_itermCalculationStartTime(TimeSpan::createInvalid())
	, m_availableHeadroom(Constants::MaxInt32)
{
}

PidControl::~PidControl()
{
}

Int32 PidControl::getPrevPidBudget()
{
	return m_prevPidBudget;
}

void PidControl::setPrevPidBudget(Int32 prevPidBudget)
{
	m_prevPidBudget = prevPidBudget;
}

Int32 PidControl::getPidBudget()
{
	return m_pidBudget;
}

void PidControl::setPidBudget(Int32 pidBudget)
{
	m_pidBudget = pidBudget;
}

void PidControl::calculatePidBudget(Int32 power)
{
	m_pidBudget = (Int32)(((double)m_prevPidBudget * m_alpha) + ((1 - m_alpha) * (double)(m_pidTarget - power)));
}

double PidControl::getPrevIterm()
{
	return m_prevIterm;
}

void PidControl::setPrevIterm(double prevIterm)
{
	m_prevIterm = prevIterm;
}

double PidControl::getIterm()
{
	return m_iterm;
}

void PidControl::setIterm(double iterm)
{
	m_iterm = iterm;
}

void PidControl::calculateIterm(TimeSpan itermCalculationTimeDelta)
{
	m_iterm = m_prevIterm + m_pidBudget * itermCalculationTimeDelta.asSeconds() * m_ki;
}

TimeSpan PidControl::getPrevItermCalculationStartTime()
{
	return m_prevItermCalculationStartTime;
}

void PidControl::setPrevItermCalculationStartTime(TimeSpan prevItermCalculationStartTime)
{
	m_prevItermCalculationStartTime = prevItermCalculationStartTime;
}

TimeSpan PidControl::getItermCalculationStartTime()
{
	return m_itermCalculationStartTime;
}

void PidControl::setItermCalculationStartTime(TimeSpan itermCalculationStartTime)
{
	m_itermCalculationStartTime = itermCalculationStartTime;
}

Int32 PidControl::getAvailableHeadroom()
{
	return m_availableHeadroom;
}

void PidControl::setAvailableHeadroom(Int32 availableHeadroom)
{
	m_availableHeadroom = availableHeadroom;
}

void PidControl::calculateAvailableHeadroom()
{
	m_availableHeadroom = (Int32)((double)m_pidTarget + ((double)m_pidBudget * m_kp) + m_iterm);
}

Int32 PidControl::getPidBudgetSlope(
	Int32 previousPidBudget,
	Int32 pidBudget,
	double weightedAverage,
	Int32 previousPidBudgetSlope)
{
	return (Int32)((weightedAverage * (double)(pidBudget - previousPidBudget) + ((1 - weightedAverage) * ((double)previousPidBudgetSlope))));
}

void PidControl::setVariables(Int32 pidTarget, double alpha, double kp, double ki)
{
	m_pidTarget = pidTarget;
	m_alpha = alpha;
	m_kp = kp;
	m_ki = ki;
}

void PidControl::setPidTarget(Int32 pidTarget)
{
	m_pidTarget = pidTarget;
}