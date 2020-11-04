/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

namespace SensorMode
{
	enum Type
	{
		Off = 0,
		Disabled = 1,
		Wof = 2,
		Vision = 3,
		Passthru = 4,
		Privacy = 5,
		Invalid = Constants::Invalid
	};

	std::string toString(SensorMode::Type sensorMode);
	SensorMode::Type toType(UIntN value);
}
