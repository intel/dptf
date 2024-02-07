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

class XmlNode;

class PerformanceControlDynamicCaps final
{
public:
	PerformanceControlDynamicCaps(UIntN currentLowerLimitIndex, UIntN currentUpperLimitIndex);
	UIntN getCurrentLowerLimitIndex(void) const;
	UIntN getCurrentUpperLimitIndex(void) const;
	Bool operator==(const PerformanceControlDynamicCaps& rhs) const;
	Bool operator!=(const PerformanceControlDynamicCaps& rhs) const;
	std::shared_ptr<XmlNode> getXml(void);
	void setCurrentLowerLimitIndex(UIntN lowerLimit);
	void setCurrentUpperLimitIndex(UIntN upperLimit);

private:
	UIntN m_currentLowerLimitIndex; // Index of PerformanceControl representing current allowed lower limit
	UIntN m_currentUpperLimitIndex; // Index of PerformanceControl representing current allowed upper limit
};
