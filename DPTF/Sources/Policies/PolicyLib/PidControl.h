/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

	virtual Int32 getPrevPidBudget() override;
	virtual void setPrevPidBudget(Int32 prevPidBudget) override;
	virtual Int32 getPidBudget() override;
	virtual void setPidBudget(Int32 pidBudget) override;
	virtual void calculatePidBudget(Int32 power) override;
	virtual double getPrevIterm() override;
	virtual void setPrevIterm(double prevIterm) override;
	virtual double getIterm() override;
	virtual void setIterm(double iterm) override;
	virtual void calculateIterm(TimeSpan itermCalculationTimeDelta) override;
	virtual TimeSpan getPrevItermCalculationStartTime() override;
	virtual void setPrevItermCalculationStartTime(TimeSpan prevItermCalculationStartTime) override;
	virtual TimeSpan getItermCalculationStartTime() override;
	virtual void setItermCalculationStartTime(TimeSpan itermCalculationStartTime) override;
	virtual Int32 getAvailableHeadroom() override;
	virtual void setAvailableHeadroom(Int32 availableHeadroom) override;
	virtual void calculateAvailableHeadroom() override;
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
	Int32 m_pidBudget;
	Int32 m_prevPidBudget;
	double m_iterm;
	double m_prevIterm;
	TimeSpan m_prevItermCalculationStartTime;
	TimeSpan m_itermCalculationStartTime;
	Int32 m_availableHeadroom;
};
