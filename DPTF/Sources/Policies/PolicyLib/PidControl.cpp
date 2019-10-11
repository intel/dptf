/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
{
}

PidControl::~PidControl()
{
}

Int32 PidControl::getPidBudget(Int32 previousPidBudget, Int32 power)
{
	return (Int32)(((double)previousPidBudget * m_alpha) + ((1 - m_alpha) * (double)(m_pidTarget - power)));
}

double PidControl::getIterm(double previousIterm, Int32 pidBudget, TimeSpan itermCalculationTimeDelta)
{
	return previousIterm + pidBudget * itermCalculationTimeDelta.asSeconds() * m_ki;
}

Int32 PidControl::getAvailableHeadroom(Int32 pidBudget, double iterm)
{
	return (Int32)((double)m_pidTarget + ((double)pidBudget * m_kp) + iterm);
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