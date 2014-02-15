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

#include "TargetUnlimitAction.h"
#include <algorithm>
using namespace std;

TargetUnlimitAction::TargetUnlimitAction(
    PolicyServicesInterfaceContainer& policyServices, std::shared_ptr<TimeInterface> time,
    ParticipantTracker& participantTracker, ThermalRelationshipTable& trt,
    std::shared_ptr<CallbackScheduler> callbackScheduler, TargetMonitor& targetMonitor, UIntN target)
    : TargetActionBase(policyServices, time, participantTracker, trt, callbackScheduler, targetMonitor, target)
{
}

TargetUnlimitAction::~TargetUnlimitAction()
{
}

void TargetUnlimitAction::execute()
{
    try
    {
        // choose sources to unlimit for target
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, "Attempting to unlimit target participant.", getTarget()));
        vector<UIntN> sourcesToUnlimit = chooseSourcesToUnlimitForTarget(getTarget());
        getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, constructMessageForSources("unlimit", getTarget(), sourcesToUnlimit)));

        if (sourcesToUnlimit.size() > 0)
        {
            for (auto source = sourcesToUnlimit.begin(); source != sourcesToUnlimit.end(); source++)
            {
                // if source is busy right now, schedule a callback as soon as possible
                if (getCallbackScheduler()->isSourceBusyNow(*source))
                {
                    getCallbackScheduler()->scheduleCallbackAsSoonAsPossible(getTarget(), *source);
                }
                else
                {
                    // unlimit appropriate domains for source and schedule a callback after the next sampling period
                    vector<UIntN> domainsToUnlimit = chooseDomainsToUnlimitForSource(getTarget(), *source);
                    getPolicyServices().messageLogging->writeMessageDebug(PolicyMessage(FLF, constructMessageForSourceDomains("unlimit", getTarget(), *source, domainsToUnlimit)));
                    for (auto domain = domainsToUnlimit.begin(); domain != domainsToUnlimit.end(); domain++)
                    {
                        unlimitDomain(*source, *domain);
                    }
                    getCallbackScheduler()->scheduleCallbackAfterNextSamplingPeriod(getTarget(), *source);
                }
            }
        }
        else
        {
            // get temperature, hysteresis, and psv for target
            ParticipantProxy& participant = getParticipantTracker()[getTarget()];
            auto hysteresis = participant.getTemperatureThresholds().getHysteresis();
            auto currentTemperature =
                participant[0].getTemperatureProperty().getCurrentTemperature();
            auto passiveTripPoints = participant.getPassiveTripPointProperty().getTripPoints();
            auto psv = passiveTripPoints.getItem(ParticipantSpecificInfoKey::PSV);

            // if temperature is between psv and (psv - hysteresis) then schedule another callback, otherwise stop
            // monitoring the target as there is nothing else to unlimit.
            if ((currentTemperature < Temperature(psv)) && (currentTemperature >= Temperature(psv - hysteresis)))
            {
                getCallbackScheduler()->scheduleCallbackAfterShortestSamplePeriod(getTarget());
            }
            else
            {
                getTargetMonitor().stopMonitoring(getTarget());
            }
        }
    }
    catch (...)
    {
        getPolicyServices().messageLogging->writeMessageWarning(PolicyMessage(FLF, "Failed to unlimit source(s) for target.", getTarget()));
    }
}

std::vector<UIntN> TargetUnlimitAction::chooseSourcesToUnlimitForTarget(UIntN target)
{
    // get TRT entries for target with sources that have controls that can be unlimited
    vector<UIntN> sourcesToLimit;
    vector<ThermalRelationshipTableEntry> availableSourcesForTarget = getTrt().getEntriesForTarget(target);
    availableSourcesForTarget = getEntriesWithControlsToUnlimit(availableSourcesForTarget);

    if (availableSourcesForTarget.size() > 0)
    {
        // choose all sources that are tied for the lowest influence value in the TRT for the target
        sort(availableSourcesForTarget.begin(), availableSourcesForTarget.end(), compareTrtTableEntriesOnInfluence);
        for (auto entry = availableSourcesForTarget.begin(); entry != availableSourcesForTarget.end(); entry++)
        {
            if (entry->thermalInfluence() == availableSourcesForTarget.back().thermalInfluence())
            {
                sourcesToLimit.push_back(entry->sourceDeviceIndex());
            }
        }
    }
    return sourcesToLimit;
}

std::vector<ThermalRelationshipTableEntry> TargetUnlimitAction::getEntriesWithControlsToUnlimit(const std::vector<ThermalRelationshipTableEntry>& sourcesForTarget)
{
    vector<ThermalRelationshipTableEntry> entriesThatCanBeUnlimited;
    for (auto entry = sourcesForTarget.begin(); entry != sourcesForTarget.end(); ++entry)
    {
        if (entry->sourceDeviceIndex() != Constants::Invalid)
        {
            // if source has controls that can be unlimited, add it to the list
            vector<UIntN> domainsWithControlKnobsToTurn =
                getDomainsWithControlKnobsToUnlimit(getParticipantTracker()[entry->sourceDeviceIndex()]);
            if (domainsWithControlKnobsToTurn.size() > 0)
            {
                entriesThatCanBeUnlimited.push_back(*entry);
            }
        }
    }
    return entriesThatCanBeUnlimited;
}

std::vector<UIntN> TargetUnlimitAction::getDomainsWithControlKnobsToUnlimit(ParticipantProxy& participant)
{
    vector<UIntN> domainsWithControlKnobsToTurn;
    auto domainIndexes = participant.getDomainIndexes();
    for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
    {
        // if domain has controls that can be unlimited, add it to the list
        if (participant[*domainIndex].canUnlimit())
        {
            domainsWithControlKnobsToTurn.push_back(*domainIndex);
        }
    }
    return domainsWithControlKnobsToTurn;
}

std::vector<UIntN> TargetUnlimitAction::chooseDomainsToUnlimitForSource(UIntN target, UIntN source)
{
    vector<UIntN> domainsWithControlKnobsToTurn = getDomainsWithControlKnobsToUnlimit(getParticipantTracker()[source]);
    set<UIntN> domainsToUnlimitSet;

    if (domainsWithControlKnobsToTurn.size() > 0)
    {
        // filter out package domains from the domains to consider for unlimiting
        vector<UIntN> packageDomains = getPackageDomains(source, domainsWithControlKnobsToTurn);
        vector<UIntN> filteredDomainList = filterDomainList(domainsWithControlKnobsToTurn, packageDomains);

        if (source == target)
        {
            // add non-package domains that don't report temperature and the domain with the lowest temperature
            vector<UIntN> domainsThatDoNotReportTemperature =
                getDomainsThatDoNotReportTemperature(source, filteredDomainList);
            domainsToUnlimitSet.insert(
                domainsThatDoNotReportTemperature.begin(), domainsThatDoNotReportTemperature.end());
            UIntN domainWithLowestTemperature = getDomainWithLowestTemperature(source, filteredDomainList);
            if (domainWithLowestTemperature != Constants::Invalid)
            {
                domainsToUnlimitSet.insert(domainWithLowestTemperature);
            }

            // if no domains were added, add package domains if there are any.
            if (domainsToUnlimitSet.size() == 0)
            {
                domainsToUnlimitSet.insert(packageDomains.begin(), packageDomains.end());
            }
        }
        else
        {
            // add non-package domains that have lowest priority
            vector<UIntN> domainsOfLowestPriority = getDomainsWithLowestPriority(source, filteredDomainList);
            domainsToUnlimitSet.insert(domainsOfLowestPriority.begin(), domainsOfLowestPriority.end());

            // if no domains were added, add package domains if there are any.
            if (domainsToUnlimitSet.size() == 0)
            {
                domainsToUnlimitSet.insert(packageDomains.begin(), packageDomains.end());
            }
        }
    }

    vector<UIntN> domainsToUnlimit(domainsToUnlimitSet.begin(), domainsToUnlimitSet.end());
    return domainsToUnlimit;
}

UIntN TargetUnlimitAction::getDomainWithLowestTemperature(UIntN source, std::vector<UIntN> domainsWithControlKnobsToTurn)
{
    pair<Temperature, UIntN> domainWithLowestTemperature(Temperature::createInvalid(), Constants::Invalid);
    for (auto domain = domainsWithControlKnobsToTurn.begin();
        domain != domainsWithControlKnobsToTurn.end();
        domain++)
    {
        try
        {
            Temperature domainTemperature =
                getParticipantTracker()[source][*domain].getTemperatureProperty().getCurrentTemperature();
            if (domainWithLowestTemperature.first.isValid())
            {
                if (domainTemperature < domainWithLowestTemperature.first)
                {
                    domainWithLowestTemperature.first = domainTemperature;
                    domainWithLowestTemperature.second = *domain;
                }
            }
            else
            {
                domainWithLowestTemperature.first = domainTemperature;
                domainWithLowestTemperature.second = *domain;
            }
        }
        catch (...)
        {
            // if get temperature fails, ignore for choosing the domain with the lowest temperature
        }
    }
    return domainWithLowestTemperature.second;
}

std::vector<UIntN> TargetUnlimitAction::getDomainsWithLowestPriority(UIntN source, std::vector<UIntN> domains)
{
    DomainPriority lowestPriority(Constants::Invalid);
    for (auto domain = domains.begin();
        domain != domains.end();
        domain++)
    {
        try
        {
            DomainPriority priority =
                getParticipantTracker()[source][*domain].getDomainPriorityProperty().getDomainPriority();
            if (priority < lowestPriority)
            {
                lowestPriority = priority;
            }
        }
        catch (...)
        {
            // ignore any failures for retrieving priority from a domain
        }
    }

    vector<UIntN> domainsWithLowestPriority;
    for (auto domain = domains.begin();
        domain != domains.end();
        domain++)
    {
        DomainPriority priority =
            getParticipantTracker()[source][*domain].getDomainPriorityProperty().getDomainPriority();
        if (priority == lowestPriority)
        {
            domainsWithLowestPriority.push_back(*domain);
        }
    }
    return domainsWithLowestPriority;
}

void TargetUnlimitAction::unlimitDomain(UIntN source, UIntN domain)
{
    getParticipantTracker()[source][domain].unlimit();
}
