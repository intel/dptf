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

#pragma once

#include "Dptf.h"

//
// This is in place to format C++ built in types (only).  For any classes that we have
// created they should have a toString() function to provide formatting.
//

namespace StatusFormat
{
	std::string friendlyValue(Bool value);
	std::string friendlyValue(UInt32 value);
	std::string friendlyValue(Int32 value);
	std::string friendlyValue(UInt64 value);
	std::string friendlyValue(double value);
	std::string friendlyValueWithPrecision(double value, UIntN precision);
};
