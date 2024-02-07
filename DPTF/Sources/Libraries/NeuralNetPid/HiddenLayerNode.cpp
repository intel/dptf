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

#include "HiddenLayerNode.h"

HiddenLayerNode::HiddenLayerNode(double weightValue, double derivativeFactor)
	: value(0)
	, prev(0)
	, k(weightValue)
	, kDerivativeMovingAverage(0)
	, currU(0)
	, prevU(0)
	, derivativeFactor(derivativeFactor)
{
}

HiddenLayerNode::~HiddenLayerNode() 
{
}

void HiddenLayerNode::zeroGrad()
{
	kDerivativeMovingAverage = 0;
}

void HiddenLayerNode::updateNode(double newVal)
{
	prev = value;
	value = newVal;
}

void HiddenLayerNode::updateU(double newVal)
{
	prevU = currU;
	currU = newVal;
}

void HiddenLayerNode::updateKDerivativeMA(double addVal)
{
	kDerivativeMovingAverage+= addVal;
}
