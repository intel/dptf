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
	void clearDomainCachedRequestData(void);
	void clearArbitrationDataForPolicy(UIntN policyIndex);
	std::shared_ptr<XmlNode> getArbitrationXmlForPolicy(UIntN policyIndex, ControlFactoryType::Type type) const;

	std::shared_ptr<XmlNode> getDiagnosticsAsXml() const;

	//
	// The following set of functions pass the call through to the actual domain.  They
	// also provide caching so each policy sees the same representation of the platform
	// for the duration of a work item.
	//

	// Activity Status
	Percentage getUtilizationThreshold();
	Percentage getResidencyUtilization();
	UInt64 getCoreActivityCounter();
	UInt32 getCoreActivityCounterWidth();
	UInt64 getTimestampCounter();
	UInt32 getTimestampCounterWidth();
	CoreActivityInfo getCoreActivityInfo();
	void setPowerShareEffectiveBias(UInt32 powerShareEffectiveBias);
	UInt32 getSocDgpuPerformanceHintPoints(void); 

	// Core controls
	CoreControlStaticCaps getCoreControlStaticCaps(void);
	CoreControlDynamicCaps getCoreControlDynamicCaps(void);
	CoreControlLpoPreference getCoreControlLpoPreference(void);
	CoreControlStatus getCoreControlStatus(void);
	void setActiveCoreControl(UIntN policyIndex, const CoreControlStatus& coreControlStatus);

	// Display controls
	DisplayControlDynamicCaps getDisplayControlDynamicCaps(void);
	UIntN getUserPreferredDisplayIndex(void);
	UIntN getUserPreferredSoftBrightnessIndex(void);
	Bool isUserPreferredIndexModified(void);
	UIntN getSoftBrightnessIndex(void);
	DisplayControlStatus getDisplayControlStatus(void);
	DisplayControlSet getDisplayControlSet(void);
	void setDisplayControl(UIntN policyIndex, UIntN displayControlIndex);
	void setSoftBrightness(UIntN policyIndex, UIntN displayControlIndex);
	void updateUserPreferredSoftBrightnessIndex(void);
	void restoreUserPreferredSoftBrightness(void);
	void setDisplayControlDynamicCaps(UIntN policyIndex, DisplayControlDynamicCaps newCapabilities);
	void setDisplayCapsLock(UIntN policyIndex, Bool lock);

	// Energy Controls
	UInt32 getRaplEnergyCounter();
	EnergyCounterInfo getRaplEnergyCounterInfo();
	double getRaplEnergyUnit();
	UInt32 getRaplEnergyCounterWidth();
	Power getInstantaneousPower();
	UInt32 getEnergyThreshold();
	void setEnergyThreshold(UInt32 energyThreshold);
	void setEnergyThresholdInterruptDisable();

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
	void setPerfPreferenceMax(UIntN policyIndex, Percentage minMaxRatio);
	void setPerfPreferenceMin(UIntN policyIndex, Percentage minMaxRatio);

	// Power controls
	PowerControlDynamicCapsSet getPowerControlDynamicCapsSet(void);
	void setPowerControlDynamicCapsSet(UIntN policyIndex, PowerControlDynamicCapsSet capsSet);
	Bool isPowerLimitEnabled(PowerControlType::Type controlType);
	Power getPowerLimit(PowerControlType::Type controlType);
	Power getPowerLimitWithoutCache(PowerControlType::Type controlType);
	Bool isSocPowerFloorEnabled();
	Bool isSocPowerFloorSupported();
	void setPowerLimit(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit);
	void setPowerLimitWithoutUpdatingEnabled(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit);
	void setPowerLimitIgnoringCaps(UIntN policyIndex, PowerControlType::Type controlType, const Power& powerLimit);
	TimeSpan getPowerLimitTimeWindow(PowerControlType::Type controlType);
	void setPowerLimitTimeWindow(UIntN policyIndex, PowerControlType::Type controlType, const TimeSpan& timeWindow);
	void setPowerLimitTimeWindowWithoutUpdatingEnabled(UIntN policyIndex, PowerControlType::Type controlType, const TimeSpan& timeWindow);
	void setPowerLimitTimeWindowIgnoringCaps(
		UIntN policyIndex,
		PowerControlType::Type controlType,
		const TimeSpan& timeWindow);
	Percentage getPowerLimitDutyCycle(PowerControlType::Type controlType);
	void setPowerLimitDutyCycle(UIntN policyIndex, PowerControlType::Type controlType, const Percentage& dutyCycle);
	void setSocPowerFloorState(UIntN policyIndex, Bool socPowerFloorState);
	void clearPowerLimit();
	void setPowerCapsLock(UIntN policyIndex, Bool lock);
	TimeSpan getPowerSharePowerLimitTimeWindow();
	Bool isPowerShareControl();
	double getPidKpTerm();
	double getPidKiTerm();
	TimeSpan getAlpha();
	TimeSpan getFastPollTime();
	TimeSpan getSlowPollTime();
	TimeSpan getWeightedSlowPollAvgConstant();
	Power getSlowPollPowerThreshold();
	void removePowerLimitPolicyRequest(UIntN polixyIndex, PowerControlType::Type controlType);
	void setPowerSharePolicyPower(const Power& powerSharePolicyPower);

	// Power status
	PowerStatus getPowerStatus(void);
	Power getAveragePower(const PowerControlDynamicCaps& capabilities);
	Power getPowerValue(void);
	void setCalculatedAveragePower(Power powerValue);

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
	Power getPlatformRestOfPower(void);
	Power getAdapterPowerRating(void);
	PlatformPowerSource::Type getPlatformPowerSource(void);
	UInt32 getACNominalVoltage(void);
	UInt32 getACOperationalCurrent(void);
	Percentage getAC1msPercentageOverload(void);
	Percentage getAC2msPercentageOverload(void);
	Percentage getAC10msPercentageOverload(void);
	void notifyForProchotDeassertion(void);

	// Priority
	DomainPriority getDomainPriority(void);

	// RF Profile Control
	RfProfileCapabilities getRfProfileCapabilities(void);
	void setRfProfileCenterFrequency(UIntN policyIndex, const Frequency& centerFrequency);
	Percentage getSscBaselineSpreadValue();
	Percentage getSscBaselineThreshold();
	Percentage getSscBaselineGuardBand();

	// RF Profile Status
	RfProfileDataSet getRfProfileDataSet(void);
	UInt32 getWifiCapabilities(void);
	UInt32 getRfiDisable(void);
	UInt64 getDvfsPoints(void);
	void setDdrRfiTable(const DdrfChannelBandPackage::WifiRfiDdr ddrRfiStruct);
	void setProtectRequest(const UInt64 frequencyRate);
	void setRfProfileOverride(UIntN participantIndex, UIntN domainIndex, const DptfBuffer& rfProfileBufferData);

	// Utilization
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
	Power getArbitratedPowerLimit(PowerControlType::Type controlType);
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
