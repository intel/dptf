/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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
#include "ParticipantTripPointsCaptureDataGenerator.h"
#include "ParticipantManagerInterface.h"
#include "TemperatureThresholds.h"
using namespace std;

ParticipantTripPointsCaptureDataGenerator::ParticipantTripPointsCaptureDataGenerator(DptfManagerInterface* dptfManager)
	: CaptureDataGenerator(dptfManager)
	, m_participantManager(dptfManager->getParticipantManager())
{
}

shared_ptr<XmlNode> ParticipantTripPointsCaptureDataGenerator::generate() const
{
	auto root = XmlNode::createRoot();
	const auto participantIds = m_participantManager->getParticipantIndexes();
	for (const auto participantId : participantIds)
	{
		addParticipantDetails(participantId, root);
	}
	return root;
}

/*
Example:
<participant>
	...
</participant>
*/
void ParticipantTripPointsCaptureDataGenerator::addParticipantDetails(UIntN participantId, std::shared_ptr<XmlNode> root) const
{
	try
	{
		const auto participantDomainRoot = XmlNode::createWrapperElement("participant");
		addBasicInfo(participantId, participantDomainRoot);
		addTripPoints(participantId, participantDomainRoot);
		root->addChild(participantDomainRoot);
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

/*
Example:
<name>SEN1</name>
<domainID>D0</domainID>
<description>EXTZ-External/Remote 1 Sensor</description>
*/
void ParticipantTripPointsCaptureDataGenerator::addBasicInfo(const UIntN participantId, const shared_ptr<XmlNode>& root)
	const
{
	const auto participant = m_participantManager->getParticipantPtr(participantId);
	const auto name = participant->getParticipantName();
	root->addChild(XmlNode::createDataElement("name", name));
	root->addChild(XmlNode::createDataElement("domainID", "D"s + to_string(0)));
	const auto participantProperties = participant->getParticipantProperties();
	const auto participantDescription = participantProperties.getDescription();
	root->addChild(XmlNode::createDataElement("description", participantDescription));
}

/*
Example:
<trippoints>
	<psv>65.0</psv>
	<cr3>70.0</cr3>
	<hot>75.0</hot>
	<crt>80.0</crt>
	<ac0>55.0</ac0>
	<ac1>59.0</ac1>
	<ac2>X</ac2>
	<ac3>X</ac3>
	<ac4>X</ac4>
	<ac5>X</ac5>
	<ac6>X</ac6>
	<ac7>X</ac7>
	<ac8>X</ac8>
	<ac9>X</ac9>
	<hyst>2.0</hyst>
	<ntt>5.0</ntt>
</trippoints>
*/
const vector<ParticipantSpecificInfoKey::Type> tripPoints{
	ParticipantSpecificInfoKey::PSV,
	ParticipantSpecificInfoKey::Warm,
	ParticipantSpecificInfoKey::Hot,
	ParticipantSpecificInfoKey::Critical,
	ParticipantSpecificInfoKey::AC0,
	ParticipantSpecificInfoKey::AC1,
	ParticipantSpecificInfoKey::AC2,
	ParticipantSpecificInfoKey::AC3,
	ParticipantSpecificInfoKey::AC4,
	ParticipantSpecificInfoKey::AC5,
	ParticipantSpecificInfoKey::AC6,
	ParticipantSpecificInfoKey::AC7,
	ParticipantSpecificInfoKey::AC8,
	ParticipantSpecificInfoKey::AC9,
	ParticipantSpecificInfoKey::NTT
};

void ParticipantTripPointsCaptureDataGenerator::addTripPoints(
	const UIntN participantId,
	const shared_ptr<XmlNode>& root) const
{
	try
	{
		if (hasTemperatureControls(participantId))
		{
			const auto participant = m_participantManager->getParticipantPtr(participantId);
			const auto specInfoResult = participant->getParticipantSpecificInfo(tripPoints);
			const auto tripPointsRoot = XmlNode::createWrapperElement("trippoints");
			for (const auto& specInfo : specInfoResult)
			{
				addTripPoint(specInfo, participantId, tripPointsRoot);
			}
			addHysteresis(participantId, tripPointsRoot);
			root->addChild(tripPointsRoot);
		}
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

void ParticipantTripPointsCaptureDataGenerator::addTripPoint(
	pair<ParticipantSpecificInfoKey::Type, Temperature> specInfo,
	const UIntN participantId,
	const shared_ptr<XmlNode>& root) const
{
	const auto name = ParticipantSpecificInfoKey::ToString(specInfo.first);
	const auto value = specInfo.second.toString();
	root->addChild(XmlNode::createDataElement(name, value));
}

void ParticipantTripPointsCaptureDataGenerator::addHysteresis(
	const UIntN participantId,
	const shared_ptr<XmlNode>& root) const
{
	auto hysteresis = Temperature::createInvalid();
	const auto domains = getDomainsWithTemperatureThresholdControls(participantId);
	if (!domains.empty())
	{
		const DptfRequest request(DptfRequestType::TemperatureControlGetTemperatureThresholds, participantId, 0);
		const auto result = m_dptfManager->getRequestDispatcher()->dispatch(PolicyRequest(Constants::Invalid, request));
		const auto tempThresholds = TemperatureThresholds::createFromDptfBuffer(result.getData());
		hysteresis = tempThresholds.getHysteresis();
	}
	root->addChild(XmlNode::createDataElement("hyst", hysteresis.toString()));
}

bool ParticipantTripPointsCaptureDataGenerator::hasTemperatureControls(const UIntN participantId) const
{
	const auto participant = m_participantManager->getParticipantPtr(participantId);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN d = 0; d < domainPropertySet.getDomainCount(); ++d)
	{
		if (domainPropertySet.getDomainProperties(d).implementsTemperatureInterface())
		{
			return true;
		}
	}
	return false;
}

set<UIntN> ParticipantTripPointsCaptureDataGenerator::getDomainsWithTemperatureThresholdControls(const UIntN participantId) const
{
	set<UIntN> domains;
	const auto participant = m_participantManager->getParticipantPtr(participantId);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN d = 0; d < domainPropertySet.getDomainCount(); ++d)
	{
		if (domainPropertySet.getDomainProperties(d).implementsTemperatureThresholdInterface())
		{
			domains.insert(d);
		}
	}
	return domains;
}