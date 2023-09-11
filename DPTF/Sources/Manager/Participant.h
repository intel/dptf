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
#include "Domain.h"
#include "ParticipantInterface.h"
#include "ParticipantServices.h"
#include "PsysPowerLimitType.h"
#include "RfProfileDataSet.h"
#include "CoreActivityInfo.h"
#include "EnergyCounterInfo.h"
class XmlNode;

// TODO: add remaining methods to the interface and then use IParticipant everywhere
// TODO: rename all *Interface classes to be I*
class dptf_export IParticipant
{
public:
	virtual ~IParticipant(){};
	virtual std::string getDiagnosticsAsXml() const = 0;
	virtual UIntN getDomainCount() const = 0;
	virtual std::string getParticipantName() const = 0;
	virtual UIntN getParticipantIndex() const = 0;
	virtual std::shared_ptr<XmlNode> getStatusAsXml(UIntN domainIndex) const = 0;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) const = 0;
	virtual std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex, ControlFactoryType::Type type) const = 0;
	virtual ParticipantProperties getParticipantProperties() const = 0;
	virtual DomainPropertiesSet getDomainPropertiesSet() const = 0;
	virtual std::map<ParticipantSpecificInfoKey::Type, Temperature> getParticipantSpecificInfo(
		const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo) const = 0;
	virtual PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(UIntN domainIndex) const = 0;
	virtual CoreControlDynamicCaps getCoreControlDynamicCaps(UIntN domainIndex) const = 0;
	virtual PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN domainIndex) const = 0;
	virtual DisplayControlDynamicCaps getDisplayControlDynamicCaps(UIntN domainIndex) const = 0;
};

class dptf_export Participant : public IParticipant
{
public:
	Participant(DptfManagerInterface* dptfManager);
	virtual ~Participant();

	void createParticipant(
		UIntN participantIndex,
		const AppParticipantDataPtr participantDataPtr,
		Bool participantEnabled);
	void destroyParticipant();

	void enableParticipant() const;
	void disableParticipant() const;
	Bool isParticipantEnabled() const;

	UIntN allocateNextDomainIndex() const;
	void createDomain(UIntN domainIndex, const AppDomainDataPtr domainDataPtr, Bool domainEnabled);
	void destroyAllDomains();
	void destroyDomain(UIntN domainIndex);

	Bool isDomainValid(UIntN domainIndex) const;
	void enableDomain(UIntN domainIndex);
	void disableDomain(UIntN domainIndex);
	Bool isDomainEnabled(UIntN domainIndex);

	UIntN getDomainCount() const override;

	// This will clear the cached data stored within the participant and associated domains within the framework.
	// It will not ask the actual participant to clear any of its data.
	void clearParticipantCachedData();

	void clearArbitrationDataForPolicy(UIntN policyIndex);

	void registerEvent(ParticipantEvent::Type participantEvent);
	void unregisterEvent(ParticipantEvent::Type participantEvent);
	Bool isEventRegistered(ParticipantEvent::Type participantEvent) const;

	std::string getParticipantName() const override;
	UIntN getParticipantIndex() const override;
	std::string getDomainName(UIntN domainIndex);
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) const override;
	virtual std::shared_ptr<XmlNode> getStatusAsXml(UIntN domainIndex) const override;
	virtual std::string getDiagnosticsAsXml() const override;
	virtual std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex, ControlFactoryType::Type type)
		const override;

	//
	// Event handlers
	//
	void connectedStandbyEntry() const;
	void connectedStandbyExit() const;
	void suspend() const;
	void resume() const;
	void activityLoggingEnabled(UInt32 domainIndex, UInt32 capabilityBitMask) const;
	void activityLoggingDisabled(UInt32 domainIndex, UInt32 capabilityBitMask) const;
	void domainCoreControlCapabilityChanged() const;
	void domainDisplayControlCapabilityChanged() const;
	void domainDisplayStatusChanged() const;
	void domainPerformanceControlCapabilityChanged() const;
	void domainPerformanceControlsChanged() const;
	void domainPowerControlCapabilityChanged() const;
	void domainPriorityChanged() const;
	void domainRadioConnectionStatusChanged(RadioConnectionStatus::Type radioConnectionStatus) const;
	void domainRfProfileChanged() const;
	void domainTemperatureThresholdCrossed() const;
	void participantSpecificInfoChanged() const;
	void domainVirtualSensorCalibrationTableChanged() const;
	void domainVirtualSensorPollingTableChanged() const;
	void domainVirtualSensorRecalcChanged() const;
	void domainBatteryStatusChanged() const;
	void domainBatteryInformationChanged() const;
	void domainBatteryHighFrequencyImpedanceChanged() const;
	void domainBatteryNoLoadVoltageChanged() const;
	void domainMaxBatteryPeakCurrentChanged() const;
	void domainPlatformPowerSourceChanged() const;
	void domainAdapterPowerRatingChanged() const;
	void domainChargerTypeChanged() const;
	void domainPlatformRestOfPowerChanged() const;
	void domainMaxBatteryPowerChanged() const;
	void domainPlatformBatterySteadyStateChanged() const;
	void domainACNominalVoltageChanged() const;
	void domainACOperationalCurrentChanged() const;
	void domainAC1msPercentageOverloadChanged() const;
	void domainAC2msPercentageOverloadChanged() const;
	void domainAC10msPercentageOverloadChanged() const;
	void domainEnergyThresholdCrossed() const;
	void domainFanCapabilityChanged() const;
	void domainSocWorkloadClassificationChanged(UInt32 socWorkloadClassification) const;
	void domainEppSensitivityHintChanged(UInt32 eppSensitivityHint) const;
	void domainExtendedWorkloadPredictionChanged(UInt32 extendedWorkloadPrediction) const;
	void domainFanOperatingModeChanged() const;
	void domainSocPowerFloorChanged(UInt32 socPowerFloorState) const;
	void domainPcieThrottleRequested(UInt32 pcieThrottleRequest) const;

	//
	// The following set of functions implement the ParticipantInterface related functionality
	//

	// Activity Status
	Percentage getUtilizationThreshold(UIntN domainIndex);
	Percentage getResidencyUtilization(UIntN domainIndex);
	UInt64 getCoreActivityCounter(UIntN domainIndex);
	UInt32 getCoreActivityCounterWidth(UIntN domainIndex);
	UInt64 getTimestampCounter(UIntN domainIndex);
	UInt32 getTimestampCounterWidth(UIntN domainIndex);
	CoreActivityInfo getCoreActivityInfo(UIntN domainIndex);
	UInt32 getSocDgpuPerformanceHintPoints(UIntN domainIndex);

	// Core controls
	CoreControlStaticCaps getCoreControlStaticCaps(UIntN domainIndex);
	CoreControlDynamicCaps getCoreControlDynamicCaps(UIntN domainIndex) const override;
	CoreControlLpoPreference getCoreControlLpoPreference(UIntN domainIndex);
	CoreControlStatus getCoreControlStatus(UIntN domainIndex);
	void setActiveCoreControl(UIntN domainIndex, UIntN policyIndex, const CoreControlStatus& coreControlStatus);

	// Display controls
	DisplayControlDynamicCaps getDisplayControlDynamicCaps(UIntN domainIndex) const override;
	DisplayControlStatus getDisplayControlStatus(UIntN domainIndex);
	UIntN getUserPreferredDisplayIndex(UIntN domainIndex);
	UIntN getUserPreferredSoftBrightnessIndex(UIntN domainIndex);
	Bool isUserPreferredIndexModified(UIntN domainIndex);
	UIntN getSoftBrightnessIndex(UIntN domainIndex);
	DisplayControlSet getDisplayControlSet(UIntN domainIndex);
	void setDisplayControl(UIntN domainIndex, UIntN policyIndex, UIntN displayControlIndex);
	void setSoftBrightness(UIntN domainIndex, UIntN policyIndex, UIntN displayControlIndex);
	void updateUserPreferredSoftBrightnessIndex(UIntN domainIndex);
	void restoreUserPreferredSoftBrightness(UIntN domainIndex);
	void setDisplayControlDynamicCaps(UIntN domainIndex, UIntN policyIndex, DisplayControlDynamicCaps newCapabilities);
	void setDisplayCapsLock(UIntN domainIndex, UIntN policyIndex, Bool lock);

	// Energy Controls
	UInt32 getRaplEnergyCounter(UIntN domainIndex);
	EnergyCounterInfo getRaplEnergyCounterInfo(UIntN domainIndex);
	double getRaplEnergyUnit(UIntN domainIndex);
	UInt32 getRaplEnergyCounterWidth(UIntN domainIndex);
	Power getInstantaneousPower(UIntN domainIndex);
	UInt32 getEnergyThreshold(UIntN domainIndex);
	void setEnergyThreshold(UIntN domainIndex, UInt32 energyThreshold);
	void setEnergyThresholdInterruptDisable(UIntN domainIndex);

	// Peak Power controls
	Power getACPeakPower(UIntN domainIndex);
	void setACPeakPower(UIntN domainIndex, UIntN policyIndex, const Power& acPeakPower);
	Power getDCPeakPower(UIntN domainIndex);
	void setDCPeakPower(UIntN domainIndex, UIntN policyIndex, const Power& dcPeakPower);

	// Performance controls
	PerformanceControlStaticCaps getPerformanceControlStaticCaps(UIntN domainIndex);
	PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(UIntN domainIndex) const override;
	PerformanceControlStatus getPerformanceControlStatus(UIntN domainIndex);
	PerformanceControlSet getPerformanceControlSet(UIntN domainIndex);
	void setPerformanceControl(UIntN domainIndex, UIntN policyIndex, UIntN performanceControlIndex);
	void setPerformanceControlDynamicCaps(
		UIntN domainIndex,
		UIntN policyIndex,
		PerformanceControlDynamicCaps newCapabilities);
	void setPerformanceCapsLock(UIntN domainIndex, UIntN policyIndex, Bool lock);

	// Power controls
	PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN domainIndex) const override;
	void setPowerControlDynamicCapsSet(UIntN domainIndex, UIntN policyIndex, const PowerControlDynamicCapsSet& capsSet);
	Bool isPowerLimitEnabled(UIntN domainIndex, PowerControlType::Type controlType);
	Power getPowerLimit(UIntN domainIndex, PowerControlType::Type controlType);
	Bool isSocPowerFloorEnabled(UIntN domainIndex);
	Bool isSocPowerFloorSupported(UIntN domainIndex);
	UInt32 getSocPowerFloorState(UIntN domainIndex);
	Power getPowerLimitWithoutCache(UIntN domainIndex, PowerControlType::Type controlType);
	void setPowerLimitMin(
		UIntN domainIndex,
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit);
	void setPowerLimit(
		UIntN domainIndex,
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit);
	void setPowerLimitWithoutUpdatingEnabled(
		UIntN domainIndex,
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit);
	void setPowerLimitIgnoringCaps(
		UIntN domainIndex,
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit);
	TimeSpan getPowerLimitTimeWindow(UIntN domainIndex, PowerControlType::Type controlType);
	void setPowerLimitTimeWindow(
		UIntN domainIndex,
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow);
	void setPowerLimitTimeWindowWithoutUpdatingEnabled(
		UIntN domainIndex,
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow);
	void setPowerLimitTimeWindowIgnoringCaps(
		UIntN domainIndex,
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow);
	Percentage getPowerLimitDutyCycle(UIntN domainIndex, PowerControlType::Type controlType);
	void setPowerLimitDutyCycle(
		UIntN domainIndex,
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const Percentage& dutyCycle);
	void setSocPowerFloorState(UIntN domainIndex, UIntN policyIndex, Bool socPowerFloorState);
	void clearPowerLimitMin(UIntN domainIndex);
	void clearPowerLimit(UIntN domainIndex, UIntN policyIndex);
	void setPowerCapsLock(UIntN domainIndex, UIntN policyIndex, Bool lock);
	TimeSpan getPowerSharePowerLimitTimeWindow(UIntN domainIndex);
	Bool isPowerShareControl(UIntN domainIndex);
	double getPidKpTerm(UIntN domainIndex);
	double getPidKiTerm(UIntN domainIndex);
	TimeSpan getAlpha(UIntN domainIndex);
	TimeSpan getFastPollTime(UIntN domainIndex);
	TimeSpan getSlowPollTime(UIntN domainIndex);
	TimeSpan getWeightedSlowPollAvgConstant(UIntN domainIndex);
	Power getSlowPollPowerThreshold(UIntN domainIndex);
	Power getThermalDesignPower(UIntN domainIndex);
	void removePowerLimitPolicyRequest(UIntN domainIndex, UIntN policyIndex, PowerControlType::Type controlType);
	void setPowerSharePolicyPower(UIntN domainIndex, const Power& powerSharePolicyPower);
	void setPowerShareEffectiveBias(UIntN domainIndex, UInt32 powerShareEffectiveBias);

	// Power status
	PowerStatus getPowerStatus(UIntN domainIndex);
	Power getAveragePower(UIntN domainIndex, const PowerControlDynamicCaps& capabilities);
	Power getPowerValue(UIntN domainIndex);
	void setCalculatedAveragePower(UIntN domainIndex, Power powerValue);

	// System Power Controls
	Bool isSystemPowerLimitEnabled(UIntN domainIndex, PsysPowerLimitType::Type limitType);
	Power getSystemPowerLimit(UIntN domainIndex, PsysPowerLimitType::Type limitType);
	void setSystemPowerLimit(
		UIntN domainIndex,
		UIntN policyIndex,
		PsysPowerLimitType::Type limitType,
		const Power& powerLimit);
	TimeSpan getSystemPowerLimitTimeWindow(UIntN domainIndex, PsysPowerLimitType::Type limitType);
	void setSystemPowerLimitTimeWindow(
		UIntN domainIndex,
		UIntN policyIndex,
		PsysPowerLimitType::Type limitType,
		const TimeSpan& timeWindow);
	Percentage getSystemPowerLimitDutyCycle(UIntN domainIndex, PsysPowerLimitType::Type limitType);
	void setSystemPowerLimitDutyCycle(
		UIntN domainIndex,
		UIntN policyIndex,
		PsysPowerLimitType::Type limitType,
		const Percentage& dutyCycle);

	// Platform Power Status
	Power getPlatformRestOfPower(UIntN domainIndex);
	Power getAdapterPowerRating(UIntN domainIndex);
	PlatformPowerSource::Type getPlatformPowerSource(UIntN domainIndex);
	UInt32 getACNominalVoltage(UIntN domainIndex);
	UInt32 getACOperationalCurrent(UIntN domainIndex);
	Percentage getAC1msPercentageOverload(UIntN domainIndex);
	Percentage getAC2msPercentageOverload(UIntN domainIndex);
	Percentage getAC10msPercentageOverload(UIntN domainIndex);
	void notifyForProcHotDeAssertion(UIntN domainIndex);

	// Priority
	DomainPriority getDomainPriority(UIntN domainIndex);

	// RF Profile Control
	RfProfileCapabilities getRfProfileCapabilities(UIntN domainIndex);
	void setRfProfileCenterFrequency(UIntN domainIndex, UIntN policyIndex, const Frequency& centerFrequency);
	Percentage getSscBaselineSpreadValue(UIntN domainIndex);
	Percentage getSscBaselineThreshold(UIntN domainIndex);
	Percentage getSscBaselineGuardBand(UIntN domainIndex);

	// RF Profile Status
	RfProfileDataSet getRfProfileDataSet(UIntN domainIndex);
	UInt32 getWifiCapabilities(UIntN domainIndex);
	UInt32 getRfiDisable(UIntN domainIndex);
	UInt64 getDvfsPoints(UIntN domainIndex);
	UInt32 getDlvrSsc(UIntN domainIndex);
	Frequency getDlvrCenterFrequency(UIntN domainIndex);
	void setDdrRfiTable(UIntN domainIndex, const DdrfChannelBandPackage::WifiRfiDdr& ddrRfiStruct);
	void sendMasterControlStatus(UIntN domainIndex, UInt32 masterControlStatus);
	void setProtectRequest(UIntN domainIndex, UInt64 frequencyRate);
	void setRfProfileOverride(UIntN participantIndex, UIntN domainIndex, const DptfBuffer& rfProfileBufferData);
	void setDlvrCenterFrequency(UIntN domainIndex, Frequency frequency);

	// Utilization
	UtilizationStatus getUtilizationStatus(UIntN domainIndex);
	Percentage getMaxCoreUtilization(UIntN domainIndex);

	// Get specific info
	std::map<ParticipantSpecificInfoKey::Type, Temperature> getParticipantSpecificInfo(
		const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo) const override;

	// Participant properties
	ParticipantProperties getParticipantProperties() const override;
	DomainPropertiesSet getDomainPropertiesSet() const override;

	// Set specific info
	void setParticipantDeviceTemperatureIndication(const Temperature& temperature) const;
	void setParticipantSpecificInfo(ParticipantSpecificInfoKey::Type tripPoint, const Temperature& tripValue) const;

private:
	// hide the copy constructor and assignment operator.
	Participant(const Participant& rhs);
	Participant& operator=(const Participant& rhs);

	Bool m_participantCreated;

	DptfManagerInterface* m_dptfManager;
	ParticipantInterface* m_theRealParticipant;
	std::shared_ptr<ParticipantServicesInterface> m_participantServices;

	UIntN m_participantIndex;
	Guid m_participantGuid;
	std::string m_participantName;

	// track the events that will be forwarded to the participant
	std::bitset<ParticipantEvent::Max> m_registeredEvents;

	std::map<UIntN, std::shared_ptr<Domain>> m_domains;

	void throwIfDomainInvalid(UIntN domainIndex) const;
	void throwIfRealParticipantIsInvalid() const;
	EsifServicesInterface* getEsifServices() const;
};
