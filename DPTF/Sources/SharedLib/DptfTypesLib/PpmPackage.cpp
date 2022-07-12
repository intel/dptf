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

#include "PpmPackage.h"
#include "StringConverter.h"
#include "StringParser.h"
#include "string.h"

namespace PpmPackage
{
	std::string toString(UInt32 package)
	{
		std::string packageName = Constants::InvalidString;

		if (package != PpmPackage::Invalid)
		{
			packageName = std::to_string(package);
			packageName.insert(0, "P");
		}

		return packageName;
	}

	UInt32 fromString(std::string packageName)
	{
		UInt32 package = PpmPackage::Invalid;
		try
		{
			if (packageName.at(0) == 'P')
			{
				packageName.erase(packageName.begin());
				package = StringConverter::toUInt32(packageName);
			}
		}
		catch (...)		
		{
			package = PpmPackage::Invalid;
		}

		return package;		
	}
}