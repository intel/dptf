/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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
// Lowest voltage threshold wins
// Lowest tcc offset wins
//

class dptf_export ArbitratorProcessorControl
{
public:
	ArbitratorProcessorControl();
	virtual ~ArbitratorProcessorControl();

	UInt32 arbitrateVoltageThresholdRequest(UIntN policyIndex, const UInt32 voltageThreshold);
	void commitPolicyVoltageThresholdRequest(UIntN policyIndex, const UInt32 voltageThreshold);
	void removeVoltageThresholdRequest(UIntN policyIndex);
	Bool arbitratedVoltageThresholdValueChanged() const;
	UInt32 getArbitratedVoltageThresholdValue() const;
	Temperature arbitrateTccOffsetRequest(UIntN policyIndex, const Temperature& tccOffset);
	void commitPolicyTccOffsetRequest(UIntN policyIndex, const Temperature& tccOffset);
	void removeTccOffsetRequest(UIntN policyIndex);
	Bool arbitratedTccOffsetValueChanged() const;
	Temperature getArbitratedTccOffsetValue() const;
	std::shared_ptr<XmlNode> getStatusForPolicy(UIntN policyIndex) const;

private:

	std::map<UIntN, UInt32> m_requestedVoltages;
	std::map<UIntN, Temperature> m_requestedOffsets;
	Bool m_arbitratedVoltageValueChangedSinceLastSet;
	Bool m_arbitratedOffsetValueChangedSinceLastSet;
	UInt32 m_arbitratedVoltage;
	Temperature m_arbitratedTccOffset;

	UInt32 getLowestVoltageRequest(std::map<UIntN, UInt32> requestedVoltages);
	Temperature getLowestTccOffsetRequest(std::map<UIntN, Temperature> requestedOffsets);
};
