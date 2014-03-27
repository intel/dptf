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

#include "PassivePolicy.h"
#include "Constants.h"
#include <algorithm>
#include <set>
#include <map>
#include "PassiveControlStatus.h"
#include "TargetLimitAction.h"
#include "TargetUnlimitAction.h"
#include "TargetNoAction.h"
#include "TargetCheckLaterAction.h"

using namespace std;

static const Guid MyGuid(0xD6, 0x41, 0xA4, 0x42, 0x6A, 0xAE, 0x2B, 0x46, 0xA8, 0x4B, 0x4A, 0x8C, 0xE7, 0x90, 0x27, 0xD3);
static const string MyName("Passive Policy");

PassivePolicy::PassivePolicy(void)
    : PolicyBase(),
    m_trt(std::vector<ThermalRelationshipTableEntry>()),
    m_utilizationBiasThreshold(Percentage(0.0))
{
}

PassivePolicy::~PassivePolicy(void)
{
}

void PassivePolicy::onCreate(void)
{
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::ParticipantSpecificInfoChanged);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainPowerControlCapabilityChanged);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainPerformanceControlCapabilityChanged);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainPerformanceControlsChanged);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainCoreControlCapabilityChanged);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainPriorityChanged);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainDisplayControlCapabilityChanged);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DomainDisplayStatusChanged);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::PolicyThermalRelationshipTableChanged);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::PolicyInitiatedCallback);
    getPolicyServices().policyEventRegistration->registerEvent(PolicyEvent::DptfResume);

    m_trt = getPolicyServices().platformConfigurationData->getThermalRelationshipTable();
    try
    {
        m_utilizationBiasThreshold = Percentage::fromWholeNumber(
            getPolicyServices().platformConfigurationData->readConfigurationUInt32(
            "PreferenceBiasUtilizationThreshold"));
    }
    catch (...)
    {
        m_utilizationBiasThreshold = Percentage(0.0);
    }

    m_callbackScheduler.reset(new CallbackScheduler(getPolicyServices(), m_trt, &m_targetMonitor, getTime()));
}

void PassivePolicy::onDestroy(void)
{
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainTemperatureThresholdCrossed);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::ParticipantSpecificInfoChanged);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainPowerControlCapabilityChanged);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainPerformanceControlCapabilityChanged);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainPerformanceControlsChanged);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainCoreControlCapabilityChanged);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainPriorityChanged);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainDisplayControlCapabilityChanged);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DomainDisplayStatusChanged);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::PolicyThermalRelationshipTableChanged);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::PolicyInitiatedCallback);
    getPolicyServices().policyEventRegistration->unregisterEvent(PolicyEvent::DptfResume);
}

void PassivePolicy::onEnable(void)
{
    reloadTrtAndCheckAllTargets();
}

void PassivePolicy::onDisable(void)
{
    // no defined action for this event
}

void PassivePolicy::onConnectedStandbyEntry(void)
{
    // no defined action for this event
}

void PassivePolicy::onConnectedStandbyExit(void)
{
    reloadTrtAndCheckAllTargets();
}

void PassivePolicy::onResume(void)
{
    // FIXME (BRG) - Reevaluate performance caps on S3 resume
    //   Is there a better solution?
    vector<UIntN> participantIndexes = getParticipantTracker().getAllTrackedIndexes();

    for (auto participantIndex = participantIndexes.begin(); 
        participantIndex != participantIndexes.end(); 
        participantIndex++)
    {
        if (getParticipantTracker()[*participantIndex].getParticipantProperties().getName() == "TCHG")
        {
            onDomainPerformanceControlCapabilityChanged(*participantIndex);
        }
    }
}

bool PassivePolicy::autoNotifyPlatformOscOnCreateDestroy() const
{
    return true;
}

bool PassivePolicy::autoNotifyPlatformOscOnConnectedStandbyEntryExit() const
{
    return false;
}

bool PassivePolicy::autoNotifyPlatformOscOnEnableDisable() const
{
    return true;
}

Guid PassivePolicy::getGuid(void) const
{
    return MyGuid;
}

string PassivePolicy::getName(void) const
{
    return MyName;
}

string PassivePolicy::getStatusAsXml(void) const
{
    XmlNode* root = XmlNode::createRoot();
    XmlNode* format = XmlNode::createComment("format_id=" + getGuid().toString());
    root->addChild(format);
    XmlNode* status = XmlNode::createWrapperElement("passive_policy_status");
    status->addChild(getXmlForPassiveTripPoints());
    status->addChild(m_trt.getXml());
    PassiveControlStatus controlStatus(m_trt, getParticipantTracker());
    status->addChild(controlStatus.getXml());
    status->addChild(getParticipantTracker().getXmlForTripPointStatistics());
    status->addChild(m_callbackScheduler->getXml());
    root->addChild(status);
    string statusString = root->toString();
    delete root;
    return statusString;
}

void PassivePolicy::onBindParticipant(UIntN participantIndex)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Binding participant.", participantIndex));
    getParticipantTracker().remember(participantIndex);
    associateParticipantInTrt(getParticipantTracker()[participantIndex]);
    m_callbackScheduler->setTrt(m_trt);
}

void PassivePolicy::onUnbindParticipant(UIntN participantIndex)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Unbinding participant.", participantIndex));
    m_trt.disassociateParticipant(participantIndex);
    m_callbackScheduler->removeParticipantFromSchedule(participantIndex);
    m_callbackScheduler->setTrt(m_trt);
    m_targetMonitor.stopMonitoring(participantIndex);
    removeAllRequestsForTarget(participantIndex);
    getParticipantTracker().forget(participantIndex);
}

void PassivePolicy::onBindDomain(UIntN participantIndex, UIntN domainIndex)
{
    if (getParticipantTracker().remembers(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Binding domain for participant.", participantIndex, domainIndex));
        DomainProxy& domain = getParticipantTracker()[participantIndex].bindDomain(domainIndex);
        domain.setTstateUtilizationThreshold(m_utilizationBiasThreshold);

        if (participantIsSourceDevice(participantIndex))
        {
            domain.initializeControls();
        }

        if (participantIsTargetDevice(participantIndex))
        {
            auto currentTemperature = 
                getParticipantTracker()[participantIndex][0].getTemperatureProperty().getCurrentTemperature();
            setParticipantTemperatureThresholdNotification(getParticipantTracker()[participantIndex], currentTemperature);
            notifyPlatformOfDeviceTemperature(getParticipantTracker()[participantIndex], currentTemperature);
            takeThermalActionForTarget(participantIndex);
        }
    }
}

void PassivePolicy::onUnbindDomain(UIntN participantIndex, UIntN domainIndex)
{
    if (getParticipantTracker().remembers(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Unbinding domain for participant.", participantIndex, domainIndex));
        getParticipantTracker()[participantIndex].unbindDomain(domainIndex);
    }
}

void PassivePolicy::onParticipantSpecificInfoChanged(UIntN participantIndex)
{
    if (participantIsTargetDevice(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Specific info changed for target participant.", participantIndex));
        ParticipantProxy& participant = getParticipantTracker()[participantIndex];
        participant.getPassiveTripPointProperty().refresh();
        if (participant.getDomainPropertiesSet().getDomainCount() > 0)
        {
            auto currentTemperature = participant[0].getTemperatureProperty().getCurrentTemperature();
            setParticipantTemperatureThresholdNotification(participant, currentTemperature);
            notifyPlatformOfDeviceTemperature(participant, currentTemperature);
            takeThermalActionForTarget(participantIndex);
        }
    }
}

void PassivePolicy::onDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
    if (participantIsTargetDevice(participantIndex))
    {
        ParticipantProxy& participant = getParticipantTracker()[participantIndex];
        if (participant.getDomainPropertiesSet().getDomainCount() > 0)
        {
            auto currentTemperature = participant[0].getTemperatureProperty().getCurrentTemperature();
            getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF,
                "Temperature threshold crossed for target participant " + to_string(participantIndex) +
                " with temperature " + currentTemperature.toString() + "."));
            participant.setThresholdCrossed(currentTemperature, getTime()->getCurrentTimeInMilliseconds());
            setParticipantTemperatureThresholdNotification(participant, currentTemperature);
            notifyPlatformOfDeviceTemperature(participant, currentTemperature);
            takeThermalActionForTarget(participantIndex);
        }
    }
}

void PassivePolicy::onDomainPowerControlCapabilityChanged(UIntN participantIndex)
{
    if (participantIsSourceDevice(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Power control capabilities changed for source participant.", participantIndex));
        auto domainIndexes = getParticipantTracker()[participantIndex].getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxy& domain = getParticipantTracker()[participantIndex][*domainIndex];
                if (domain.getPowerControl().supportsPowerControls())
                {
                    domain.getPowerControl().refreshCapabilities();
                    domain.clearAllPowerControlRequests();
                    domain.getPowerControl().initializeControlsIfNeeded();
                }
            }
            catch (std::exception& ex)
            {
                getPolicyServices().messageLogging->writeMessageError(PolicyMessage(FLF, ex.what()));
            }
        }
    }
}

void PassivePolicy::onDomainPerformanceControlCapabilityChanged(UIntN participantIndex)
{
    if (participantIsSourceDevice(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Performance control capabilities changed for source participant.", participantIndex));
        auto domainIndexes = getParticipantTracker()[participantIndex].getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxy& domain = getParticipantTracker()[participantIndex][*domainIndex];
                if (domain.getPerformanceControl().supportsPerformanceControls())
                {
                    domain.getPerformanceControl().refreshCapabilities();
                    domain.clearAllPerformanceControlRequests();
                    domain.getPerformanceControl().initializeControlsIfNeeded();
                }
            }
            catch (std::exception& ex)
            {
                getPolicyServices().messageLogging->writeMessageError(PolicyMessage(FLF, ex.what()));
            }
        }
    }
}

void PassivePolicy::onDomainPerformanceControlsChanged(UIntN participantIndex)
{
    if (participantIsSourceDevice(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Performance control set changed for source participant.", participantIndex));
        auto domainIndexes = getParticipantTracker()[participantIndex].getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxy& domain = getParticipantTracker()[participantIndex][*domainIndex];
                if (domain.getPerformanceControl().supportsPerformanceControls())
                {
                    domain.getPerformanceControl().refreshControls();
                }
            }
            catch (std::exception& ex)
            {
                getPolicyServices().messageLogging->writeMessageError(PolicyMessage(FLF, ex.what()));
            }
        }
    }
}

void PassivePolicy::onDomainCoreControlCapabilityChanged(UIntN participantIndex)
{
    if (participantIsSourceDevice(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Core control capabilities changed for source participant.", participantIndex));
        auto domainIndexes = getParticipantTracker()[participantIndex].getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxy& domain = getParticipantTracker()[participantIndex][*domainIndex];
                if (domain.getCoreControl().supportsCoreControls())
                {
                    domain.getCoreControl().refreshCapabilities();
                    domain.clearAllCoreControlRequests();
                    domain.getCoreControl().initializeControlsIfNeeded();
                }
            }
            catch (std::exception& ex)
            {
                getPolicyServices().messageLogging->writeMessageError(PolicyMessage(FLF, ex.what()));
            }
        }
    }
}

void PassivePolicy::onDomainDisplayControlCapabilityChanged(UIntN participantIndex)
{
    if (participantIsSourceDevice(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Display control capabilities changed for source participant.", participantIndex));
        auto domainIndexes = getParticipantTracker()[participantIndex].getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxy& domain = getParticipantTracker()[participantIndex][*domainIndex];
                if (domain.getDisplayControl().supportsDisplayControls())
                {
                    domain.getDisplayControl().refreshCapabilities();
                    domain.clearAllDisplayControlRequests();
                }
            }
            catch (std::exception& ex)
            {
                getPolicyServices().messageLogging->writeMessageError(PolicyMessage(FLF, ex.what()));
            }
        }
    }
}

void PassivePolicy::onDomainDisplayStatusChanged(UIntN participantIndex)
{
    if (participantIsSourceDevice(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Display status changed for source participant.", participantIndex));
        auto domainIndexes = getParticipantTracker()[participantIndex].getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxy& domain = getParticipantTracker()[participantIndex][*domainIndex];
                if (domain.getDisplayControl().supportsDisplayControls())
                {
                    domain.getDisplayControl().invalidateStatus();
                }
            }
            catch (std::exception& ex)
            {
                getPolicyServices().messageLogging->writeMessageError(PolicyMessage(FLF, ex.what()));
            }
        }
    }
}

void PassivePolicy::onDomainPriorityChanged(UIntN participantIndex)
{
    if (participantIsSourceDevice(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Domain priority changed for source participant.", participantIndex));
        auto domainIndexes = getParticipantTracker()[participantIndex].getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxy& domain = getParticipantTracker()[participantIndex][*domainIndex];
                domain.getDomainPriorityProperty().refresh();
            }
            catch (std::exception& ex)
            {
                getPolicyServices().messageLogging->writeMessageError(PolicyMessage(FLF, ex.what()));
            }
        }
    }
}

void PassivePolicy::onThermalRelationshipTableChanged(void)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Thermal Relationship Table changed"));
    m_trt = getPolicyServices().platformConfigurationData->getThermalRelationshipTable();
    vector<UIntN> allIndicies = getParticipantTracker().getAllTrackedIndexes();
    for (auto index = allIndicies.begin(); index != allIndicies.end(); ++index)
    {
        associateParticipantInTrt(getParticipantTracker()[*index]);
    }
    m_callbackScheduler->setTrt(m_trt);
}

void PassivePolicy::onPolicyInitiatedCallback(UInt64 eventCode, UInt64 param1, void* param2)
{
    UIntN targetIndex = (UIntN) param1;
    if (participantIsTargetDevice(targetIndex))
    {
        m_callbackScheduler->acknowledgeCallback(targetIndex);
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF,
            "Callback received for target participant " + to_string(targetIndex) + ".", targetIndex));
        takeThermalActionForTarget(targetIndex);
    }
}

void PassivePolicy::onOverrideTimeObject(std::shared_ptr<TimeInterface> timeObject)
{
    m_callbackScheduler->setTimeObject(timeObject);
}

void PassivePolicy::takeThermalActionForTarget(UIntN target)
{
    TargetActionBase* action = determineAction(target);
    action->execute();
    delete action;
}

TargetActionBase* PassivePolicy::determineAction(UIntN target)
{
    ParticipantProxy& participant = getParticipantTracker()[target];
    if (participant.getDomainPropertiesSet().getDomainCount() > 0)
    {
        auto currentTemperature = participant[0].getTemperatureProperty().getCurrentTemperature();
        auto passiveTripPoints = participant.getPassiveTripPointProperty().getTripPoints();
        auto psv = passiveTripPoints.getItem(ParticipantSpecificInfoKey::PSV);
        if (currentTemperature > Temperature(psv))
        {
            return new TargetLimitAction(
                getPolicyServices(), getTime(), getParticipantTracker(), m_trt, m_callbackScheduler, 
                m_targetMonitor, target);
        }
        else if ((currentTemperature < Temperature(psv)) && (m_targetMonitor.isMonitoring(target)))
        {
            return new TargetUnlimitAction(
                getPolicyServices(), getTime(), getParticipantTracker(), m_trt, m_callbackScheduler, 
                m_targetMonitor, target);
        }
        else if (currentTemperature == Temperature(psv))
        {
            return new TargetCheckLaterAction(
                getPolicyServices(), getTime(), getParticipantTracker(), m_trt, m_callbackScheduler, 
                m_targetMonitor, target);
        }
        else
        {
            return new TargetNoAction(
                getPolicyServices(), getTime(), getParticipantTracker(), m_trt, m_callbackScheduler, 
                m_targetMonitor, target);
        }
    }
    else
    {
        return new TargetNoAction(
            getPolicyServices(), getTime(), getParticipantTracker(), m_trt, m_callbackScheduler, 
            m_targetMonitor, target);
    }
}

void PassivePolicy::setParticipantTemperatureThresholdNotification(
    ParticipantProxy& participant,
    Temperature currentTemperature)
{
    auto passiveTripPoints = participant.getPassiveTripPointProperty().getTripPoints();
    Temperature psv = passiveTripPoints.getItem(ParticipantSpecificInfoKey::PSV);
    Temperature lowerBoundTemperature(Temperature::createInvalid());
    Temperature upperBoundTemperature = psv;
    if (currentTemperature >= psv)
    {
        if (passiveTripPoints.hasItem(ParticipantSpecificInfoKey::NTT))
        {
            // lower bound is psv + some multiple of ntt
            // upper bound is lower bound + ntt
            auto ntt = passiveTripPoints.getItem(ParticipantSpecificInfoKey::NTT);
            lowerBoundTemperature = psv;
            while ((lowerBoundTemperature + ntt) <= currentTemperature)
            {
                lowerBoundTemperature = lowerBoundTemperature + ntt;
            }
            upperBoundTemperature = lowerBoundTemperature + ntt;
        }
        else
        {
            lowerBoundTemperature = psv;
            upperBoundTemperature = Temperature::createInvalid();
        }
    }
    participant.setTemperatureThresholds(lowerBoundTemperature, upperBoundTemperature);
}

void PassivePolicy::notifyPlatformOfDeviceTemperature(ParticipantProxy& participant, UIntN currentTemperature)
{
    auto passiveTripPoints = participant.getPassiveTripPointProperty().getTripPoints();
    if (passiveTripPoints.hasItem(ParticipantSpecificInfoKey::NTT))
    {
        auto ntt = passiveTripPoints.getItem(ParticipantSpecificInfoKey::NTT);
        auto psv = passiveTripPoints.getItem(ParticipantSpecificInfoKey::PSV);
        if (currentTemperature >= (psv + ntt))
        {
            participant.notifyPlatformOfDeviceTemperature(currentTemperature);
        }
    }
}

void PassivePolicy::associateParticipantInTrt(ParticipantProxy& participant)
{
    m_trt.associateParticipant(
        participant.getParticipantProperties().getAcpiInfo().getAcpiScope(),
        participant.getIndex());
}

void PassivePolicy::takeThermalActionForAllTargetsForSource(UIntN source)
{
    vector<ThermalRelationshipTableEntry> entries = m_trt.getEntriesForSource(source);
    for (auto entry = entries.begin(); entry != entries.end(); entry++)
    {
        takeThermalActionForTarget(entry->targetDeviceIndex());
    }
}

void PassivePolicy::reloadTrtAndCheckAllTargets()
{
    m_trt = getPolicyServices().platformConfigurationData->getThermalRelationshipTable();
    vector<UIntN> allParticipants = getParticipantTracker().getAllTrackedIndexes();
    for (auto index = allParticipants.begin(); index != allParticipants.end(); index++)
    {
        associateParticipantInTrt(getParticipantTracker()[*index]);
        if (m_trt.isParticipantTargetDevice(*index))
        {
            getParticipantTracker()[*index].getPassiveTripPointProperty().refresh();
        }
    }
    m_callbackScheduler->setTrt(m_trt);

    for (auto index = allParticipants.begin(); index != allParticipants.end(); index++)
    {
        if (m_trt.isParticipantTargetDevice(*index))
        {
            auto currentTemperature =
                getParticipantTracker()[*index][0].getTemperatureProperty().getCurrentTemperature();
            setParticipantTemperatureThresholdNotification(getParticipantTracker()[*index], currentTemperature);
            notifyPlatformOfDeviceTemperature(getParticipantTracker()[*index], currentTemperature);
            takeThermalActionForTarget(*index);
        }
    }
}

void PassivePolicy::removeAllRequestsForTarget(UIntN target)
{
    if (participantIsTargetDevice(target))
    {
        vector<UIntN> participantIndexes = getParticipantTracker().getAllTrackedIndexes();
        for (auto participantIndex = participantIndexes.begin(); 
            participantIndex != participantIndexes.end(); 
            participantIndex++)
        {
            vector<UIntN> domainIndexes = getParticipantTracker()[*participantIndex].getDomainIndexes();
            for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
            {
                getParticipantTracker()[*participantIndex][*domainIndex].clearAllRequestsForTarget(target);
            }
        }
    }
}

Bool PassivePolicy::participantIsSourceDevice(UIntN participantIndex) const
{
    return getParticipantTracker().remembers(participantIndex) &&
           m_trt.isParticipantSourceDevice(participantIndex);
}

Bool PassivePolicy::participantIsTargetDevice(UIntN participantIndex) const
{
    return getParticipantTracker().remembers(participantIndex) &&
           m_trt.isParticipantTargetDevice(participantIndex);
}

Bool PassivePolicy::participantIsSourceOrTargetDevice(UIntN participantIndex) const
{
    return participantIsSourceDevice(participantIndex) ||
           participantIsTargetDevice(participantIndex);
}

XmlNode* PassivePolicy::getXmlForPassiveTripPoints() const
{
    XmlNode* allStatus = XmlNode::createWrapperElement("passive_trip_point_status");
    vector<UIntN> participantIndexes = getParticipantTracker().getAllTrackedIndexes();
    for (auto participantIndex = participantIndexes.begin(); 
        participantIndex != participantIndexes.end(); 
        participantIndex++)
    {
        ParticipantProxy& participant = getParticipantTracker()[*participantIndex];
        if (participantIsTargetDevice(*participantIndex) && 
            participant.getPassiveTripPointProperty().supportsProperty())
        {
            allStatus->addChild(participant.getXmlForPassiveTripPoints());
        }
    }
    return allStatus;
}