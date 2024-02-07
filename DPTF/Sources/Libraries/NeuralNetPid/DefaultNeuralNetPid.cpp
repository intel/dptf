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

#include <stdexcept>
#include "DefaultNeuralNetPid.h"

using namespace std;

DefaultNeuralNetPid::DefaultNeuralNetPid(const NeuralNetPidConfig& pidConfig)
	: m_learningRate(pidConfig.learningRate)
	, m_batchSize(pidConfig.batchSize)
	, m_target(pidConfig.target)
	, m_pidValuesClippingFactorLower(pidConfig.pidValuesClippingFactorLower)
	, m_pidValuesClippingFactorUpper(pidConfig.pidValuesClippingFactorUpper)
	, m_netOutput(0)
	, m_counterInBatch(1)
{
	m_targetProcessInputNode = std::make_shared<InputLayerNode>(pidConfig.targetProcessInputNodeWeights);
	m_actualProcessInputNode = std::make_shared<InputLayerNode>(pidConfig.actualProcessInputNodeWeights);
	m_p = std::make_shared<HiddenLayerNode>(pidConfig.hiddenLayerWeights[0], pidConfig.derivativeFactor);
	m_i = std::make_shared<HiddenLayerNode>(pidConfig.hiddenLayerWeights[1], pidConfig.derivativeFactor);
	m_d = std::make_shared<HiddenLayerNode>(pidConfig.hiddenLayerWeights[2], pidConfig.derivativeFactor);
}

DefaultNeuralNetPid::~DefaultNeuralNetPid()
{
}

double DefaultNeuralNetPid::getLearningRate() const
{
	return m_learningRate;
}

unsigned int DefaultNeuralNetPid::getBatchSize() const
{
	return m_batchSize;
}

double DefaultNeuralNetPid::getTarget() const
{
	return m_target;
}

double DefaultNeuralNetPid::getNetOutput() const
{
	return m_netOutput;
}

unsigned int DefaultNeuralNetPid::getCounterInBatch() const
{
	return m_counterInBatch;
}

double DefaultNeuralNetPid::getPidValuesClippingFactorLower() const
{
	return m_pidValuesClippingFactorLower;
}
double DefaultNeuralNetPid::getPidValuesClippingFactorUpper() const
{
	return m_pidValuesClippingFactorUpper;
}

double DefaultNeuralNetPid::getActualProcessNodeW2() const
{
	return m_actualProcessInputNode->w2;
}
double DefaultNeuralNetPid::getTargetProcessNodeW2() const
{
	return m_targetProcessInputNode->w2;
}

void DefaultNeuralNetPid::setLearningRate(double newVal)
{
	m_learningRate = newVal;
}
void DefaultNeuralNetPid::setBatchSize(unsigned int newVal)
{
	m_batchSize = newVal;
}
void DefaultNeuralNetPid::setTarget(double newVal)
{
	m_target = newVal;
}

void DefaultNeuralNetPid::setNetOutput(double newVal)
{
	m_netOutput = newVal;
}

void DefaultNeuralNetPid::setPidValuesClippingFactorLower(double newVal)
{
	m_pidValuesClippingFactorLower = newVal;
}
void DefaultNeuralNetPid::setPidValuesClippingFactorUpper(double newVal)
{
	m_pidValuesClippingFactorUpper = newVal;
}

void DefaultNeuralNetPid::setIntegralValue(double newVal)
{
	m_i->value = newVal;
}

void DefaultNeuralNetPid::setActualProcessNodeW2(double newVal)
{
	m_actualProcessInputNode->w2 = newVal;
}

void DefaultNeuralNetPid::setTargetProcessNodeW2(double newVal)
{
	m_targetProcessInputNode->w2 = newVal;
}

//Update parameters based on Gradient Decent
void DefaultNeuralNetPid::updateWeights() const
{
	m_p->k = m_p->k + m_learningRate * m_p->kDerivativeMovingAverage;
	m_i->k = m_i->k + m_learningRate * m_i->kDerivativeMovingAverage;
	m_d->k = m_d->k + m_learningRate * m_d->kDerivativeMovingAverage;

	m_actualProcessInputNode->w1 = m_actualProcessInputNode->w1 +
		m_learningRate * m_actualProcessInputNode->w1DerivativeMovingAverage;
	m_actualProcessInputNode->w2 = m_actualProcessInputNode->w2 +
		m_learningRate * m_actualProcessInputNode->w2DerivativeMovingAverage;
	m_actualProcessInputNode->w3 = m_actualProcessInputNode->w3 +
		m_learningRate * m_actualProcessInputNode->w3DerivativeMovingAverage;
	
	m_targetProcessInputNode->w1 = m_targetProcessInputNode->w1 +
		m_learningRate * m_targetProcessInputNode->w1DerivativeMovingAverage;
	m_targetProcessInputNode->w2 = m_targetProcessInputNode->w2 +
		m_learningRate * m_targetProcessInputNode->w2DerivativeMovingAverage;
	m_targetProcessInputNode->w3 = m_targetProcessInputNode->w3 +
		m_learningRate * m_targetProcessInputNode->w3DerivativeMovingAverage;
}

void DefaultNeuralNetPid::zeroGrad() const
{
	m_p->zeroGrad();
	m_i->zeroGrad();
	m_d->zeroGrad();
	m_targetProcessInputNode->zeroGrad();
	m_actualProcessInputNode->zeroGrad();
}

double DefaultNeuralNetPid::forward(double setPoint, double currentProcessVariable) const
{
	m_p->updateU(m_targetProcessInputNode->w1 * setPoint + m_actualProcessInputNode->w1 * currentProcessVariable);
	m_i->updateU(m_targetProcessInputNode->w2 * setPoint + m_actualProcessInputNode->w2 * currentProcessVariable);
	m_d->updateU(m_targetProcessInputNode->w3 * setPoint + m_actualProcessInputNode->w3 * currentProcessVariable);

	
	m_p->updateNode(clip(m_p->currU, m_pidValuesClippingFactorLower, m_pidValuesClippingFactorUpper));
	m_i->updateNode(clip(m_i->value + m_i->currU, m_pidValuesClippingFactorLower, m_pidValuesClippingFactorUpper));
	m_d->updateNode(clip(m_d->currU - m_d->prevU, m_pidValuesClippingFactorLower, m_pidValuesClippingFactorUpper));
	
	const double netOutput = m_p->k * m_p->value +
		m_i->k * m_i->value +
		m_d->k * m_d->value;
	
	return netOutput;
}

void DefaultNeuralNetPid::backward(double setPoint, double currentProcessVariable) const
{
	backwardPassOutputLayer(setPoint, currentProcessVariable);
	backwardPassHiddenLayer(setPoint, currentProcessVariable);
}

double DefaultNeuralNetPid::execute(
	double setPoint,
	double currentProcessVariable,
	NeuralNetPidExecutionMode::Mode execMode)
{
	
	double netOut = m_netOutput;
	switch (execMode)
	{
	case NeuralNetPidExecutionMode::InferenceAndLearn:
		if (m_counterInBatch % int(m_batchSize) == 0)
		{
			updateWeights();
			zeroGrad();
			m_counterInBatch = 1;
		}
		else
		{
			m_counterInBatch++;
		}
		netOut = forward(setPoint, currentProcessVariable);
		backward(setPoint, currentProcessVariable);
		break;

	case NeuralNetPidExecutionMode::InferenceOnly:
		netOut = forward(setPoint, currentProcessVariable);
		break;

	case NeuralNetPidExecutionMode::NoOp:	// get the previous value
		break;

	default:
		throw runtime_error("Unexpected invalid NeuralNetPidExecutionMode");
		break;
	}
	m_netOutput = netOut;

	return m_netOutput;
}

void DefaultNeuralNetPid::backwardPassOutputLayer(double setPoint, double currentProcessVariable) const
{
	const double der1 = setPoint - currentProcessVariable;
	const double der2 = 1;
	const double batchSize = (double)m_batchSize; // have the value in double to avoid calculation errors

	//Calculate derivatives
	m_p->updateKDerivativeMA((1 / batchSize) * (der1 * der2 * m_p->value));
	m_i->updateKDerivativeMA((1 / batchSize) * (der1 * der2 * m_i->value));
	m_d->updateKDerivativeMA((1 / batchSize) * (der1 * der2 * m_d->value));
}

void DefaultNeuralNetPid::backwardPassHiddenLayer(double setPoint, double currentProcessVariable) const
{
	const double der1 = setPoint - currentProcessVariable;
	const double der2 = 1;

	const double batchSize = (double)m_batchSize; // have the value in double to avoid calculation errors
	constexpr double p_derivative = 1;
	const double i_derivative = sign((m_i->value - m_i->prev + m_i->derivativeFactor) *
		(m_i->currU - m_i->prevU + m_i->derivativeFactor));
	const double d_derivative = sign((m_d->value - m_d->prev + m_d->derivativeFactor) *
		(m_d->currU - m_d->prevU + m_d->derivativeFactor));

	m_actualProcessInputNode->updateW1DerivativeMA((1 / batchSize) * der1 * der2 * p_derivative * m_p->k * currentProcessVariable);
	m_actualProcessInputNode->updateW2DerivativeMA((1 / batchSize) * der1 * der2 * i_derivative * m_i->k * currentProcessVariable);
	m_actualProcessInputNode->updateW3DerivativeMA((1 / batchSize) * der1 * der2 * d_derivative * m_d->k * currentProcessVariable);

	m_targetProcessInputNode->updateW1DerivativeMA((1 / batchSize) * der1 * der2 * p_derivative * m_p->k * setPoint);
	m_targetProcessInputNode->updateW2DerivativeMA((1 / batchSize) * der1 * der2 * i_derivative * m_i->k * setPoint);
	m_targetProcessInputNode->updateW3DerivativeMA((1 / batchSize) * der1 * der2 * d_derivative * m_d->k * setPoint);
}

InputLayerNode& DefaultNeuralNetPid::getTargetProcessInputNode() const
{
	return *m_targetProcessInputNode;
}

InputLayerNode& DefaultNeuralNetPid::getActualProcessInputNode() const
{
	return *m_actualProcessInputNode;
}

HiddenLayerNode& DefaultNeuralNetPid::getHiddenLayerPNode() const
{
	return *m_p;
}

HiddenLayerNode& DefaultNeuralNetPid::getHiddenLayerINode() const
{
	return *m_i;
}

HiddenLayerNode& DefaultNeuralNetPid::getHiddenLayerDNode() const
{
	return *m_d;
}

std::vector<double> DefaultNeuralNetPid::getInputLayerWeights() const
{
	return {
		m_targetProcessInputNode->w1,
		m_targetProcessInputNode->w2,
		m_targetProcessInputNode->w3, 
		m_actualProcessInputNode->w1,
		m_actualProcessInputNode->w2,
		m_actualProcessInputNode->w3
	};
}

std::vector<double> DefaultNeuralNetPid::getHiddenLayerWeights() const
{
	return { m_p->k, m_i->k, m_d->k};
}

std::vector<double> DefaultNeuralNetPid::getHiddenLayerValues() const
{
	return { m_p->value, m_i->value, m_d->value };
}

std::vector<double> DefaultNeuralNetPid::getHiddenLayerDerivativeMovingAverage() const
{
	return {
		m_p->kDerivativeMovingAverage,
		m_i->kDerivativeMovingAverage,
		m_d->kDerivativeMovingAverage
	};
}

std::vector<double> DefaultNeuralNetPid::getInputLayerDerivativeMovingAverage() const
{
	return {
		m_targetProcessInputNode->w1DerivativeMovingAverage,
		m_targetProcessInputNode->w2DerivativeMovingAverage,
		m_targetProcessInputNode->w3DerivativeMovingAverage,
		m_actualProcessInputNode->w1DerivativeMovingAverage,
		m_actualProcessInputNode->w2DerivativeMovingAverage,
		m_actualProcessInputNode->w3DerivativeMovingAverage
	};
}

std::vector<double> DefaultNeuralNetPid::getHiddenLayerUValues() const
{
	return {
		m_p->currU,
		m_p->currU,
		m_i->currU,
		m_i->prevU,
		m_d->prevU,
		m_d->prevU
	};
}

double DefaultNeuralNetPid::clip(double val, double lower, double upper)
{
	return (std::max)(lower, (std::min)(val, upper));
}

double DefaultNeuralNetPid::scale(double value, double scalingFactor)
{
	return value * scalingFactor;
}

double DefaultNeuralNetPid::smooth(double value, double previousValue, double smoothingFactor)
{
	return smoothingFactor * value + (1 - smoothingFactor) * previousValue;
}

int DefaultNeuralNetPid::sign(double x)
{
	return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
}