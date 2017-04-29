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
#include "ParticipantInterface.h"
#include "UnifiedDomain.h"
#include "ParticipantGetSpecificInfoBase.h"
#include "ParticipantSetSpecificInfoBase.h"
#include "ControlFactoryList.h"
#include "RfProfileDataSet.h"

class UnifiedParticipant : public ParticipantInterface
{
public:
	// The default constructor will use the standard class factories
	UnifiedParticipant(void);

	// The second constructor allows the factories to be passed in for validation purposes
	UnifiedParticipant(const ControlFactoryList& classFactories);

	// The destructor will delete all of the class factory pointers
	~UnifiedParticipant(void);

	// Participant
	virtual void createParticipant(
		const Guid& guid,
		UIntN participantIndex,
		Bool enabled,
		const std::string& name,
		const std::string& description,
		BusType::Type busType,
		const PciInfo& pciInfo,
		const AcpiInfo& acpiInfo,
		std::shared_ptr<ParticipantServicesInterface> participantServicesInterface) override;
	virtual void destroyParticipant(void) override;
	virtual void enableParticipant(void) override;
	virtual void disableParticipant(void) override;
	virtual Bool isParticipantEnabled(void) override;

	// Domain
	virtual void createDomain(
		const Guid& guid,
		UIntN participantIndex,
		UIntN domainIndex,
		Bool enabled,
		DomainType::Type domainType,
		const std::string& name,
		const std::string& description,
		DomainFunctionalityVersions domainFunctionalityVersions) override;
	virtual void destroyDomain(const Guid& guid) override;
	virtual void enableDomain(UIntN domainIndex) override;
	virtual void disableDomain(UIntN domainIndex) override;
	virtual Bool isDomainEnabled(UIntN domainIndex) override;

	// Misc
	virtual std::string getName() const override;
	virtual std::shared_ptr<XmlNode> getXml(UIntN domainIndex) const override;
	virtual std::shared_ptr<XmlNode> getStatusAsXml(UIntN domainIndex) const override;

	// Event handlers
	virtual void connectedStandbyEntry(void) override;
	virtual void connectedStandbyExit(void) override;
	virtual void suspend(void) override;
	virtual void resume(void) override;
	virtual void activityLoggingEnabled(UInt32 domainIndex, UInt32 capabilityBitMask) override;
	virtual void activityLoggingDisabled(UInt32 domainIndex, UInt32 capabilityBitMask) override;
	virtual void domainConfigTdpCapabilityChanged(void) override;
	virtual void domainCoreControlCapabilityChanged(void) override;
	virtual void domainDisplayControlCapabilityChanged(void) override;
	virtual void domainDisplayStatusChanged(void) override;
	virtual void domainPerformanceControlCapabilityChanged(void) override;
	virtual void domainPerformanceControlsChanged(void) override;
	virtual void domainPowerControlCapabilityChanged(void) override;
	virtual void domainPriorityChanged(void) override;
	virtual void domainRadioConnectionStatusChanged(RadioConnectionStatus::Type radioConnectionStatus) override;
	virtual void domainRfProfileChanged(void) override;
	virtual void domainTemperatureThresholdCrossed(void) override;
	virtual void participantSpecificInfoChanged(void) override;
	virtual void domainVirtualSensorCalibrationTableChanged(void) override;
	virtual void domainVirtualSensorPollingTableChanged(void) override;
	virtual void domainVirtualSensorRecalcChanged(void) override;
	virtual void domainBatteryStatusChanged(void) override;
	virtual void domainBatteryInformationChanged(void) override;
	virtual void domainPlatformPowerSourceChanged(void) override;
	virtual void domainAdapterPowerRatingChanged(void) override;
	virtual void domainChargerTypeChanged(void) override;
	virtual void domainPlatformRestOfPowerChanged(void) override;
	virtual void domainMaxBatteryPowerChanged(void) override;
	virtual void domainPlatformBatterySteadyStateChanged(void) override;
	virtual void domainACNominalVoltageChanged(void) override;
	virtual void domainACOperationalCurrentChanged(void) override;
	virtual void domainAC1msPercentageOverloadChanged(void) override;
	virtual void domainAC2msPercentageOverloadChanged(void) override;
	virtual void domainAC10msPercentageOverloadChanged(void) override;
	virtual void domainEnergyThresholdCrossed(void) override;

	// Active Controls
	virtual ActiveControlStaticCaps getActiveControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override;
	virtual ActiveControlStatus getActiveControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual ActiveControlSet getActiveControlSet(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setActiveControl(UIntN participantIndex, UIntN domainIndex, UIntN controlIndex) override;
	virtual void setActiveControl(UIntN participantIndex, UIntN domainIndex, const Percentage& fanSpeed) override;

	// Activity Status
	virtual UInt32 getEnergyThreshold(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setEnergyThreshold(UIntN participantIndex, UIntN domainIndex, UInt32 energyThreshold) override;
	virtual Temperature getPowerShareTemperatureThreshold(UIntN participantIndex, UIntN domainIndex) override;
	virtual Percentage getUtilizationThreshold(UIntN participantIndex, UIntN domainIndex) override;
	virtual Percentage getResidencyUtilization(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setEnergyThresholdInterruptFlag(
		UIntN participantIndex,
		UIntN domainIndex,
		UInt32 energyThresholdInterruptFlag) override;

	// Config TDP Controls
	virtual ConfigTdpControlDynamicCaps getConfigTdpControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
		override;
	virtual ConfigTdpControlStatus getConfigTdpControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual ConfigTdpControlSet getConfigTdpControlSet(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setConfigTdpControl(UIntN participantIndex, UIntN domainIndex, UIntN controlIndex) override;

	// Core Controls
	virtual CoreControlStaticCaps getCoreControlStaticCaps(UIntN participantIndex, UIntN domainIndex) override;
	virtual CoreControlDynamicCaps getCoreControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override;
	virtual CoreControlLpoPreference getCoreControlLpoPreference(UIntN participantIndex, UIntN domainIndex) override;
	virtual CoreControlStatus getCoreControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setActiveCoreControl(
		UIntN participantIndex,
		UIntN domainIndex,
		const CoreControlStatus& coreControlStatus) override;

	// Display Controls
	virtual DisplayControlDynamicCaps getDisplayControlDynamicCaps(UIntN participantIndex, UIntN domainIndex) override;
	virtual DisplayControlStatus getDisplayControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual UIntN getUserPreferredDisplayIndex(UIntN participantIndex, UIntN domainIndex) override;
	virtual Bool isUserPreferredIndexModified(UIntN participantIndex, UIntN domainIndex) override;
	virtual DisplayControlSet getDisplayControlSet(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setDisplayControl(UIntN participantIndex, UIntN domainIndex, UIntN displayControlIndex) override;
	virtual void setDisplayControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		DisplayControlDynamicCaps newCapabilities) override;
	virtual void setDisplayCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;

	// Peak Power Controls
	virtual Power getACPeakPower(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setACPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& acPeakPower) override;
	virtual Power getDCPeakPower(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setDCPeakPower(UIntN participantIndex, UIntN domainIndex, const Power& dcPeakPower) override;

	// Performance Controls
	virtual PerformanceControlStaticCaps getPerformanceControlStaticCaps(UIntN participantIndex, UIntN domainIndex)
		override;
	virtual PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(UIntN participantIndex, UIntN domainIndex)
		override;
	virtual PerformanceControlStatus getPerformanceControlStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual PerformanceControlSet getPerformanceControlSet(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setPerformanceControl(UIntN participantIndex, UIntN domainIndex, UIntN performanceControlIndex)
		override;
	virtual void setPerformanceControlDynamicCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PerformanceControlDynamicCaps newCapabilities) override;
	virtual void setPerformanceCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;

	// Power Controls
	virtual Bool isPowerLimitEnabled(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType)
		override;
	virtual Power getPowerLimit(UIntN participantIndex, UIntN domainIndex, PowerControlType::Type controlType) override;
	virtual void setPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	virtual void setPowerLimitIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Power& powerLimit) override;
	virtual TimeSpan getPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	virtual void setPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	virtual void setPowerLimitTimeWindowIgnoringCaps(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) override;
	virtual Percentage getPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType) override;
	virtual void setPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlType::Type controlType,
		const Percentage& dutyCycle) override;
	virtual void setPowerCapsLock(UIntN participantIndex, UIntN domainIndex, Bool lock) override;

	virtual Bool isPowerShareControl(UIntN participantIndex, UIntN domainIndex) override;
	virtual double getPidKpTerm(UIntN participantIndex, UIntN domainIndex) override;
	virtual double getPidKiTerm(UIntN participantIndex, UIntN domainIndex) override;
	virtual TimeSpan getTau(UIntN participantIndex, UIntN domainIndex) override;
	virtual TimeSpan getFastPollTime(UIntN participantIndex, UIntN domainIndex) override;
	virtual TimeSpan getSlowPollTime(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getWeightedSlowPollAvgConstant(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getRaplEnergyCounter(UIntN participantIndex, UIntN domainIndex) override;
	virtual double getRaplEnergyUnit(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getRaplEnergyCounterWidth(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getSlowPollPowerThreshold(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getInstantaneousPower(UIntN participantIndex, UIntN domainIndex) override;

	virtual PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(UIntN participantIndex, UIntN domainIndex)
		override;
	virtual void setPowerControlDynamicCapsSet(
		UIntN participantIndex,
		UIntN domainIndex,
		PowerControlDynamicCapsSet capsSet) override;

	// Power Status
	virtual PowerStatus getPowerStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getAveragePower(
		UIntN participantIndex,
		UIntN domainIndex,
		const PowerControlDynamicCaps& capabilities) override;
	virtual void setCalculatedAveragePower(UIntN participantIndex, UIntN domainIndex, Power powerValue) override;

	// Platform Power Controls
	virtual Bool isPlatformPowerLimitEnabled(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType) override;
	virtual Power getPlatformPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType) override;
	virtual void setPlatformPowerLimit(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType,
		const Power& powerLimit) override;

	virtual TimeSpan getPlatformPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType) override;
	virtual void setPlatformPowerLimitTimeWindow(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType,
		const TimeSpan& timeWindow) override;
	virtual Percentage getPlatformPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType) override;
	virtual void setPlatformPowerLimitDutyCycle(
		UIntN participantIndex,
		UIntN domainIndex,
		PlatformPowerLimitType::Type limitType,
		const Percentage& dutyCycle) override;

	// Platform Power Status
	virtual Power getMaxBatteryPower(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getPlatformRestOfPower(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getAdapterPowerRating(UIntN participantIndex, UIntN domainIndex) override;
	virtual DptfBuffer getBatteryStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual DptfBuffer getBatteryInformation(UIntN participantIndex, UIntN domainIndex) override;
	virtual PlatformPowerSource::Type getPlatformPowerSource(UIntN participantIndex, UIntN domainIndex) override;
	virtual ChargerType::Type getChargerType(UIntN participantIndex, UIntN domainIndex) override;
	virtual Power getPlatformBatterySteadyState(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getACNominalVoltage(UIntN participantIndex, UIntN domainIndex) override;
	virtual UInt32 getACOperationalCurrent(UIntN participantIndex, UIntN domainIndex) override;
	virtual Percentage getAC1msPercentageOverload(UIntN participantIndex, UIntN domainIndex) override;
	virtual Percentage getAC2msPercentageOverload(UIntN participantIndex, UIntN domainIndex) override;
	virtual Percentage getAC10msPercentageOverload(UIntN participantIndex, UIntN domainIndex) override;

	// Domain Priority
	virtual DomainPriority getDomainPriority(UIntN participantIndex, UIntN domainIndex) override;

	// RF Profile Control
	virtual RfProfileCapabilities getRfProfileCapabilities(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setRfProfileCenterFrequency(
		UIntN participantIndex,
		UIntN domainIndex,
		const Frequency& centerFrequency) override;

	// RF Profile Status
	virtual RfProfileDataSet getRfProfileDataSet(UIntN participantIndex, UIntN domainIndex) override;

	// TCC Offset Control
	virtual Temperature getTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex, const Temperature& tccOffset)
		override;
	virtual Temperature getMaxTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex) override;
	virtual Temperature getMinTccOffsetTemperature(UIntN participantIndex, UIntN domainIndex) override;

	// Temperature
	virtual TemperatureStatus getTemperatureStatus(UIntN participantIndex, UIntN domainIndex) override;
	virtual TemperatureThresholds getTemperatureThresholds(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setTemperatureThresholds(
		UIntN participantIndex,
		UIntN domainIndex,
		const TemperatureThresholds& temperatureThresholds) override;
	virtual DptfBuffer getCalibrationTable(UIntN participantIndex, UIntN domainIndex) override;
	virtual DptfBuffer getPollingTable(UIntN participantIndex, UIntN domainIndex) override;
	virtual Bool isVirtualTemperature(UIntN participantIndex, UIntN domainIndex) override;
	virtual void setVirtualTemperature(UIntN participantIndex, UIntN domainIndex, const Temperature& temperature)
		override;

	// Utilization
	virtual UtilizationStatus getUtilizationStatus(UIntN participantIndex, UIntN domainIndex) override;

	//  Get Specific Info
	virtual std::map<ParticipantSpecificInfoKey::Type, Temperature> getParticipantSpecificInfo(
		UIntN participantIndex,
		const std::vector<ParticipantSpecificInfoKey::Type>& requestedInfo) override;

	// ParticipantProperties
	virtual ParticipantProperties getParticipantProperties(UIntN participantIndex) const override;
	virtual DomainPropertiesSet getDomainPropertiesSet(UIntN participantIndex) const override;

	// Set Specific Info
	virtual void setParticipantDeviceTemperatureIndication(UIntN participantIndex, const Temperature& temperature)
		override;
	virtual void setParticipantSpecificInfo(
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

	std::shared_ptr<ParticipantGetSpecificInfoBase> m_getSpecificInfo;
	std::shared_ptr<ParticipantSetSpecificInfoBase> m_setSpecificInfo;

	std::map<UIntN, std::shared_ptr<UnifiedDomain>> m_domains;

	ControlFactoryList m_classFactories;

	void initialize(void);

	void initializeSpecificInfoControl(void);

	void clearAllCachedData(void);
	void destroyAllDomains(void);
	void throwIfDomainIndexLocationInvalid(UIntN domainIndex);
	void insertDomainAtIndexLocation(std::shared_ptr<UnifiedDomain> domain, UIntN domainIndex);

	Bool m_configTdpEventsRegistered;
	Bool m_coreControlEventsRegistered;
	Bool m_displayControlEventsRegistered;
	Bool m_domainPriorityEventsRegistered;
	Bool m_performanceControlEventsRegistered;
	Bool m_powerControlEventsRegistered;
	Bool m_rfProfileEventsRegistered;
	Bool m_temperatureEventsRegistered;
	Bool m_powerStatusEventsRegistered;
	Bool m_loggingEventsRegistered;
	Bool m_energyEventsRegistered;

	void updateDomainEventRegistrations(void);
	Bool updateDomainEventRegistration(
		UIntN total,
		Bool currentlyRegistered,
		std::set<ParticipantEvent::Type> participantEvents);

	void throwIfDomainInvalid(UIntN domainIndex) const;

	void sendConfigTdpInfoToAllDomainsAndCreateNotification(void);
	ConfigTdpControlStatus getFirstConfigTdpControlStatus(void);
	ConfigTdpControlSet getFirstConfigTdpControlSet(void);

	Power snapPowerToAbovePL1MinValue(UIntN participantIndex, UIntN domainIndex, Power powerToSet);

	// Activity Logging Utility functions
	void enableActivityLoggingForDomain(UInt32 domainIndex, UInt32 capabilityBitMask);
	void disableActivityLoggingForDomain(UInt32 domainIndex, UInt32 capabilityBitMask);
	void sendActivityLoggingDataIfEnabled(UInt32 domainIndex, eEsifCapabilityType capability);
};
