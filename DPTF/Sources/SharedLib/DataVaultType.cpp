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

#include "DataVaultType.h"
#include "DptfExceptions.h"

std::string DataVaultType::ToString(DataVaultType::Type type)
{
	switch (type)
	{
	case Dptf:
		return "dptf";
	case Override:
		return "override";
	default:
		throw dptf_exception("Invalid DataVault type.");
	}
}

DataVaultType::Type DataVaultType::ToType(std::string value)
{
	if (value == "dptf")
	{
		return DataVaultType::Dptf;
	}
	else if (value == "override")
	{
		return DataVaultType::Override;
	}
	else
	{
		throw dptf_exception("Invalid DataVault name.");
	}
}
