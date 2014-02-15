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

#include "TargetActionBase.h"
#include <tuple>
using namespace std;

TargetActionBase::TargetActionBase(
    PolicyServicesInterfaceContainer& policyServices, std::shared_ptr<TimeInterface> time,
    ParticipantTracker& participantTracker, ThermalRelationshipTable& trt,
    std::shared_ptr<CallbackScheduler> callbackScheduler, TargetMonitor& targetMonitor, UIntN target)
    : m_policyServices(policyServices), m_time(time), m_participantTracker(participantTracker),
    m_trt(trt), m_callbackScheduler(callbackScheduler), m_targetMonitor(targetMonitor), m_target(target)
{
}

TargetActionBase::~TargetActionBase()
{
}

vector<UIntN> TargetActionBase::getPackageDomains(UIntN source, const vector<UIntN>& domainsWithControlKnobsToTurn)
{
    vector<UIntN> packageDomains;
    for (auto domain = domainsWithControlKnobsToTurn.begin(); domain != domainsWithControlKnobsToTurn.end(); domain++)
    {
        DomainProxy& domainProxy = getParticipantTracker()[source][*domain];
        DomainType::Type domainType = domainProxy.getDomainProperties().getDomainType();
        if (domainType == DomainType::MultiFunction)
        {
            packageDomains.push_back(*domain);
        }
    }
    return packageDomains;
}

std::vector<UIntN> TargetActionBase::filterDomainList(
    const std::vector<UIntN>& domainList, const std::vector<UIntN>& filterList) const
{
    vector<UIntN> filtered;
    for (auto domain = domainList.begin(); domain != domainList.end(); domain++)
    {
        Bool isInFilterList(false);
        for (auto filter = filterList.begin(); filter != filterList.end(); filter++)
        {
            if (*domain == *filter)
            {
                isInFilterList = true;
                break;
            }
        }

        if (isInFilterList == false)
        {
            filtered.push_back(*domain);
        }
    }

    return filtered;
}

std::vector<UIntN> TargetActionBase::getDomainsThatDoNotReportTemperature(UIntN source, std::vector<UIntN> domains)
{
    vector<UIntN> domainsWithNoTemperature;
    for (auto domain = domains.begin(); domain != domains.end(); domain++)
    {
        if (!getParticipantTracker()[source][*domain].getTemperatureProperty().implementsTemperatureInterface())
        {
            domainsWithNoTemperature.push_back(*domain);
        }
    }
    return domainsWithNoTemperature;
}

Bool TargetActionBase::compareTrtTableEntriesOnInfluence(
    const ThermalRelationshipTableEntry& left,
    const ThermalRelationshipTableEntry& right)
{
    return (left.thermalInfluence() > right.thermalInfluence());
}

Bool TargetActionBase::compareDomainsOnPriorityAndUtilization(
    const tuple<UIntN, DomainPriority, UtilizationStatus>& left,
    const tuple<UIntN, DomainPriority, UtilizationStatus>& right)
{
    if (get<1>(left) < get<1>(right))
    {
        return true;
    }
    else if (get<1>(left) == get<1>(right))
    {
        if (get<2>(left).getCurrentUtilization() >
            get<2>(right).getCurrentUtilization())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

std::string TargetActionBase::constructMessageForSources(
    std::string actionName, UIntN target, const std::vector<UIntN>& sources)
{
    stringstream message;
    if (sources.size() > 0)
    {
        message << "Choosing to " << actionName << " sources {";
        for (UIntN sourceIndex = 0; sourceIndex < sources.size(); sourceIndex++)
        {
            message << sources[sourceIndex];
            if (sourceIndex + 1 < sources.size())
            {
                message << ", ";
            }
        }
        message << "} for target " << target << ".";
    }
    else
    {
        message << "No sources to " << actionName << " for target " << target << ".";
    }
    return message.str();
}

std::string TargetActionBase::constructMessageForSourceDomains(
    std::string actionName, UIntN target, UIntN source, const std::vector<UIntN>& domains)
{
    stringstream message;
    if (domains.size() > 0)
    {
        message << "Choosing to " << actionName << " source " << source << " for target " << target << " on domains {";
        for (UIntN domainIndex = 0; domainIndex < domains.size(); domainIndex++)
        {
            message << domains[domainIndex];
            if (domainIndex + 1 < domains.size())
            {
                message << ", ";
            }
        }
        message << "}.";
    }
    else
    {
        message << "No domains to " << actionName << " for source " << source << " and target " << target << ".";
    }
    return message.str();
}


PolicyServicesInterfaceContainer TargetActionBase::getPolicyServices() const
{
    return m_policyServices;
}

std::shared_ptr<TimeInterface> TargetActionBase::getTime() const
{
    return m_time;
}

ParticipantTracker& TargetActionBase::getParticipantTracker() const
{
    return m_participantTracker;
}

ThermalRelationshipTable& TargetActionBase::getTrt() const
{
    return m_trt;
}

std::shared_ptr<CallbackScheduler> TargetActionBase::getCallbackScheduler() const
{
    return m_callbackScheduler;
}

TargetMonitor& TargetActionBase::getTargetMonitor() const
{
    return m_targetMonitor;
}

UIntN TargetActionBase::getTarget() const
{
    return m_target;
}
