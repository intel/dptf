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

#include "SmtType.h"

namespace SmtType
{
	std::string toString(SmtType::Type smtType)
	{
		switch (smtType)
		{
		case NoSmt:
			return "NoSmt";
		case SymmetricSmt:
			return "SymmetricSmt";
		case AsymmetricSmt:
			return "AsymmetricSmt";
		default:
			return "InvalidSmt";
		}
	}

	SmtType::Type fromString(std::string smtType)
	{
		if (smtType == toString(SmtType::NoSmt))
		{
			return SmtType::NoSmt;
		}
		else if (smtType == toString(SmtType::SymmetricSmt))
		{
			return SmtType::SymmetricSmt;
		}
		else if (smtType == toString(SmtType::AsymmetricSmt))
		{
			return SmtType::AsymmetricSmt;
		}
		else
		{
			return SmtType::InvalidSmt;
		}
	}
}
