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
#include "XmlNode.h"

static std::vector<std::string> FanCapabilityNames = {
	"Fan Speed Control",
	"Fan Operating Mode Control",
	"Fan RPM Report",
	"Fan Maintenance Mode"};

class dptf_export FanCapabilities
{
public:
	FanCapabilities();
	FanCapabilities(UInt32 fanCapabilities);
	UInt32 getFanCapabilities() const;
	void setFanCapabilities(UInt32 fanCapabilities);
	std::shared_ptr<XmlNode> getXml() const;

	virtual ~FanCapabilities();

private:
	UInt32 m_fanCapabilities;

	std::shared_ptr<XmlNode> createCapabilityXmlNode(std::string capabilityName, UInt32 capabilityIndex) const;
};
