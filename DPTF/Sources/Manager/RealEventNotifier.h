/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#pragma once
#include "EventNotifierInterface.h"
#include <map>
#include <memory>
#include <set>

class RealEventNotifier : public EventNotifierInterface
{
public:
	RealEventNotifier() = default;
	~RealEventNotifier() override = default;
	void notify(FrameworkEvent::Type event, const DptfBuffer& eventPayload) override;
	void registerObserver(
		const std::shared_ptr<EventObserverInterface>& observer, 
		std::set<FrameworkEvent::Type> events) override;
	void unregisterObserverFromAllEvents(const std::shared_ptr<EventObserverInterface>& observer) override;
	void unregisterObserver(
		const std::shared_ptr<EventObserverInterface>& observer, 
		std::set<FrameworkEvent::Type> events) override;

private:
	std::map<FrameworkEvent::Type, std::set<std::shared_ptr<EventObserverInterface>>> m_registrations;
};