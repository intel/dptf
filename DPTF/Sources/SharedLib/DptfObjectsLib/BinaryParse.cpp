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

#include "BinaryParse.h"
#include "StringParser.h"

UInt64 BinaryParse::extractBits(UInt16 startBit, UInt16 stopBit, UInt64 data)
{
	if (startBit < stopBit)
	{
		throw dptf_exception("The start bit must be greater than the stop bit.");
	}

	UInt64 bitCount = (startBit - stopBit) + 1;
	UInt64 mask = ((UInt64)1 << bitCount) - 1;

	return (data >> stopBit) & mask;
}

// TODO : Values come in from ESIF as u64's and the majority of the time I cast them to UIntN...

std::string BinaryParse::normalizeAcpiScope(const std::string& acpiScope)
{
	std::string parsedScope = StringParser::removeCharacter(acpiScope, '\0');
	if (parsedScope == Constants::InvalidString || parsedScope == Constants::NotAvailableString)
	{
		return parsedScope;
	}

	std::stringstream normalizedAcpiScope;
	UIntN charsSinceLastDot(0);
	for (UIntN pos = 0; pos < parsedScope.size(); pos++)
	{
		if (parsedScope[pos] == '\\')
		{
			normalizedAcpiScope << parsedScope[pos];
			charsSinceLastDot = 0;
		}
		else if (parsedScope[pos] == '.')
		{
			IntN underscoresToAdd = 4 - charsSinceLastDot;
			while (underscoresToAdd > 0)
			{
				normalizedAcpiScope << '_';
				underscoresToAdd--;
			}
			normalizedAcpiScope << parsedScope[pos];
			charsSinceLastDot = 0;
		}
		else
		{
			normalizedAcpiScope << parsedScope[pos];
			charsSinceLastDot++;
		}
	}

	if (parsedScope.size() > 0)
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
