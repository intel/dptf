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

#include "DataFormat.h"
using namespace std;

unsigned int DataFormat::calculateChecksum(const vector<unsigned char>& payload)
{
	constexpr unsigned int FnvPrime = 16777619;
	unsigned int hashValue = 0;
	for (const auto byte : payload)
	{
		hashValue = hashValue ^ byte;
		hashValue = hashValue * FnvPrime;
	}
	return hashValue;
}
