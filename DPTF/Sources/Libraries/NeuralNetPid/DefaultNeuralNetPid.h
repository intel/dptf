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

#include <memory>
#include "NeuralNetPid.h"
#include "NeuralNetPidConfig.h"
#include "HiddenLayerNode.h"
#include "InputLayerNode.h"

class DefaultNeuralNetPid: public NeuralNetPid
{
public:
	DefaultNeuralNetPid(const NeuralNetPidConfig& pidConfig);
	~DefaultNeuralNetPid();

	double getLearningRate() const override;
	unsigned int getBatchSize() const override;
	double getTarget() const override;
	double getNetOutput() const override;
	unsigned int getCounterInBatch() const override;
	double getPidValuesClippingFactorLower() const override;
	double getPidValuesClippingFactorUpper() const override;
	double getActualProcessNodeW2() const override;
	double getTargetProcessNodeW2() const override;

	std::vector<double> getInputLayerWeights() const override;
	std::vector<double> getHiddenLayerValues() const override;
	std::vector<double> getHiddenLayerWeights() const override;
	std::vector<double> getHiddenLayerUValues() const override;

	void setLearningRate(double newVal) override;
	void setBatchSize(unsigned int newVal) override;
	void setTarget(double newVal) override;
	void setNetOutput(double newVal) override;
	void setPidValuesClippingFactorLower(double newVal) override;
	void setPidValuesClippingFactorUpper(double newVal) override;
	void setIntegralValue(double newVal) override;
	void setActualProcessNodeW2(double newVal) override;
	void setTargetProcessNodeW2(double newVal) override;
	double execute(double setPoint, double currentProcessVariable, NeuralNetPidExecutionMode::Mode execMode) override;

	void updateWeights() const;
	void zeroGrad() const;
	double forward(double setPoint, double currentProcessVariable) const;
	void backward(double setPoint, double currentProcessVariable) const;
	void backwardPassOutputLayer(double setPoint, double currentProcessVariable) const;
	void backwardPassHiddenLayer(double setPoint, double currentProcessVariable) const;
	
	// for debug
	InputLayerNode& getTargetProcessInputNode() const;
	InputLayerNode& getActualProcessInputNode() const;
	HiddenLayerNode& getHiddenLayerPNode() const;
	HiddenLayerNode& getHiddenLayerINode() const;
	HiddenLayerNode& getHiddenLayerDNode() const;

	std::vector<double> getHiddenLayerDerivativeMovingAverage() const;
	std::vector<double> getInputLayerDerivativeMovingAverage() const;
	///

	static double clip(double val, double lower, double upper);
	static double scale(double val, double scalingFactor);
	static double smooth(double val, double previousValue, double smoothingFactor);

private:
	std::shared_ptr<InputLayerNode> m_targetProcessInputNode;
	std::shared_ptr<InputLayerNode> m_actualProcessInputNode;

	std::shared_ptr<HiddenLayerNode> m_p;
	std::shared_ptr<HiddenLayerNode> m_i;
	std::shared_ptr<HiddenLayerNode> m_d;

	double m_learningRate;
	unsigned int m_batchSize;
	double m_target;
	double m_pidValuesClippingFactorLower;
	double m_pidValuesClippingFactorUpper;
	double m_netOutput;
	unsigned int m_counterInBatch;

	static int sign(double x);
};