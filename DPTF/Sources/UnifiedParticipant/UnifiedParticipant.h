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
#include "ParticipantInterface.h"
#include "UnifiedDomain.h"
#include "ParticipantGetSpecificInfoBase.h"
#include "ParticipantSetSpecificInfoBase.h"
#include "ControlFactoryList.h"
#include "RfProfileDataSet.h"
#include "CoreActivityInfo.h"
#include "EnergyCounterInfo.h"

class UnifiedParticipant : public ParticipantInterface
{
public:
	// The default constructor will use the standard class factories
	UnifiedParticipant();

	// The second constructor allows the factories to be passed in for validation purposes
	UnifiedParticipant(const ControlFactoryList& classFactories);

	// The destructor will delete all of the class factory pointers
	~UnifiedParticipant() override;

	// Participant
	void createParticipant(
		const Guid& guid,
		UIntN participantIndex,
		Bool enabled,
		const std::string& name,
		const std::string& description,
		BusType::Type busType,
		const PciInfo& pciInfo,
		const AcpiInfo& acpiInfo,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface) override;
	void destroyParticipant() override;
	void enableParticipant() override;
	void disableParticipant() override;
	Bool isParticipantEnabled() override;

	// Domain
	void createDomain(
		const Guid& guid,
		UIntN participantIndex,
		UIntN domainIndex,
		Bool enabled,
		DomainType::Type domainType,
		const std::string& name,
		const std::string& description,
		DomainFunctionalityVersions domainFunctionalityVersions) override;
	void destroyDomain(const Guid& guid) override;
	void enableDomain(UIntN domainIndex) override;
	void disableDomain(UIntN domainIndex) override;
	Bool isDomainEnabled(UIntN domainIndex) override;

	// Misc
	std::string getName() const override;
	std::shared_ptr<XmlNode> getXml(UIntN domainIndex) const override;
	std::shared_ptr<XmlNode> getStatusAsXml(UIntN domainIndex) const override;
	std::shared_ptr<XmlNode> getDiagnosticsAsXml(UIntN domainIndex) const override;
	std::shared_ptr<XmlNode> getArbitratorStatusForPolicy(
		UIntN domainIndex,
		UIntN policyIndex,
		ControlFactoryType::Type type) const override;
	void clearCachedData() override;
	void clearCachedResults() override;
	void clearTemperatureControlCachedData() override;
	void clearBatteryStatusControlCachedData() override;
	void setRfProfileOverride(
		UIntN participantIndex, UIntN domainIndex, const DptfBuffer& rfProfileBufferData)
		override; 

	// Event handlers
	void connectedStandbyEntry() override;
	void connectedStandbyExit() override;
	void suspend() override;
	void resume() override;
	void activityLoggingEnabled(UInt32 domainIndex, UInt32 capabilityBitMask) override;
	void activityLoggingDisabled(UInt32 domainIndex, UInt32 capabilityBitMask) override;
	void domainCoreControlCapabilityChanged() override;
	void domainDisplayControlCapabilityChanged() override;
	void domainDisplayStatusChanged() override;
	void domainPerformanceControlCapabilityChanged() override;
	void domainPerformanceControlsChanged() override;
	void domainPowerControlCapabilityChanged() override;
	void domainPriorityChanged() override;
	void domainRadioConnectionStatusChanged(RadioConnectionStatus::Type radioConnectionStatus) override;
	void domainRfProfileChanged() override;
	void participantSpecificInfoChanged() override;
	void domainPlatformPowerSourceChanged() override;
	void domainAdapterPowerRatingChanged() override;
	void domainPlatformRestOfPowerChanged() override;
	void domainACNominalVoltageChanged() override;
	void domainACOperationalCurrentChanged() override;
	void domainAC1msPercentageOverloadChanged() override;
	void domainAC2msPercentageOverloadChanged() override;
	void domainAC10msPercentageOverloadChanged() override;
	void domainEnergyThresholdCrossed() override;
	void domainFanCapabilityChanged() override;
	void domainSocWorkloadClassificationChanged(UInt32 socWorkloadClassification) override;
	void domainEppSensitivityHintChanged(UInt32 eppSensitivityHint) override;
	void domainExtendedWorkloadPredictionChanged(UInt32 extendedWorkloadPrediction) override;
	void domainFanOperatingModeChanged() override;
	void domainSocPowerFloorChanged(UInt32 socPowerFloorState) override;
	void domainPcieThrottleRequested(UInt32 pcieThrottleRequest) override;

	// Activity Status
	Percentage getUtilizationThreshold(UIntN participantIndex, UIntN domainIndex) override;
	Percentage getResidencyUtilization(UIntN participantIndex, UIntN domainIndex) override;
	UInt64 getCoreActivityCounter(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getCoreActivityCounterWidth(UIntN participantIndex, UIntN domainIndex) override;
	UInt64 getTimestampCounter(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getTimestampCounterWidth(UIntN participantIndex, UIntN domainIndex) override;
	CoreActivityInfo getCoreActivityInfo(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getSocDgpuPerformanceHintPoints(UIntN participantIndex, UIntN domainIndex) override; 

	// Core Controls
	CoreControlStaticCaps getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override;
	CoreControlDynamicCaps getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override;
	CoreControlLpoPreference getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex) override;
	CoreControlStatus getCoreControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	void setActiveCoreControl(
		UIntN participantIndex,
		UIntN domainIndex,
		const CoreControlStatus& coreControlStatus) override;

	// Display Controls
	DisplayControlDynamicCaps getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override;
	DisplayControlStatus getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	UIntN getUserPreferredDisplayIndex(UIntN participantIndex, UIntN domainIndex) override;
	UIntN getUserPreferredSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex) override;
	Bool isUserPreferredIndexModified(UIntN participantIndex, UIntN domainIndex) override;
	UIntN getSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex) override;
	DisplayControlSet getDisplayControlSet(UIntN participantIndex, UIntN domainIndex) override;
	void setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex) override;
	void setSoftBrightness(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex) override;
	void updateUserPreferredSoftBrightnessIndex(UIntN participantIndex, UIntN domainIndex) override;
	void restoreUserPreferredSoftBrightness(UIntN participantIndex, UIntN domainIndex) override;
	void setDisplayControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		DisplayControlDynamicCaps newCapabilities) override;
	void setDisplayCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;

	// Energy Controls
	UInt32 getRaplEnergyCounter(UIntN participantIndex, UIntN domainIndex) override;
	double getRaplEnergyUnit(UIntN participantIndex, UIntN domainIndex) override;
	EnergyCounterInfo getRaplEnergyCounterInfo(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getRaplEnergyCounterWidth(UIntN participantIndex, UIntN domainIndex) override;
	Power getInstantaneousPower(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getEnergyThreshold(UIntN participantIndex, UIntN domainIndex) override;
	void setEnergyThreshold(UIntN participantIndex, UIntN domainIndex, UInt32 energyThreshold) override;
	void setEnergyThresholdInterruptDisable(UIntN participantIndex, UIntN domainIndex) override;

	// Peak Power Controls
	Power getACPeakPower(UIntN participantIndex, UIntN domainIndex) override;
	void setACPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& acPeakPower) override;
	Power getDCPeakPower(UIntN participantIndex, UIntN domainIndex) override;
	void setDCPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& dcPeakPower) override;

	// Performance Controls
	PerformanceControlStaticCaps getPerformanceControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
		override;
	PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
		override;
	PerformanceControlStatus getPerformanceControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	PerformanceControlSet getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex) override;
	void setPerformanceControl(UIntN participantIndex, UIntN domainIndex, UIntN performanceControlIndex)
		override;
	void setPerformanceControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PerformanceControlDynamicCaps newCapabilities) override;
	void setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;
	virtual UInt32 getPcieThrottleRequestState(UIntN participantIndex, UIntN domainIndex);

	// Power Controls
	Bool isPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType)
		override;
	Power getPowerLimit(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType) override;
	Power getPowerLimitWithoutCache(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	Bool isSocPowerFloorEnabled(UIntN participantIndex, UIntN domainIndex) override;
	Bool isSocPowerFloorSupported(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getSocPowerFloorState(UIntN participantIndex, UIntN domainIndex) override;
	void setPowerLimitMin(
		UIntN participantIndex, 
		UIntN domainIndex, 
		PowerControlType::Type controlType, 
		const Power& powerLimit) override;
	void setPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	void setPowerLimitWithoutUpdatingEnabled(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	void setPowerLimitIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	TimeSpan getPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	void setPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	void setPowerLimitTimeWindowWithoutUpdatingEnabled(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	void setPowerLimitTimeWindowIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	Percentage getPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	void setPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Percentage& dutyCycle) override;
	void setSocPowerFloorState(UIntN participantIndex, UIntN domainIndex, Bool socPowerFloorState) override;
	void clearPowerLimitMin(UIntN participantIndex, UIntN domainIndex) override;
	void clearPowerLimit(UIntN participantIndex, UIntN domainIndex) override;
	void clearCachedPowerLimits(UIntN participantIndex, UIntN domainIndex) override;
	void setPowerCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;
	TimeSpan getPowerSharePowerLimitTimeWindow(UIntN participantIndex, UIntN domainIndex) override;

	Bool isPowerShareControl(UIntN participantIndex, UIntN domainIndex) override;
	double getPidKpTerm(UIntN participantIndex, UIntN domainIndex) override;
	double getPidKiTerm(UIntN participantIndex, UIntN domainIndex) override;
	TimeSpan getAlpha(UIntN participantIndex, UIntN domainIndex) override;
	TimeSpan getFastPollTime(UIntN participantIndex, UIntN domainIndex) override;
	TimeSpan getSlowPollTime(UIntN participantIndex, UIntN domainIndex) override;
	TimeSpan getWeightedSlowPollAvgConstant(UIntN participantIndex, UIntN domainIndex) override;
	Power getSlowPollPowerThreshold(UIntN participantIndex, UIntN domainIndex) override;
	Power getThermalDesignPower(UIntN participantIndex, UIntN domainIndex) override;
	void removePowerLimitPolicyRequest(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	void setPowerSharePolicyPower(
		UIntN participantIndex, 
		UIntN domainIndex, 
		const Power& powerSharePolicyPower) override;
	void setPowerShareEffectiveBias(
		UIntN participantIndex, 
		UIntN domainIndex, 
		UInt32 powerShareEffectiveBias) override;

	PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex)
		override;
	void setPowerControlDynamicCapsSet(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlDynamicCapsSet capsSet) override;

	// Power Status
	PowerStatus getPowerStatus(UIntN participantIndex, UIntN domainIndex) override;
	Power getAveragePower(
		UIntN participantIndex,
		UIntN domainIndex,
		const PowerControlDynamicCaps& capabilities) override;
	Power getPowerValue(UIntN participantIndex, UIntN domainIndex) override;
	void setCalculatedAveragePower(UIntN participantIndex, UIntN domainIndex, Power powerValue) override;

	// System Power Controls
	Bool isSystemPowerLimitEnabled(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType) override;
	Power getSystemPowerLimit(UIntN participantIndex, UIntN domainIndex, PsysPowerLimitType::Type limitType)
		override;
	void setSystemPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType,
		const Power& powerLimit) override;

	TimeSpan getSystemPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType) override;
	void setSystemPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType,
		const TimeSpan& timeWindow) override;
	Percentage getSystemPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType) override;
	void setSystemPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PsysPowerLimitType::Type limitType,
		const Percentage& dutyCycle) override;

	// Platform Power Status
	Power getPlatformRestOfPower(UIntN participantIndex, UIntN domainIndex) override;
	Power getAdapterPowerRating(UIntN participantIndex, UIntN domainIndex) override;
	PlatformPowerSource::Type getPlatformPowerSource(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getACNominalVoltage(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getACOperationalCurrent(UIntN participantIndex, UIntN domainIndex) override;
	Percentage getAC1msPercentageOverload(UIntN participantIndex, UIntN domainIndex) override;
	Percentage getAC2msPercentageOverload(UIntN participantIndex, UIntN domainIndex) override;
	Percentage getAC10msPercentageOverload(UIntN participantIndex, UIntN domainIndex) override;
	void notifyForProcHotDeAssertion(UIntN participantIndex, UIntN domainIndex) override;

	// Domain Priority
	DomainPriority getDomainPriority(UIntN participantIndex, UIntN domainIndex) override;

	// RF Profile Control
	RfProfileCapabilities getRfProfileCapabilities(UIntN participantIndex, UIntN domainIndex) override;
	void setRfProfileCenterFrequency(
		UIntN participantIndex,
		UIntN domainIndex,
		const Frequency& centerFrequency) override;
	Percentage getSscBaselineSpreadValue(UIntN participantIndex, UIntN domainIndex) override;
	Percentage getSscBaselineThreshold(UIntN participantIndex, UIntN domainIndex) override;
	Percentage getSscBaselineGuardBand(UIntN participantIndex, UIntN domainIndex) override;

	// RF Profile Status
	RfProfileDataSet getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getWifiCapabilities(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getRfiDisable(UIntN participantIndex, UIntN domainIndex) override;
	UInt32 getDlvrSsc(UIntN participantIndex, UIntN domainIndex) override;
	UInt64 getDvfsPoints(UIntN participantIndex, UIntN domainIndex) override;
	Frequency getDlvrCenterFrequency(UIntN participantIndex, UIntN domainIndex) override;
	void setDdrRfiTable(
		UIntN participantIndex,
		UIntN domainIndex,
		const DdrfChannelBandPackage::WifiRfiDdr& ddrRfiStruct) override;
	void sendMasterControlStatus(UIntN participantIndex, UIntN domainIndex, UInt32 masterControlStatus)
		override;
	void setProtectRequest(UIntN participantIndex, UIntN domainIndex, UInt64 frequencyRate) override;
	void setDlvrCenterFrequency(UIntN participantIndex, UIntN domainIndex, Frequency frequency) override;

	// Utilization
	UtilizationStatus getUtilizationStatus(UIntN participantIndex, UIntN domainIndex) override;
	Percentage getMaxCoreUtilization(UIntN participantIndex, UIntN domainIndex) override;

	//  Get Specific Info
	std::map<ParticipantSpecificInfoKey::Type, Temperature> getParticipantSpecificInfo(
		UIntN participantIndex,
		const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo) override;

	// ParticipantProperties
	ParticipantProperties getParticipantProperties(UIntN participantIndex) const override;
	DomainPropertiesSet getDomainPropertiesSet(UIntN participantIndex) const override;

	// Set Specific Info
	void setParticipantDeviceTemperatureIndication(UIntN participantIndex, const Temperature& temperature)
		override;
	void setParticipantSpecificInfo(
		UIntN participantIndex,
		ParticipantSpecificInfoKey::Type tripPoint,
		const Temperature& tripValue) override;

private:
	// hide the copy constructor and = operator
	UnifiedParticipant(const UnifiedParticipant& rhs);
	UnifiedParticipant& operator=(const UnifiedParticipant& rhs);

	static const UIntN GetSpecificInfoVersionDefault = 1;
	static const UIntN SetSpecificInfoVersionDefault = 1;

	Guid m_guid;
	UIntN m_participantIndex;
	Bool m_enabled;
	std::string m_name;
	std::string m_description;
	BusType::Type m_busType;
	PciInfo m_pciInfo;
	AcpiInfo m_acpiInfo;
	std::shared_ptr<ParticipantServicesInterface> m_participantServicesInterface;
	std::shared_ptr<ParticipantServicesInterface> getParticipantServices() const;

	std::shared_ptr<ParticipantGetSpecificInfoBase> m_getSpecificInfo;
	std::shared_ptr<ParticipantSetSpecificInfoBase> m_setSpecificInfo;

	std::map<UIntN, std::shared_ptr<UnifiedDomain>> m_domains;

	ControlFactoryList m_classFactories;

	void initialize();

	void initializeSpecificInfoControl();

	void clearAllCachedData();
	void destroyAllDomains();
	void throwIfDomainIndexLocationInvalid(UIntN domainIndex);
	void insertDomainAtIndexLocation(std::shared_ptr<UnifiedDomain> domain, UIntN domainIndex);

	Bool m_coreControlEventsRegistered;
	Bool m_displayControlEventsRegistered;
	Bool m_domainPriorityEventsRegistered;
	Bool m_performanceControlEventsRegistered;
	Bool m_processorControlEventsRegistered;
	Bool m_powerControlEventsRegistered;
	Bool m_rfProfileEventsRegistered;
	Bool m_temperatureEventsRegistered;
	Bool m_powerStatusEventsRegistered;
	Bool m_batteryStatusEventsRegistered;
	Bool m_loggingEventsRegistered;
	Bool m_energyEventsRegistered;
	Bool m_activeControlEventsRegistered;
	Bool m_socWorkloadClassificationEventsRegistered;
	Bool m_eppSensitivityHintEventsRegistered;
	Bool m_extendedWorkloadPredictionEventsRegistered;

	void updateDomainEventRegistrations();
	Bool updateDomainEventRegistration(
		UIntN total,
		Bool currentlyRegistered,
		std::set<ParticipantEvent::Type> participantEvents);

	void throwIfDomainInvalid(UIntN domainIndex) const;

	Power snapPowerToAbovePL1MinValue(UIntN participantIndex, UIntN domainIndex, Power powerToSet);

	// Activity Logging Utility functions
	void enableActivityLoggingForDomain(UInt32 domainIndex, UInt32 capabilityBitMask);
	void disableActivityLoggingForDomain(UInt32 domainIndex, UInt32 capabilityBitMask);
	void sendActivityLoggingDataIfEnabled(UInt32 domainIndex, eEsifCapabilityType capability);
};
