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
#include "RelationshipTableEntryBase.h"
#include "XmlNode.h"

class dptf_export ThermalRelationshipTableEntry : public RelationshipTableEntryBase
{
public:
	ThermalRelationshipTableEntry(
		const std::string& sourceDeviceScope,
		const std::string& targetDeviceScope,
		UInt32 thermalInfluence,
		const TimeSpan& thermalSamplingPeriod);

	const UInt32& thermalInfluence() const;
	const TimeSpan& thermalSamplingPeriod() const;

	std::shared_ptr<XmlNode> getXml();
	Bool isSameAs(const ThermalRelationshipTableEntry& trtEntry) const;
	Bool operator==(const ThermalRelationshipTableEntry& trtEntry) const;

private:
	UInt32 m_thermalInfluence;
	TimeSpan m_thermalSamplingPeriod;
};
