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
#include "DptfManagerInterface.h"
#include "CaptureDataGenerator.h"
#include "XmlNode.h"
#include "TableObjectType.h"

class dptf_export ParticipantCaptureDataGenerator : public CaptureDataGenerator
{
public:
	ParticipantCaptureDataGenerator(DptfManagerInterface* dptfManager);
	std::shared_ptr<XmlNode> generate() const override;

private:
	ParticipantManagerInterface* m_participantManager;
	void addParticipantDetails(const UIntN participantIndex, const UIntN domainIndex, std::shared_ptr<XmlNode> root) const;
	void addBasicInfo(const UIntN participantIndex, const UIntN domainIndex, const std::shared_ptr<XmlNode>& root) const;
	void addParticipantTables(const UIntN participantIndex, const std::shared_ptr<XmlNode>& root) const;
	void addDefaultParticipantTableIfSupported(
		TableObjectType::Type tableType,
		const UIntN participantIndex,
		const std::shared_ptr<XmlNode>& root) const;
	void addTripPoints(const UIntN participantIndex, const std::shared_ptr<XmlNode>& root) const;
	void addCoreControl(const UIntN participantIndex, const UIntN domainIndex, const std::shared_ptr<XmlNode>& root) const;
	void addPerformanceControl(const UIntN participantIndex, const UIntN domainIndex, const std::shared_ptr<XmlNode>& root) const;
	void addPowerControl(const UIntN participantIndex, const UIntN domainIndex, const std::shared_ptr<XmlNode>& root) const;
	void addDisplayControl(const UIntN participantIndex, const UIntN domainIndex, const std::shared_ptr<XmlNode>& root) const;
	void addFanControl(const UIntN participantIndex, const UIntN domainIndex, const std::shared_ptr<XmlNode>& root) const;
	void addTripPoint(
		std::pair<ParticipantSpecificInfoKey::Type, Temperature> specInfo,
		const UIntN participantIndex,
		const std::shared_ptr<XmlNode>& root) const;
	void addHysteresis(const UIntN participantIndex, const std::shared_ptr<XmlNode>& root) const;
	bool hasTemperatureControls(UIntN participantIndex) const;
	bool hasCoreControls(UIntN participantIndex) const;
	bool hasPowerControls(UIntN participantIndex) const;
	bool hasPerformanceControls(UIntN participantIndex) const;
	bool hasDisplayControls(UIntN participantIndex) const;
	bool hasFanControls(UIntN participantIndex) const;
	std::set<UIntN> getDomainsWithTemperatureThresholdControls(UIntN participantIndex) const;
	std::set<UIntN> getDomainsWithCoreControls(UIntN participantIndex) const;
	std::set<UIntN> getDomainsWithPerformanceControls(UIntN participantIndex) const;
	std::set<UIntN> getDomainsWithPowerControls(UIntN participantIndex) const;
	std::set<UIntN> getDomainsWithDisplayControls(UIntN participantIndex) const;
	std::set<UIntN> getDomainsWithFanControls(UIntN participantIndex) const;
};
