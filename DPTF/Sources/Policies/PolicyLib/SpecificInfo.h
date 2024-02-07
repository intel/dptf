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
#include "ParticipantSpecificInfoKey.h"
#include "XmlNode.h"

// wraps specific info with functions to sort the data, access items, and generate xml status
class dptf_export SpecificInfo
{
public:
	SpecificInfo();
	SpecificInfo(const std::map<ParticipantSpecificInfoKey::Type, Temperature>& specificInfo);
	virtual ~SpecificInfo() = default;

	SpecificInfo(const SpecificInfo& other) = default;
	SpecificInfo& operator=(const SpecificInfo& other) = default;
	SpecificInfo(SpecificInfo&& other) = default;
	SpecificInfo& operator=(SpecificInfo&& other) = default;

	std::vector<std::pair<ParticipantSpecificInfoKey::Type, Temperature>> getSortedByValue();
	std::vector<std::pair<ParticipantSpecificInfoKey::Type, Temperature>> getSortedByKey();

	Bool hasKey(ParticipantSpecificInfoKey::Type key);
	Temperature getTemperature(ParticipantSpecificInfoKey::Type key);
	std::shared_ptr<XmlNode> getXml() const;

	Bool operator==(const SpecificInfo& rhs) const;
	Bool operator!=(const SpecificInfo& rhs) const;

private:
	std::map<ParticipantSpecificInfoKey::Type, Temperature> m_specificInfo;
	std::vector<std::pair<ParticipantSpecificInfoKey::Type, Temperature>> m_sortedTripPointsByValue;
	std::vector<std::pair<ParticipantSpecificInfoKey::Type, Temperature>> m_sortedTripPointsByKey;

	void sortInfoByValue();
	void sortInfoByKey();
};
