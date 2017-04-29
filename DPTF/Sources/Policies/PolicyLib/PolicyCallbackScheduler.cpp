/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "PolicyCallbackScheduler.h"
#include "StatusFormat.h"

using namespace std;
using namespace StatusFormat;

PolicyCallbackScheduler::PolicyCallbackScheduler(
	const PolicyServicesInterfaceContainer& policyServicesContainer,
	std::shared_ptr<TimeInterface> timeInterface)
	: m_policyServices(policyServicesContainer)
	, m_time(timeInterface)
{
}

PolicyCallbackScheduler::~PolicyCallbackScheduler()
{
}

void PolicyCallbackScheduler::suspend(UIntN participantIndex, const TimeSpan& time)
{
	suspend(ParticipantRole::NA, participantIndex, time);
}

void PolicyCallbackScheduler::suspend(UIntN participantIndex, const TimeSpan& fromTime, const TimeSpan& suspendTime)
{
	cancelCallback(participantIndex);
	scheduleCallback(participantIndex, fromTime, suspendTime);
}

void PolicyCallbackScheduler::suspend(
	ParticipantRole::Type participantRole,
	UIntN participantIndex,
	const TimeSpan& time)
{
	auto currentTime = getCurrentTime();
	cancelCallback(participantRole, participantIndex);
	scheduleCallback(participantRole, participantIndex, currentTime, time);
}

void PolicyCallbackScheduler::cancelCallback(UIntN participantIndex)
{
	cancelCallback(ParticipantRole::NA, participantIndex);
}

void PolicyCallbackScheduler::cancelCallback(ParticipantRole::Type participantRole, UIntN participantIndex)
{
	auto row = m_schedule.find(std::make_pair(participantRole, participantIndex));
	if (row != m_schedule.end())
	{
		m_policyServices.policyInitiatedCallback->removePolicyInitiatedCallback(row->second.getCallbackHandle());
		acknowledgeCallback(participantRole, participantIndex);
	}
}

Bool PolicyCallbackScheduler::hasCallbackWithinTimeRange(
	UIntN target,
	const TimeSpan& beginTime,
	const TimeSpan& endTime) const
{
	auto row = m_schedule.find(std::make_pair(ParticipantRole::NA, target));
	if (row == m_schedule.end())
	{
		return false;
	}

	auto scheduledTime = row->second.getTimeStamp() + row->second.getTimeDelta();
	if ((beginTime <= scheduledTime) && (scheduledTime <= endTime))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void PolicyCallbackScheduler::acknowledgeCallback(UIntN participantIndex)
{
	acknowledgeCallback(ParticipantRole::NA, participantIndex);
}

void PolicyCallbackScheduler::acknowledgeCallback(ParticipantRole::Type participantRole, UIntN participantIndex)
{
	m_schedule.erase(std::make_pair(participantRole, participantIndex));
}

void PolicyCallbackScheduler::setTimeObject(std::shared_ptr<TimeInterface> time)
{
	m_time = time;
}

std::shared_ptr<XmlNode> PolicyCallbackScheduler::getStatus()
{
	auto status = XmlNode::createWrapperElement("policy_callback_scheduler");
	for (auto participant = m_schedule.begin(); participant != m_schedule.end(); ++participant)
	{
		status->addChild(getStatusForParticipant(ParticipantRole::NA, participant->first.second));
	}

	return status;
}

std::shared_ptr<XmlNode> PolicyCallbackScheduler::getStatusForParticipant(UIntN participantIndex)
{
	return getStatusForParticipant(ParticipantRole::NA, participantIndex);
}

std::shared_ptr<XmlNode> PolicyCallbackScheduler::getStatusForParticipant(
	ParticipantRole::Type participantRole,
	UIntN participantIndex)
{
	auto status = XmlNode::createWrapperElement("participant_callback");

	try
	{
		status->addChild(XmlNode::createDataElement("participant_index", friendlyValue(participantIndex)));
		status->addChild(XmlNode::createDataElement("participant_role", ParticipantRole::ToString(participantRole)));

		auto scheduledCallback = m_schedule.find(std::make_pair(participantRole, participantIndex));
		if (scheduledCallback != m_schedule.end())
		{
			auto expireTime = scheduledCallback->second.getTimeStamp() + scheduledCallback->second.getTimeDelta();
			auto currentTime = getCurrentTime();
			auto currentPoll = scheduledCallback->second.getTimeDelta().toStringSeconds();
			if (expireTime >= currentTime)
			{
				auto expireTimeDiffFromNow = expireTime - currentTime;
				status->addChild(
					XmlNode::createDataElement("time_until_expires", expireTimeDiffFromNow.toStringSeconds()));
				status->addChild(XmlNode::createDataElement("current_polling_period", currentPoll));
			}
			else
			{
				status->addChild(XmlNode::createDataElement("time_until_expires", Constants::InvalidString));
				status->addChild(XmlNode::createDataElement("current_polling_period", Constants::InvalidString));
			}
		}
		else
		{
			status->addChild(XmlNode::createDataElement("time_until_expires", Constants::InvalidString));
		}
	}
	catch (dptf_exception)
	{
		status->addChild(XmlNode::createDataElement("time_until_expires", Constants::InvalidString));
	}

	return status;
}

TimeSpan PolicyCallbackScheduler::getCurrentTime() const
{
	return m_time->getCurrentTime();
}

void PolicyCallbackScheduler::scheduleCallback(
	UIntN participantIndex,
	const TimeSpan& currentTime,
	const TimeSpan& pollTime)
{
	scheduleCallback(ParticipantRole::NA, participantIndex, currentTime, pollTime);
}

void PolicyCallbackScheduler::scheduleCallback(
	ParticipantRole::Type participantRole,
	UIntN participantIndex,
	const TimeSpan& currentTime,
	const TimeSpan& pollTime)
{
	UInt64 callbackHandle = m_policyServices.policyInitiatedCallback->createPolicyInitiatedDeferredCallback(
		participantRole, participantIndex, nullptr, pollTime);
	m_schedule[std::make_pair(participantRole, participantIndex)] =
		ParticipantCallback(pollTime, currentTime, callbackHandle);

	m_policyServices.messageLogging->writeMessageDebug(PolicyMessage(
		FLF,
		"Scheduled a callback in " + pollTime.toStringMilliseconds() + " ms" + " for participant "
			+ std::to_string(participantIndex)
			+ " with event code = "
			+ std::to_string(participantRole)
			+ ".",
		participantIndex));
}
