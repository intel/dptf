/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "DomainProxy.h"

using namespace std;

static const Guid MyGuid(0x89, 0xC3, 0x95, 0x3A, 0xB8, 0xE4, 0x29, 0x46, 0xA5, 0x26, 0xC5, 0x2C, 0x88, 0x62, 0x6B, 0xAE);
static const string MyName("Active Policy");

ActivePolicy::ActivePolicy(void)
    : PolicyBase()
{
}

ActivePolicy::~ActivePolicy(void)
{
}

void ActivePolicy::onCreate(void)
{
    try
    {
        m_art = std::make_shared<ActiveRelationshipTable>(ActiveRelationshipTable::createArtFromDptfBuffer(
            getPolicyServices().platformConfigurationData->getActiveRelationshipTable()));
    }
    catch (std::exception& ex)
    {
        getPolicyServices().messageLogging->writeMessageInfo(PolicyMessage(
            FLF, "No active relationship table was found. " + string(ex.what())));
        m_art.reset(new ActiveRelationshipTable());
    }

    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::ParticipantSpecificInfoChanged);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::PolicyActiveRelationshipTableChanged);
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

Bool ActivePolicy::autoNotifyPlatformOscOnCreateDestroy() const
{
    return true;
}

Bool ActivePolicy::autoNotifyPlatformOscOnConnectedStandbyEntryExit() const
{
    return false;
}

Bool ActivePolicy::autoNotifyPlatformOscOnEnableDisable() const
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
    auto root = XmlNode::createRoot();
    auto format = XmlNode::createComment("format_id=" + getGuid().toString());
    root->addChild(format);
    auto status = XmlNode::createWrapperElement("active_policy_status");
    status->addChild(getXmlForActiveCoolingControls());
    status->addChild(getXmlForActiveTripPoints());
    status->addChild(m_art->getXml());
    root->addChild(status);
    string statusString = root->toString();
    return statusString;
}

void ActivePolicy::onBindParticipant(UIntN participantIndex)
{
    getParticipantTracker()->remember(participantIndex);
    associateParticipantInArt(getParticipantTracker()->getParticipant(participantIndex));
}

void ActivePolicy::onUnbindParticipant(UIntN participantIndex)
{
    m_art->disassociateParticipant(participantIndex);
    getParticipantTracker()->forget(participantIndex);
}

void ActivePolicy::onBindDomain(UIntN participantIndex, UIntN domainIndex)
{
    if (getParticipantTracker()->remembers(participantIndex))
    {
        auto participant = getParticipantTracker()->getParticipant(participantIndex);
        participant->refreshDomainProperties();
        auto domain = std::make_shared<DomainProxy>(
            domainIndex,
            participant,
            getPolicyServices());
        participant->bindDomain(domain);

        if (participantIsTargetDevice(participantIndex))
        {
            coolTargetParticipant(participant);
        }
        else if (participantIsSourceDevice(participantIndex))
        {
            // take possible cooling action for each target who is associated with this fan
            vector<ActiveRelationshipTableEntry> entries = m_art->getEntriesForSource(participantIndex);
            for (auto entry = entries.begin(); entry != entries.end(); entry++)
            {
                if (participantIsTargetDevice(entry->getTargetDeviceIndex()) && 
                    (getParticipantTracker()->getParticipant(entry->getTargetDeviceIndex())->supportsTemperatureInterface()))
                {
                    coolTargetParticipant(getParticipantTracker()->getParticipant(entry->getTargetDeviceIndex()));
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
    if (getParticipantTracker()->remembers(participantIndex))
    {
        if (participantIsTargetDevice(participantIndex))
        {
            auto entries = m_art->getEntriesForTarget(participantIndex);
            for (auto entry = entries.begin(); entry != entries.end(); entry++)
            {
                if (participantIsSourceDevice(entry->getSourceDeviceIndex()))
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

        auto participant = getParticipantTracker()->getParticipant(participantIndex);
        participant->refreshDomainProperties();
        participant->unbindDomain(domainIndex);
    }
}

void ActivePolicy::onParticipantSpecificInfoChanged(UIntN participantIndex)
{
    if (participantIsTargetDevice(participantIndex))
    {
        auto participant = getParticipantTracker()->getParticipant(participantIndex);
        participant->getActiveTripPointProperty().refresh();
        participant->refreshHysteresis();
        coolTargetParticipant(participant);
    }
}

void ActivePolicy::onDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
    if (participantIsTargetDevice(participantIndex))
    {
        coolTargetParticipant(getParticipantTracker()->getParticipant(participantIndex));
    }
}

void ActivePolicy::onActiveRelationshipTableChanged(void)
{
    vector<UIntN> indexes = m_art->getAllTargets();
    for (auto participantIndex = indexes.begin(); participantIndex != indexes.end(); participantIndex++)
    {
        try
        {
            auto target = getParticipantTracker()->getParticipant(*participantIndex);
            target->setTemperatureThresholds(Temperature::createInvalid(), Temperature::createInvalid());
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, "Failed to reset temperature thresholds for participant: " + std::string(ex.what())));
        }

        auto entries = m_art->getEntriesForTarget(*participantIndex);
        for (auto entry = entries.begin(); entry != entries.end(); entry++)
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
    m_art.reset(new ActiveRelationshipTable(ActiveRelationshipTable::createArtFromDptfBuffer(
        getPolicyServices().platformConfigurationData->getActiveRelationshipTable())));
    associateAllParticipantsInArt();

    for (UIntN entryIndex = 0; entryIndex < m_art->getNumberOfEntries(); entryIndex++)
    {
        if (participantIsTargetDevice((*m_art)[entryIndex].getTargetDeviceIndex()))
        {
            coolTargetParticipant(getParticipantTracker()->getParticipant((*m_art)[entryIndex].getTargetDeviceIndex()));
        }
    }
}

void ActivePolicy::coolTargetParticipant(ParticipantProxyInterface* participant)
{
    if (participant->getActiveTripPointProperty().supportsProperty())
    {
        if (participant->getDomainPropertiesSet().getDomainCount() > 0)
        {
            auto currentTemperature = participant->getFirstDomainTemperature();
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Considering actions based on temperature of " + 
                currentTemperature.toString() + "."));
            setTripPointNotificationForTarget(participant, currentTemperature);
            requestFanSpeedChangesForTarget(participant, currentTemperature);
        }
    }
}

void ActivePolicy::requestFanSpeedChangesForTarget(ParticipantProxyInterface* target, const Temperature& currentTemperature)
{
    auto artEntriesForTarget = m_art->getEntriesForTarget(target->getIndex());
    for (auto entry = artEntriesForTarget.begin(); entry != artEntriesForTarget.end(); entry++)
    {
        if (participantIsSourceDevice(entry->getSourceDeviceIndex()))
        {
            requestFanSpeedChange(*entry, currentTemperature);
        }
    }
}

void ActivePolicy::requestFanSpeedChange(
    const ActiveRelationshipTableEntry& entry,
    const Temperature& currentTemperature)
{
    auto tripPoints = getParticipantTracker()->getParticipant(entry.getTargetDeviceIndex())->getActiveTripPointProperty().getTripPoints();
    auto domainIndexes = getParticipantTracker()->getParticipant(entry.getSourceDeviceIndex())->getDomainIndexes();
    for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
    {
        auto sourceParticipant = getParticipantTracker()->getParticipant(entry.getSourceDeviceIndex());
        auto sourceDomain = sourceParticipant->getDomain(*domainIndex);
        ActiveCoolingControl& coolingControl = sourceDomain->getActiveCoolingControl();
        if (coolingControl.supportsFineGrainControl())
        {
            Percentage fanSpeed = selectFanSpeed(entry, tripPoints, currentTemperature);
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Requesting fan speed of " + fanSpeed.toString() + "."));
            coolingControl.requestFanSpeedPercentage(entry.getTargetDeviceIndex(), fanSpeed);
        }
        else
        {
            UIntN activeControlIndex = selectActiveControlIndex(entry, tripPoints, currentTemperature);
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
                FLF, "Requesting fan speed index of " + StlOverride::to_string(activeControlIndex) + "."));
            coolingControl.requestActiveControlIndex(entry.getTargetDeviceIndex(), activeControlIndex);
        }
    }
}

void ActivePolicy::requestFanTurnedOff(const ActiveRelationshipTableEntry& entry)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
        FLF, "Requesting fan turned off for participant " + StlOverride::to_string(entry.getSourceDeviceIndex()) + "."));
    auto domainIndexes = getParticipantTracker()->getParticipant(entry.getSourceDeviceIndex())->getDomainIndexes();
    for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
    {
        auto sourceParticipant = getParticipantTracker()->getParticipant(entry.getSourceDeviceIndex());
        auto sourceDomain = sourceParticipant->getDomain(*domainIndex);
        ActiveCoolingControl& coolingControl = sourceDomain->getActiveCoolingControl();
        if (coolingControl.supportsFineGrainControl())
        {
            coolingControl.requestFanSpeedPercentage(
                entry.getTargetDeviceIndex(),
                Percentage(0.0));
        }
        else
        {
            coolingControl.requestActiveControlIndex(
                entry.getTargetDeviceIndex(),
                ActiveCoolingControl::FanOffIndex);
        }
    }
}

void ActivePolicy::turnOffAllFans()
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Turning off all fans."));
    vector<UIntN> sources = m_art->getAllSources();
    for (auto source = sources.begin(); source != sources.end(); source++)
    {
        auto domainIndexes = getParticipantTracker()->getParticipant(*source)->getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                auto sourceParticipant = getParticipantTracker()->getParticipant(*source);
                auto sourceDomain = sourceParticipant->getDomain(*domainIndex);
                ActiveCoolingControl& coolingControl = sourceDomain->getActiveCoolingControl();
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
    reloadArt();
    takeCoolingActionsForAllParticipants();
}

void ActivePolicy::reloadArt()
{
    m_art.reset(new ActiveRelationshipTable(ActiveRelationshipTable::createArtFromDptfBuffer(
        getPolicyServices().platformConfigurationData->getActiveRelationshipTable())));
    associateAllParticipantsInArt();
}

void ActivePolicy::takeCoolingActionsForAllParticipants()
{
    vector<UIntN> targets = m_art->getAllTargets();
    for (auto target = targets.begin(); target != targets.end(); target++)
    {
        getParticipantTracker()->getParticipant(*target)->getActiveTripPointProperty().refresh();
        coolTargetParticipant(getParticipantTracker()->getParticipant(*target));
    }
}

void ActivePolicy::setTripPointNotificationForTarget(ParticipantProxyInterface* target, const Temperature& currentTemperature)
{
    auto tripPoints = target->getActiveTripPointProperty().getTripPoints();
    Temperature lowerTemperatureThreshold = determineLowerTemperatureThreshold(currentTemperature, tripPoints);
    Temperature upperTemperatureThreshold = determineUpperTemperatureThreshold(currentTemperature, tripPoints);
    target->setTemperatureThresholds(lowerTemperatureThreshold, upperTemperatureThreshold);
    target->notifyPlatformOfDeviceTemperature(currentTemperature);
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
        for (UIntN entryAcIndex = crossedTripPointIndex; entryAcIndex < ActiveCoolingControl::FanOffIndex; ++entryAcIndex)
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
        return (ActiveCoolingControl::FanOffIndex);
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
    vector<UIntN> participantIndicies = getParticipantTracker()->getAllTrackedIndexes();
    for (auto index = participantIndicies.begin(); index != participantIndicies.end(); index++)
    {
        associateParticipantInArt(getParticipantTracker()->getParticipant(*index));
    }
}

void ActivePolicy::associateParticipantInArt(ParticipantProxyInterface* participant)
{
    m_art->associateParticipant(
        participant->getParticipantProperties().getAcpiInfo().getAcpiScope(),
        participant->getIndex());
}

Bool ActivePolicy::participantIsTargetDevice(UIntN participantIndex)
{
    return getParticipantTracker()->remembers(participantIndex) &&
           m_art->isParticipantTargetDevice(participantIndex);
}

Bool ActivePolicy::participantIsSourceDevice(UIntN participantIndex)
{
    return getParticipantTracker()->remembers(participantIndex) &&
           m_art->isParticipantSourceDevice(participantIndex);
}

std::shared_ptr<XmlNode> ActivePolicy::getXmlForActiveTripPoints() const
{
    auto allStatus = XmlNode::createWrapperElement("active_trip_point_status");
    vector<UIntN> indexes = getParticipantTracker()->getAllTrackedIndexes();
    for (auto participantIndex = indexes.begin(); participantIndex != indexes.end(); participantIndex++)
    {
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(*participantIndex);
        if (m_art->isParticipantTargetDevice(*participantIndex) && 
            participant->getActiveTripPointProperty().supportsProperty())
        {
            allStatus->addChild(participant->getXmlForActiveTripPoints());
        }
    }
    return allStatus;
}

std::shared_ptr<XmlNode> ActivePolicy::getXmlForActiveCoolingControls() const
{
    auto fanStatus = XmlNode::createWrapperElement("fan_status");
    vector<UIntN> participantTndexes = getParticipantTracker()->getAllTrackedIndexes();
    for (auto participantIndex = participantTndexes.begin(); 
        participantIndex != participantTndexes.end(); 
        participantIndex++)
    {
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(*participantIndex);
        auto domainIndexes = participant->getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            if (participant->getDomain(*domainIndex)->getActiveCoolingControl().supportsActiveCoolingControls())
            {
                try
                {
                    fanStatus->addChild(participant->getDomain(*domainIndex)->getActiveCoolingControl().getXml());
                }
                catch (...)
                {                	
                }
            }
        }
    }
    return fanStatus;
}