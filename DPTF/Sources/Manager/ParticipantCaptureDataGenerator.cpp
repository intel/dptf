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
#include "ParticipantCaptureDataGenerator.h"
#include "DataManager.h"
#include "ParticipantManagerInterface.h"
#include "TemperatureThresholds.h"
#include "EsifServicesInterface.h"
#include "ActiveControlDynamicCaps.h"

using namespace std;

ParticipantCaptureDataGenerator::ParticipantCaptureDataGenerator(DptfManagerInterface* dptfManager)
	: CaptureDataGenerator(dptfManager)
	, m_participantManager(dptfManager->getParticipantManager())
{
}

shared_ptr<XmlNode> ParticipantCaptureDataGenerator::generate() const
{
	auto root = XmlNode::createRoot();
	const auto participantIndexes = m_participantManager->getParticipantIndexes();
	for (const auto participantIndex : participantIndexes)
	{
		const auto participant = m_participantManager->getParticipantPtr(participantIndex);
		const auto domainPropertySet = participant->getDomainPropertiesSet();
		for (UIntN domainIndex = 0; domainIndex < domainPropertySet.getDomainCount(); ++domainIndex)
		{
			addParticipantDetails(participantIndex, domainIndex, root);
		}
	}
	return root;
}

/*
Example:
<participant>
	...
</participant>
*/
void ParticipantCaptureDataGenerator::addParticipantDetails(
	const UIntN participantIndex,
	const UIntN domainIndex,
	shared_ptr<XmlNode> root) const
{
	try
	{
		const auto participantDomainRoot = XmlNode::createWrapperElement("participant");
		addBasicInfo(participantIndex, domainIndex, participantDomainRoot);
		addTripPoints(participantIndex, participantDomainRoot);
		addCoreControl(participantIndex, domainIndex, participantDomainRoot);
		addPerformanceControl(participantIndex, domainIndex, participantDomainRoot);
		addPowerControl(participantIndex, domainIndex, participantDomainRoot);
		addDisplayControl(participantIndex, domainIndex, participantDomainRoot);
		addFanControl(participantIndex, domainIndex, participantDomainRoot);
		addParticipantTables(participantIndex, participantDomainRoot);
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
void ParticipantCaptureDataGenerator::addBasicInfo(
	const UIntN participantIndex,
	const UIntN domainIndex,
	const shared_ptr<XmlNode>& root) const
{
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto name = participant->getParticipantName();
	root->addChild(XmlNode::createDataElement("name", name));
	root->addChild(XmlNode::createDataElement("domainID", "D"s + to_string(domainIndex)));
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

void ParticipantCaptureDataGenerator::addParticipantTables(
	const UIntN participantIndex,
	const shared_ptr<XmlNode>& root) const
{
	const auto tablesRoot = XmlNode::createWrapperElement("Tables");

	for (TableObjectType::Type tableType = TableObjectType::FIRST; tableType != TableObjectType::LAST;
		 tableType = (TableObjectType::Type)(tableType + 1))
	{
		addDefaultParticipantTableIfSupported(tableType, participantIndex, tablesRoot);
	}

	if (tablesRoot->getChildren().empty() == false)
	{
		root->addChild(tablesRoot);
	}
}

void ParticipantCaptureDataGenerator::addDefaultParticipantTableIfSupported(
	TableObjectType::Type tableType,
	UIntN participantIndex,
	const shared_ptr<XmlNode>& root) const
{
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto participantName = participant->getParticipantName();
	const string participantTableSearch = "/participants/*" + participantName + "*";
	string esifResponseFromOverrideDv, esifResponseFromDptfDv = Constants::EmptyString;

	try
	{
		esifResponseFromOverrideDv = m_dptfManager->getEsifServices()->readConfigurationString(
			DataVaultType::ToString(DataVaultType::Override), participantTableSearch);
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}

	try
	{
		esifResponseFromDptfDv = m_dptfManager->getEsifServices()->readConfigurationString(
			DataVaultType::ToString(DataVaultType::Dptf), participantTableSearch);
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}

	bool isParticipantTableInDV =
		(esifResponseFromOverrideDv.find(TableObjectType::ToString(tableType)) != string::npos)
		|| (esifResponseFromDptfDv.find(TableObjectType::ToString(tableType)) != string::npos);
		
	// Example: /participants/VIR1/vspt

	if (isParticipantTableInDV)
	{
		string uuid = Constants::EmptyString;
		const auto result = m_dptfManager->getDataManager()->getTableObject(tableType, uuid, participantIndex).getXml();

		if (result != nullptr)
		{
			const auto tablesRoot =
				XmlNode::createWrapperElement("default_" + TableObjectType::ToString(tableType));
			tablesRoot->addChild(result);
			root->addChild(tablesRoot);
		}
	}	
}

void ParticipantCaptureDataGenerator::addTripPoints(
	const UIntN participantIndex,
	const shared_ptr<XmlNode>& root) const
{
	try
	{
		if (hasTemperatureControls(participantIndex))
		{
			const auto participant = m_participantManager->getParticipantPtr(participantIndex);
			const auto specInfoResult = participant->getParticipantSpecificInfo(tripPoints);
			const auto tripPointsRoot = XmlNode::createWrapperElement("trippoints");
			for (const auto& specInfo : specInfoResult)
			{
				addTripPoint(specInfo, participantIndex, tripPointsRoot);
			}
			addHysteresis(participantIndex, tripPointsRoot);
			root->addChild(tripPointsRoot);
		}
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

void ParticipantCaptureDataGenerator::addTripPoint(
	pair<ParticipantSpecificInfoKey::Type, Temperature> specInfo,
	const UIntN participantIndex,
	const shared_ptr<XmlNode>& root) const
{
	const auto name = ParticipantSpecificInfoKey::ToString(specInfo.first);
	const auto value = specInfo.second.toString();
	root->addChild(XmlNode::createDataElement(name, value));
}

void ParticipantCaptureDataGenerator::addHysteresis(
	const UIntN participantIndex,
	const shared_ptr<XmlNode>& root) const
{
	auto hysteresis = Temperature::createInvalid();
	const auto domains = getDomainsWithTemperatureThresholdControls(participantIndex);
	if (!domains.empty())
	{
		const DptfRequest request(DptfRequestType::TemperatureControlGetTemperatureThresholds, participantIndex, 0);
		const auto result = m_dptfManager->getRequestDispatcher()->dispatch(PolicyRequest(Constants::Invalid, request));
		const auto tempThresholds = TemperatureThresholds::createFromDptfBuffer(result.getData());
		hysteresis = tempThresholds.getHysteresis();
	}
	root->addChild(XmlNode::createDataElement("hyst", hysteresis.toString()));
}

void ParticipantCaptureDataGenerator::addCoreControl(
	const UIntN participantIndex,
	const UIntN domainIndex,
	const shared_ptr<XmlNode>& root) const
{
	try
	{
		if (hasCoreControls(participantIndex))
		{
			const auto coreControlRoot = XmlNode::createWrapperElement("core_control");
			const auto participant = m_participantManager->getParticipantPtr(participantIndex);
			auto coreControlDynamicCapabilities = participant->getCoreControlDynamicCaps(domainIndex);

			coreControlRoot->addChild(coreControlDynamicCapabilities.getXml());
			root->addChild(coreControlRoot);
		}
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

void ParticipantCaptureDataGenerator::addPerformanceControl(
	const UIntN participantIndex,
	const UIntN domainIndex,
	const shared_ptr<XmlNode>& root) const
{
	try
	{
		if (hasPerformanceControls(participantIndex))
		{
			const auto performanceControlRoot = XmlNode::createWrapperElement("performance_control");
			const auto participant = m_participantManager->getParticipantPtr(participantIndex);
			auto perfCapabilities = participant->getPerformanceControlDynamicCaps(domainIndex);

			performanceControlRoot->addChild(perfCapabilities.getXml());
			root->addChild(performanceControlRoot);
		}
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

void ParticipantCaptureDataGenerator::addPowerControl(
	const UIntN participantIndex,
	const UIntN domainIndex,
	const shared_ptr<XmlNode>& root) const
{
	try
	{
		if (hasPowerControls(participantIndex))
		{
			const auto powerControlRoot = XmlNode::createWrapperElement("power_control");
			const auto ppccRoot = XmlNode::createWrapperElement("ppcc");

			const auto participant = m_participantManager->getParticipantPtr(participantIndex);
			const auto powerCapabilitiesSet = participant->getPowerControlDynamicCapsSet(domainIndex);
			const auto powerControlTypes = powerCapabilitiesSet.getControlTypes();

			for (const auto powerControlType : powerControlTypes)
			{
				const auto powerCapability = powerCapabilitiesSet.getCapability(powerControlType);
				ppccRoot->addChild(powerCapability.getXml());
			}

			powerControlRoot->addChild(ppccRoot);
			root->addChild(powerControlRoot);
		}
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

void ParticipantCaptureDataGenerator::addDisplayControl(
	const UIntN participantIndex,
	const UIntN domainIndex,
	const shared_ptr<XmlNode>& root) const
{
	try
	{
		if (hasDisplayControls(participantIndex))
		{
			const auto displaylRoot = XmlNode::createWrapperElement("display_control");
			const auto participant = m_participantManager->getParticipantPtr(participantIndex);
			auto displayCapabilities = participant->getDisplayControlDynamicCaps(domainIndex);

			displaylRoot->addChild(displayCapabilities.getXml());
			root->addChild(displaylRoot);
		}
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

void ParticipantCaptureDataGenerator::addFanControl(
	const UIntN participantIndex,
	const UIntN domainIndex,
	const shared_ptr<XmlNode>& root) const
{
	try
	{
		if (hasFanControls(participantIndex))
		{
			const auto fanControlRoot = XmlNode::createWrapperElement("fan_control");
			const auto fcdcRoot = XmlNode::createWrapperElement("fan_control_capabilities");
			const DptfRequest request(DptfRequestType::ActiveControlGetDynamicCaps, participantIndex, domainIndex);
			const auto result =
				m_dptfManager->getRequestDispatcher()->dispatch(PolicyRequest(Constants::Invalid, request));
			const auto fanControlDynamicCaps = ActiveControlDynamicCaps::createFromFcdc(result.getData());

			fcdcRoot->addChild(fanControlDynamicCaps.getXml());
			fanControlRoot->addChild(fcdcRoot);
			root->addChild(fanControlRoot);
		}
	}
	catch (const exception& e)
	{
		logMessage(e.what());
	}
}

bool ParticipantCaptureDataGenerator::hasTemperatureControls(const UIntN participantIndex) const
{
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN domainIndex = 0; domainIndex < domainPropertySet.getDomainCount(); ++domainIndex)
	{
		if (domainPropertySet.getDomainProperties(domainIndex).implementsTemperatureInterface())
		{
			return true;
		}
	}
	return false;
}

bool ParticipantCaptureDataGenerator::hasCoreControls(UIntN participantIndex) const
{
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN domainIndex = 0; domainIndex < domainPropertySet.getDomainCount(); ++domainIndex)
	{
		if (domainPropertySet.getDomainProperties(domainIndex).implementsCoreControlInterface())
		{
			return true;
		}
	}
	return false;
}

bool ParticipantCaptureDataGenerator::hasPowerControls(UIntN participantIndex) const
{
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN domainIndex = 0; domainIndex < domainPropertySet.getDomainCount(); ++domainIndex)
	{
		if (domainPropertySet.getDomainProperties(domainIndex).implementsPowerControlInterface())
		{
			return true;
		}
	}
	return false;
}

bool ParticipantCaptureDataGenerator::hasPerformanceControls(UIntN participantIndex) const
{
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN domainIndex = 0; domainIndex < domainPropertySet.getDomainCount(); ++domainIndex)
	{
		if (domainPropertySet.getDomainProperties(domainIndex).implementsPerformanceControlInterface())
		{
			return true;
		}
	}
	return false;
}

bool ParticipantCaptureDataGenerator::hasDisplayControls(UIntN participantIndex) const
{
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN domainIndex = 0; domainIndex < domainPropertySet.getDomainCount(); ++domainIndex)
	{
		if (domainPropertySet.getDomainProperties(domainIndex).implementsDisplayControlInterface())
		{
			return true;
		}
	}
	return false;
}

bool ParticipantCaptureDataGenerator::hasFanControls(UIntN participantIndex) const
{
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN domainIndex = 0; domainIndex < domainPropertySet.getDomainCount(); ++domainIndex)
	{
		if (domainPropertySet.getDomainProperties(domainIndex).implementsActiveControlInterface())
		{
			return true;
		}
	}
	return false;
}

set<UIntN> ParticipantCaptureDataGenerator::getDomainsWithTemperatureThresholdControls(const UIntN participantIndex) const
{
	set<UIntN> domains;
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
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

set<UIntN> ParticipantCaptureDataGenerator::getDomainsWithCoreControls(const UIntN participantIndex) const
{
	set<UIntN> domains;
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN d = 0; d < domainPropertySet.getDomainCount(); ++d)
	{
		if (domainPropertySet.getDomainProperties(d).implementsCoreControlInterface())
		{
			domains.insert(d);
		}
	}
	return domains;
}

set<UIntN> ParticipantCaptureDataGenerator::getDomainsWithPerformanceControls(const UIntN participantIndex) const
{
	set<UIntN> domains;
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN d = 0; d < domainPropertySet.getDomainCount(); ++d)
	{
		if (domainPropertySet.getDomainProperties(d).implementsPerformanceControlInterface())
		{
			domains.insert(d);
		}
	}
	return domains;
}

set<UIntN> ParticipantCaptureDataGenerator::getDomainsWithPowerControls(const UIntN participantIndex) const
{
	set<UIntN> domains;
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN d = 0; d < domainPropertySet.getDomainCount(); ++d)
	{
		if (domainPropertySet.getDomainProperties(d).implementsPowerControlInterface())
		{
			domains.insert(d);
		}
	}
	return domains;
}

set<UIntN> ParticipantCaptureDataGenerator::getDomainsWithDisplayControls(const UIntN participantIndex) const
{
	set<UIntN> domains;
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN d = 0; d < domainPropertySet.getDomainCount(); ++d)
	{
		if (domainPropertySet.getDomainProperties(d).implementsDisplayControlInterface())
		{
			domains.insert(d);
		}
	}
	return domains;
}

set<UIntN> ParticipantCaptureDataGenerator::getDomainsWithFanControls(const UIntN participantIndex) const
{
	set<UIntN> domains;
	const auto participant = m_participantManager->getParticipantPtr(participantIndex);
	const auto domainPropertySet = participant->getDomainPropertiesSet();
	for (UIntN d = 0; d < domainPropertySet.getDomainCount(); ++d)
	{
		if (domainPropertySet.getDomainProperties(d).implementsActiveControlInterface())
		{
			domains.insert(d);
		}
	}
	return domains;
}