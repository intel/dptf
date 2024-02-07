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

struct InputLayerNode
{
	InputLayerNode(const std::vector<double>& weights);
	~InputLayerNode();

	void updateW1DerivativeMA(double addVal);
	void updateW2DerivativeMA(double addVal);
	void updateW3DerivativeMA(double addVal);
	void zeroGrad();

	double w1;
	double w2;
	double w3;
	double w1DerivativeMovingAverage;
	double w2DerivativeMovingAverage;
	double w3DerivativeMovingAverage;
};

