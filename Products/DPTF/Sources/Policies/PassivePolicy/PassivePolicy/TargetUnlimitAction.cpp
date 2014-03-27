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
        getPolicyServices().messageLogging->writeMessageDebug(
            PolicyMessage(FLF, "Attempting to unlimit target participant.", getTarget()));

        UInt64 time = getTime()->getCurrentTimeInMilliseconds();
        vector<UIntN> sourcesToUnlimit = chooseSourcesToUnlimitForTarget(getTarget());
        if (sourcesToUnlimit.size() > 0)
        {
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, constructMessageForSources("unlimit", getTarget(), sourcesToUnlimit)));

            for (auto source = sourcesToUnlimit.begin(); source != sourcesToUnlimit.end(); source++)
            {
                if (getCallbackScheduler()->isFreeForRequests(getTarget(), *source, time))
                {
                    vector<UIntN> domains = chooseDomainsToUnlimitForSource(getTarget(), *source);
                    getPolicyServices().messageLogging->writeMessageDebug(
                        PolicyMessage(FLF, constructMessageForSourceDomains("unlimit", getTarget(), *source, domains)));
                    for (auto domain = domains.begin(); domain != domains.end(); domain++)
                    {
                        requestUnlimit(*source, *domain, getTarget());
                    }
                    getCallbackScheduler()->markBusyForRequests(getTarget(), *source, time);
                }
                getCallbackScheduler()->ensureCallbackByNextSamplePeriod(getTarget(), *source, time);

                if (getCallbackScheduler()->isFreeForCommits(*source, time))
                {
                    commitUnlimit(*source, time);
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
                getCallbackScheduler()->ensureCallbackByShortestSamplePeriod(getTarget(), time);
            }
            else
            {
                getTargetMonitor().stopMonitoring(getTarget());
                removeAllRequestsForTarget(getTarget());
            }
        }
    }
    catch (...)
    {
        getPolicyServices().messageLogging->writeMessageWarning(
            PolicyMessage(FLF, "Failed to unlimit source(s) for target.", getTarget()));
    }
}

std::vector<UIntN> TargetUnlimitAction::chooseSourcesToUnlimitForTarget(UIntN target)
{
    // get TRT entries for target with sources that have controls that can be unlimited
    vector<UIntN> sourcesToLimit;
    vector<ThermalRelationshipTableEntry> availableSourcesForTarget = getTrt().getEntriesForTarget(target);
    availableSourcesForTarget = getEntriesWithControlsToUnlimit(target, availableSourcesForTarget);

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

std::vector<ThermalRelationshipTableEntry> TargetUnlimitAction::getEntriesWithControlsToUnlimit(
    UIntN target, const std::vector<ThermalRelationshipTableEntry>& sourcesForTarget)
{
    vector<ThermalRelationshipTableEntry> entriesThatCanBeUnlimited;
    for (auto entry = sourcesForTarget.begin(); entry != sourcesForTarget.end(); ++entry)
    {
        if (entry->sourceDeviceIndex() != Constants::Invalid)
        {
            // if source has controls that can be unlimited, add it to the list
            vector<UIntN> domainsWithControlKnobsToTurn =
                getDomainsWithControlKnobsToUnlimit(getParticipantTracker()[entry->sourceDeviceIndex()], target);
            if (domainsWithControlKnobsToTurn.size() > 0)
            {
                entriesThatCanBeUnlimited.push_back(*entry);
            }
        }
    }
    return entriesThatCanBeUnlimited;
}

std::vector<UIntN> TargetUnlimitAction::getDomainsWithControlKnobsToUnlimit(ParticipantProxy& participant, UIntN target)
{
    vector<UIntN> domainsWithControlKnobsToTurn;
    auto domainIndexes = participant.getDomainIndexes();
    for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
    {
        // if domain has controls that can be unlimited, add it to the list
        if (participant[*domainIndex].canUnlimit(target))
        {
            domainsWithControlKnobsToTurn.push_back(*domainIndex);
        }
    }
    return domainsWithControlKnobsToTurn;
}

std::vector<UIntN> TargetUnlimitAction::chooseDomainsToUnlimitForSource(UIntN target, UIntN source)
{
    vector<UIntN> domainsWithControlKnobsToTurn = getDomainsWithControlKnobsToUnlimit(getParticipantTracker()[source], target);
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

UIntN TargetUnlimitAction::getDomainWithLowestTemperature(
    UIntN source, std::vector<UIntN> domainsWithControlKnobsToTurn)
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

void TargetUnlimitAction::requestUnlimit(UIntN source, UIntN domain, UIntN target)
{
    getParticipantTracker()[source][domain].requestUnlimit(target);
}

void TargetUnlimitAction::commitUnlimit(UIntN source, UInt64 time)
{
    Bool madeChanges(false);
    vector<UIntN> domainIndexes = getParticipantTracker()[source].getDomainIndexes();
    for (auto domain = domainIndexes.begin(); domain != domainIndexes.end(); domain++)
    {
        try
        {
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, "Committing limits to source.", source, *domain));

            Bool madeChange = getParticipantTracker()[source][*domain].commitLimits();
            if (madeChange)
            {
                madeChanges = true;
            }
        }
        catch (std::exception& ex)
        {
            getPolicyServices().messageLogging->writeMessageWarning(
                PolicyMessage(FLF, "Failed to limit source: " + string(ex.what()), source, *domain));
        }
    }

    if (madeChanges)
    {
        getCallbackScheduler()->markSourceAsBusy(source, getTargetMonitor(), time);
    }
}

void TargetUnlimitAction::removeAllRequestsForTarget(UIntN target)
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
