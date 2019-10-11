/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#include "PassiveDomainProxy.h"
#include "PolicyLogger.h"

using namespace std;

TargetUnlimitAction::TargetUnlimitAction(
	PolicyServicesInterfaceContainer& policyServices,
	std::shared_ptr<TimeInterface> time,
	std::shared_ptr<ParticipantTrackerInterface> participantTracker,
	std::shared_ptr<ThermalRelationshipTable> trt,
	std::shared_ptr<CallbackScheduler> callbackScheduler,
	TargetMonitor& targetMonitor,
	UIntN target)
	: TargetActionBase(policyServices, time, participantTracker, trt, callbackScheduler, targetMonitor, target)
{
}

TargetUnlimitAction::~TargetUnlimitAction()
{
}

void TargetUnlimitAction::execute()
{
	auto targetIndex = getTarget();
	try
	{
		if (getParticipantTracker()->remembers(targetIndex))
		{
			// choose sources to unlimit for target
			// TODO: want to pass in participant index
			POLICY_LOG_MESSAGE_DEBUG({
				std::stringstream message;
				message << "Attempting to unlimit target participant."
						<< " ParticipantIndex = " << targetIndex;
				return message.str();
			});

			auto time = getTime()->getCurrentTime();
			vector<UIntN> sourcesToUnlimit = chooseSourcesToUnlimitForTarget(targetIndex);
			if (sourcesToUnlimit.size() > 0)
			{
				POLICY_LOG_MESSAGE_DEBUG(
					{ return constructMessageForSources("unlimit", targetIndex, sourcesToUnlimit); });

				for (auto source = sourcesToUnlimit.begin(); source != sourcesToUnlimit.end(); source++)
				{
					if (getParticipantTracker()->remembers(*source))
					{
						if (getCallbackScheduler()->isFreeForRequests(targetIndex, *source, time))
						{
							vector<UIntN> domains = chooseDomainsToUnlimitForSource(targetIndex, *source);
							POLICY_LOG_MESSAGE_DEBUG(
								{ return constructMessageForSourceDomains("unlimit", targetIndex, *source, domains); });

							for (auto domain = domains.begin(); domain != domains.end(); domain++)
							{
								requestUnlimit(*source, *domain, targetIndex);
							}
							getCallbackScheduler()->markBusyForRequests(targetIndex, *source, time);
						}
						getCallbackScheduler()->ensureCallbackByNextSamplePeriod(targetIndex, *source, time);

						if (getCallbackScheduler()->isFreeForCommits(*source, time))
						{
							commitUnlimit(*source, time);
						}
					}
				}
			}
			else
			{
				// get temperature, hysteresis, and psv for target
				ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(targetIndex);
				auto hysteresis = participant->getTemperatureThresholds().getHysteresis();
				auto currentTemperature = participant->getFirstDomainTemperature();
				auto passiveTripPoints = participant->getPassiveTripPointProperty().getTripPoints();
				auto psv = passiveTripPoints.getTemperature(ParticipantSpecificInfoKey::PSV);

				// if temperature is between psv and (psv - hysteresis) then schedule another callback, otherwise stop
				// monitoring the target as there is nothing else to unlimit.
				if ((currentTemperature < Temperature(psv)) && (currentTemperature >= Temperature(psv - hysteresis)))
				{
					getCallbackScheduler()->ensureCallbackByShortestSamplePeriod(targetIndex, time);
				}
				else
				{
					getTargetMonitor().stopMonitoring(targetIndex);
					removeAllRequestsForTarget(targetIndex);
				}
			}
		}
	}
	catch (...)
	{
		// TODO: want to pass in participant index
		POLICY_LOG_MESSAGE_WARNING({
			std::stringstream message;
			message << "Failed to unlimit source(s) for target."
					<< " ParticipantIndex = " << targetIndex;
			return message.str();
		});
	}
}

std::vector<UIntN> TargetUnlimitAction::chooseSourcesToUnlimitForTarget(UIntN target)
{
	// get TRT entries for target with sources that have controls that can be unlimited
	vector<UIntN> sourcesToLimit;
	auto availableSourcesForTarget = getTrt()->getEntriesForTarget(target);
	availableSourcesForTarget = getEntriesWithControlsToUnlimit(target, availableSourcesForTarget);

	if (availableSourcesForTarget.size() > 0)
	{
		// choose all sources that are tied for the lowest influence value in the TRT for the target
		sort(availableSourcesForTarget.begin(), availableSourcesForTarget.end(), compareTrtTableEntriesOnInfluence);
		for (auto entry = availableSourcesForTarget.begin(); entry != availableSourcesForTarget.end(); entry++)
		{
			if ((*entry)->thermalInfluence() == availableSourcesForTarget.back()->thermalInfluence())
			{
				sourcesToLimit.push_back((*entry)->getSourceDeviceIndex());
			}
		}
	}
	return sourcesToLimit;
}

std::vector<std::shared_ptr<ThermalRelationshipTableEntry>> TargetUnlimitAction::getEntriesWithControlsToUnlimit(
	UIntN target,
	const std::vector<shared_ptr<ThermalRelationshipTableEntry>>& sourcesForTarget)
{
	vector<std::shared_ptr<ThermalRelationshipTableEntry>> entriesThatCanBeUnlimited;
	for (auto entry = sourcesForTarget.begin(); entry != sourcesForTarget.end(); ++entry)
	{
		auto sourceIndex = (*entry)->getSourceDeviceIndex();
		if (sourceIndex != Constants::Invalid && getParticipantTracker()->remembers(sourceIndex))
		{
			// if source has controls that can be unlimited, add it to the list
			vector<UIntN> domainsWithControlKnobsToTurn =
				getDomainsWithControlKnobsToUnlimit(getParticipantTracker()->getParticipant(sourceIndex), target);
			if (domainsWithControlKnobsToTurn.size() > 0)
			{
				entriesThatCanBeUnlimited.push_back(*entry);
			}
		}
	}
	return entriesThatCanBeUnlimited;
}

std::vector<UIntN> TargetUnlimitAction::getDomainsWithControlKnobsToUnlimit(
	ParticipantProxyInterface* participant,
	UIntN target)
{
	vector<UIntN> domainsWithControlKnobsToTurn;
	auto domainIndexes = participant->getDomainIndexes();
	for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
	{
		// if domain has controls that can be unlimited, add it to the list
		auto domain = std::dynamic_pointer_cast<PassiveDomainProxy>(participant->getDomain(*domainIndex));
		if (domain->canUnlimit(target))
		{
			domainsWithControlKnobsToTurn.push_back(*domainIndex);
		}
	}
	return domainsWithControlKnobsToTurn;
}

std::vector<UIntN> TargetUnlimitAction::chooseDomainsToUnlimitForSource(UIntN target, UIntN source)
{
	set<UIntN> domainsToUnlimitSet;
	vector<UIntN> domainsWithControlKnobsToTurn =
		getDomainsWithControlKnobsToUnlimit(getParticipantTracker()->getParticipant(source), target);

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
	UIntN source,
	std::vector<UIntN> domainsWithControlKnobsToTurn)
{
	pair<Temperature, UIntN> domainWithLowestTemperature(Temperature::createInvalid(), Constants::Invalid);
	auto sourceParticipant = getParticipantTracker()->getParticipant(source);
	for (auto domain = domainsWithControlKnobsToTurn.begin(); domain != domainsWithControlKnobsToTurn.end(); domain++)
	{
		try
		{
			auto sourceDomain = sourceParticipant->getDomain(*domain);
			Temperature domainTemperature = sourceDomain->getTemperatureControl()->getCurrentTemperature();
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
	vector<UIntN> domainsWithLowestPriority;
	DomainPriority lowestPriority(Constants::Invalid);
	auto sourceParticipant = getParticipantTracker()->getParticipant(source);
	for (auto domain = domains.begin(); domain != domains.end(); domain++)
	{
		try
		{
			auto sourceDomain = sourceParticipant->getDomain(*domain);
			DomainPriority priority = sourceDomain->getDomainPriorityProperty().getDomainPriority();
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

	for (auto domain = domains.begin(); domain != domains.end(); domain++)
	{
		auto sourceDomain = sourceParticipant->getDomain(*domain);
		DomainPriority priority = sourceDomain->getDomainPriorityProperty().getDomainPriority();
		if (priority == lowestPriority)
		{
			domainsWithLowestPriority.push_back(*domain);
		}
	}

	return domainsWithLowestPriority;
}

void TargetUnlimitAction::requestUnlimit(UIntN source, UIntN domain, UIntN target)
{
	auto sourceParticipant = getParticipantTracker()->getParticipant(source);
	auto sourceDomain = std::dynamic_pointer_cast<PassiveDomainProxy>(sourceParticipant->getDomain(domain));
	sourceDomain->requestUnlimit(target);
}

void TargetUnlimitAction::commitUnlimit(UIntN source, const TimeSpan& time)
{
	Bool madeChanges(false);
	auto sourceParticipant = getParticipantTracker()->getParticipant(source);
	vector<UIntN> domainIndexes = sourceParticipant->getDomainIndexes();
	for (auto domain = domainIndexes.begin(); domain != domainIndexes.end(); domain++)
	{
		try
		{
			// TODO: want to pass in participant index and domain index instead
			POLICY_LOG_MESSAGE_DEBUG({
				std::stringstream message;
				message << "Committing limits to source."
						<< " Source=" + std::to_string(source) << ". Domain = " + std::to_string(*domain);
				return message.str();
			});

			auto sourceDomain = std::dynamic_pointer_cast<PassiveDomainProxy>(sourceParticipant->getDomain(*domain));
			Bool madeChange = sourceDomain->commitLimits();
			if (madeChange)
			{
				madeChanges = true;
			}
		}
		catch (std::exception& ex)
		{
			// TODO: want to pass in participant index and domain index instead
			POLICY_LOG_MESSAGE_WARNING_EX({
				std::stringstream message;
				message << "Failed to limit source: " << string(ex.what()) << ". Source=" + std::to_string(source)
						<< ". Domain = " + std::to_string(*domain);
				return message.str();
			});
		}
	}

	if (madeChanges)
	{
		getCallbackScheduler()->markSourceAsBusy(source, getTargetMonitor(), time);
	}
}

void TargetUnlimitAction::removeAllRequestsForTarget(UIntN target)
{
	vector<UIntN> participantIndexes = getParticipantTracker()->getAllTrackedIndexes();
	for (auto participantIndex = participantIndexes.begin(); participantIndex != participantIndexes.end();
		 participantIndex++)
	{
		ParticipantProxyInterface* participant = getParticipantTracker()->getParticipant(*participantIndex);
		vector<UIntN> domainIndexes = participant->getDomainIndexes();
		for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
		{
			auto domain = std::dynamic_pointer_cast<PassiveDomainProxy>(participant->getDomain(*domainIndex));
			domain->clearAllRequestsForTarget(target);
		}
	}
}
