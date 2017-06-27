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
#include "esif_sdk_iface_app.h"
#include "Arbitrator.h"
#include "PlatformPowerLimitType.h"
#include "RfProfileDataSet.h"
class Domain
{
public:
	Domain(DptfManagerInterface* dptfManager);
	~Domain(void);

	void createDomain(
		UIntN participantIndex,
		UIntN domainIndex,
		ParticipantInterface* participantInterface,
		const AppDomainDataPtr domainDataPtr,
		Bool domainEnabled);
	void destroyDomain(void);

	void enableDomain(void);
	void disableDomain(void);
	Bool isDomainEnabled(void);
	Bool isCreated(void);

	std::string getDomainName(void) const;

	// This will clear the cached data stored within this class in the framework.  It will not ask the
	// actual domain to clear its cache.
	void clearDomainCachedData(void);

	void clearArbitrationDataForPolicy(UIntN policyIndex);
	std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex) const;

	//
	// The following set of functions pass the call through to the actual domain.  They
	// also provide caching so each policy sees the same representation of the platform
	// for the duration of a work item.
	//

	// Active Controls
	ActiveControlStaticCaps getActiveControlStaticCaps(void);
	ActiveControlStatus getActiveControlStatus(void);
	ActiveControlSet getActiveControlSet(void);
	void setActiveControl(UIntN policyIndex, UIntN controlIndex);
	void setActiveControl(UIntN policyIndex, const Percentage& fanSpeed);

	// Activity Status
	UInt32 getEnergyThreshold();
	void setEnergyThreshold(UInt32 energyThreshold);
	Temperature getPowerShareTemperatureThreshold();
	Percentage getUtilizationThreshold();
	Percentage getResidencyUtilization();
	void setEnergyThresholdInterruptDisable();

	// ConfigTdp controls
	ConfigTdpControlDynamicCaps getConfigTdpControlDynamicCaps(void);
	ConfigTdpControlStatus getConfigTdpControlStatus(void);
	ConfigTdpControlSet getConfigTdpControlSet(void);
	void setConfigTdpControl(UIntN policyIndex, UIntN controlIndex);

	// Core controls
	CoreControlStaticCaps getCoreControlStaticCaps(void);
	CoreControlDynamicCaps getCoreControlDynamicCaps(void);
	CoreControlLpoPreference getCoreControlLpoPreference(void);
	CoreControlStatus getCoreControlStatus(void);
	void setActiveCoreControl(UIntN policyIndex, const CoreControlStatus& coreControlStatus);

	// Display controls
	DisplayControlDynamicCaps getDisplayControlDynamicCaps(void);
	UIntN getUserPreferredDisplayIndex(void);
	Bool isUserPreferredIndexModified(void);
	DisplayControlStatus getDisplayControlStatus(void);
	DisplayControlSet getDisplayControlSet(void);
	void setDisplayControl(UIntN policyIndex, UIntN displayControlIndex);
	void setDisplayControlDynamicCaps(UIntN policyIndex, DisplayControlDynamicCaps newCapabilities);
	void setDisplayCapsLock(UIntN policyIndex, Bool lock);

	// Peak Power Controls
	Power getACPeakPower(void);
	void setACPeakPower(UIntN policyIndex, const Power& acPeakPower);
	Power getDCPeakPower(void);
	void setDCPeakPower(UIntN policyIndex, const Power& dcPeakPower);

	// Performance controls
	PerformanceControlStaticCaps getPerformanceControlStaticCaps(void);
	PerformanceControlDynamicCaps getPerformanceControlDynamicCaps(void);
	PerformanceControlStatus getPerformanceControlStatus(void);
	PerformanceControlSet getPerformanceControlSet(void);
	void setPerformanceControl(UIntN policyIndex, UIntN performanceControlIndex);
	void setPerformanceControlDynamicCaps(UIntN policyIndex, PerformanceControlDynamicCaps newCapabilities);
	void setPerformanceCapsLock(UIntN policyIndex, Bool lock);

	// Power controls
	PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(void);
	void setPowerControlDynamicCapsSet(UIntN policyIndex, PowerControlDynamicCapsSet capsSet);
	Bool isPowerLimitEnabled(PowerControlType::Type controlType);
	Power getPowerLimit(PowerControlType::Type controlType);
	void setPowerLimit(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit);
	void setPowerLimitIgnoringCaps(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit);
	TimeSpan getPowerLimitTimeWindow(PowerControlType::Type controlType);
	void setPowerLimitTimeWindow(UIntN policyIndex, PowerControlType::Type controlType, const TimeSpan& timeWindow);
	void setPowerLimitTimeWindowIgnoringCaps(
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow);
	Percentage getPowerLimitDutyCycle(PowerControlType::Type controlType);
	void setPowerLimitDutyCycle(UIntN policyIndex, PowerControlType::Type controlType, const Percentage& dutyCycle);
	void setPowerCapsLock(UIntN policyIndex, Bool lock);
	Bool isPowerShareControl();
	double getPidKpTerm();
	double getPidKiTerm();
	TimeSpan getAlpha();
	TimeSpan getFastPollTime();
	TimeSpan getSlowPollTime();
	TimeSpan getWeightedSlowPollAvgConstant();
	UInt32 getRaplEnergyCounter();
	double getRaplEnergyUnit();
	UInt32 getRaplEnergyCounterWidth();
	Power getSlowPollPowerThreshold();
	Power getInstantaneousPower();

	// Power status
	PowerStatus getPowerStatus(void);
	Power getAveragePower(const PowerControlDynamicCaps& capabilities);
	void setCalculatedAveragePower(Power powerValue);

	// Platform Power Controls
	Bool isPlatformPowerLimitEnabled(PlatformPowerLimitType::Type limitType);
	Power getPlatformPowerLimit(PlatformPowerLimitType::Type limitType);
	void setPlatformPowerLimit(UIntN policyIndex, PlatformPowerLimitType::Type limitType, const Power& powerLimit);
	TimeSpan getPlatformPowerLimitTimeWindow(PlatformPowerLimitType::Type limitType);
	void setPlatformPowerLimitTimeWindow(
		UIntN policyIndex,
		PlatformPowerLimitType::Type limitType,
		const TimeSpan& timeWindow);
	Percentage getPlatformPowerLimitDutyCycle(PlatformPowerLimitType::Type limitType);
	void setPlatformPowerLimitDutyCycle(
		UIntN policyIndex,
		PlatformPowerLimitType::Type limitType,
		const Percentage& dutyCycle);

	// Platform Power Status
	Power getMaxBatteryPower(void);
	Power getPlatformRestOfPower(void);
	Power getAdapterPowerRating(void);
	DptfBuffer getBatteryStatus(void);
	DptfBuffer getBatteryInformation(void);
	PlatformPowerSource::Type getPlatformPowerSource(void);
	ChargerType::Type getChargerType(void);
	Power getPlatformBatterySteadyState(void);
	UInt32 getACNominalVoltage(void);
	UInt32 getACOperationalCurrent(void);
	Percentage getAC1msPercentageOverload(void);
	Percentage getAC2msPercentageOverload(void);
	Percentage getAC10msPercentageOverload(void);
	void notifyForProchotDeassertion(void);

	// priority
	DomainPriority getDomainPriority(void);

	// RF Profile Control
	RfProfileCapabilities getRfProfileCapabilities(void);
	void setRfProfileCenterFrequency(UIntN policyIndex, const Frequency& centerFrequency);

	// RF Profile Status
	RfProfileDataSet getRfProfileDataSet(void);

	// TCC Offset Control
	Temperature getTccOffsetTemperature();
	void setTccOffsetTemperature(UIntN policyIndex, const Temperature& tccOffset);
	Temperature getMaxTccOffsetTemperature();
	Temperature getMinTccOffsetTemperature();

	// temperature
	TemperatureStatus getTemperatureStatus(void);
	TemperatureThresholds getTemperatureThresholds(void);
	void setTemperatureThresholds(UIntN policyIndex, const TemperatureThresholds& temperatureThresholds);
	DptfBuffer getVirtualSensorCalibrationTable(void);
	DptfBuffer getVirtualSensorPollingTable(void);
	Bool isVirtualTemperature(void);
	void setVirtualTemperature(const Temperature& temperature);

	// utilization
	UtilizationStatus getUtilizationStatus(void);

private:
	// hide the copy constructor and assignment operator.
	Domain(const Domain& rhs);
	Domain& operator=(const Domain& rhs);

	Bool m_domainCreated;

	DptfManagerInterface* m_dptfManager;
	ParticipantInterface* m_theRealParticipant;

	UIntN m_participantIndex;
	UIntN m_domainIndex;
	Guid m_domainGuid;
	std::string m_domainName;
	DomainType::Type m_domainType;
	DomainFunctionalityVersions m_domainFunctionalityVersions;

	Arbitrator* m_arbitrator;

	//
	// Cached data.
	// FIXME:  consider adding a dirty bit to speed up clearing the cache.
	//

	// Active Controls
	ActiveControlStaticCaps* m_activeControlStaticCaps;
	ActiveControlStatus* m_activeControlStatus;
	ActiveControlSet* m_activeControlSet;

	// ConfigTdp controls
	ConfigTdpControlDynamicCaps* m_configTdpControlDynamicCaps;
	ConfigTdpControlStatus* m_configTdpControlStatus;
	ConfigTdpControlSet* m_configTdpControlSet;

	// Core controls
	CoreControlStaticCaps* m_coreControlStaticCaps;
	CoreControlDynamicCaps* m_coreControlDynamicCaps;
	CoreControlLpoPreference* m_coreControlLpoPreference;
	CoreControlStatus* m_coreControlStatus;

	// Display controls
	DisplayControlDynamicCaps* m_displayControlDynamicCaps;
	DisplayControlStatus* m_displayControlStatus;
	DisplayControlSet* m_displayControlSet;

	// Performance controls
	PerformanceControlStaticCaps* m_performanceControlStaticCaps;
	PerformanceControlDynamicCaps* m_performanceControlDynamicCaps;
	PerformanceControlStatus* m_performanceControlStatus;
	PerformanceControlSet* m_performanceControlSet;

	// Power controls
	PowerControlDynamicCapsSet* m_powerControlDynamicCapsSet;
	Bool* m_isPowerShareControl;
	std::map<PowerControlType::Type, Bool> m_powerLimitEnabled;
	std::map<PowerControlType::Type, Power> m_powerLimit;
	std::map<PowerControlType::Type, TimeSpan> m_powerLimitTimeWindow;
	std::map<PowerControlType::Type, Percentage> m_powerLimitDutyCycle;

	// Power status
	PowerStatus* m_powerStatus;

	// Platform Power Controls
	std::map<PlatformPowerLimitType::Type, Bool> m_platformPowerLimitEnabled;
	std::map<PlatformPowerLimitType::Type, Power> m_platformPowerLimit;
	std::map<PlatformPowerLimitType::Type, TimeSpan> m_platformPowerLimitTimeWindow;
	std::map<PlatformPowerLimitType::Type, Percentage> m_platformPowerLimitDutyCycle;

	// Platform Power Status
	Power* m_maxBatteryPower;
	Power* m_adapterRating;
	Power* m_platformRestOfPower;
	PlatformPowerSource::Type* m_platformPowerSource;
	ChargerType::Type* m_chargerType;
	DptfBuffer m_batteryStatusBuffer;
	DptfBuffer m_batteryInformationBuffer;
	Power* m_batterySteadyState;
	UInt32* m_acNominalVoltage;
	UInt32* m_acOperationalCurrent;
	Percentage* m_ac1msPercentageOverload;
	Percentage* m_ac2msPercentageOverload;
	Percentage* m_ac10msPercentageOverload;

	// priority
	DomainPriority* m_domainPriority;

	// RF Profile Control
	RfProfileCapabilities* m_rfProfileCapabilities;

	// RF Profile Status
	RfProfileDataSet* m_rfProfileData;

	// temperature
	TemperatureStatus* m_temperatureStatus;
	TemperatureThresholds* m_temperatureThresholds;
	DptfBuffer m_virtualSensorCalculationTableBuffer;
	DptfBuffer m_virtualSensorPollingTableBuffer;
	Bool* m_isVirtualTemperature;

	// utilization
	UtilizationStatus* m_utilizationStatus;

	void clearDomainCachedDataActiveControl();
	void clearDomainCachedDataConfigTdpControl();
	void clearDomainCachedDataCoreControl();
	void clearDomainCachedDataDisplayControl();
	void clearDomainCachedDataPerformanceControl();
	void clearDomainCachedDataPowerControl();
	void clearDomainCachedDataPowerStatus();
	void clearDomainCachedDataPlatformPowerControl();
	void clearDomainCachedDataPriority();
	void clearDomainCachedDataRfProfileControl();
	void clearDomainCachedDataRfProfileStatus();
	void clearDomainCachedDataTemperature();
	void clearDomainCachedDataUtilizationStatus();
	void clearDomainCachedDataPlatformPowerStatus();
};
