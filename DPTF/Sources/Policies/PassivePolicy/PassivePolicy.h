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
#include "PolicyBase.h"
#include "ParticipantTracker.h"
#include "ThermalRelationshipTable.h"
#include "CallbackScheduler.h"
#include "DptfTime.h"
#include "TargetMonitor.h"
#include "TargetActionBase.h"

class dptf_export PassivePolicy final : public PolicyBase
{
public:
	PassivePolicy(void);
	virtual ~PassivePolicy(void);

	virtual void onCreate(void) override;
	virtual void onDestroy(void) override;
	virtual void onEnable(void) override;
	virtual void onDisable(void) override;
	virtual void onConnectedStandbyEntry(void) override;
	virtual void onConnectedStandbyExit(void) override;

	virtual Bool autoNotifyPlatformOscOnCreateDestroy() const override;
	virtual Bool autoNotifyPlatformOscOnConnectedStandbyEntryExit() const override;
	virtual Bool autoNotifyPlatformOscOnEnableDisable() const override;
	virtual Bool hasPassiveControlCapability() const override;

	virtual Guid getGuid(void) const override;
	virtual std::string getName(void) const override;
	virtual std::string getStatusAsXml(void) const override;
	virtual std::string getDiagnosticsAsXml(void) const override;

	virtual void onBindParticipant(UIntN participantIndex) override;
	virtual void onUnbindParticipant(UIntN participantIndex) override;
	virtual void onBindDomain(UIntN participantIndex, UIntN domainIndex) override;
	virtual void onUnbindDomain(UIntN participantIndex, UIntN domainIndex) override;
	virtual void onDomainTemperatureThresholdCrossed(UIntN participantIndex) override;
	virtual void onParticipantSpecificInfoChanged(UIntN participantIndex) override;
	virtual void onDomainPowerControlCapabilityChanged(UIntN participantIndex) override;
	virtual void onDomainPerformanceControlCapabilityChanged(UIntN participantIndex) override;
	virtual void onDomainPerformanceControlsChanged(UIntN participantIndex) override;
	virtual void onDomainCoreControlCapabilityChanged(UIntN participantIndex) override;
	virtual void onDomainPriorityChanged(UIntN participantIndex) override;
	virtual void onDomainDisplayControlCapabilityChanged(UIntN participantIndex) override;
	virtual void onThermalRelationshipTableChanged(void) override;

	virtual void onPolicyInitiatedCallback(UInt64 eventCode, UInt64 param1, void* param2) override;
	virtual void onOverrideTimeObject(const std::shared_ptr<TimeInterface>& timeObject) override;

private:
	// policy state
	std::shared_ptr<ThermalRelationshipTable> m_trt;
	std::shared_ptr<CallbackScheduler> m_callbackScheduler;
	TargetMonitor m_targetMonitor;
	UtilizationStatus m_utilizationBiasThreshold;

	// thermal action decisions
	TargetActionBase* determineAction(UIntN target);
	void takeThermalActionForTarget(UIntN target);
	void removeAllRequestsForTarget(UIntN target);
	void takePossibleThermalActionForAllTargets();
	void takePossibleThermalActionForTarget(UIntN participantIndex);
	void takePossibleThermalActionForTarget(UIntN participantIndex, const Temperature& temperature);
	void clearAllSourceControls();

	// TRT actions
	void associateParticipantInTrt(
		ParticipantProxyInterface* participant,
		std::shared_ptr<ThermalRelationshipTable> trt);
	void reloadTrtIfDifferent();
	void associateAllParticipantsInTrt(std::shared_ptr<ThermalRelationshipTable> trt);

	// temperature notification actions
	void notifyPlatformOfDeviceTemperature(ParticipantProxyInterface* participant, Temperature currentTemperature);
	void setParticipantTemperatureThresholdNotification(
		ParticipantProxyInterface* participant,
		Temperature currentTemperature);

	// testing participant qualifications
	Bool participantIsSourceDevice(UIntN participantIndex) const;
	Bool participantIsTargetDevice(UIntN participantIndex) const;

	// status
	std::shared_ptr<XmlNode> getXmlForPassiveTripPoints() const;
};
