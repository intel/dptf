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

#include "ParticipantProxy.h"
#include "StatusFormat.h"
using namespace std;
using namespace StatusFormat;

ParticipantProxy::ParticipantProxy()
    : m_index(Constants::Invalid),
    m_participantProperties(m_policyServices, Constants::Invalid),
    m_criticalTripPointProperty(m_policyServices, Constants::Invalid),
    m_activeTripPointProperty(m_policyServices, Constants::Invalid),
    m_passiveTripPointProperty(m_policyServices, Constants::Invalid),
    m_domainSetProperty(m_policyServices, Constants::Invalid),
    m_previousLowerBound(Temperature::createInvalid()),
    m_previousUpperBound(Temperature::createInvalid()),
    m_lastIndicationTemperatureLowerBound(Temperature::createInvalid()),
    m_lastThresholdCrossedTemperature(Temperature::createInvalid()),
    m_timeOfLastThresholdCrossed(0),
    m_supportsDscp(false),
    m_supportsDscpValid(false),
    m_supportsScp(false),
    m_supportsScpValid(false)
{
}

ParticipantProxy::ParticipantProxy(
    UIntN participantIndex,
    const PolicyServicesInterfaceContainer& policyServices,
    std::shared_ptr<TimeInterface> time)
    : m_index(participantIndex),
    m_participantProperties(policyServices, participantIndex),
    m_criticalTripPointProperty(policyServices, participantIndex),
    m_activeTripPointProperty(policyServices, participantIndex),
    m_passiveTripPointProperty(policyServices, participantIndex),
    m_domainSetProperty(policyServices, participantIndex),
    m_previousLowerBound(Temperature::createInvalid()),
    m_previousUpperBound(Temperature::createInvalid()),
    m_lastIndicationTemperatureLowerBound(Temperature::createInvalid()),
    m_lastThresholdCrossedTemperature(Temperature::createInvalid()),
    m_timeOfLastThresholdCrossed(0),
    m_supportsDscp(false),
    m_supportsDscpValid(false),
    m_supportsScp(false),
    m_supportsScpValid(false),
    m_policyServices(policyServices),
    m_time(time)
    {
}

ParticipantProxy::~ParticipantProxy()
{
}

UIntN ParticipantProxy::getIndex() const
{
    return m_index;
}

const DomainPropertiesSet& ParticipantProxy::getDomainPropertiesSet()
{
    refreshDomainSetIfUninitialized();
    return m_domainSetProperty.getDomainPropertiesSet();
}

const ParticipantProperties& ParticipantProxy::getParticipantProperties()
{
    return m_participantProperties.getParticipantProperties();
}

CriticalTripPointsCachedProperty& ParticipantProxy::getCriticalTripPointProperty()
{
    return m_criticalTripPointProperty;
}

ActiveTripPointsCachedProperty& ParticipantProxy::getActiveTripPointProperty()
{
    return m_activeTripPointProperty;
}

PassiveTripPointsCachedProperty& ParticipantProxy::getPassiveTripPointProperty()
{
    return m_passiveTripPointProperty;
}

std::vector<UIntN> ParticipantProxy::getDomainIndexes()
{
    std::vector<UIntN> domainIndexes;
    for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
    {
        domainIndexes.push_back(domain->first);
    }
    return domainIndexes;
}

DomainProxyInterface* ParticipantProxy::bindDomain(UIntN domainIndex)
{
    m_domainSetProperty.refresh();
    m_domains[domainIndex] =
        DomainProxy(
            getIndex(),
            domainIndex,
            m_domainSetProperty.getDomainProperties(domainIndex),
            getParticipantProperties(),
            m_policyServices);
    return getDomain(domainIndex); 
}

void ParticipantProxy::unbindDomain(UIntN domainIndex)
{
    m_domains.erase(domainIndex);
}

void ParticipantProxy::refreshDomains()
{
    m_previousLowerBound = Temperature::createInvalid();
    m_previousUpperBound = Temperature::createInvalid();
    m_domains.clear();
    m_domainSetProperty.refresh();
    UIntN numberOfDomains = m_domainSetProperty.getDomainPropertiesSet().getDomainCount();
    for (UIntN setIndex = 0; setIndex < numberOfDomains; ++setIndex)
    {
        DomainProperties domainProperties = m_domainSetProperty.getDomainProperties(setIndex);
        m_domains[domainProperties.getDomainIndex()] =
            DomainProxy(
            getIndex(),
            domainProperties.getDomainIndex(),
            domainProperties,
            getParticipantProperties(),
            m_policyServices);
    }
}

void ParticipantProxy::setTemperatureThresholds(const Temperature& lowerBound, const Temperature& upperBound)
{
    refreshDomainSetIfUninitialized();
    if (m_domains.find(0) != m_domains.end())
    {
        if (m_domains[0].getTemperatureControl()->supportsTemperatureThresholds())
        {
            m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
                "Setting thresholds to " + lowerBound.toString() + ":" + upperBound.toString() + "."));
            m_domains[0].getTemperatureControl()->setTemperatureNotificationThresholds(lowerBound, upperBound);
        }
    }

    m_previousLowerBound = lowerBound;
    m_previousUpperBound = upperBound;
}

Bool ParticipantProxy::supportsTemperatureInterface()
{
    refreshDomainSetIfUninitialized();
    for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
    {
        if (domain->second.getTemperatureControl()->supportsTemperatureControls())
        {
            return true;
        }
    }
    return false;
}

Temperature ParticipantProxy::getFirstDomainTemperature()
{
    auto participantTemperature = Temperature::createInvalid();
    refreshDomainSetIfUninitialized();
    if (m_domains.size() > 0)
    {
        for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
        {
            if (domain->second.getTemperatureControl()->supportsTemperatureControls())
            {
                participantTemperature = domain->second.getTemperatureControl()->getCurrentTemperature();
                break;
            }
        }
    }
    return participantTemperature;
}

void ParticipantProxy::refreshDomainSetIfUninitialized()
{
    if (m_domains.size() == 0)
    {
        refreshDomains();
    }
}

TemperatureThresholds ParticipantProxy::getTemperatureThresholds()
{
    refreshDomainSetIfUninitialized();
    for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
    {
        try
        {
            return domain->second.getTemperatureControl()->getTemperatureNotificationThresholds();
        }
        catch (...)
        {
            // swallow the error
        }
    }
    throw dptf_exception("Failed to get temperature thresholds for participant " + StlOverride::to_string(getIndex()) + ".");
}

void ParticipantProxy::notifyPlatformOfDeviceTemperature(const Temperature& currentTemperature)
{
    try
    {
        if (m_lastIndicationTemperatureLowerBound != m_previousLowerBound)
        {
            m_lastIndicationTemperatureLowerBound = m_previousLowerBound;
            m_policyServices.participantSetSpecificInfo->setParticipantDeviceTemperatureIndication(
                getIndex(), currentTemperature);
        }
    }
    catch (dptf_exception ex)
    {
        m_policyServices.messageLogging->writeMessageWarning(PolicyMessage(FLF,
            string("Set Device Temperature Indication failed with error \"") + 
            string(ex.what()) + string("\"") +
            string(".")));
    }
    catch (...)
    {
        m_policyServices.messageLogging->writeMessageWarning(PolicyMessage(FLF,
            string("Set Device Temperature Indication failed.")));
    }
}

void ParticipantProxy::setThresholdCrossed(const Temperature& temperature, UInt64 timestampInMs)
{
    m_lastThresholdCrossedTemperature = temperature;
    m_timeOfLastThresholdCrossed = timestampInMs;
    m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(FLF,
        "Temperature threshold crossed for participant with temperature " + temperature.toString() + ".", getIndex()));
}

UInt64 ParticipantProxy::getTimeOfLastThresholdCrossed() const
{
    return m_timeOfLastThresholdCrossed;
}

Temperature ParticipantProxy::getTemperatureOfLastThresholdCrossed() const
{
    return m_lastThresholdCrossedTemperature;
}

DomainProxyInterface* ParticipantProxy::getDomain(UIntN domainIndex)
{
    refreshDomainSetIfUninitialized();
    return &m_domains.at(domainIndex);
}

XmlNode* ParticipantProxy::getXmlForCriticalTripPoints()
{
    XmlNode* participant = XmlNode::createWrapperElement("participant");
    participant->addChild(XmlNode::createDataElement("index", 
        friendlyValue(m_index)));
    participant->addChild(XmlNode::createDataElement("name", 
        m_participantProperties.getParticipantProperties().getName()));
    participant->addChild(XmlNode::createDataElement("temperature", 
        getTemperatureForStatus(getDomain(0)).toString()));
    participant->addChild(m_criticalTripPointProperty.getXml());
    return participant;
}

XmlNode* ParticipantProxy::getXmlForActiveTripPoints()
{
    XmlNode* participant = XmlNode::createWrapperElement("participant");
    participant->addChild(XmlNode::createDataElement("index", 
        friendlyValue(m_index)));
    participant->addChild(XmlNode::createDataElement("name", 
        m_participantProperties.getParticipantProperties().getName()));
    participant->addChild(XmlNode::createDataElement("temperature", 
        getTemperatureForStatus(getDomain(0)).toString()));
    participant->addChild(getTemperatureThresholdsForStatus().getXml());
    participant->addChild(m_activeTripPointProperty.getXml());
    return participant;
}

XmlNode* ParticipantProxy::getXmlForPassiveTripPoints()
{
    XmlNode* participant = XmlNode::createWrapperElement("participant");
    participant->addChild(XmlNode::createDataElement("index", 
        friendlyValue(m_index)));
    participant->addChild(XmlNode::createDataElement("name", 
        m_participantProperties.getParticipantProperties().getName()));
    participant->addChild(XmlNode::createDataElement("temperature", 
        getTemperatureForStatus(getDomain(0)).toString()));
    participant->addChild(getTemperatureThresholdsForStatus().getXml());
    participant->addChild(m_passiveTripPointProperty.getXml());
    return participant;
}

XmlNode* ParticipantProxy::getXmlForPassiveControlKnobs()
{
    XmlNode* participant = XmlNode::createWrapperElement("passive_control_knobs");
    for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
    {
        participant->addChild(domain->second.getXmlForPassiveControlKnobs());
    }
    return participant;
}

XmlNode* ParticipantProxy::getXmlForScpDscpSupport()
{
    XmlNode* participant = XmlNode::createWrapperElement("participant");
    participant->addChild(XmlNode::createDataElement("index", StlOverride::to_string(m_index)));
    std::string name = m_participantProperties.getParticipantProperties().getName();
    participant->addChild(XmlNode::createDataElement("name", name));
    participant->addChild(XmlNode::createDataElement("supports_scp", supportsScp() ? "true" : "false"));
    participant->addChild(XmlNode::createDataElement("supports_dscp", supportsDscp() ? "true" : "false"));
    return participant;
}

XmlNode* ParticipantProxy::getXmlForTripPointStatistics()
{
    XmlNode* stats = XmlNode::createWrapperElement("participant_trip_point_statistics");
    stats->addChild
        (XmlNode::createDataElement("participant_index", friendlyValue(m_index)));
    stats->addChild(
        XmlNode::createDataElement("participant_name", m_participantProperties.getParticipantProperties().getName()));
    Bool supportsTripPoints = getDomain(0)->getTemperatureControl()->supportsTemperatureControls();
    stats->addChild(
        XmlNode::createDataElement("supports_trip_points", friendlyValue(supportsTripPoints)));

    if (m_timeOfLastThresholdCrossed == 0)
    {
        stats->addChild(XmlNode::createDataElement("time_since_last_trip", "X"));
    }
    else
    {
        double timeSinceLastTrip(0.0);
        timeSinceLastTrip = (double)m_time->getCurrentTimeInMilliseconds() - (double)m_timeOfLastThresholdCrossed;
        timeSinceLastTrip = (double)timeSinceLastTrip / (double)1000;
        stats->addChild(XmlNode::createDataElement("time_since_last_trip", friendlyValue(timeSinceLastTrip)));
    }

    stats->addChild(
        XmlNode::createDataElement(
            "temperature_of_last_trip", m_lastThresholdCrossedTemperature.toString()));

    return stats;
}

XmlNode* ParticipantProxy::getXmlForConfigTdpLevel()
{
    XmlNode* status = XmlNode::createWrapperElement("participant_config_tdp_level");
    status->addChild(XmlNode::createDataElement(
        "participant_index", StatusFormat::friendlyValue(getIndex())));
    status->addChild(XmlNode::createDataElement(
        "participant_name", getParticipantProperties().getName()));
    for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
    {
        status->addChild(domain->second.getXmlForConfigTdpLevel());
    }
    return status;
}

Temperature ParticipantProxy::getTemperatureForStatus(DomainProxyInterface* domainProxy)
{
    if (domainProxy->getTemperatureControl()->supportsTemperatureControls())
    {
        try
        {
            return domainProxy->getTemperatureControl()->getCurrentTemperature();
        }
        catch (...)
        {
            return Temperature::createInvalid();
        }
    }
    else
    {
        return Temperature::createInvalid();
    }
}

TemperatureThresholds ParticipantProxy::getTemperatureThresholdsForStatus()
{
    try
    {
        return getTemperatureThresholds();
    }
    catch (...)
    {
        return TemperatureThresholds::createInvalid();
    }
}

void ParticipantProxy::refreshHysteresis()
{
    for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
    {
        if (domain->second.getTemperatureControl()->supportsTemperatureThresholds())
        {
            domain->second.getTemperatureControl()->refreshHysteresis();
        }
    }
}

void ParticipantProxy::refreshVirtualSensorTables()
{
    for (auto domain = m_domains.begin(); domain != m_domains.end(); domain++)
    {
        if (domain->second.getTemperatureControl()->supportsTemperatureControls())
        {
            domain->second.getTemperatureControl()->refreshVirtualSensorTables();
        }
    }
}

Bool ParticipantProxy::hasSetDscpSupported()
{
    return m_supportsDscpValid;
}

Bool ParticipantProxy::hasSetScpSupported()
{
    return m_supportsScpValid;
}

Bool ParticipantProxy::supportsDscp()
{
    return m_supportsDscp;
}

Bool ParticipantProxy::supportsScp()
{
    return m_supportsScp;
}

void ParticipantProxy::setDscpSupport(Bool dscpSupported)
{
    m_supportsDscp = dscpSupported;
    m_supportsDscpValid = true;
}

void ParticipantProxy::setScpSupport(Bool scpSupported)
{
    m_supportsScp = scpSupported;
    m_supportsScpValid = true;
}

void ParticipantProxy::setCoolingPolicy(const DptfBuffer& coolingPreference, CoolingPreferenceType::Type type)
{
    m_policyServices.participantSetSpecificInfo->setParticipantCoolingPolicy(m_index, coolingPreference, type);
}
