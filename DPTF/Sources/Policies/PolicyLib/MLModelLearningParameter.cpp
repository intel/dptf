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

#include "MLModelLearningParameter.h"
#include "StringConverter.h"

using namespace std;

MLModelLearningParameter::MLModelLearningParameter(
	const string& learningRate,
	const string& smoothingFactor,
	const string& tempOffset,
	const string& batchSize)
	: learningRate(StringConverter::toDouble(learningRate))
	, smoothingFactor(StringConverter::toDouble(smoothingFactor))
	, tempOffset(StringConverter::toDouble(tempOffset))
	, batchSize(StringConverter::toUInt32(batchSize))
{
}

bool MLModelLearningParameter::operator==(const MLModelLearningParameter& other) const
{
	if (learningRate == other.learningRate
		&& smoothingFactor == other.smoothingFactor
		&& tempOffset == other.tempOffset
		&& batchSize == other.batchSize)
	{
		return true;
	}

	return false;
}
