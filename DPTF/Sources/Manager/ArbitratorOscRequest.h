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
#include <XmlNode.h>

class dptf_export ArbitratorOscRequest
{
public:
	ArbitratorOscRequest();
	virtual ~ArbitratorOscRequest();

	void updateRequest(std::string policyName, const UInt32 value);
	Bool arbitratedOscValueChanged() const;
	UInt32 getArbitratedOscValue() const;
	std::shared_ptr<XmlNode> getOscArbitratorStatus() const;

private:
	std::map<std::string, UInt32> m_requests;
	Bool m_arbitratedValueChangedSinceLastSet;
	UInt32 m_arbitratedValue;
	std::string toString(UInt32 value) const;
};
