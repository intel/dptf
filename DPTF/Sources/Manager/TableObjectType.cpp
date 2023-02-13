/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
	case Acpr:
		return "acpr";
	case Apat:
		return "apat";
	case Apct:
		return "apct";
	case Art:
		return "_art";
	case Ddrf:
		return "ddrf";
	case Dynamic_Idsp:
		return "dynamic-idsp";
	case Epot:
		return "epot";
	case Itmt:
		return "itmt";
	case Odvp:
		return "odvp";
	case Pbat:
		return "pbat";
	case Pbct:
		return "pbct";
	case Pbmt:
		return "pbmt";
	case Pida:
		return "pida";
	case Psh2:
		return "psh2";
	case Psha:
		return "psha";
	case Psvt:
		return "psvt";
	case SwOemVariables:
		return "sw-oem-variables";
	case Tpga:
		return "tpga";
	case Trt:
		return "_trt";
	case Vsct:
		return "vsct";
	case Vspt:
		return "vspt";
	case Vtmt:
		return "uvth";
	default:
		throw dptf_exception("Invalid table type.");
	}
}

TableObjectType::Type TableObjectType::ToType(std::string value)
{
	if (value == "acpr")
	{
		return TableObjectType::Acpr;
	}
	else if (value == "apat")
	{
		return TableObjectType::Apat;
	}
	else if (value == "apct")
	{
		return TableObjectType::Apct;
	}
	else if ((value == "art") || (value == "_art"))
	{
		return TableObjectType::Art;
	}
	else if (value == "ddrf")
	{
		return TableObjectType::Ddrf;
	}
	else if (value == "dynamic-idsp")
	{
		return TableObjectType::Dynamic_Idsp;
	}
	else if (value == "epot")
	{
		return TableObjectType::Epot;
	}
	else if (value == "itmt")
	{
		return TableObjectType::Itmt;
	}
	else if (value == "odvp")
	{
		return TableObjectType::Odvp;
	}
	else if (value == "pbat")
	{
		return TableObjectType::Pbat;
	}
	else if (value == "pbct")
	{
		return TableObjectType::Pbct;
	}
	else if (value == "pbmt")
	{
		return TableObjectType::Pbmt;
	}
	else if (value == "pida")
	{
		return TableObjectType::Pida;
	}
	else if (value == "psh2")
	{
		return TableObjectType::Psh2;
	}
	else if (value == "psha")
	{
		return TableObjectType::Psha;
	}
	else if (value == "psvt")
	{
		return TableObjectType::Psvt;
	}
	else if (value == "sw-oem-variables")
	{
		return TableObjectType::SwOemVariables;
	}
	else if (value == "tpga")
	{
		return TableObjectType::Tpga;
	}
	else if ((value == "trt") || (value == "_trt"))
	{
		return TableObjectType::Trt;
	}
	else if (value == "vsct")
	{
		return TableObjectType::Vsct;
	}
	else if (value == "vspt")
	{
		return TableObjectType::Vspt;
	}
	else if (value == "vtmt")
	{
		return TableObjectType::Vtmt;
	}
	else
	{
		throw dptf_exception("Invalid table name.");
	}
}