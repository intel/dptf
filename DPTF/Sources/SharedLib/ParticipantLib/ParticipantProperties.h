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
#include "BusType.h"
#include "PciInfo.h"
#include "AcpiInfo.h"

class XmlNode;

class ParticipantProperties final
{
public:
	ParticipantProperties();
	ParticipantProperties(
		const Guid& guid,
		const std::string& name,
		const std::string& description,
		BusType::Type busType,
		const PciInfo& pciInfo,
		const AcpiInfo& acpiInfo);

	Guid getGuid(void) const;
	std::string getName(void) const;
	std::string getDescription(void) const;
	BusType::Type getBusType(void) const;
	PciInfo getPciInfo(void) const;
	AcpiInfo getAcpiInfo(void) const;
	std::shared_ptr<XmlNode> getXml(void) const;

private:
	Guid m_guid;
	std::string m_name;
	std::string m_description;
	BusType::Type m_busType;
	PciInfo m_pciInfo;
	AcpiInfo m_acpiInfo;
};
