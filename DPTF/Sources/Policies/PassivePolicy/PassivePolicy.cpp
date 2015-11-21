/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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
#include <algorithm>
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
    try
    {
        m_trt = getPolicyServices().platformConfigurationData->getThermalRelationshipTable();
    }
    catch (std::exception& ex)
    {
        getPolicyServices().messageLogging->writeMessageWarning(PolicyMessage(
            FLF, "No thermal relationship table was found. Passive Policy will not load without it" 
            + string(ex.what())));
        throw;
    }

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
}

void PassivePolicy::onEnable(void)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Policy enable event received."));
    reloadTrtIfDifferent();
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
    // no defined action for this event
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
    status->addChild(getParticipantTracker()->getXmlForTripPointStatistics());
    status->addChild(m_callbackScheduler->getXml());
    status->addChild(XmlNode::createDataElement("utilization_threshold", m_utilizationBiasThreshold.getCurrentUtilization().toString()));
    root->addChild(status);
    string statusString = root->toString();
    delete root;
    return statusString;
}

void PassivePolicy::onBindParticipant(UIntN participantIndex)
{
    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Binding participant.", participantIndex));
    getParticipantTracker()->remember(participantIndex);
    associateParticipantInTrt(getParticipantTracker()->getParticipant(participantIndex), m_trt);
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
    getParticipantTracker()->forget(participantIndex);
}

void PassivePolicy::onBindDomain(UIntN participantIndex, UIntN domainIndex)
{
    if (getParticipantTracker()->remembers(participantIndex))
    {
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Binding domain for participant.", participantIndex, domainIndex));
        DomainProxyInterface* domain = participant->bindDomain(domainIndex);
        domain->setTstateUtilizationThreshold(m_utilizationBiasThreshold);

        if (participantIsSourceDevice(participantIndex))
        {
            domain->initializeControls();
        }

        takePossibleThermalActionForTarget(participantIndex);
    }
}

void PassivePolicy::onUnbindDomain(UIntN participantIndex, UIntN domainIndex)
{
    if (getParticipantTracker()->remembers(participantIndex))
    {
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Unbinding domain for participant.", participantIndex, domainIndex));
        participant->unbindDomain(domainIndex);
    }
}

void PassivePolicy::onParticipantSpecificInfoChanged(UIntN participantIndex)
{
    if (participantIsTargetDevice(participantIndex))
    {
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Specific info changed for target participant.", participantIndex));
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
        participant->getPassiveTripPointProperty().refresh();
        participant->refreshHysteresis();
        if (participant->getDomainPropertiesSet().getDomainCount() > 0)
        {
            takePossibleThermalActionForTarget(participantIndex);
        }
    }
}

void PassivePolicy::onDomainTemperatureThresholdCrossed(UIntN participantIndex)
{
    try
    {
        if (participantIsTargetDevice(participantIndex))
        {
            ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
            if (participant->getDomainPropertiesSet().getDomainCount() > 0)
            {
                auto currentTemperature = participant->getFirstDomainTemperature();
                participant->setThresholdCrossed(currentTemperature, getTime()->getCurrentTimeInMilliseconds());
                takePossibleThermalActionForTarget(participantIndex, currentTemperature);
            }
        }
    }
    catch (std::exception& ex)
    {
        getPolicyServices().messageLogging->writeMessageWarning(PolicyMessage(
            FLF, "Failed to take thermal action for target on temperature threshold crossed event: " 
            + string(ex.what()), participantIndex));
    }
}

void PassivePolicy::onDomainPowerControlCapabilityChanged(UIntN participantIndex)
{
    if (participantIsSourceDevice(participantIndex))
    {
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Power control capabilities changed for source participant.", participantIndex));
        auto domainIndexes = participant->getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxyInterface* domain = participant->getDomain(*domainIndex);
                if (domain->getPowerControl()->supportsPowerControls())
                {
                    domain->getPowerControl()->refreshCapabilities();
                    domain->adjustPowerRequests();
                    domain->setArbitratedPowerLimit();
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
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Performance control capabilities changed for source participant.", participantIndex));
        auto domainIndexes = participant->getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxyInterface* domain = participant->getDomain(*domainIndex);
                if (domain->getPerformanceControl()->supportsPerformanceControls())
                {
                    domain->getPerformanceControl()->refreshCapabilities();
                    domain->adjustPerformanceRequests();
                    domain->setArbitratedPerformanceLimit();
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
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Performance control set changed for source participant.", participantIndex));
        auto domainIndexes = participant->getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxyInterface* domain = participant->getDomain(*domainIndex);
                if (domain->getPerformanceControl()->supportsPerformanceControls())
                {
                    domain->getPerformanceControl()->refreshControls();
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
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Core control capabilities changed for source participant.", participantIndex));
        auto domainIndexes = participant->getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxyInterface* domain = participant->getDomain(*domainIndex);
                if (domain->getCoreControl()->supportsCoreControls())
                {
                    domain->getCoreControl()->refreshCapabilities();
                    domain->adjustCoreRequests();
                    domain->setArbitratedCoreLimit();
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
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Display control capabilities changed for source participant.", participantIndex));
        auto domainIndexes = participant->getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxyInterface* domain = participant->getDomain(*domainIndex);
                if (domain->getDisplayControl()->supportsDisplayControls())
                {
                    domain->getDisplayControl()->refreshCapabilities();
                    domain->clearAllDisplayControlRequests();
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
    }
}

void PassivePolicy::onDomainPriorityChanged(UIntN participantIndex)
{
    if (participantIsSourceDevice(participantIndex))
    {
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(
            FLF, "Domain priority changed for source participant.", participantIndex));
        auto domainIndexes = participant->getDomainIndexes();
        for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
        {
            try
            {
                DomainProxyInterface* domain = participant->getDomain(*domainIndex);
                domain->getDomainPriorityProperty().refresh();
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
    reloadTrtIfDifferent();
}

void PassivePolicy::onPolicyInitiatedCallback(UInt64 eventCode, UInt64 param1, void* param2)
{
    UIntN targetIndex = (UIntN) param1;
    if (participantIsTargetDevice(targetIndex))
    {
        m_callbackScheduler->acknowledgeCallback(targetIndex);
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF,
            "Callback received for target participant " + StlOverride::to_string(targetIndex) + ".", targetIndex));
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
    ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(target);
    if (participant->getDomainPropertiesSet().getDomainCount() > 0)
    {
        auto currentTemperature = participant->getFirstDomainTemperature();
        auto passiveTripPoints = participant->getPassiveTripPointProperty().getTripPoints();
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
    ParticipantProxyInterface* participant,
    Temperature currentTemperature)
{
    auto passiveTripPoints = participant->getPassiveTripPointProperty().getTripPoints();
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
    participant->setTemperatureThresholds(lowerBoundTemperature, upperBoundTemperature);
}

void PassivePolicy::notifyPlatformOfDeviceTemperature(ParticipantProxyInterface* participant, UIntN currentTemperature)
{
    auto passiveTripPoints = participant->getPassiveTripPointProperty().getTripPoints();
    if (passiveTripPoints.hasItem(ParticipantSpecificInfoKey::NTT))
    {
        auto ntt = passiveTripPoints.getItem(ParticipantSpecificInfoKey::NTT);
        auto psv = passiveTripPoints.getItem(ParticipantSpecificInfoKey::PSV);
        if (currentTemperature >= (psv + ntt))
        {
            participant->notifyPlatformOfDeviceTemperature(currentTemperature);
        }
    }
}

void PassivePolicy::associateParticipantInTrt(ParticipantProxyInterface* participant, ThermalRelationshipTable& trt)
{
    trt.associateParticipant(
        participant->getParticipantProperties().getAcpiInfo().getAcpiScope(),
        participant->getIndex());
}

void PassivePolicy::reloadTrtIfDifferent()
{
    auto newTrt = getPolicyServices().platformConfigurationData->getThermalRelationshipTable();
    associateAllParticipantsInTrt(newTrt);
    if (m_trt != newTrt)
    {
        vector<UIntN> indexes = getParticipantTracker()->getAllTrackedIndexes();
        for (auto participantIndex = indexes.begin(); participantIndex != indexes.end(); participantIndex++)
        {
            if (m_trt.isParticipantTargetDevice(*participantIndex))
            {
                ParticipantProxyInterface* target = getParticipantTracker()->getParticipant(*participantIndex);
                target->setTemperatureThresholds(Temperature::createInvalid(), Temperature::createInvalid());
            }
        }

        clearAllSourceControls(); // resets the sources from current TRT
        m_trt = newTrt;
        associateAllParticipantsInTrt(m_trt);
        m_targetMonitor.stopMonitoringAll();
        m_callbackScheduler.reset(new CallbackScheduler(getPolicyServices(), m_trt, &m_targetMonitor, getTime()));
        clearAllSourceControls(); // resets the sources from new TRT
        takePossibleThermalActionForAllTargets();
    }
}

void PassivePolicy::removeAllRequestsForTarget(UIntN target)
{
    if (participantIsTargetDevice(target))
    {
        vector<UIntN> participantIndexes = getParticipantTracker()->getAllTrackedIndexes();
        for (auto participantIndex = participantIndexes.begin(); 
            participantIndex != participantIndexes.end(); 
            participantIndex++)
        {
            ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(*participantIndex);
            vector<UIntN> domainIndexes = participant->getDomainIndexes();
            for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
            {
                DomainProxyInterface* domain = participant->getDomain(*domainIndex);
                domain->clearAllRequestsForTarget(target);
            }
        }
    }
}

Bool PassivePolicy::participantIsSourceDevice(UIntN participantIndex) const
{
    return getParticipantTracker()->remembers(participantIndex) &&
           m_trt.isParticipantSourceDevice(participantIndex);
}

Bool PassivePolicy::participantIsTargetDevice(UIntN participantIndex) const
{
    return getParticipantTracker()->remembers(participantIndex) &&
           m_trt.isParticipantTargetDevice(participantIndex);
}

XmlNode* PassivePolicy::getXmlForPassiveTripPoints() const
{
    XmlNode* allStatus = XmlNode::createWrapperElement("passive_trip_point_status");
    vector<UIntN> participantIndexes = getParticipantTracker()->getAllTrackedIndexes();
    for (auto participantIndex = participantIndexes.begin(); 
        participantIndex != participantIndexes.end(); 
        participantIndex++)
    {
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(*participantIndex);
        if (participantIsTargetDevice(*participantIndex) && 
            participant->getPassiveTripPointProperty().supportsProperty())
        {
            allStatus->addChild(participant->getXmlForPassiveTripPoints());
        }
    }
    return allStatus;
}

void PassivePolicy::clearAllSourceControls()
{
    vector<UIntN> allIndicies = getParticipantTracker()->getAllTrackedIndexes();
    for (auto index = allIndicies.begin(); index != allIndicies.end(); ++index)
    {
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(*index);
        if (participantIsSourceDevice(participant->getIndex()))
        {
            auto domainIndexes = participant->getDomainIndexes();
            for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
            {
                auto domain = participant->getDomain(*domainIndex);
                domain->clearAllControlKnobRequests();
                domain->setControlsToMax();
            }
        }
    }
}

void PassivePolicy::associateAllParticipantsInTrt(ThermalRelationshipTable& trt)
{
    vector<UIntN> allIndicies = getParticipantTracker()->getAllTrackedIndexes();
    for (auto index = allIndicies.begin(); index != allIndicies.end(); ++index)
    {
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(*index);
        associateParticipantInTrt(participant, trt);
    }
}

void PassivePolicy::takePossibleThermalActionForAllTargets()
{
    vector<UIntN> allIndicies = getParticipantTracker()->getAllTrackedIndexes();
    for (auto index = allIndicies.begin(); index != allIndicies.end(); ++index)
    {
        takePossibleThermalActionForTarget(*index);
    }
}

void PassivePolicy::takePossibleThermalActionForTarget(UIntN participantIndex)
{
    try
    {
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
        if (participantIsTargetDevice(participant->getIndex()) && participant->supportsTemperatureInterface())
        {
            auto currentTemperature = participant->getFirstDomainTemperature();
            takePossibleThermalActionForTarget(participantIndex, currentTemperature);
        }
    }
    catch (std::exception& ex)
    {
        getPolicyServices().messageLogging->writeMessageWarning(PolicyMessage(
            FLF, "Failed to take thermal action for target: " + string(ex.what()), participantIndex));
    }
}

void PassivePolicy::takePossibleThermalActionForTarget(UIntN participantIndex, const Temperature& temperature)
{
    try
    {
        ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(participantIndex);
        if (participantIsTargetDevice(participant->getIndex()) && participant->supportsTemperatureInterface())
        {
            setParticipantTemperatureThresholdNotification(participant, temperature);
            notifyPlatformOfDeviceTemperature(participant, temperature);
            takeThermalActionForTarget(participant->getIndex());
        }
    }
    catch (std::exception& ex)
    {
        getPolicyServices().messageLogging->writeMessageWarning(PolicyMessage(
            FLF, "Failed to take thermal action for target: " + string(ex.what()), participantIndex));
    }
}
