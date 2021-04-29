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

#include "Dptf.h"

class dptf_export PidControlInterface
{
public:
	virtual ~PidControlInterface(){};

	virtual Int32 getPrevPidBudget() = 0;
	virtual void setPrevPidBudget(Int32 prevPidBudget) = 0;
	virtual Int32 getPidBudget() = 0;
	virtual void setPidBudget(Int32 pidBudget) = 0;
	virtual void calculatePidBudget(Int32 power) = 0;
	virtual double getPrevIterm() = 0;
	virtual void setPrevIterm(double prevIterm) = 0;
	virtual double getIterm() = 0;
	virtual void setIterm(double iterm) = 0;
	virtual void calculateIterm(TimeSpan itermCalculationTimeDelta) = 0;
	virtual TimeSpan getPrevItermCalculationStartTime() = 0;
	virtual void setPrevItermCalculationStartTime(TimeSpan prevItermCalculationStartTime) = 0;
	virtual TimeSpan getItermCalculationStartTime() = 0;
	virtual void setItermCalculationStartTime(TimeSpan itermCalculationStartTime) = 0;
	virtual Int32 getAvailableHeadroom() = 0;
	virtual void setAvailableHeadroom(Int32 availableHeadroom) = 0;
	virtual void calculateAvailableHeadroom() = 0;
	virtual Int32 getPidBudgetSlope(
		Int32 previousPidBudget,
		Int32 pidBudget,
		double weightedAverage,
		Int32 previousPidBudgetSlope) = 0;
	virtual void setVariables(Int32 pidTarget, double alpha, double kp, double ki) = 0;
	virtual void setPidTarget(Int32 pidTarget) = 0;
};
