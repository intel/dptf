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

#include "CriticalPolicy.h"
#include "Constants.h"
#include "XmlNode.h"

using namespace std;

static const Guid MyGuid(0xE7, 0x8A, 0xC6, 0x97, 0xFA, 0x15, 0x9C, 0x49, 0xB8, 0xC9, 0x5D, 0xA8, 0x1D, 0x60, 0x6E, 0x0A);
static const string MyName("Critical Policy");

CriticalPolicy::CriticalPolicy(void)
    : PolicyBase()
{
}

CriticalPolicy::~CriticalPolicy(void)
{
}

void CriticalPolicy::onCreate(void)
{
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::ParticipantSpecificInfoChanged);
}

void CriticalPolicy::onDestroy(void)
{
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::ParticipantSpecificInfoChanged);
}

void CriticalPolicy::onEnable(void)
{
}

void CriticalPolicy::onDisable(void)
{
}

void CriticalPolicy::onConnectedStandbyEntry(void)
{
}

void CriticalPolicy::onConnectedStandbyExit(void)
{
}

bool CriticalPolicy::autoNotifyPlatformOscOnCreateDestroy() const
{
    return true;
}

bool CriticalPolicy::autoNotifyPlatformOscOnConnectedStandbyEntryExit() const
{
    return false;
}

bool CriticalPolicy::autoNotifyPlatformOscOnEnableDisable() const
{
    return true;
}

Guid CriticalPolicy::getGuid(void) const
{
    return MyGuid;
}

string CriticalPolicy::getName(void) const
{
    return MyName;
}

string CriticalPolicy::getStatusAsXml(void) const
{
    XmlNode* format = XmlNode::createComment("format_id=" + getGuid().toString());
    XmlNode* status = XmlNode::createWrapperElement("critical_policy_status");
    status->addChild(m_stats.getXml());
    status->addChild(getXmlForCriticalTripPoints());
    XmlNode* root = XmlNode::createRoot();
    root->addChild(format);
    root->addChild(status);
    string statusString = root->toString();
    delete root;
    return statusString;
}

void CriticalPolicy::onBindParticipant(UIntN participantIndex)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Binding participant.", participantIndex));
    getParticipantTracker().remember(participantIndex);
}

void CriticalPolicy::onUnbindParticipant(UIntN participantIndex)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Unbinding participant.", participantIndex));
    getParticipantTracker().forget(participantIndex);
}

void CriticalPolicy::onBindDomain(UIntN participantIndex, UIntN domainIndex)
{
    if (getParticipantTracker().remembers(participantIndex))
    {
        getParticipantTracker()[participantIndex].bindDomain(domainIndex);
        if (participantHasDesiredProperties(getParticipantTracker()[participantIndex]))
        {
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Binding domain for participant.", participantIndex, domainIndex));
            takePowerActionBasedOnThermalState(getParticipantTracker()[participantIndex]);
        }
    }
}

void CriticalPolicy::onUnbindDomain(UIntN participantIndex, UIntN domainIndex)
{
    if (getParticipantTracker().remembers(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Unbinding domain for participant.", participantIndex, domainIndex));
        getParticipantTracker()[participantIndex].unbindDomain(domainIndex);
    }
}

void CriticalPolicy::onParticipantSpecificInfoChanged(UIntN participantIndex)
{
    if (getParticipantTracker().remembers(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Specific info changed for participant.", participantIndex));
        getParticipantTracker()[participantIndex].getCriticalTripPointProperty().refresh();
        if (participantHasDesiredProperties(getParticipantTracker()[participantIndex]))
        {
            takePowerActionBasedOnThermalState(getParticipantTracker()[participantIndex]);
        }
    }
}

void CriticalPolicy::onDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
    if (getParticipantTracker().remembers(participantIndex) &&
        participantHasDesiredProperties(getParticipantTracker()[participantIndex]))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Temperature threshold crossed for participant.", participantIndex));
        takePowerActionBasedOnThermalState(getParticipantTracker()[participantIndex]);
    }
}

void CriticalPolicy::takePowerActionBasedOnThermalState(ParticipantProxy& participant)
{
    auto currentTemperature = participant[0].getTemperatureProperty().getCurrentTemperature();
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
        FLF, "Considering actions based on temperature of " + currentTemperature.toString() + "."));
    auto tripPoints = participant.getCriticalTripPointProperty().getTripPoints();
    setParticipantTemperatureThresholdNotification(currentTemperature, tripPoints.getSortedByValue(), participant);
    auto tripPointCrossed = findTripPointCrossed(tripPoints.getSortedByValue(), currentTemperature);
    Temperature crossedTripPointTemperature = Temperature::createInvalid();
    if (tripPointCrossed != ParticipantSpecificInfoKey::None)
    {
        crossedTripPointTemperature = tripPoints.getItem(tripPointCrossed);
    }
    takePowerAction(currentTemperature, tripPointCrossed, crossedTripPointTemperature);
}

void CriticalPolicy::takePowerAction(const Temperature& currentTemperature, 
    ParticipantSpecificInfoKey::Type crossedTripPoint, const Temperature& crossedTripPointTemperature)
{
    switch (crossedTripPoint)
    {
        case ParticipantSpecificInfoKey::Warm:
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, "Instructing system to sleep."));
            m_stats.sleepSignalled();
            getPolicyServices().platformPowerState->sleep();
            break;
        case ParticipantSpecificInfoKey::Hot:
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, "Instructing system to hibernate."));
            m_stats.hibernateSignalled();
            getPolicyServices().platformPowerState->hibernate();
            break;
        case ParticipantSpecificInfoKey::Critical:
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, string("Instructing system to shut down. ")
                + string("Current temperature is ") + currentTemperature.toString() + string(". ")
                + string("Trip point temperature is ") + crossedTripPointTemperature.toString() + string(".")));
            m_stats.shutdownSignalled();
            getPolicyServices().platformPowerState->shutDown(currentTemperature, crossedTripPointTemperature);
            break;
        case ParticipantSpecificInfoKey::None:
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, "No power action needed."));
            break;
        default:
            throw dptf_exception("An invalid trip point has been selected.");
    }
}

void CriticalPolicy::setParticipantTemperatureThresholdNotification(
    Temperature currentTemperature,
    std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>> tripPoints,
    ParticipantProxy& participant)
{
    Temperature lowerTemperatureThreshold = determineLowerTemperatureThreshold(currentTemperature, tripPoints);
    Temperature upperTemperatureThreshold = determineUpperTemperatureThreshold(currentTemperature, tripPoints);
    participant.setTemperatureThresholds(lowerTemperatureThreshold, upperTemperatureThreshold);
}

Temperature CriticalPolicy::determineLowerTemperatureThreshold(
    Temperature currentTemperature,
    std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>> tripPoints)
{
    Temperature lowerTemperatureThreshold(Temperature::createInvalid());
    for (auto tp = tripPoints.begin(); tp != tripPoints.end(); ++tp)
    {
        if (currentTemperature >= Temperature(tp->second))
        {
            lowerTemperatureThreshold = tp->second;
        }
        else
        {
            break;
        }
    }
    return lowerTemperatureThreshold;
}

Temperature CriticalPolicy::determineUpperTemperatureThreshold(
    Temperature currentTemperature,
    std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>> tripPoints)
{
    Temperature upperTemperatureThreshold(Temperature::createInvalid());
    for (auto tp = tripPoints.begin(); tp != tripPoints.end(); ++tp)
    {
        if (currentTemperature < Temperature(tp->second))
        {
            upperTemperatureThreshold = tp->second;
            break;
        }
    }
    return upperTemperatureThreshold;
}

ParticipantSpecificInfoKey::Type CriticalPolicy::findTripPointCrossed(
    const std::vector<std::pair<ParticipantSpecificInfoKey::Type, UIntN>>& tripPoints,
    const Temperature& currentTemperature)
{
    auto crossedTripPoint = ParticipantSpecificInfoKey::None;
    for (auto tp = tripPoints.begin(); tp != tripPoints.end(); ++tp)
    {
        if (currentTemperature >= Temperature(tp->second))
        {
            crossedTripPoint = tp->first;
        }
        else
        {
            break;
        }
    }
    return crossedTripPoint;
}

Bool CriticalPolicy::participantHasDesiredProperties(ParticipantProxy& newParticipant)
{
    return (newParticipant.supportsTemperatureInterface() &&
            newParticipant.getCriticalTripPointProperty().supportsProperty());
}

XmlNode* CriticalPolicy::getXmlForCriticalTripPoints() const
{
    XmlNode* allStatus = XmlNode::createWrapperElement("critical_trip_point_status");
    vector<UIntN> participantTndexes = getParticipantTracker().getAllTrackedIndexes();
    for (auto participantIndex = participantTndexes.begin(); 
        participantIndex != participantTndexes.end(); 
        participantIndex++)
    {
        ParticipantProxy& participant = getParticipantTracker()[*participantIndex];
        if (participant.getCriticalTripPointProperty().supportsProperty())
        {
            allStatus->addChild(participant.getXmlForCriticalTripPoints());
        }
    }
    return allStatus;
}