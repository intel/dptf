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

#include <vector>
#include "NeuralNetPidExecutionMode.h"

class NeuralNetPid
{
public:

	NeuralNetPid() = default;
	virtual ~NeuralNetPid() = default;
	NeuralNetPid(const NeuralNetPid & other) = default;
	NeuralNetPid(NeuralNetPid && other) noexcept = default;
	NeuralNetPid& operator=(const NeuralNetPid & other) = default;
	NeuralNetPid& operator=(NeuralNetPid && other) noexcept = default;
	
	virtual double execute(
		double setPoint,
		double currentProcessVariable,
		NeuralNetPidExecutionMode::Mode execMode) = 0;

	virtual double getLearningRate() const = 0;
	virtual unsigned int getBatchSize() const = 0;
	virtual double getTarget() const = 0;
	virtual double getNetOutput() const = 0;
	virtual unsigned int getCounterInBatch() const = 0;
	virtual double getPidValuesClippingFactorLower() const = 0;
	virtual double getPidValuesClippingFactorUpper() const = 0;
	virtual double getActualProcessNodeW2() const = 0;
	virtual double getTargetProcessNodeW2() const = 0;

	//for trace logging
	virtual std::vector<double> getInputLayerWeights() const = 0;
	virtual std::vector<double> getHiddenLayerValues() const = 0;
	virtual std::vector<double> getHiddenLayerWeights() const = 0;
	virtual std::vector<double> getHiddenLayerUValues() const = 0;



	virtual void setLearningRate(double newVal) = 0;
	virtual void setBatchSize(unsigned int newVal) = 0;
	virtual void setTarget(double newVal) = 0;
	virtual void setNetOutput(double newVal) = 0;
	virtual void setPidValuesClippingFactorLower(double newVal) = 0;
	virtual void setPidValuesClippingFactorUpper(double newVal) = 0;
	virtual void setIntegralValue(double newVal) = 0;
	virtual void setActualProcessNodeW2(double newVal) = 0;
	virtual void setTargetProcessNodeW2(double newVal) = 0;

};