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

#include "TableObjectType.h"
#include "DptfExceptions.h"

std::string TableObjectType::ToString(TableObjectType::Type type)
{
	switch (type)
	{
	case Apat:
		return "apat";
	case Apct:
		return "apct";
	case Dynamic_Idsp:
		return "dynamic-idsp";
	case Ddrf:
		return "ddrf";
	case Itmt:
		return "itmt";
	case Epot:
		return "epot";
	case Tpga:
		return "tpga";
	case SwOemVariables:
		return "sw-oem-variables";
	default:
		throw dptf_exception("Invalid table type.");
	}
}

TableObjectType::Type TableObjectType::ToType(std::string value)
{
	if (value == "apat")
	{
		return TableObjectType::Apat;
	}
	else if (value == "apct")
	{
		return TableObjectType::Apct;
	}
	else if (value == "dynamic-idsp")
	{
		return TableObjectType::Dynamic_Idsp;
	}
	else if (value == "ddrf")
	{
		return TableObjectType::Ddrf;
	}
	else if (value == "itmt")
	{
		return TableObjectType::Itmt;
	}
	else if (value == "epot")
	{
		return TableObjectType::Epot;
	}
	else if (value == "tpga")
	{
		return TableObjectType::Tpga;
	}
	else if (value == "sw-oem-variables")
	{
		return TableObjectType::SwOemVariables;
	}
	else
	{
		throw dptf_exception("Invalid table name.");
	}
}
