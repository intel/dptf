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

#include "TargetLimitAction.h"
#include <algorithm>
using namespace std;

TargetLimitAction::TargetLimitAction(
    PolicyServicesInterfaceContainer& policyServices, std::shared_ptr<TimeInterface> time,
    ParticipantTracker& participantTracker, ThermalRelationshipTable& trt,
    std::shared_ptr<CallbackScheduler> callbackScheduler, TargetMonitor& targetMonitor, UIntN target)
    : TargetActionBase(policyServices, time, participantTracker, trt, callbackScheduler, targetMonitor, target)
{
}

TargetLimitAction::~TargetLimitAction()
{
}

void TargetLimitAction::execute()
{
    try
    {
        // make sure target is now being monitored
        getTargetMonitor().startMonitoring(getTarget());
        
        getPolicyServices().messageLogging->writeMessageDebug(
            PolicyMessage(FLF, "Attempting to limit target participant.", getTarget()));

        // choose sources to limit for target
        UInt64 time = getTime()->getCurrentTimeInMilliseconds();
        vector<UIntN> sourcesToLimit = chooseSourcesToLimitForTarget(getTarget());
        if (sourcesToLimit.size() > 0)
        {
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, constructMessageForSources("limit", getTarget(), sourcesToLimit)));

            for (auto source = sourcesToLimit.begin(); source != sourcesToLimit.end(); source++)
            {
                if (getCallbackScheduler()->isFreeForRequests(getTarget(), *source, time))
                {
                    vector<UIntN> domains = chooseDomainsToLimitForSource(getTarget(), *source);
                    getPolicyServices().messageLogging->writeMessageDebug(
                        PolicyMessage(FLF, constructMessageForSourceDomains("limit", getTarget(), *source, domains)));
                    for (auto domain = domains.begin(); domain != domains.end(); domain++)
                    {
                        requestLimit(*source, *domain, getTarget());
                    }
                    getCallbackScheduler()->markBusyForRequests(getTarget(), *source, time);
                }
                getCallbackScheduler()->ensureCallbackByNextSamplePeriod(getTarget(), *source, time);

                if (getCallbackScheduler()->isFreeForCommits(*source, time))
                {
                    commitLimit(*source, time);
                }
            }
        }
        else
        {
            // schedule a callback as soon as possible if there are no sources that can be limited
            getPolicyServices().messageLogging->writeMessageDebug(
                PolicyMessage(FLF, "No sources to limit for target.", getTarget()));
            getCallbackScheduler()->ensureCallbackByShortestSamplePeriod(getTarget(), time);
        }
    }
    catch (...)
    {
        getPolicyServices().messageLogging->writeMessageWarning(
            PolicyMessage(FLF, "Failed to limit source(s) for target.", getTarget()));
    }
}

std::vector<UIntN> TargetLimitAction::chooseSourcesToLimitForTarget(UIntN target)
{
    // choose sources that are tied for the highest influence in the TRT
    vector<UIntN> sourcesToLimit;
    vector<ThermalRelationshipTableEntry> availableSourcesForTarget = getTrt().getEntriesForTarget(target);
    availableSourcesForTarget = getEntriesWithControlsToLimit(target, availableSourcesForTarget);
    if (availableSourcesForTarget.size() > 0)
    {
        sort(availableSourcesForTarget.begin(), availableSourcesForTarget.end(), compareTrtTableEntriesOnInfluence);
        for (auto entry = availableSourcesForTarget.begin(); entry != availableSourcesForTarget.end(); entry++)
        {
            if (entry->thermalInfluence() == availableSourcesForTarget.front().thermalInfluence())
            {
                sourcesToLimit.push_back(entry->sourceDeviceIndex());
            }
        }
    }
    return sourcesToLimit;
}

std::vector<ThermalRelationshipTableEntry> TargetLimitAction::getEntriesWithControlsToLimit(
    UIntN target, const std::vector<ThermalRelationshipTableEntry>& sourcesForTarget)
{
    // choose TRT entries whose source has domains that can be limited
    std::vector<ThermalRelationshipTableEntry> entriesThatCanBeLimited;
    for (auto entry = sourcesForTarget.begin(); entry != sourcesForTarget.end(); ++entry)
    {
        if (entry->sourceDeviceIndex() != Constants::Invalid)
        {
            vector<UIntN> domainsWithControlKnobsToTurn =
                getDomainsWithControlKnobsToLimit(getParticipantTracker()[entry->sourceDeviceIndex()], target);
            if (domainsWithControlKnobsToTurn.size() > 0)
            {
                entriesThatCanBeLimited.push_back(*entry);
            }
        }
    }
    return entriesThatCanBeLimited;
}

std::vector<UIntN> TargetLimitAction::getDomainsWithControlKnobsToLimit(ParticipantProxy& participant, UIntN target)
{
    // choose domains in the participant that have controls that can be limited
    vector<UIntN> domainsWithControlKnobsToTurn;
    auto domainIndexes = participant.getDomainIndexes();
    for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
    {
        if (participant[*domainIndex].canLimit(target))
        {
            domainsWithControlKnobsToTurn.push_back(*domainIndex);
        }
    }
    return domainsWithControlKnobsToTurn;
}

std::vector<UIntN> TargetLimitAction::chooseDomainsToLimitForSource(UIntN target, UIntN source)
{
    // select domains that can be limited for the source
    vector<UIntN> domainsWithControlKnobsToTurn = 
        getDomainsWithControlKnobsToLimit(getParticipantTracker()[source], target);
    set<UIntN> domainsToLimitSet;
    if (domainsWithControlKnobsToTurn.size() > 0)
    {
        if (source == target)
        {
            // if selected domain list contains package domains, choose to limit those first.  
            // otherwise, choose domains that do not report temperature as well as the domain with the highest 
            // temperature.
            vector<UIntN> domainsThatDoNotReportTemperature =
                getDomainsThatDoNotReportTemperature(source, domainsWithControlKnobsToTurn);
            domainsToLimitSet.insert(
                domainsThatDoNotReportTemperature.begin(), domainsThatDoNotReportTemperature.end());
            vector<UIntN> packageDomains = getPackageDomains(source, domainsWithControlKnobsToTurn);
            domainsToLimitSet.insert(packageDomains.begin(), packageDomains.end());
            if (packageDomains.size() == 0)
            {
                UIntN domainWithHighestTemperature = getDomainWithHighestTemperature(
                    source, domainsWithControlKnobsToTurn);
                if (domainWithHighestTemperature != Constants::Invalid)
                {
                    domainsToLimitSet.insert(domainWithHighestTemperature);
                }
            }
        }
        else
        {
            // Limit package domains first.  If there are no package domains that have controls to limit, 
            // choose domains that do not report utilization and the domain whose priority and 
            // utilization is highest.
            vector<UIntN> packageDomains = getPackageDomains(source, domainsWithControlKnobsToTurn);
            domainsToLimitSet.insert(packageDomains.begin(), packageDomains.end());
            if (packageDomains.size() == 0)
            {
                vector<pair<UIntN, UtilizationStatus>> domainsSortedByPreference =
                    getDomainsSortedByPriorityThenUtilization(source, domainsWithControlKnobsToTurn);
                for (auto domain = domainsSortedByPreference.begin(); 
                    domain != domainsSortedByPreference.end(); 
                    domain++)
                {
                    if (domain->second.getCurrentUtilization().isValid() == false)
                    {
                        domainsToLimitSet.insert(domain->first);
                    }
                    else
                    {
                        domainsToLimitSet.insert(domain->first);
                        break;
                    }
                }
            }
        }
    }

    vector<UIntN> domainsToLimit(domainsToLimitSet.begin(), domainsToLimitSet.end());
    return domainsToLimit;
}

UIntN TargetLimitAction::getDomainWithHighestTemperature(UIntN source, 
    const std::vector<UIntN>& domainsWithControlKnobsToTurn)
{
    pair<Temperature, UIntN> domainWithHighestTemperature(0, Constants::Invalid);
    for (auto domain = domainsWithControlKnobsToTurn.begin();
        domain != domainsWithControlKnobsToTurn.end();
        domain++)
    {
        try
        {
            Temperature domainTemperature =
                getParticipantTracker()[source][*domain].getTemperatureProperty().getCurrentTemperature();
            if (domainTemperature > domainWithHighestTemperature.first)
            {
                domainWithHighestTemperature.first = domainTemperature;
                domainWithHighestTemperature.second = *domain;
            }
        }
        catch (...)
        {
            // ignore domain if get temperature fails
        }
    }
    return domainWithHighestTemperature.second;
}

std::vector<std::pair<UIntN, UtilizationStatus>> TargetLimitAction::getDomainsSortedByPriorityThenUtilization(
    UIntN source, std::vector<UIntN> domains)
{
    // sort all domains by priority, then by utilization
    vector<tuple<UIntN, DomainPriority, UtilizationStatus>> domainPreference;
    for (auto domain = domains.begin(); domain != domains.end(); domain++)
    {
        DomainPriority domainPriority =
            getParticipantTracker()[source][*domain].getDomainPriorityProperty().getDomainPriority();
        UtilizationStatus utilStatus = UtilizationStatus(Percentage::createInvalid());
        if (domainReportsUtilization(source, *domain))
        {
            try
            {
                utilStatus = getParticipantTracker()[source][*domain].getUtilizationStatus();
            }
            catch (...)
            {
                // best effort to get utilization.  assume invalid utilization if failure.
            }
        }
        domainPreference.push_back(
            tuple<UIntN, DomainPriority, UtilizationStatus>(*domain, domainPriority, utilStatus));
    }
    sort(domainPreference.begin(), domainPreference.end(), compareDomainsOnPriorityAndUtilization);

    // return list of domains with just utilization information
    vector<pair<UIntN, UtilizationStatus>> domainsSortedByPriority;
    for (auto priorityTuple = domainPreference.begin(); priorityTuple != domainPreference.end(); priorityTuple++)
    {
        domainsSortedByPriority.push_back(
            pair<UIntN, UtilizationStatus>(get<0>(*priorityTuple), get<2>(*priorityTuple)));
    }
    return domainsSortedByPriority;
}

Bool TargetLimitAction::domainReportsUtilization(UIntN source, UIntN domain)
{
    return getParticipantTracker()[source][domain].getDomainProperties().implementsUtilizationInterface();
}

void TargetLimitAction::requestLimit(UIntN source, UIntN domain, UIntN target)
{
    getParticipantTracker()[source][domain].requestLimit(target);
}

void TargetLimitAction::commitLimit(UIntN source, UInt64 time)
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