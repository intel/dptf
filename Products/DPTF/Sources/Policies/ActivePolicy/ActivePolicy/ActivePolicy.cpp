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

#include "ActivePolicy.h"

using namespace std;

static const Guid MyGuid(0x89, 0xC3, 0x95, 0x3A, 0xB8, 0xE4, 0x29, 0x46, 0xA5, 0x26, 0xC5, 0x2C, 0x88, 0x62, 0x6B, 0xAE);
static const string MyName("Active Policy");

ActivePolicy::ActivePolicy(void)
    : PolicyBase(),
    m_art(vector<ActiveRelationshipTableEntry>())
{
}

ActivePolicy::~ActivePolicy(void)
{
}

void ActivePolicy::onCreate(void)
{
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::ParticipantSpecificInfoChanged);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::PolicyActiveRelationshipTableChanged);
    m_art = getPolicyServices().platformConfigurationData->getActiveRelationshipTable();
}

void ActivePolicy::onDestroy(void)
{
    turnOffAllFans();
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::ParticipantSpecificInfoChanged);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::PolicyActiveRelationshipTableChanged);
}

void ActivePolicy::onEnable(void)
{
    refreshArtAndTargetsAndTakeCoolingAction();
}

void ActivePolicy::onDisable(void)
{
    turnOffAllFans();
}

void ActivePolicy::onConnectedStandbyEntry()
{
    turnOffAllFans();
}

void ActivePolicy::onConnectedStandbyExit()
{
    refreshArtAndTargetsAndTakeCoolingAction();
}

bool ActivePolicy::autoNotifyPlatformOscOnCreateDestroy() const
{
    return true;
}

bool ActivePolicy::autoNotifyPlatformOscOnConnectedStandbyEntryExit() const
{
    return false;
}

bool ActivePolicy::autoNotifyPlatformOscOnEnableDisable() const
{
    return true;
}

Guid ActivePolicy::getGuid(void) const
{
    return MyGuid;
}

string ActivePolicy::getName(void) const
{
    return MyName;
}

string ActivePolicy::getStatusAsXml(void) const
{
    XmlNode* root = XmlNode::createRoot();
    XmlNode* format = XmlNode::createComment("format_id=" + getGuid().toString());
    root->addChild(format);
    XmlNode* status = XmlNode::createWrapperElement("active_policy_status");
    status->addChild(getParticipantTracker().getXmlForActiveCoolingControls());
    status->addChild(getParticipantTracker().getXmlForActiveTripPoints());
    status->addChild(m_art.getXml());
    root->addChild(status);
    string statusString = root->toString();
    delete root;
    return statusString;
}

void ActivePolicy::onBindParticipant(UIntN participantIndex)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Binding participant.", participantIndex));
    getParticipantTracker().remember(participantIndex);
    associateParticipantInArt(getParticipantTracker()[participantIndex]);
}

void ActivePolicy::onUnbindParticipant(UIntN participantIndex)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Unbinding participant.", participantIndex));
    m_art.disassociateParticipant(participantIndex);
    getParticipantTracker().forget(participantIndex);
}

void ActivePolicy::onBindDomain(UIntN participantIndex, UIntN domainIndex)
{
    if (getParticipantTracker().remembers(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Binding domain for participant.", participantIndex, domainIndex));
        getParticipantTracker()[participantIndex].bindDomain(domainIndex);
        if (participantIsTargetDevice(participantIndex))
        {
            coolTargetParticipant(getParticipantTracker()[participantIndex]);
        }
        else if (participantIsSourceDevice(participantIndex))
        {
            // take possible cooling action for each target who is associated with this fan
            vector<ActiveRelationshipTableEntry> entries = m_art.getEntriesForSource(participantIndex);
            for (auto entry = entries.begin(); entry != entries.end(); entry++)
            {
                if (participantIsTargetDevice(entry->targetDeviceIndex()))
                {
                    coolTargetParticipant(getParticipantTracker()[entry->targetDeviceIndex()]);
                }
            }
        }
        else
        {
            // not a target or source, so do nothing
        }
    }
}

void ActivePolicy::onUnbindDomain(UIntN participantIndex, UIntN domainIndex)
{
    if (getParticipantTracker().remembers(participantIndex))
    {
        if (participantIsTargetDevice(participantIndex))
        {
            auto entries = m_art.getEntriesForTarget(participantIndex);
            for (auto entry = entries.begin(); entry != entries.end(); entry++)
            {
                if (participantIsSourceDevice(entry->sourceDeviceIndex()))
                {
                    try
                    {
                        requestFanTurnedOff(*entry);
                    }
                    catch (...)
                    {
                        // no action for failure.  make best attempt to turn off the fan.
                    }
                }
            }
        }
        else if (participantIsSourceDevice(participantIndex))
        {
            auto entries = m_art.getEntriesForSource(participantIndex);
            for (auto entry = entries.begin(); entry != entries.end(); entry++)
            {
                if (participantIsSourceDevice(entry->sourceDeviceIndex()))
                {
                    try
                    {
                        requestFanTurnedOff(*entry);
                    }
                    catch (...)
                    {
                        // no action for failure.  make best attempt to turn off the fan.
                    }
                }
            }
        }
        getParticipantTracker()[participantIndex].unbindDomain(domainIndex);
    }
}

void ActivePolicy::onParticipantSpecificInfoChanged(UIntN participantIndex)
{
    if (participantIsTargetDevice(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Specific info changed for target participant.", participantIndex));
        getParticipantTracker()[participantIndex].getActiveTripPointProperty().refresh();
        coolTargetParticipant(getParticipantTracker()[participantIndex]);
    }
}

void ActivePolicy::onDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
    if (participantIsTargetDevice(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Temperature threshold crossed for target participant.", participantIndex));
        coolTargetParticipant(getParticipantTracker()[participantIndex]);
    }
}

void ActivePolicy::onActiveRelationshipTableChanged(void)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Active Relationship Table changed."));
    m_art = getPolicyServices().platformConfigurationData->getActiveRelationshipTable();
    associateAllParticipantsInArt();

    for (UIntN entryIndex = 0; entryIndex < m_art.getNumberOfEntries(); entryIndex++)
    {
        if (participantIsTargetDevice(m_art[entryIndex].targetDeviceIndex()))
        {
            coolTargetParticipant(getParticipantTracker()[m_art[entryIndex].targetDeviceIndex()]);
        }
    }
}

void ActivePolicy::coolTargetParticipant(ParticipantProxy& participant)
{
    if (participant.getActiveTripPointProperty().supportsProperty())
    {
        if (participant.getDomainPropertiesSet().getDomainCount() > 0)
        {
            auto currentTemperature = participant[0].getTemperatureProperty().getCurrentTemperature();
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Considering actions based on temperature of " + 
                currentTemperature.toString() + "."));
            setTripPointNotificationForTarget(participant, currentTemperature);
            requestFanSpeedChangesForTarget(participant, currentTemperature);
        }
    }
}

void ActivePolicy::requestFanSpeedChangesForTarget(ParticipantProxy& target, const Temperature& currentTemperature)
{
    auto artEntriesForTarget = m_art.getEntriesForTarget(target.getIndex());
    for (auto entry = artEntriesForTarget.begin(); entry != artEntriesForTarget.end(); entry++)
    {
        if (participantIsSourceDevice(entry->sourceDeviceIndex()))
        {
            requestFanSpeedChange(*entry, currentTemperature);
        }
    }
}

void ActivePolicy::requestFanSpeedChange(
    const ActiveRelationshipTableEntry& entry,
    const Temperature& currentTemperature)
{
    auto tripPoints = getParticipantTracker()[entry.targetDeviceIndex()].getActiveTripPointProperty().getTripPoints();
    auto domainIndexes = getParticipantTracker()[entry.sourceDeviceIndex()].getDomainIndexes();
    for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
    {
        ActiveCoolingControl& coolingControl =
            getParticipantTracker()[entry.sourceDeviceIndex()][*domainIndex].getActiveCoolingControl();
        if (coolingControl.supportsFineGrainControl())
        {
            Percentage fanSpeed = selectFanSpeed(entry, tripPoints, currentTemperature);
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Requesting fan speed of " + fanSpeed.toString() + "."));
            coolingControl.requestFanSpeedPercentage(entry.targetDeviceIndex(), fanSpeed);
        }
        else
        {
            UIntN activeControlIndex = selectActiveControlIndex(entry, tripPoints, currentTemperature);
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
                FLF, "Requesting fan speed index of " + to_string(activeControlIndex) + "."));
            coolingControl.requestActiveControlIndex(entry.targetDeviceIndex(), activeControlIndex);
        }
    }
}

void ActivePolicy::requestFanTurnedOff(const ActiveRelationshipTableEntry& entry)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
        FLF, "Requesting fan turned off for participant " + to_string(entry.sourceDeviceIndex()) + "."));
    auto domainIndexes = getParticipantTracker()[entry.sourceDeviceIndex()].getDomainIndexes();
    for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
    {
        ActiveCoolingControl& coolingControl =
            getParticipantTracker()[entry.sourceDeviceIndex()][*domainIndex].getActiveCoolingControl();
        if (coolingControl.supportsFineGrainControl())
        {
            coolingControl.requestFanSpeedPercentage(
                entry.targetDeviceIndex(),
                Percentage(0.0));
        }
        else
        {
            coolingControl.requestActiveControlIndex(
                entry.targetDeviceIndex(),
                ActiveRelationshipTableEntry::FanOffIndex);
        }
    }
}

void ActivePolicy::turnOffAllFans()
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Turning off all fans."));
    vector<UIntN> sources = m_art.getAllSources();
    for (auto source = sources.begin(); source != sources.end(); source++)
    {
        auto domainIndexes = getParticipantTracker()[*source].getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                ActiveCoolingControl& coolingControl =
                    getParticipantTracker()[*source][*domainIndex].getActiveCoolingControl();
                coolingControl.forceFanOff();
            }
            catch (...)
            {
                // swallow errors when attempting to force fans off
            }
        }
    }
}

void ActivePolicy::refreshArtAndTargetsAndTakeCoolingAction()
{
    m_art = getPolicyServices().platformConfigurationData->getActiveRelationshipTable();
    associateAllParticipantsInArt();

    vector<UIntN> targets = m_art.getAllTargets();
    for (auto target = targets.begin(); target != targets.end(); target++)
    {
        getParticipantTracker()[*target].getActiveTripPointProperty().refresh();
        coolTargetParticipant(getParticipantTracker()[*target]);
    }
}

void ActivePolicy::setTripPointNotificationForTarget(ParticipantProxy& target, const Temperature& currentTemperature)
{
    auto tripPoints = target.getActiveTripPointProperty().getTripPoints();
    Temperature lowerTemperatureThreshold = determineLowerTemperatureThreshold(currentTemperature, tripPoints);
    Temperature upperTemperatureThreshold = determineUpperTemperatureThreshold(currentTemperature, tripPoints);
    target.setTemperatureThresholds(lowerTemperatureThreshold, upperTemperatureThreshold);
    target.notifyPlatformOfDeviceTemperature(currentTemperature);
}

Temperature ActivePolicy::determineLowerTemperatureThreshold(const Temperature& currentTemperature, SpecificInfo& tripPoints) const
{
    auto trips = tripPoints.getSortedByKey();
    Temperature lowerTemperatureThreshold(Temperature::createInvalid());
    for (UIntN ac = 0; ac < trips.size(); ++ac)
    {
        if (currentTemperature >= Temperature(trips[ac].second))
        {
            lowerTemperatureThreshold = trips[ac].second;
            break;
        }
    }
    return lowerTemperatureThreshold;
}

Temperature ActivePolicy::determineUpperTemperatureThreshold(const Temperature& currentTemperature, SpecificInfo& tripPoints) const
{
    auto trips = tripPoints.getSortedByKey();
    Temperature upperTemperatureThreshold(Temperature::createInvalid());
    for (UIntN ac = 0; ac < trips.size(); ++ac)
    {
        if ((currentTemperature < Temperature(trips[ac].second)) &&
            (trips[ac].second != Constants::Invalid))
        {
            upperTemperatureThreshold = trips[ac].second;
        }
    }
    return upperTemperatureThreshold;
}

Percentage ActivePolicy::selectFanSpeed(const ActiveRelationshipTableEntry& entry, SpecificInfo& tripPoints, const Temperature& temperature)
{
    Percentage fanSpeed = Percentage::createInvalid();
    UIntN crossedTripPointIndex = findTripPointCrossed(tripPoints, temperature);
    if (crossedTripPointIndex != Constants::Invalid)
    {
        // find fan speed at index or greater
        fanSpeed = 0.0;
        for (UIntN entryAcIndex = crossedTripPointIndex; entryAcIndex < entry.FanOffIndex; ++entryAcIndex)
        {
            if (entry.ac(entryAcIndex) != Constants::Invalid)
            {
                fanSpeed = (double)entry.ac(entryAcIndex) / 100.0;
                break;
            }
        }
    }
    else
    {
        fanSpeed = 0.0;
    }
    return fanSpeed;
}

UIntN ActivePolicy::selectActiveControlIndex(const ActiveRelationshipTableEntry& entry, SpecificInfo& tripPoints, const Temperature& temperature)
{
    UIntN crossedTripPointIndex = findTripPointCrossed(tripPoints, temperature);
    if (crossedTripPointIndex != Constants::Invalid)
    {
        return crossedTripPointIndex;
    }
    else
    {
        // pick control with highest index
        return (ActiveRelationshipTableEntry::FanOffIndex);
    }
}

UIntN ActivePolicy::findTripPointCrossed(SpecificInfo& tripPoints, const Temperature& temperature)
{
    auto trips = tripPoints.getSortedByKey();
    for (UIntN index = 0; index < trips.size(); index++)
    {
        if (temperature >= Temperature(trips[index].second))
        {
            UIntN acIndex = trips[index].first - ParticipantSpecificInfoKey::AC0;
            return acIndex;
        }
    }
    return Constants::Invalid;
}

void ActivePolicy::associateAllParticipantsInArt()
{
    vector<UIntN> participantIndicies = getParticipantTracker().getAllTrackedIndexes();
    for (auto index = participantIndicies.begin(); index != participantIndicies.end(); index++)
    {
        associateParticipantInArt(getParticipantTracker()[*index]);
    }
}

void ActivePolicy::associateParticipantInArt(ParticipantProxy& participant)
{
    m_art.associateParticipant(
        participant.getParticipantProperties().getAcpiInfo().getAcpiScope(),
        participant.getIndex());
}

Bool ActivePolicy::participantIsTargetDevice(UIntN participantIndex)
{
    return getParticipantTracker().remembers(participantIndex) &&
           m_art.isParticipantTargetDevice(participantIndex);
}

Bool ActivePolicy::participantIsSourceDevice(UIntN participantIndex)
{
    return getParticipantTracker().remembers(participantIndex) &&
           m_art.isParticipantSourceDevice(participantIndex);
}