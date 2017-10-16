/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "DptfManager.h"
#include "EsifServicesInterface.h"
#include "Policy.h"
#include "PolicyServicesInterfaceContainer.h"
#include "PolicyServicesDomainActiveControl.h"
#include "PolicyServicesDomainActivityStatus.h"
#include "PolicyServicesDomainConfigTdpControl.h"
#include "PolicyServicesDomainCoreControl.h"
#include "PolicyServicesDomainDisplayControl.h"
#include "PolicyServicesDomainEnergyControl.h"
#include "PolicyServicesDomainPeakPowerControl.h"
#include "PolicyServicesDomainPerformanceControl.h"
#include "PolicyServicesDomainPowerControl.h"
#include "PolicyServicesDomainPowerStatus.h"
#include "PolicyServicesDomainPriority.h"
#include "PolicyServicesDomainRfProfileControl.h"
#include "PolicyServicesDomainRfProfileStatus.h"
#include "PolicyServicesDomainTccOffsetControl.h"
#include "PolicyServicesDomainTemperature.h"
#include "PolicyServicesDomainUtilization.h"
#include "PolicyServicesParticipantGetSpecificInfo.h"
#include "PolicyServicesParticipantProperties.h"
#include "PolicyServicesParticipantSetSpecificInfo.h"
#include "PolicyServicesPlatformConfigurationData.h"
#include "PolicyServicesPlatformNotification.h"
#include "PolicyServicesPlatformPowerState.h"
#include "PolicyServicesPolicyEventRegistration.h"
#include "PolicyServicesPolicyInitiatedCallback.h"
#include "PolicyServicesMessageLogging.h"
#include "PolicyWorkloadHintConfiguration.h"
#include "PolicyServicesDomainPlatformPowerControl.h"
#include "PolicyServicesDomainPlatformPowerStatus.h"
#include "PolicyServicesPlatformState.h"
#include "esif_ccb_string.h"
#include "Ver.h"
#include "AppVersion.h"

Policy::Policy(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
	, m_theRealPolicy(nullptr)
	, m_theRealPolicyCreated(false)
	, m_policyIndex(Constants::Invalid)
	, m_isPolicyLoggingEnabled(false)
	, m_esifLibrary(nullptr)
	, m_getAppVersion(nullptr)
	, m_createPolicyInstanceFuncPtr(nullptr)
	, m_destroyPolicyInstanceFuncPtr(nullptr)
{
}

Policy::~Policy(void)
{
}

void Policy::createPolicy(
	const std::string& policyFileName,
	UIntN newPolicyIndex,
	std::shared_ptr<SupportedPolicyList> supportedPolicyList)
{
	// If an exception is thrown while trying to create the policy, the PolicyManager will
	// delete the Policy instance and remove the policy completely.

	m_policyFileName = policyFileName;
	m_policyIndex = newPolicyIndex;

	// Load the .dll/.so.  If it can't load the library it will throw an exception and we're done.
	m_esifLibrary = new EsifLibrary(m_policyFileName);
	m_esifLibrary->load();

	// Make sure all of the required 'C' functions are exposed.  If there is a problem getting the function pointer
	// an exception is thrown by the EsifLibrary class.
	m_getAppVersion = (GetAppVersionFuncPtr)m_esifLibrary->getFunctionPtr("GetAppVersion");
	m_createPolicyInstanceFuncPtr = (CreatePolicyInstanceFuncPtr)m_esifLibrary->getFunctionPtr("CreatePolicyInstance");
	m_destroyPolicyInstanceFuncPtr =
		(DestroyPolicyInstanceFuncPtr)m_esifLibrary->getFunctionPtr("DestroyPolicyInstance");

	// Check app version of the policy to make sure it matches the manager version
	auto managerAppVersion = AppVersion(VER_MAJOR, VER_MINOR, VER_HOTFIX, VER_BUILD);
	auto policyAppVersion = AppVersion(m_getAppVersion());
	if (managerAppVersion != policyAppVersion)
	{
		std::stringstream message;
		message << m_policyFileName << ": Policy application version, " << policyAppVersion.toString()
			<< ", does not match the manager app version of " << managerAppVersion.toString() << ".";
		throw dptf_exception(message.str());
	}

	// Call the function that is exposed in the .dll/.so and ask it to create an instance of the class
	m_theRealPolicy = m_createPolicyInstanceFuncPtr();
	if (m_theRealPolicy == nullptr)
	{
		std::stringstream message;
		message << "Error while trying to create the policy instance: " << m_policyFileName;
		throw dptf_exception(message.str());
	}

	// Now we need to check the guid provided by the policy and make sure it is in the list of policies that are to
	// be loaded.

	m_guid = m_theRealPolicy->getGuid();

	Bool policySupported = supportedPolicyList->isPolicySupported(m_guid);
	if (policySupported == false)
	{
		std::stringstream message;
		message << "Policy [" << m_policyFileName << "] will not be loaded.  GUID not in supported policy list ["
			<< m_guid << "]";
		m_dptfManager->getEsifServices()->writeMessageWarning(message.str());
		throw policy_not_in_idsp_list();
	}

	// create all of the classes necessary to fill in the policy services interface container
	createPolicyServices();

	m_theRealPolicyCreated = true;
	m_policyName = m_theRealPolicy->getName();
	m_theRealPolicy->create(true, m_policyServices, newPolicyIndex);
	sendPolicyLogDataIfLoggingEnabled(true);
}

void Policy::destroyPolicy()
{
	try
	{
		if ((m_theRealPolicy != nullptr) && (m_theRealPolicyCreated == true))
		{
			sendPolicyLogDataIfLoggingEnabled(false);
			m_theRealPolicy->destroy();
		}
	}
	catch (...)
	{
	}

	destroyPolicyServices();

	// Call the function that is exposed in the .dll/.so and ask it to destroy the instance of the class
	if ((m_destroyPolicyInstanceFuncPtr != nullptr) && (m_theRealPolicy != nullptr))
	{
		m_destroyPolicyInstanceFuncPtr(m_theRealPolicy);
	}

	if (m_esifLibrary != nullptr)
	{
		m_esifLibrary->unload();
	}

	DELETE_MEMORY_TC(m_esifLibrary);
}

Guid Policy::getGuid(void)
{
	return m_guid;
}

void Policy::bindParticipant(UIntN participantIndex)
{
	m_theRealPolicy->bindParticipant(participantIndex);
}

void Policy::unbindParticipant(UIntN participantIndex)
{
	m_theRealPolicy->unbindParticipant(participantIndex);
}

void Policy::bindDomain(UIntN participantIndex, UIntN domainIndex)
{
	m_theRealPolicy->bindDomain(participantIndex, domainIndex);
}

void Policy::unbindDomain(UIntN participantIndex, UIntN domainIndex)
{
	m_theRealPolicy->unbindDomain(participantIndex, domainIndex);
}

void Policy::enable(void)
{
	m_theRealPolicy->enable();
}

void Policy::disable(void)
{
	m_theRealPolicy->disable();
}

std::string Policy::getName(void) const
{
	return m_policyName;
}

std::string Policy::getPolicyFileName(void) const
{
	return m_policyFileName;
}

std::string Policy::getStatusAsXml(void) const
{
	return m_theRealPolicy->getStatusAsXml();
}

std::string Policy::getDiagnosticsAsXml(void) const
{
	return m_theRealPolicy->getDiagnosticsAsXml();
}

void Policy::executeConnectedStandbyEntry(void)
{
	if (isEventRegistered(PolicyEvent::DptfConnectedStandbyEntry))
	{
		m_theRealPolicy->connectedStandbyEntry();
	}
}

void Policy::executeConnectedStandbyExit(void)
{
	if (isEventRegistered(PolicyEvent::DptfConnectedStandbyExit))
	{
		m_theRealPolicy->connectedStandbyExit();
	}
}

void Policy::executeSuspend(void)
{
	if (isEventRegistered(PolicyEvent::DptfSuspend))
	{
		m_theRealPolicy->suspend();
	}
}

void Policy::executeResume(void)
{
	if (isEventRegistered(PolicyEvent::DptfResume))
	{
		m_theRealPolicy->resume();
	}
}

void Policy::executeDomainConfigTdpCapabilityChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainConfigTdpCapabilityChanged))
	{
		m_theRealPolicy->domainConfigTdpCapabilityChanged(participantIndex);
	}
}

void Policy::executeDomainCoreControlCapabilityChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainCoreControlCapabilityChanged))
	{
		m_theRealPolicy->domainCoreControlCapabilityChanged(participantIndex);
	}
}

void Policy::executeDomainDisplayControlCapabilityChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainDisplayControlCapabilityChanged))
	{
		m_theRealPolicy->domainDisplayControlCapabilityChanged(participantIndex);
	}
}

void Policy::executeDomainDisplayStatusChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainDisplayStatusChanged))
	{
		m_theRealPolicy->domainDisplayStatusChanged(participantIndex);
	}
}

void Policy::executeDomainPerformanceControlCapabilityChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainPerformanceControlCapabilityChanged))
	{
		m_theRealPolicy->domainPerformanceControlCapabilityChanged(participantIndex);
	}
}

void Policy::executeDomainPerformanceControlsChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainPerformanceControlsChanged))
	{
		m_theRealPolicy->domainPerformanceControlsChanged(participantIndex);
	}
}

void Policy::executeDomainPowerControlCapabilityChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainPowerControlCapabilityChanged))
	{
		m_theRealPolicy->domainPowerControlCapabilityChanged(participantIndex);
	}
}

void Policy::executeDomainPriorityChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainPriorityChanged))
	{
		m_theRealPolicy->domainPriorityChanged(participantIndex);
	}
}

void Policy::executeDomainRadioConnectionStatusChanged(
	UIntN participantIndex,
	RadioConnectionStatus::Type radioConnectionStatus)
{
	if (isEventRegistered(PolicyEvent::DomainRadioConnectionStatusChanged))
	{
		m_theRealPolicy->domainRadioConnectionStatusChanged(participantIndex, radioConnectionStatus);
	}
}

void Policy::executeDomainRfProfileChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainRfProfileChanged))
	{
		m_theRealPolicy->domainRfProfileChanged(participantIndex);
	}
}

void Policy::executeDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainTemperatureThresholdCrossed))
	{
		m_theRealPolicy->domainTemperatureThresholdCrossed(participantIndex);
	}
}

void Policy::executeDomainEnergyThresholdCrossed(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainEnergyThresholdCrossed))
	{
		m_theRealPolicy->domainEnergyThresholdCrossed(participantIndex);
	}
}

void Policy::executeDomainVirtualSensorCalibrationTableChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainVirtualSensorCalibrationTableChanged))
	{
		m_theRealPolicy->domainVirtualSensorCalibrationTableChanged(participantIndex);
	}
}

void Policy::executeDomainVirtualSensorPollingTableChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainVirtualSensorPollingTableChanged))
	{
		m_theRealPolicy->domainVirtualSensorPollingTableChanged(participantIndex);
	}
}

void Policy::executeDomainVirtualSensorRecalcChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainVirtualSensorRecalcChanged))
	{
		m_theRealPolicy->domainVirtualSensorRecalcChanged(participantIndex);
	}
}

void Policy::executeParticipantSpecificInfoChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::ParticipantSpecificInfoChanged))
	{
		m_theRealPolicy->participantSpecificInfoChanged(participantIndex);
	}
}

void Policy::executePolicyActiveRelationshipTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyActiveRelationshipTableChanged))
	{
		m_theRealPolicy->activeRelationshipTableChanged();
	}
}

void Policy::executePolicyCoolingModePolicyChanged(CoolingMode::Type coolingMode)
{
	if (isEventRegistered(PolicyEvent::PolicyCoolingModePolicyChanged))
	{
		m_theRealPolicy->coolingModePolicyChanged(coolingMode);
	}
}

void Policy::executePolicyForegroundApplicationChanged(const std::string& foregroundApplicationName)
{
	if (isEventRegistered(PolicyEvent::PolicyForegroundApplicationChanged))
	{
		m_theRealPolicy->foregroundApplicationChanged(foregroundApplicationName);
	}
}

void Policy::executePolicyInitiatedCallback(UInt64 policyDefinedEventCode, UInt64 param1, void* param2)
{
	m_theRealPolicy->policyInitiatedCallback(policyDefinedEventCode, param1, param2);
}

void Policy::executePolicyOperatingSystemConfigTdpLevelChanged(UIntN configTdpLevel)
{
	if (isEventRegistered(PolicyEvent::PolicyOperatingSystemConfigTdpLevelChanged))
	{
		m_theRealPolicy->operatingSystemConfigTdpLevelChanged(configTdpLevel);
	}
}

void Policy::executePolicyOperatingSystemPowerSourceChanged(OsPowerSource::Type powerSource)
{
	if (isEventRegistered(PolicyEvent::PolicyOperatingSystemPowerSourceChanged))
	{
		m_theRealPolicy->operatingSystemPowerSourceChanged(powerSource);
	}
}

void Policy::executePolicyOperatingSystemLidStateChanged(OsLidState::Type lidState)
{
	if (isEventRegistered(PolicyEvent::PolicyOperatingSystemLidStateChanged))
	{
		m_theRealPolicy->operatingSystemLidStateChanged(lidState);
	}
}

void Policy::executePolicyOperatingSystemBatteryPercentageChanged(UIntN batteryPercentage)
{
	if (isEventRegistered(PolicyEvent::PolicyOperatingSystemBatteryPercentageChanged))
	{
		m_theRealPolicy->operatingSystemBatteryPercentageChanged(batteryPercentage);
	}
}

void Policy::executePolicyOperatingSystemPlatformTypeChanged(OsPlatformType::Type platformType)
{
	if (isEventRegistered(PolicyEvent::PolicyOperatingSystemPlatformTypeChanged))
	{
		m_theRealPolicy->operatingSystemPlatformTypeChanged(platformType);
	}
}

void Policy::executePolicyOperatingSystemDockModeChanged(OsDockMode::Type dockMode)
{
	if (isEventRegistered(PolicyEvent::PolicyOperatingSystemDockModeChanged))
	{
		m_theRealPolicy->operatingSystemDockModeChanged(dockMode);
	}
}

void Policy::executePolicyOperatingSystemEmergencyCallModeStateChanged(OnOffToggle::Type emergencyCallModeState)
{
	if (isEventRegistered(PolicyEvent::PolicyOperatingSystemMobileNotification))
	{
		m_theRealPolicy->operatingSystemEmergencyCallModeStateChanged(emergencyCallModeState);
	}
}

void Policy::executePolicyOperatingSystemMobileNotification(
	OsMobileNotificationType::Type notificationType,
	UIntN value)
{
	if (isEventRegistered(PolicyEvent::PolicyOperatingSystemMobileNotification))
	{
		m_theRealPolicy->operatingSystemMobileNotification(notificationType, value);
	}
}

void Policy::executePolicyPassiveTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyPassiveTableChanged))
	{
		m_theRealPolicy->passiveTableChanged();
	}
}

void Policy::executePolicySensorOrientationChanged(SensorOrientation::Type sensorOrientation)
{
	if (isEventRegistered(PolicyEvent::PolicySensorOrientationChanged))
	{
		m_theRealPolicy->sensorOrientationChanged(sensorOrientation);
	}
}

void Policy::executePolicySensorMotionChanged(OnOffToggle::Type sensorMotion)
{
	if (isEventRegistered(PolicyEvent::PolicySensorMotionChanged))
	{
		m_theRealPolicy->sensorMotionChanged(sensorMotion);
	}
}

void Policy::executePolicySensorSpatialOrientationChanged(SensorSpatialOrientation::Type sensorSpatialOrientation)
{
	if (isEventRegistered(PolicyEvent::PolicySensorSpatialOrientationChanged))
	{
		m_theRealPolicy->sensorSpatialOrientationChanged(sensorSpatialOrientation);
	}
}

void Policy::executePolicyThermalRelationshipTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyThermalRelationshipTableChanged))
	{
		m_theRealPolicy->thermalRelationshipTableChanged();
	}
}

void Policy::executePolicyAdaptivePerformanceConditionsTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyAdaptivePerformanceConditionsTableChanged))
	{
		m_theRealPolicy->adaptivePerformanceConditionsTableChanged();
	}
}

void Policy::executePolicyAdaptivePerformanceParticipantConditionTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyAdaptivePerformanceParticipantConditionTableChanged))
	{
		m_theRealPolicy->adaptivePerformanceParticipantConditionTableChanged();
	}
}

void Policy::executePolicyAdaptivePerformanceActionsTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyAdaptivePerformanceActionsTableChanged))
	{
		m_theRealPolicy->adaptivePerformanceActionsTableChanged();
	}
}

void Policy::executePolicyOemVariablesChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyOemVariablesChanged))
	{
		m_theRealPolicy->oemVariablesChanged();
	}
}

void Policy::executePolicyPowerBossConditionsTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyPowerBossConditionsTableChanged))
	{
		m_theRealPolicy->powerBossConditionsTableChanged();
	}
}

void Policy::executePolicyPowerBossActionsTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyPowerBossActionsTableChanged))
	{
		m_theRealPolicy->powerBossActionsTableChanged();
	}
}

void Policy::executePolicyPowerBossMathTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyPowerBossMathTableChanged))
	{
		m_theRealPolicy->powerBossMathTableChanged();
	}
}

void Policy::executePolicyOperatingSystemPowerSchemePersonalityChanged(
	OsPowerSchemePersonality::Type powerSchemePersonality)
{
	if (isEventRegistered(PolicyEvent::PolicyOperatingSystemPowerSchemePersonalityChanged))
	{
		m_theRealPolicy->operatingSystemPowerSchemePersonalityChanged(powerSchemePersonality);
	}
}

void Policy::executePolicyActivityLoggingEnabled(void)
{
	enablePolicyLogging();
	sendPolicyLogDataIfLoggingEnabled(true);
}

void Policy::executePolicyActivityLoggingDisabled(void)
{
	disablePolicyLogging();
}

void Policy::executePolicyEmergencyCallModeTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyEmergencyCallModeTableChanged))
	{
		m_theRealPolicy->emergencyCallModeTableChanged();
	}
}

void Policy::executePolicyPidAlgorithmTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyPidAlgorithmTableChanged))
	{
		m_theRealPolicy->pidAlgorithmTableChanged();
	}
}

void Policy::executePolicyActiveControlPointRelationshipTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyActiveControlPointRelationshipTableChanged))
	{
		m_theRealPolicy->activeControlPointRelationshipTableChanged();
	}
}

void Policy::executePolicyPowerShareAlgorithmTableChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyPowerShareAlgorithmTableChanged))
	{
		m_theRealPolicy->powerShareAlgorithmTableChanged();
	}
}

void Policy::executePowerLimitChanged(void)
{
	if (isEventRegistered(PolicyEvent::PowerLimitChanged))
	{
		m_theRealPolicy->powerLimitChanged();
	}
}

void Policy::executePolicyWorkloadHintConfigurationChanged(void)
{
	if (isEventRegistered(PolicyEvent::PolicyWorkloadHintConfigurationChanged))
	{
		m_theRealPolicy->workloadHintConfigurationChanged();
	}
}

void Policy::sendPolicyLogDataIfLoggingEnabled(Bool loaded)
{
	try
	{
		if (isPolicyLoggingEnabled() == true)
		{
			EsifPolicyLogData policyData;
			policyData.policyIndex = m_policyIndex;
			policyData.loaded = loaded;
			m_guid.copyToBuffer(policyData.policyGuid);
			esif_ccb_strncpy(policyData.policyName, m_policyName.c_str(), sizeof(policyData.policyName));
			esif_ccb_strncpy(policyData.policyFileName, m_policyFileName.c_str(), sizeof(policyData.policyFileName));

			esif_data esifEventData = {
				esif_data_type::ESIF_DATA_STRUCTURE, &policyData, sizeof(policyData), sizeof(policyData) };

			m_dptfManager->getEsifServices()->sendDptfEvent(
				PolicyEvent::ToFrameworkEvent(PolicyEvent::DptfPolicyLoadedUnloadedEvent),
				Constants::Esif::NoParticipant,
				Constants::Esif::NoDomain,
				esifEventData);
		}
	}
	catch (...)
	{
	}
}

void Policy::registerEvent(PolicyEvent::Type policyEvent)
{
	m_registeredEvents.set(policyEvent);
}

void Policy::unregisterEvent(PolicyEvent::Type policyEvent)
{
	m_registeredEvents.reset(policyEvent);
}

Bool Policy::isEventRegistered(PolicyEvent::Type policyEvent)
{
	return m_registeredEvents.test(policyEvent);
}

void Policy::createPolicyServices(void)
{
	m_policyServices.domainActiveControl = new PolicyServicesDomainActiveControl(m_dptfManager, m_policyIndex);
	m_policyServices.domainActivityStatus = new PolicyServicesDomainActivityStatus(m_dptfManager, m_policyIndex);
	m_policyServices.domainConfigTdpControl = new PolicyServicesDomainConfigTdpControl(m_dptfManager, m_policyIndex);
	m_policyServices.domainCoreControl = new PolicyServicesDomainCoreControl(m_dptfManager, m_policyIndex);
	m_policyServices.domainDisplayControl = new PolicyServicesDomainDisplayControl(m_dptfManager, m_policyIndex);
	m_policyServices.domainEnergyControl = new PolicyServicesDomainEnergyControl(m_dptfManager, m_policyIndex);
	m_policyServices.domainPeakPowerControl = new PolicyServicesDomainPeakPowerControl(m_dptfManager, m_policyIndex);
	m_policyServices.domainPerformanceControl =
		new PolicyServicesDomainPerformanceControl(m_dptfManager, m_policyIndex);
	m_policyServices.domainPowerControl = new PolicyServicesDomainPowerControl(m_dptfManager, m_policyIndex);
	m_policyServices.domainPowerStatus = new PolicyServicesDomainPowerStatus(m_dptfManager, m_policyIndex);
	m_policyServices.domainPlatformPowerControl =
		new PolicyServicesDomainPlatformPowerControl(m_dptfManager, m_policyIndex);
	m_policyServices.domainPlatformPowerStatus =
		new PolicyServicesDomainPlatformPowerStatus(m_dptfManager, m_policyIndex);
	m_policyServices.domainPriority = new PolicyServicesDomainPriority(m_dptfManager, m_policyIndex);
	m_policyServices.domainRfProfileControl = new PolicyServicesDomainRfProfileControl(m_dptfManager, m_policyIndex);
	m_policyServices.domainRfProfileStatus = new PolicyServicesDomainRfProfileStatus(m_dptfManager, m_policyIndex);
	m_policyServices.domainTccOffsetControl = new PolicyServicesDomainTccOffsetControl(m_dptfManager, m_policyIndex);
	m_policyServices.domainTemperature = new PolicyServicesDomainTemperature(m_dptfManager, m_policyIndex);
	m_policyServices.domainUtilization = new PolicyServicesDomainUtilization(m_dptfManager, m_policyIndex);
	m_policyServices.participantGetSpecificInfo =
		new PolicyServicesParticipantGetSpecificInfo(m_dptfManager, m_policyIndex);
	m_policyServices.participantProperties = new PolicyServicesParticipantProperties(m_dptfManager, m_policyIndex);
	m_policyServices.participantSetSpecificInfo =
		new PolicyServicesParticipantSetSpecificInfo(m_dptfManager, m_policyIndex);
	m_policyServices.platformConfigurationData =
		new PolicyServicesPlatformConfigurationData(m_dptfManager, m_policyIndex);
	m_policyServices.platformNotification = new PolicyServicesPlatformNotification(m_dptfManager, m_policyIndex);
	m_policyServices.platformPowerState = new PolicyServicesPlatformPowerState(m_dptfManager, m_policyIndex);
	m_policyServices.policyEventRegistration = new PolicyServicesPolicyEventRegistration(m_dptfManager, m_policyIndex);
	m_policyServices.policyInitiatedCallback = new PolicyServicesPolicyInitiatedCallback(m_dptfManager, m_policyIndex);
	m_policyServices.messageLogging = new PolicyServicesMessageLogging(m_dptfManager, m_policyIndex);
	m_policyServices.workloadHintConfiguration =
		new PolicyWorkloadHintConfiguration(m_policyServices.platformConfigurationData);
	m_policyServices.platformState = new PolicyServicesPlatformState(m_dptfManager, m_policyIndex);
}

void Policy::destroyPolicyServices(void)
{
	DELETE_MEMORY_TC(m_policyServices.domainActiveControl);
	DELETE_MEMORY_TC(m_policyServices.domainActivityStatus);
	DELETE_MEMORY_TC(m_policyServices.domainConfigTdpControl);
	DELETE_MEMORY_TC(m_policyServices.domainCoreControl);
	DELETE_MEMORY_TC(m_policyServices.domainDisplayControl);
	DELETE_MEMORY_TC(m_policyServices.domainEnergyControl);
	DELETE_MEMORY_TC(m_policyServices.domainPeakPowerControl);
	DELETE_MEMORY_TC(m_policyServices.domainPerformanceControl);
	DELETE_MEMORY_TC(m_policyServices.domainPowerControl);
	DELETE_MEMORY_TC(m_policyServices.domainPowerStatus);
	DELETE_MEMORY_TC(m_policyServices.domainPlatformPowerControl);
	DELETE_MEMORY_TC(m_policyServices.domainPlatformPowerStatus);
	DELETE_MEMORY_TC(m_policyServices.domainPriority);
	DELETE_MEMORY_TC(m_policyServices.domainRfProfileControl);
	DELETE_MEMORY_TC(m_policyServices.domainRfProfileStatus);
	DELETE_MEMORY_TC(m_policyServices.domainTccOffsetControl);
	DELETE_MEMORY_TC(m_policyServices.domainTemperature);
	DELETE_MEMORY_TC(m_policyServices.domainUtilization);
	DELETE_MEMORY_TC(m_policyServices.participantGetSpecificInfo);
	DELETE_MEMORY_TC(m_policyServices.participantProperties);
	DELETE_MEMORY_TC(m_policyServices.participantSetSpecificInfo);
	DELETE_MEMORY_TC(m_policyServices.platformConfigurationData);
	DELETE_MEMORY_TC(m_policyServices.platformNotification);
	DELETE_MEMORY_TC(m_policyServices.platformPowerState);
	DELETE_MEMORY_TC(m_policyServices.policyEventRegistration);
	DELETE_MEMORY_TC(m_policyServices.policyInitiatedCallback);
	DELETE_MEMORY_TC(m_policyServices.messageLogging);
	DELETE_MEMORY_TC(m_policyServices.workloadHintConfiguration);
	DELETE_MEMORY_TC(m_policyServices.platformState);
}

void Policy::executeDomainBatteryStatusChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainBatteryStatusChanged))
	{
		m_theRealPolicy->domainBatteryStatusChanged(participantIndex);
	}
}

void Policy::executeDomainBatteryInformationChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainBatteryInformationChanged))
	{
		m_theRealPolicy->domainBatteryInformationChanged(participantIndex);
	}
}

void Policy::executeDomainPlatformPowerSourceChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainPlatformPowerSourceChanged))
	{
		m_theRealPolicy->domainPlatformPowerSourceChanged(participantIndex);
	}
}

void Policy::executeDomainAdapterPowerRatingChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainAdapterPowerRatingChanged))
	{
		m_theRealPolicy->domainAdapterPowerRatingChanged(participantIndex);
	}
}

void Policy::executeDomainChargerTypeChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainChargerTypeChanged))
	{
		m_theRealPolicy->domainChargerTypeChanged(participantIndex);
	}
}

void Policy::executeDomainPlatformRestOfPowerChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainPlatformRestOfPowerChanged))
	{
		m_theRealPolicy->domainPlatformRestOfPowerChanged(participantIndex);
	}
}

void Policy::executeDomainMaxBatteryPowerChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainMaxBatteryPowerChanged))
	{
		m_theRealPolicy->domainMaxBatteryPowerChanged(participantIndex);
	}
}

void Policy::executeDomainPlatformBatterySteadyStateChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainPlatformBatterySteadyStateChanged))
	{
		m_theRealPolicy->domainPlatformBatterySteadyStateChanged(participantIndex);
	}
}

void Policy::executeDomainACNominalVoltageChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainACNominalVoltageChanged))
	{
		m_theRealPolicy->domainACNominalVoltageChanged(participantIndex);
	}
}

void Policy::executeDomainACOperationalCurrentChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainACOperationalCurrentChanged))
	{
		m_theRealPolicy->domainACOperationalCurrentChanged(participantIndex);
	}
}

void Policy::executeDomainAC1msPercentageOverloadChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainAC1msPercentageOverloadChanged))
	{
		m_theRealPolicy->domainAC1msPercentageOverloadChanged(participantIndex);
	}
}

void Policy::executeDomainAC2msPercentageOverloadChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainAC2msPercentageOverloadChanged))
	{
		m_theRealPolicy->domainAC2msPercentageOverloadChanged(participantIndex);
	}
}

void Policy::executeDomainAC10msPercentageOverloadChanged(UIntN participantIndex)
{
	if (isEventRegistered(PolicyEvent::DomainAC10msPercentageOverloadChanged))
	{
		m_theRealPolicy->domainAC10msPercentageOverloadChanged(participantIndex);
	}
}

Bool Policy::isPolicyLoggingEnabled()
{
	return m_isPolicyLoggingEnabled;
}

void Policy::enablePolicyLogging(void)
{
	m_isPolicyLoggingEnabled = true;
}

void Policy::disablePolicyLogging(void)
{
	m_isPolicyLoggingEnabled = false;
}