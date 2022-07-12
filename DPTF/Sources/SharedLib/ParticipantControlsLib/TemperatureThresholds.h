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

#pragma once

#include "Dptf.h"

class XmlNode;

class TemperatureThresholds final
{
public:
	TemperatureThresholds(void);
	TemperatureThresholds(Temperature aux0, Temperature aux1, Temperature hysteresis);
	static TemperatureThresholds createInvalid();
	Temperature getAux0(void) const;
	Temperature getAux1(void) const;
	Temperature getHysteresis(void) const;
	std::shared_ptr<XmlNode> getXml(void);
	Bool operator==(const TemperatureThresholds& thresholds) const;
	DptfBuffer toDptfBuffer() const;
	static TemperatureThresholds createFromDptfBuffer(const DptfBuffer& buffer);

private:
	Temperature m_aux0; // lower bound in Tenth Kelvin
	Temperature m_aux1; // upper bound in Tenth Kelvin
	Temperature m_hysteresis; // In Tenth Kelvin
};
