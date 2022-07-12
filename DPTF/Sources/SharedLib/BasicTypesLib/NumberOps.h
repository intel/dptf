/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

class NumberOps
{
public:
	static UInt64 concatenateFourWords(UInt16 word1, UInt16 word2, UInt16 word3, UInt16 word4)
	{
		return ((UInt64)word1 << 48) | ((UInt64)word2 << 32) | ((UInt64)word3 << 16) | ((UInt64)word4 << 0);
	};

	static UInt16 getWord(UInt64 value, UInt8 wordIndex)
	{
		if (wordIndex > 3)
		{
			throw dptf_exception("NumberOps::getWord(): index is too large");
		}
		UInt64 mask = 0xFFFF;
		UInt8 shiftAmount = wordIndex * 16;
		UInt16 wordValue = (UInt16)((value & (mask << shiftAmount)) >> shiftAmount);
		return wordValue;
	};
};