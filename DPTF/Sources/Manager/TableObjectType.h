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

#pragma once
#include <string>

namespace TableObjectType
{
	enum Type
	{
		FIRST,
		Acpr = FIRST,
		Apat,
		Apct,
		Art,
		Ddrf,
		Dynamic_Idsp,
		Epot,
		Itmt,
		Odvp,
		Pbat,
		Pbct,
		Pbmt,
		Pida,
		Psh2,
		Psha,
		Psvt,
		SwOemVariables,
		Tpga,
		Trt,
		Vsct,
		Vspt,
		Vtmt,
		LAST
	};

	std::string ToString(TableObjectType::Type type);
	TableObjectType::Type ToType(std::string value);
};
