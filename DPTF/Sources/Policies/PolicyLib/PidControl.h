/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "PidControlInterface.h"
#include "Dptf.h"

class dptf_export PidControl : public PidControlInterface
{
public:
	PidControl(Int32 pidTarget, double alpha, double kp, double ki);
	virtual ~PidControl();

	virtual Int32 getPidBudget(Int32 previousPidBudget, Int32 power) override;
	virtual double getIterm(double previousIterm, Int32 pidBudget, TimeSpan itermCalculationTimeDelta) override;
	virtual Int32 getAvailableHeadroom(Int32 pidBudget, double iterm) override;
	virtual Int32 getPidBudgetSlope(
		Int32 previousPidBudget,
		Int32 pidBudget,
		double weightedAverage,
		Int32 previousPidBudgetSlope) override;
	virtual void setVariables(Int32 pidTarget, double alpha, double kp, double ki) override;
	virtual void setPidTarget(Int32 pidTarget) override;

private:
	Int32 m_pidTarget;
	double m_alpha;
	double m_kp;
	double m_ki;
};
