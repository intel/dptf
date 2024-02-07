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

#include "DttGamingMode.h"
#include "DptfExceptions.h"

using namespace std;

namespace DttGamingMode
{
	std::string toString(Type gamingMode)
	{
		switch (gamingMode)
		{
		case MaxPerformance:
			return "MaxPerformance"s;
		case EnduranceGaming:
			return "EnduranceGaming"s;
		case Invalid:
			return "Invalid"s;
		default:
			throw dptf_exception("DTT Gaming Mode type is unknown"s);
		}
	}

	Type toType(UIntN value)
	{
		if (value == MaxPerformance)
		{
			return MaxPerformance;
		}

		if (value == EnduranceGaming)
		{
			return EnduranceGaming;
		}

		if (value == Invalid)
		{
			return Invalid;
		}

		throw dptf_exception("DTT Gaming Mode value is unknown"s);
	}

	Type fromString(const std::string& dttGamingMode)
	{
		if (dttGamingMode == toString(MaxPerformance))
		{
			return MaxPerformance;
		}

		if (dttGamingMode == toString(EnduranceGaming))
		{
			return EnduranceGaming;
		}

		return Invalid;
	}
}
