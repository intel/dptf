/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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
	virtual ~IParticipant(void){};
	virtual std::string getDiagnosticsAsXml() const = 0;
};

class dptf_export Participant : public IParticipant
{
public:
	Participant(DptfManagerInterface* dptfManager);
	virtual ~Participant(void);

	void createParticipant(
		UIntN participantIndex,
		const AppParticipantDataPtr participantDataPtr,
		Bool participantEnabled);
	void destroyParticipant(void);

	void enableParticipant(void);
	void disableParticipant(void);
	Bool isParticipantEnabled(void);

	UIntN allocateNextDomainIndex(void);
	void createDomain(UIntN domainIndex, const AppDomainDataPtr domainDataPtr, Bool domainEnabled);
	void destroyAllDomains(void);
	void destroyDomain(UIntN domainIndex);

	Bool isDomainValid(UIntN domainIndex) const;
	void enableDomain(UIntN domainIndex);
	void disableDomain(UIntN domainIndex);
	Bool isDomainEnabled(UIntN domainIndex);

	UIntN getDomainCount(void) const;

	// This will clear the cached data stored within the participant and associated domains within the framework.
	// It will not ask the actual participant to clear any of its data.
	void clearParticipantCachedData(void);

	void clearArbitrationDataForPolicy(UIntN policyIndex);

	void registerEvent(ParticipantEvent::Type participantEvent);
	void unregisterEvent(ParticipantEvent::Type participantEvent);
	Bool isEventRegistered(ParticipantEvent::Type participantEvent);

	std::string getParticipantName(void) const;
	std::string getDomainName(UIntN domainIndex);
	std::shared_ptr<XmlNode> getXml(UIntN domainIndex) const;
	std::shared_ptr<XmlNode> getStatusAsXml(UIntN domainIndex) const;
	virtual std::string getDiagnosticsAsXml() const override;
	std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex, ControlFactoryType::Type type) const;

	//
	// Event handlers
	//
	void connectedStandbyEntry(void);
	void connectedStandbyExit(void);
	void suspend(void);
	void resume(void);
	void activityLoggingEnabled(UInt32 domainId, UInt32 capabilityBitMask);
	void activityLoggingDisabled(UInt32 domainId, UInt32 capabilityBitMask);
	void domainCoreControlCapabilityChanged(void);
	void domainDisplayControlCapabilityChanged(void);
	void domainDisplayStatusChanged(void);
	void domainPerformanceControlCapabilityChanged(void);
	void domainPerformanceControlsChanged(void);
	void domainPowerControlCapabilityChanged(void);
	void domainPriorityChanged(void);
	void domainRadioConnectionStatusChanged(RadioConnectionStatus::Type radioConnectionStatus);
	void domainRfProfileChanged(void);
	void domainTemperatureThresholdCrossed(void);
	void participantSpecificInfoChanged(void);
	void domainVirtualSensorCalibrationTableChanged(void);
	void domainVirtualSensorPollingTableChanged(void);
	void domainVirtualSensorRecalcChanged(void);
	void domainBatteryStatusChanged(void);
	void domainBatteryInformationChanged(void);
	void domainBatteryHighFrequencyImpedanceChanged(void);
	void domainBatteryNoLoadVoltageChanged(void);
	void domainMaxBatteryPeakCurrentChanged(void);
	void domainPlatformPowerSourceChanged(void);
	void domainAdapterPowerRatingChanged(void);
	void domainChargerTypeChanged(void);
	void domainPlatformRestOfPowerChanged(void);
	void domainMaxBatteryPowerChanged(void);
	void domainPlatformBatterySteadyStateChanged(void);
	void domainACNominalVoltageChanged(void);
	void domainACOperationalCurrentChanged(void);
	void domainAC1msPercentageOverloadChanged(void);
	void domainAC2msPercentageOverloadChanged(void);
	void domainAC10msPercentageOverloadChanged(void);
	void domainEnergyThresholdCrossed(void);
	void domainFanCapabilityChanged(void);
	void domainSocWorkloadClassificationChanged(UInt32 socWorkloadClassification);
	void domainEppSensitivityHintChanged(UInt32 eppSensitivityHint);
	
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
	void setPowerShareEffectiveBias(UIntN domainIndex, UInt32 powerShareEffectiveBias);
	UInt32 getSocDgpuPerformanceHintPoints(UIntN domainIndex);

	// Core controls
	CoreControlStaticCaps getCoreControlStaticCaps(UIntN domainIndex);
	CoreControlDynamicCaps getCoreControlDynamicCaps(UIntN domainIndex);
	CoreControlLpoPreference getCoreControlLpoPreference(UIntN domainIndex);
	CoreControlStatus getCoreControlStatus(UIntN domainIndex);
	void setActiveCoreControl(UIntN domainIndex, UIntN policyIndex, const CoreControlStatus& coreControlStatus);

	// Display controls
	DisplayControlDynamicCaps getDisplayControlDynamicCaps(UIntN domainIndex);
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
	PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(UIntN domainIndex);
	PerformanceControlStatus getPerformanceControlStatus(UIntN domainIndex);
	PerformanceControlSet getPerformanceControlSet(UIntN domainIndex);
	void setPerformanceControl(UIntN domainIndex, UIntN policyIndex, UIntN performanceControlIndex);
	void setPerformanceControlDynamicCaps(
		UIntN domainIndex,
		UIntN policyIndex,
		PerformanceControlDynamicCaps newCapabilities);
	void setPerformanceCapsLock(UIntN domainIndex, UIntN policyIndex, Bool lock);

	// Power controls
	PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN domainIndex);
	void setPowerControlDynamicCapsSet(UIntN domainIndex, UIntN policyIndex, PowerControlDynamicCapsSet capsSet);
	Bool isPowerLimitEnabled(UIntN domainIndex, PowerControlType::Type controlType);
	Power getPowerLimit(UIntN domainIndex, PowerControlType::Type controlType);
	Bool isSocPowerFloorEnabled(UIntN domainIndex);
	Bool isSocPowerFloorSupported(UIntN domainIndex);
	Power getPowerLimitWithoutCache(UIntN domainIndex, PowerControlType::Type controlType);
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
	void removePowerLimitPolicyRequest(UIntN domainIndex, UIntN policyIndex, PowerControlType::Type controlType);
	void setPowerSharePolicyPower(UIntN domainindex, const Power& powerSharePolicyPower);

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
	void notifyForProchotDeassertion(UIntN domainIndex);

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
	void setDdrRfiTable(UIntN domainIndex, DdrfChannelBandPackage::WifiRfiDdr ddrRfiStruct);
	void setProtectRequest(UIntN domainIndex, UInt64 frequencyRate);

	// Utilization
	UtilizationStatus getUtilizationStatus(UIntN domainIndex);

	// Get specific info
	std::map<ParticipantSpecificInfoKey::Type, Temperature> getParticipantSpecificInfo(
		const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo);

	// Participant properties
	ParticipantProperties getParticipantProperties(void) const;
	DomainPropertiesSet getDomainPropertiesSet(void) const;

	// Set specific info
	void setParticipantDeviceTemperatureIndication(const Temperature& temperature);
	void setParticipantSpecificInfo(ParticipantSpecificInfoKey::Type tripPoint, const Temperature& tripValue);

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
