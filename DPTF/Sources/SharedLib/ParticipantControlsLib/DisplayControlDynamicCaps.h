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

class DisplayControlDynamicCaps final
{
public:
	DisplayControlDynamicCaps(UIntN currentUpperLimit, UIntN currentLowerLimit);
	UIntN getCurrentUpperLimit(void) const;
	UIntN getCurrentLowerLimit(void) const;
	Bool operator==(const DisplayControlDynamicCaps& rhs) const;
	Bool operator!=(const DisplayControlDynamicCaps& rhs) const;
	std::shared_ptr<XmlNode> getXml(void);
	void setCurrentUpperLimit(UIntN upperLimit);
	void setCurrentLowerLimit(UIntN lowerLimit);

private:
	UIntN m_currentUpperLimit; // Index of DisplayControl representing current allowed upper limit
	UIntN m_currentLowerLimit; // Index of DisplayControl representing current allowed lower limit
};
