/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
#include "PassiveDomainProxy.h"
#include "PolicyLogger.h"

using namespace std;

TargetLimitAction::TargetLimitAction(
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

TargetLimitAction::~TargetLimitAction()
{
}

void TargetLimitAction::execute()
{
	auto target = getTarget();
	try
	{
		// make sure target is now being monitored
		getTargetMonitor().startMonitoring(target);

		POLICY_LOG_MESSAGE_DEBUG({
			// TODO: want to pass in participant index
			std::stringstream message;
			message << "Attempting to limit target participant."
					<< " ParticipantIndex = " << target;
			return message.str();
		});

		// choose sources to limit for target
		auto time = getTime()->getCurrentTime();
		vector<UIntN> sourcesToLimit = chooseSourcesToLimitForTarget(target);
		if (sourcesToLimit.size() > 0)
		{
			POLICY_LOG_MESSAGE_DEBUG({ return constructMessageForSources("limit", target, sourcesToLimit); });

			for (auto source = sourcesToLimit.begin(); source != sourcesToLimit.end(); source++)
			{
				if (getParticipantTracker()->remembers(*source))
				{
					if (getCallbackScheduler()->isFreeForRequests(target, *source, time))
					{
						vector<UIntN> domains = chooseDomainsToLimitForSource(target, *source);
						POLICY_LOG_MESSAGE_DEBUG(
							{ return constructMessageForSourceDomains("limit", target, *source, domains); });
						for (auto domain = domains.begin(); domain != domains.end(); domain++)
						{
							requestLimit(*source, *domain, target);
						}
						getCallbackScheduler()->markBusyForRequests(target, *source, time);
					}
					getCallbackScheduler()->ensureCallbackByNextSamplePeriod(target, *source, time);

					if (getCallbackScheduler()->isFreeForCommits(*source, time))
					{
						commitLimit(*source, time);
					}
				}
			}
		}
		else
		{
			// schedule a callback as soon as possible if there are no sources that can be limited
			// TODO: want to pass in participant index
			POLICY_LOG_MESSAGE_DEBUG({
				std::stringstream message;
				message << "No sources to limit for target."
						<< " ParticipantIndex = " << target;
				return message.str();
			});
			getCallbackScheduler()->ensureCallbackByShortestSamplePeriod(target, time);
		}
	}
	catch (...)
	{
		// TODO: want to pass in participant index
		POLICY_LOG_MESSAGE_WARNING({
			std::stringstream message;
			message << "Failed to limit source(s) for target."
					<< " ParticipantIndex = " << target;
			return message.str();
		});
	}
}

std::vector<UIntN> TargetLimitAction::chooseSourcesToLimitForTarget(UIntN target)
{
	// choose sources that are tied for the highest influence in the TRT
	vector<UIntN> sourcesToLimit;
	auto availableSourcesForTarget = getTrt()->getEntriesForTarget(target);
	availableSourcesForTarget = getEntriesWithControlsToLimit(target, availableSourcesForTarget);
	if (availableSourcesForTarget.size() > 0)
	{
		sort(availableSourcesForTarget.begin(), availableSourcesForTarget.end(), compareTrtTableEntriesOnInfluence);
		for (auto entry = availableSourcesForTarget.begin(); entry != availableSourcesForTarget.end(); entry++)
		{
			if ((*entry)->thermalInfluence() == availableSourcesForTarget.front()->thermalInfluence())
			{
				sourcesToLimit.push_back((*entry)->getSourceDeviceIndex());
			}
		}
	}
	return sourcesToLimit;
}

std::vector<std::shared_ptr<ThermalRelationshipTableEntry>> TargetLimitAction::getEntriesWithControlsToLimit(
	UIntN target,
	const std::vector<std::shared_ptr<ThermalRelationshipTableEntry>>& sourcesForTarget)
{
	// choose TRT entries whose source has domains that can be limited
	std::vector<std::shared_ptr<ThermalRelationshipTableEntry>> entriesThatCanBeLimited;
	for (auto entry = sourcesForTarget.begin(); entry != sourcesForTarget.end(); ++entry)
	{
		auto sourceIndex = (*entry)->getSourceDeviceIndex();
		if (sourceIndex != Constants::Invalid && getParticipantTracker()->remembers(sourceIndex))
		{
			vector<UIntN> domainsWithControlKnobsToTurn =
				getDomainsWithControlKnobsToLimit(getParticipantTracker()->getParticipant(sourceIndex), target);
			if (domainsWithControlKnobsToTurn.size() > 0)
			{
				entriesThatCanBeLimited.push_back(*entry);
			}
		}
	}
	return entriesThatCanBeLimited;
}

std::vector<UIntN> TargetLimitAction::getDomainsWithControlKnobsToLimit(
	ParticipantProxyInterface* participant,
	UIntN target)
{
	// choose domains in the participant that have controls that can be limited
	vector<UIntN> domainsWithControlKnobsToTurn;
	auto domainIndexes = participant->getDomainIndexes();
	for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
	{
		auto domain = std::dynamic_pointer_cast<PassiveDomainProxy>(participant->getDomain(*domainIndex));
		if (domain->canLimit(target))
		{
			domainsWithControlKnobsToTurn.push_back(*domainIndex);
		}
	}
	return domainsWithControlKnobsToTurn;
}

std::vector<UIntN> TargetLimitAction::chooseDomainsToLimitForSource(UIntN target, UIntN source)
{
	set<UIntN> domainsToLimitSet;

	// select domains that can be limited for the source
	vector<UIntN> domainsWithControlKnobsToTurn =
		getDomainsWithControlKnobsToLimit(getParticipantTracker()->getParticipant(source), target);
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
				UIntN domainWithHighestTemperature =
					getDomainWithHighestTemperature(source, domainsWithControlKnobsToTurn);
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
				Bool isDomainChosenWithHighPriorityAndUtilization = false;
				for (auto domain = domainsSortedByPreference.begin(); domain != domainsSortedByPreference.end();
					 domain++)
				{
					if (domain->second.getCurrentUtilization().isValid() == false)
					{
						// Need to limit all domains that do not support Utilization capability
						domainsToLimitSet.insert(domain->first);
					}
					else if (isDomainChosenWithHighPriorityAndUtilization == false)
					{
						// Need to limit only the Domain with highest Priority and Utilization
						isDomainChosenWithHighPriorityAndUtilization = true;
						domainsToLimitSet.insert(domain->first);
					}
				}
			}
		}
	}

	vector<UIntN> domainsToLimit(domainsToLimitSet.begin(), domainsToLimitSet.end());
	return domainsToLimit;
}

UIntN TargetLimitAction::getDomainWithHighestTemperature(
	UIntN source,
	const std::vector<UIntN>& domainsWithControlKnobsToTurn)
{
	pair<Temperature, UIntN> domainWithHighestTemperature(
		Temperature::fromCelsius(ESIF_SDK_MIN_AUX_TRIP), Constants::Invalid);
	for (auto domain = domainsWithControlKnobsToTurn.begin(); domain != domainsWithControlKnobsToTurn.end(); domain++)
	{
		try
		{
			auto sourceParticipant = getParticipantTracker()->getParticipant(source);
			auto sourceDomain = sourceParticipant->getDomain(*domain);
			Temperature domainTemperature = sourceDomain->getTemperatureControl()->getCurrentTemperature();
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
	UIntN source,
	std::vector<UIntN> domains)
{
	// sort all domains by priority, then by utilization
	vector<tuple<UIntN, DomainPriority, UtilizationStatus>> domainPreference;
	for (auto domain = domains.begin(); domain != domains.end(); domain++)
	{
		auto sourceParticipant = getParticipantTracker()->getParticipant(source);
		auto sourceDomain = sourceParticipant->getDomain(*domain);
		DomainPriority domainPriority = sourceDomain->getDomainPriorityProperty().getDomainPriority();
		UtilizationStatus utilStatus = UtilizationStatus(Percentage::createInvalid());
		if (domainReportsUtilization(source, *domain))
		{
			try
			{
				utilStatus = sourceDomain->getUtilizationStatus();
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
	return getParticipantTracker()
		->getParticipant(source)
		->getDomain(domain)
		->getDomainProperties()
		.implementsUtilizationInterface();
}

void TargetLimitAction::requestLimit(UIntN source, UIntN domainIndex, UIntN target)
{
	auto participant = getParticipantTracker()->getParticipant(source);
	auto domain = std::dynamic_pointer_cast<PassiveDomainProxy>(participant->getDomain(domainIndex));
	domain->requestLimit(target);
}

void TargetLimitAction::commitLimit(UIntN source, const TimeSpan& time)
{
	Bool madeChanges(false);
	auto participant = getParticipantTracker()->getParticipant(source);
	vector<UIntN> domainIndexes = participant->getDomainIndexes();
	for (auto domainIndex = domainIndexes.begin(); domainIndex != domainIndexes.end(); domainIndex++)
	{
		try
		{
			// TODO: want to pass in participant index and domain
			POLICY_LOG_MESSAGE_DEBUG({
				std::stringstream message;
				message << "Committing limits to source."
						<< " ParticipantIndex = " << source << ". Domain = " << *domainIndex;
				return message.str();
			});

			auto domain = std::dynamic_pointer_cast<PassiveDomainProxy>(participant->getDomain(*domainIndex));
			Bool madeChange = domain->commitLimits();
			if (madeChange)
			{
				madeChanges = true;
			}
		}
		catch (std::exception& ex)
		{
			// TODO: want to pass in participant index and domain
			POLICY_LOG_MESSAGE_WARNING_EX({
				std::stringstream message;
				message << "Failed to limit source: " << string(ex.what()) << ". ParticipantIndex = " << source
						<< ". Domain = " << *domainIndex;
				return message.str();
			});
		}
	}

	if (madeChanges)
	{
		getCallbackScheduler()->markSourceAsBusy(source, getTargetMonitor(), time);
	}
}
