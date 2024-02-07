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
#include "DomainType.h"
#include "XmlNode.h"
#include "RfProfileData.h"

class XmlNode;

class RfProfileDataSet final
{
public:
	RfProfileDataSet();
	RfProfileDataSet(const std::vector<RfProfileData>& rfProfileData);
	static RfProfileDataSet createActiveRfProfileDataFromDptfBuffer(const DptfBuffer& buffer);
	static RfProfileDataSet createActiveRfProfileDataFromEmptyData();
	std::vector<RfProfileData> getRfProfileData() const;
	Bool operator==(const RfProfileDataSet& rhs) const;
	std::shared_ptr<XmlNode> getXml(DomainType::Type domainType);
	RfProfileData operator[](UIntN index) const;

private:
	static void throwIfBufferIsUnexpectedSize(const DptfBuffer& buffer);
	std::vector<RfProfileData> m_rfProfileDataSet;
};
