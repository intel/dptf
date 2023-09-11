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
#include "ParticipantInterface.h"
#include "esif_sdk_iface_app.h"
#include "Arbitrator.h"
#include "PsysPowerLimitType.h"
#include "RfProfileDataSet.h"
#include "CoreActivityInfo.h"
#include "EnergyCounterInfo.h"
#include "DptfManagerInterface.h"

class Domain
{
public:
	Domain(DptfManagerInterface* dptfManager);
	~Domain();

	void createDomain(
		UIntN participantIndex,
		UIntN domainIndex,
		ParticipantInterface* participantInterface,
		const AppDomainDataPtr domainDataPtr,
		Bool domainEnabled);
	void destroyDomain();

	void enableDomain() const;
	void disableDomain() const;
	Bool isDomainEnabled() const;
	Bool isCreated() const;

	[[nodiscard]] std::string getDomainName() const;

	// This will clear the cached data stored within this class in the framework.  It will not ask the
	// actual domain to clear its cache.
	void clearDomainCachedData();
	void clearDomainCachedRequestData() const;
	void clearArbitrationDataForPolicy(UIntN policyIndex) const;
	[[nodiscard]] std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex, ControlFactoryType::Type type) const;

	[[nodiscard]] std::shared_ptr<XmlNode> getDiagnosticsAsXml() const;

	//
	// The following set of functions pass the call through to the actual domain.  They
	// also provide caching so each policy sees the same representation of the platform
	// for the duration of a work item.
	//

	// Activity Status
	Percentage getUtilizationThreshold() const;
	Percentage getResidencyUtilization() const;
	UInt64 getCoreActivityCounter() const;
	UInt32 getCoreActivityCounterWidth() const;
	UInt64 getTimestampCounter() const;
	UInt32 getTimestampCounterWidth() const;
	CoreActivityInfo getCoreActivityInfo() const;
	UInt32 getSocDgpuPerformanceHintPoints() const; 

	// Core controls
	CoreControlStaticCaps getCoreControlStaticCaps();
	CoreControlDynamicCaps getCoreControlDynamicCaps();
	CoreControlLpoPreference getCoreControlLpoPreference();
	CoreControlStatus getCoreControlStatus();
	void setActiveCoreControl(UIntN policyIndex, const CoreControlStatus& coreControlStatus);

	// Display controls
	DisplayControlDynamicCaps getDisplayControlDynamicCaps();
	UIntN getUserPreferredDisplayIndex() const;
	UIntN getUserPreferredSoftBrightnessIndex() const;
	Bool isUserPreferredIndexModified() const;
	UIntN getSoftBrightnessIndex() const;
	DisplayControlStatus getDisplayControlStatus();
	DisplayControlSet getDisplayControlSet();
	void setDisplayControl(UIntN policyIndex, UIntN displayControlIndex);
	void setSoftBrightness(UIntN policyIndex, UIntN displayControlIndex) const;
	void updateUserPreferredSoftBrightnessIndex() const;
	void restoreUserPreferredSoftBrightness() const;
	void setDisplayControlDynamicCaps(UIntN policyIndex, DisplayControlDynamicCaps newCapabilities);
	void setDisplayCapsLock(UIntN policyIndex, Bool lock) const;

	// Energy Controls
	UInt32 getRaplEnergyCounter() const;
	EnergyCounterInfo getRaplEnergyCounterInfo() const;
	double getRaplEnergyUnit() const;
	UInt32 getRaplEnergyCounterWidth() const;
	Power getInstantaneousPower() const;
	UInt32 getEnergyThreshold() const;
	void setEnergyThreshold(UInt32 energyThreshold) const;
	void setEnergyThresholdInterruptDisable() const;

	// Peak Power Controls
	Power getACPeakPower() const;
	void setACPeakPower(UIntN policyIndex, const Power& acPeakPower) const;
	Power getDCPeakPower() const;
	void setDCPeakPower(UIntN policyIndex, const Power& dcPeakPower) const;

	// Performance controls
	PerformanceControlStaticCaps getPerformanceControlStaticCaps();
	PerformanceControlDynamicCaps getPerformanceControlDynamicCaps();
	PerformanceControlStatus getPerformanceControlStatus();
	PerformanceControlSet getPerformanceControlSet();
	void setPerformanceControl(UIntN policyIndex, UIntN performanceControlIndex);
	void setPerformanceControlDynamicCaps(UIntN policyIndex, PerformanceControlDynamicCaps newCapabilities);
	void setPerformanceCapsLock(UIntN policyIndex, Bool lock) const;

	// Power controls
	PowerControlDynamicCapsSet getPowerControlDynamicCapsSet();
	void setPowerControlDynamicCapsSet(UIntN policyIndex, const PowerControlDynamicCapsSet& capsSet);
	Bool isPowerLimitEnabled(PowerControlType::Type controlType);
	Power getPowerLimit(PowerControlType::Type controlType);
	Power getPowerLimitWithoutCache(PowerControlType::Type controlType) const;
	Bool isSocPowerFloorEnabled() const;
	Bool isSocPowerFloorSupported() const;
	UInt32 getSocPowerFloorState() const;
	void setPowerLimitMin(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit) const;
	void setPowerLimit(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit);
	void setPowerLimitWithoutUpdatingEnabled(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit);
	void setPowerLimitIgnoringCaps(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit) const;
	TimeSpan getPowerLimitTimeWindow(PowerControlType::Type controlType);
	void setPowerLimitTimeWindow(UIntN policyIndex, PowerControlType::Type controlType, const TimeSpan& timeWindow);
	void setPowerLimitTimeWindowWithoutUpdatingEnabled(UIntN policyIndex, PowerControlType::Type controlType, const TimeSpan& timeWindow);
	void setPowerLimitTimeWindowIgnoringCaps(
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow) const;
	Percentage getPowerLimitDutyCycle(PowerControlType::Type controlType);
	void setPowerLimitDutyCycle(UIntN policyIndex, PowerControlType::Type controlType, const Percentage& dutyCycle);
	void setSocPowerFloorState(UIntN policyIndex, Bool socPowerFloorState) const;
	void clearPowerLimitMin() const;
	void clearPowerLimit(UIntN policyIndex);
	void setPowerCapsLock(UIntN policyIndex, Bool lock) const;
	TimeSpan getPowerSharePowerLimitTimeWindow() const;
	Bool isPowerShareControl();
	double getPidKpTerm() const;
	double getPidKiTerm() const;
	TimeSpan getAlpha() const;
	TimeSpan getFastPollTime() const;
	TimeSpan getSlowPollTime() const;
	TimeSpan getWeightedSlowPollAvgConstant() const;
	Power getSlowPollPowerThreshold() const;
	Power getThermalDesignPower() const;
	void removePowerLimitPolicyRequest(UIntN policyIndex, PowerControlType::Type controlType);
	void setPowerSharePolicyPower(const Power& powerSharePolicyPower) const;
	void setPowerShareEffectiveBias(UInt32 powerShareEffectiveBias) const;

	// Power status
	PowerStatus getPowerStatus();
	Power getAveragePower(const PowerControlDynamicCaps& capabilities) const;
	Power getPowerValue() const;
	void setCalculatedAveragePower(Power powerValue) const;

	// System Power Controls
	Bool isSystemPowerLimitEnabled(PsysPowerLimitType::Type limitType);
	Power getSystemPowerLimit(PsysPowerLimitType::Type limitType);
	void setSystemPowerLimit(UIntN policyIndex, PsysPowerLimitType::Type limitType, const Power& powerLimit);
	TimeSpan getSystemPowerLimitTimeWindow(PsysPowerLimitType::Type limitType);
	void setSystemPowerLimitTimeWindow(
		UIntN policyIndex,
		PsysPowerLimitType::Type limitType,
		const TimeSpan& timeWindow);
	Percentage getSystemPowerLimitDutyCycle(PsysPowerLimitType::Type limitType);
	void setSystemPowerLimitDutyCycle(
		UIntN policyIndex,
		PsysPowerLimitType::Type limitType,
		const Percentage& dutyCycle);

	// Platform Power Status
	Power getPlatformRestOfPower();
	Power getAdapterPowerRating();
	PlatformPowerSource::Type getPlatformPowerSource();
	UInt32 getACNominalVoltage();
	UInt32 getACOperationalCurrent();
	Percentage getAC1msPercentageOverload();
	Percentage getAC2msPercentageOverload();
	Percentage getAC10msPercentageOverload();
	void notifyForProcHotDeAssertion() const;

	// Priority
	DomainPriority getDomainPriority();

	// RF Profile Control
	RfProfileCapabilities getRfProfileCapabilities();
	void setRfProfileCenterFrequency(UIntN policyIndex, const Frequency& centerFrequency);
	Percentage getSscBaselineSpreadValue() const;
	Percentage getSscBaselineThreshold() const;
	Percentage getSscBaselineGuardBand() const;

	// RF Profile Status
	RfProfileDataSet getRfProfileDataSet();
	UInt32 getWifiCapabilities() const;
	UInt32 getRfiDisable() const;
	UInt64 getDvfsPoints() const;
	UInt32 getDlvrSsc() const;
	Frequency getDlvrCenterFrequency() const;
	void setDdrRfiTable(const DdrfChannelBandPackage::WifiRfiDdr& ddrRfiStruct) const;
	void sendMasterControlStatus(UInt32 masterControlStatus) const;
	void setProtectRequest(const UInt64 frequencyRate) const;
	void setRfProfileOverride(UIntN participantIndex, UIntN domainIndex, const DptfBuffer& rfProfileBufferData) const;
	void setDlvrCenterFrequency(Frequency frequency) const;

	// Utilization
	UtilizationStatus getUtilizationStatus();
	Percentage getMaxCoreUtilization() const;

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
	Power getArbitratedPowerLimit(PowerControlType::Type controlType) const;
	PowerControlDynamicCapsSet* m_powerControlDynamicCapsSet;
	Bool* m_isPowerShareControl;
	std::map<PowerControlType::Type, Bool> m_powerLimitEnabled;
	std::map<PowerControlType::Type, Power> m_powerLimit;
	std::map<PowerControlType::Type, TimeSpan> m_powerLimitTimeWindow;
	std::map<PowerControlType::Type, Percentage> m_powerLimitDutyCycle;

	// Power status
	PowerStatus* m_powerStatus;

	// System Power Controls
	std::map<PsysPowerLimitType::Type, Bool> m_systemPowerLimitEnabled;
	std::map<PsysPowerLimitType::Type, Power> m_systemPowerLimit;
	std::map<PsysPowerLimitType::Type, TimeSpan> m_systemPowerLimitTimeWindow;
	std::map<PsysPowerLimitType::Type, Percentage> m_systemPowerLimitDutyCycle;

	// Platform Power Status
	Power* m_adapterRating;
	Power* m_platformRestOfPower;
	PlatformPowerSource::Type* m_platformPowerSource;
	UInt32* m_acNominalVoltage;
	UInt32* m_acOperationalCurrent;
	Percentage* m_ac1msPercentageOverload;
	Percentage* m_ac2msPercentageOverload;
	Percentage* m_ac10msPercentageOverload;

	// Priority
	DomainPriority* m_domainPriority;

	// RF Profile Control
	RfProfileCapabilities* m_rfProfileCapabilities;

	// RF Profile Status
	RfProfileDataSet* m_rfProfileData;

	// Utilization
	UtilizationStatus* m_utilizationStatus;

	void clearDomainCachedDataCoreControl();
	void clearDomainCachedDataDisplayControl();
	void clearDomainCachedDataPerformanceControl();
	void clearDomainCachedDataPowerControl();
	void clearDomainCachedDataPowerStatus();
	void clearDomainCachedDataSystemPowerControl();
	void clearDomainCachedDataPriority();
	void clearDomainCachedDataRfProfileControl();
	void clearDomainCachedDataRfProfileStatus();
	void clearDomainCachedDataUtilizationStatus();
	void clearDomainCachedDataPlatformPowerStatus();
};
