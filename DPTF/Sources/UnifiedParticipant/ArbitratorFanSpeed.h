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
#include <XmlNode.h>

//
// Arbitration Rule:
//
// 1) if fine grain fan control is supported then we only arbitrate based on percentage.
// 2) for percentage the highest fan speed wins
//

// TODO: remove the version of this class in manager once moved over to new request interface
class dptf_export ArbitratorFanSpeed
{
public:
	ArbitratorFanSpeed();
	virtual ~ArbitratorFanSpeed();

	void commitRequest(UIntN policyIndex, const Percentage& value);
	void removeRequest(UIntN policyIndex);
	Percentage getArbitratedValue() const;
	Percentage calculateNewArbitratedValue(UIntN policyIndex, const Percentage& newValue) const;
	std::shared_ptr<XmlNode> getStatusForPolicy(UIntN policyIndex) const;

private:
	std::map<UIntN, Percentage> m_requests;
	static Percentage getHighestRequest(const std::map<UIntN, Percentage>& requests);
};
