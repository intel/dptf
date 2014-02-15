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
#include <string>
#include <memory>
#include <vector>
#include <map>
#include "CriticalPolicyStatistics.h"

class dptf_export CriticalPolicy final : public PolicyBase
{
public:

    CriticalPolicy(void);
    virtual ~CriticalPolicy(void);

    virtual void onCreate(void) override;
    virtual void onDestroy(void) override;
    virtual void onEnable(void) override;
    virtual void onDisable(void) override;
    virtual void onConnectedStandbyEntry(void) override;
    virtual void onConnectedStandbyExit(void) override;

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

private:

    mutable CriticalPolicyStatistics m_stats;

    bool participantHasDesiredProperties(ParticipantProxy& newParticipant);
    void takePowerActionBasedOnThermalState(ParticipantProxy& participant);
    void setParticipantTemperatureThresholdNotification(
        Temperature currentTemperature,
        std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>> tripPoints,
        ParticipantProxy& participant);
    Temperature determineLowerTemperatureThreshold(
        Temperature currentTemperature,
        std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>> tripPoints);
    Temperature determineUpperTemperatureThreshold(
        Temperature currentTemperature,
        std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>> tripPoints);
    void takePowerAction(const Temperature& currentTemperature, ParticipantSpecificInfoKey::Type crossedTripPoint, const Temperature& crossedTripPointTemperature);
    ParticipantSpecificInfoKey::Type findTripPointCrossed(
        const std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>>& tripPoints,
        const Temperature& currentTemperature);
    XmlNode* getXmlForCriticalTripPoints() const;
};