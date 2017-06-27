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

#pragma once

#include "Dptf.h"
#include "Domain.h"
#include "ParticipantInterface.h"
#include "ParticipantServices.h"
#include "PlatformPowerLimitType.h"
#include "RfProfileDataSet.h"
class XmlNode;

class dptf_export Participant
{
public:
	Participant(DptfManagerInterface* dptfManager);
	~Participant(void);

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
	std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex) const;

	//
	// Event handlers
	//
	void connectedStandbyEntry(void);
	void connectedStandbyExit(void);
	void suspend(void);
	void resume(void);
	void activityLoggingEnabled(UInt32 domainId, UInt32 capabilityBitMask);
	void activityLoggingDisabled(UInt32 domainId, UInt32 capabilityBitMask);
	void domainConfigTdpCapabilityChanged(void);
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

	//
	// The following set of functions implement the ParticipantInterface related functionality
	//

	// Active Controls
	ActiveControlStaticCaps getActiveControlStaticCaps(UIntN domainIndex);
	ActiveControlStatus getActiveControlStatus(UIntN domainIndex);
	ActiveControlSet getActiveControlSet(UIntN domainIndex);
	void setActiveControl(UIntN domainIndex, UIntN policyIndex, UIntN controlIndex);
	void setActiveControl(UIntN domainIndex, UIntN policyIndex, const Percentage& fanSpeed);

	// Activity Status
	UInt32 getEnergyThreshold(UIntN domainIndex);
	void setEnergyThreshold(UIntN domainIndex, UInt32 energyThreshold);
	Temperature getPowerShareTemperatureThreshold(UIntN domainIndex);
	Percentage getUtilizationThreshold(UIntN domainIndex);
	Percentage getResidencyUtilization(UIntN domainIndex);
	void setEnergyThresholdInterruptDisable(UIntN domainIndex);

	// ConfigTdp controls
	ConfigTdpControlDynamicCaps getConfigTdpControlDynamicCaps(UIntN domainIndex);
	ConfigTdpControlStatus getConfigTdpControlStatus(UIntN domainIndex);
	ConfigTdpControlSet getConfigTdpControlSet(UIntN domainIndex);
	void setConfigTdpControl(UIntN domainIndex, UIntN policyIndex, UIntN controlIndex);

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
	Bool isUserPreferredIndexModified(UIntN domainIndex);
	DisplayControlSet getDisplayControlSet(UIntN domainIndex);
	void setDisplayControl(UIntN domainIndex, UIntN policyIndex, UIntN displayControlIndex);
	void setDisplayControlDynamicCaps(UIntN domainIndex, UIntN policyIndex, DisplayControlDynamicCaps newCapabilities);
	void setDisplayCapsLock(UIntN domainIndex, UIntN policyIndex, Bool lock);

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
	void setPowerLimit(
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
	void setPowerCapsLock(UIntN domainIndex, UIntN policyIndex, Bool lock);
	Bool isPowerShareControl(UIntN domainIndex);
	double getPidKpTerm(UIntN domainIndex);
	double getPidKiTerm(UIntN domainIndex);
	TimeSpan getAlpha(UIntN domainIndex);
	TimeSpan getFastPollTime(UIntN domainIndex);
	TimeSpan getSlowPollTime(UIntN domainIndex);
	TimeSpan getWeightedSlowPollAvgConstant(UIntN domainIndex);
	UInt32 getRaplEnergyCounter(UIntN domainIndex);
	double getRaplEnergyUnit(UIntN domainIndex);
	UInt32 getRaplEnergyCounterWidth(UIntN domainIndex);
	Power getSlowPollPowerThreshold(UIntN domainIndex);
	Power getInstantaneousPower(UIntN domainIndex);

	// Power status
	PowerStatus getPowerStatus(UIntN domainIndex);
	Power getAveragePower(UIntN domainIndex, const PowerControlDynamicCaps& capabilities);
	void setCalculatedAveragePower(UIntN domainIndex, Power powerValue);

	// Platform Power Controls
	Bool isPlatformPowerLimitEnabled(UIntN domainIndex, PlatformPowerLimitType::Type limitType);
	Power getPlatformPowerLimit(UIntN domainIndex, PlatformPowerLimitType::Type limitType);
	void setPlatformPowerLimit(
		UIntN domainIndex,
		UIntN policyIndex,
		PlatformPowerLimitType::Type limitType,
		const Power& powerLimit);
	TimeSpan getPlatformPowerLimitTimeWindow(UIntN domainIndex, PlatformPowerLimitType::Type limitType);
	void setPlatformPowerLimitTimeWindow(
		UIntN domainIndex,
		UIntN policyIndex,
		PlatformPowerLimitType::Type limitType,
		const TimeSpan& timeWindow);
	Percentage getPlatformPowerLimitDutyCycle(UIntN domainIndex, PlatformPowerLimitType::Type limitType);
	void setPlatformPowerLimitDutyCycle(
		UIntN domainIndex,
		UIntN policyIndex,
		PlatformPowerLimitType::Type limitType,
		const Percentage& dutyCycle);

	// Platform Power Status
	Power getMaxBatteryPower(UIntN domainIndex);
	Power getPlatformRestOfPower(UIntN domainIndex);
	Power getAdapterPowerRating(UIntN domainIndex);
	DptfBuffer getBatteryStatus(UIntN domainIndex);
	DptfBuffer getBatteryInformation(UIntN domainIndex);
	PlatformPowerSource::Type getPlatformPowerSource(UIntN domainIndex);
	ChargerType::Type getChargerType(UIntN domainIndex);
	Power getPlatformBatterySteadyState(UIntN domainIndex);
	UInt32 getACNominalVoltage(UIntN domainIndex);
	UInt32 getACOperationalCurrent(UIntN domainIndex);
	Percentage getAC1msPercentageOverload(UIntN domainIndex);
	Percentage getAC2msPercentageOverload(UIntN domainIndex);
	Percentage getAC10msPercentageOverload(UIntN domainIndex);
	void notifyForProchotDeassertion(UIntN domainIndex);

	// priority
	DomainPriority getDomainPriority(UIntN domainIndex);

	// RF Profile Control
	RfProfileCapabilities getRfProfileCapabilities(UIntN domainIndex);
	void setRfProfileCenterFrequency(UIntN domainIndex, UIntN policyIndex, const Frequency& centerFrequency);

	// RF Profile Status
	RfProfileDataSet getRfProfileDataSet(UIntN domainIndex);

	// TCC Offset Control
	Temperature getTccOffsetTemperature(UIntN domainIndex);
	void setTccOffsetTemperature(UIntN domainIndex, UIntN policyIndex, const Temperature& tccOffset);
	Temperature getMaxTccOffsetTemperature(UIntN domainIndex);
	Temperature getMinTccOffsetTemperature(UIntN domainIndex);

	// temperature
	TemperatureStatus getTemperatureStatus(UIntN domainIndex);
	TemperatureThresholds getTemperatureThresholds(UIntN domainIndex);
	void setTemperatureThresholds(
		UIntN domainIndex,
		UIntN policyIndex,
		const TemperatureThresholds& temperatureThresholds);
	DptfBuffer getVirtualSensorCalibrationTable(UIntN domainIndex);
	DptfBuffer getVirtualSensorPollingTable(UIntN domainIndex);
	Bool isVirtualTemperature(UIntN domainIndex);
	void setVirtualTemperature(UIntN domainIndex, const Temperature& temperature);

	// utilization
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
};
