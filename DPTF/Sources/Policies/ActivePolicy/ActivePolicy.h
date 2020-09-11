/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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
#include "ActiveRelationshipTable.h"

class dptf_export ActivePolicy final : public PolicyBase
{
public:
	ActivePolicy(void);
	virtual ~ActivePolicy(void);

	virtual void onCreate(void) override;
	virtual void onDestroy(void) override;
	virtual void onEnable(void) override;
	virtual void onDisable(void) override;

	virtual void onConnectedStandbyEntry() override;
	virtual void onConnectedStandbyExit() override;

	virtual Bool autoNotifyPlatformOscOnCreateDestroy() const override;
	virtual Bool autoNotifyPlatformOscOnConnectedStandbyEntryExit() const override;
	virtual Bool autoNotifyPlatformOscOnEnableDisable() const override;
	virtual Bool hasActiveControlCapability() const override;

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
	virtual void onActiveRelationshipTableChanged(void) override;
	virtual void onDomainFanCapabilityChanged(UIntN participantIndex) override;

private:
	std::shared_ptr<ActiveRelationshipTable> m_art;

	// cooling targets
	Temperature getCurrentTemperature(ParticipantProxyInterface* participant);
	void updateTargetRequest(ParticipantProxyInterface* participant);
	void updateThresholdsAndCoolTargetParticipant(ParticipantProxyInterface* participant);
	void requestFanSpeedChangesForTarget(ParticipantProxyInterface* target, const Temperature& currentTemperature);
	void requestFanSpeedChange(
		std::shared_ptr<ActiveRelationshipTableEntry> entry,
		const Temperature& currentTemperature);
	void requestFanTurnedOff(std::shared_ptr<ActiveRelationshipTableEntry> entry);
	void turnOffAllFans();
	void refreshArtAndTargetsAndTakeCoolingAction();
	void reloadArt();
	void takeCoolingActionsForAllParticipants();

	// setting target trip point notification
	void setTripPointNotificationForTarget(ParticipantProxyInterface* target, const Temperature& currentTemperature);
	Temperature determineLowerTemperatureThreshold(const Temperature& currentTemperature, SpecificInfo& tripPoints)
		const;
	Temperature determineUpperTemperatureThreshold(const Temperature& currentTemperature, SpecificInfo& tripPoints)
		const;

	// selecting a fan speed
	Percentage selectFanSpeed(
		std::shared_ptr<ActiveRelationshipTableEntry> entry,
		SpecificInfo& tripPoints,
		const Temperature& temperature);
	UIntN findTripPointCrossed(SpecificInfo& tripPoints, const Temperature& temperature);

	// associating participants with entries in the ART
	void associateAllParticipantsInArt();
	void associateParticipantInArt(ParticipantProxyInterface* participant);

	// selecting participants
	Bool participantIsSourceDevice(UIntN participantIndex);
	Bool participantIsTargetDevice(UIntN participantIndex);

	// status
	std::shared_ptr<XmlNode> getXmlForActiveTripPoints() const;
	std::shared_ptr<XmlNode> getXmlForActiveCoolingControls() const;
};
