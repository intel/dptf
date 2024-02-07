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

#include "InputLayerNode.h"

InputLayerNode::InputLayerNode(const std::vector<double>& weights)
	: w1(weights[0])
	, w2(weights[1])
	, w3(weights[2])
	, w1DerivativeMovingAverage(0)
	, w2DerivativeMovingAverage(0)
	, w3DerivativeMovingAverage(0)
{
}

InputLayerNode::~InputLayerNode()
{
}

void InputLayerNode::updateW1DerivativeMA(double addVal)
{
	w1DerivativeMovingAverage += addVal;
}

void InputLayerNode::updateW2DerivativeMA(double addVal)
{
	w2DerivativeMovingAverage += addVal;
}

void InputLayerNode::updateW3DerivativeMA(double addVal)
{
	w3DerivativeMovingAverage += addVal;
}

void InputLayerNode::zeroGrad()
{
	w1DerivativeMovingAverage = 0;
	w2DerivativeMovingAverage = 0;
	w3DerivativeMovingAverage = 0;
}
