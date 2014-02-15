/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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
#include "PolicyBase.h"
#include "ParticipantTracker.h"

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

    virtual bool autoNotifyPlatformOscOnCreateDestroy() const override;
    virtual bool autoNotifyPlatformOscOnConnectedStandbyEntryExit() const override;
    virtual bool autoNotifyPlatformOscOnEnableDisable() const override;

    virtual Guid getGuid(void) const override;
    virtual std::string getName(void) const override;
    virtual std::string getStatusAsXml(void) const override;

    virtual void onBindParticipant(UIntN participantIndex) override;
    virtual void onUnbindParticipant(UIntN participantIndex) override;
    virtual void onBindDomain(UIntN participantIndex, UIntN domainIndex) override;
    virtual void onUnbindDomain(UIntN participantIndex, UIntN domainIndex) override;

    virtual void onDomainTemperatureThresholdCrossed(UIntN participantIndex) override;
    virtual void onParticipantSpecificInfoChanged(UIntN participantIndex) override;
    virtual void onActiveRelationshipTableChanged(void) override;

private:

    mutable ActiveRelationshipTable m_art;

    // cooling targets
    void coolTargetParticipant(ParticipantProxy& participant);
    void requestFanSpeedChangesForTarget(ParticipantProxy& target, const Temperature& currentTemperature);
    void requestFanSpeedChange(const ActiveRelationshipTableEntry& entry, const Temperature& currentTemperature);
    void requestFanTurnedOff(const ActiveRelationshipTableEntry& entry);
    void turnOffAllFans();
    void refreshArtAndTargetsAndTakeCoolingAction();

    // setting target trip point notification
    void setTripPointNotificationForTarget(ParticipantProxy& target, const Temperature& currentTemperature);
    Temperature determineLowerTemperatureThreshold(const Temperature& currentTemperature, SpecificInfo& tripPoints) const;
    Temperature determineUpperTemperatureThreshold(const Temperature& currentTemperature, SpecificInfo& tripPoints) const;

    // selecting a fan speed
    Percentage selectFanSpeed(const ActiveRelationshipTableEntry& entry, SpecificInfo& tripPoints, const Temperature& temperature);
    UIntN selectActiveControlIndex(const ActiveRelationshipTableEntry& entry, SpecificInfo& tripPoints, const Temperature& temperature);
    UIntN findTripPointCrossed(SpecificInfo& tripPoints, const Temperature& temperature);

    // associating participants with entries in the ART
    void associateAllParticipantsInArt();
    void associateParticipantInArt(ParticipantProxy& participant);

    // selecting participants
    Bool participantIsSourceDevice(UIntN participantIndex);
    Bool participantIsTargetDevice(UIntN participantIndex);

    // status
    XmlNode* getXmlForActiveTripPoints() const;
    XmlNode* getXmlForActiveCoolingControls() const;
};