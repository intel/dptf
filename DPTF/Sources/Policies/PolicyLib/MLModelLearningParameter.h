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

#include <string>
#include "Dptf.h"

class MLModelLearningParameter
{
public:
	MLModelLearningParameter() = default;
	MLModelLearningParameter(
		const std::string& learningRate,
		const std::string& smoothingFactor,
		const std::string& tempOffset,
		const std::string& batchSize);
	~MLModelLearningParameter() = default;
	bool operator==(const MLModelLearningParameter& other) const;

	double learningRate;
	double smoothingFactor;
	double tempOffset;
	UInt32 batchSize;
};
