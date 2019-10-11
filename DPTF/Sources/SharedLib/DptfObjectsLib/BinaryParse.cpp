/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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

#include "BinaryParse.h"

UInt64 BinaryParse::extractBits(UInt16 startBit, UInt16 stopBit, UInt64 data)
{
	if (startBit < stopBit)
	{
		throw dptf_exception("The start bit must be greater than the stop bit.");
	}

	UInt64 bitCount = (startBit - stopBit) + 1;
	UInt64 mask = (1 << bitCount) - 1;

	return (data >> stopBit) & mask;
}

// TODO : Values come in from ESIF as u64's and the majority of the time I cast them to UIntN...

std::string BinaryParse::normalizeAcpiScope(const std::string& acpiScope)
{
	if (acpiScope == Constants::InvalidString || acpiScope == Constants::NotAvailableString)
	{
		return acpiScope;
	}

	std::stringstream normalizedAcpiScope;
	UIntN charsSinceLastDot(0);
	for (UIntN pos = 0; pos < acpiScope.size(); pos++)
	{
		if (acpiScope[pos] == '\\')
		{
			normalizedAcpiScope << acpiScope[pos];
			charsSinceLastDot = 0;
		}
		else if (acpiScope[pos] == '.')
		{
			IntN underscoresToAdd = 4 - charsSinceLastDot;
			while (underscoresToAdd > 0)
			{
				normalizedAcpiScope << '_';
				underscoresToAdd--;
			}
			normalizedAcpiScope << acpiScope[pos];
			charsSinceLastDot = 0;
		}
		else if (acpiScope[pos] == '\0')
		{
			charsSinceLastDot++;
			continue;
		}
		else
		{
			normalizedAcpiScope << acpiScope[pos];
			charsSinceLastDot++;
		}
	}

	if (acpiScope.size() > 0)
	{
		IntN underscoresToAdd = 4 - charsSinceLastDot;
		while (underscoresToAdd > 0)
		{
			normalizedAcpiScope << '_';
			underscoresToAdd--;
		}

		auto fullScope = normalizedAcpiScope.str();
		auto lastDotPos = fullScope.find_last_of('.');
		auto name = fullScope.substr(lastDotPos + 1);
		return name;
	}

	return normalizedAcpiScope.str();
}
