/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
#include "PowerControlDynamicCaps.h"
#include "DptfBuffer.h"

class XmlNode;

class PowerControlDynamicCapsSet
{
public:
	PowerControlDynamicCapsSet();
	PowerControlDynamicCapsSet(const std::vector<PowerControlDynamicCaps>& powerControlDynamicCaps);
	~PowerControlDynamicCapsSet();

	static PowerControlDynamicCapsSet createFromPpcc(const DptfBuffer& buffer, Power pl4PowerLimit);
	Bool isEmpty() const;
	Bool hasCapability(PowerControlType::Type controlType) const;
	const PowerControlDynamicCaps& getCapability(PowerControlType::Type controlType) const;
	void setCapability(const PowerControlDynamicCaps& capability);
	std::set<PowerControlType::Type> getControlTypes() const;
	Power snapToCapability(PowerControlType::Type controlType, Power powerValue) const;
	TimeSpan snapToCapability(PowerControlType::Type controlType, TimeSpan timeValue) const;
	DptfBuffer toPpccBinary() const;
	std::shared_ptr<XmlNode> getXml(void) const;

	Bool operator==(const PowerControlDynamicCapsSet& rhs) const;
	Bool operator!=(const PowerControlDynamicCapsSet& rhs) const;

private:
	std::map<PowerControlType::Type, PowerControlDynamicCaps> m_capabilities;
};
