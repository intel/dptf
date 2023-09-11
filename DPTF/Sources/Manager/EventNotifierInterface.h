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
#include "FrameworkEvent.h"
#include "DptfBuffer.h"
#include "EventObserverInterface.h"

class EventNotifierInterface
{
public:
	virtual ~EventNotifierInterface() = default;
	virtual void notify(FrameworkEvent::Type event, const DptfBuffer& eventPayload) = 0;
	virtual void registerObserver(
		const std::shared_ptr<EventObserverInterface>& observer, 
		std::set<FrameworkEvent::Type> events) = 0;
	virtual void unregisterObserverFromAllEvents(const std::shared_ptr<EventObserverInterface>& observer) = 0;
	virtual void unregisterObserver(
		const std::shared_ptr<EventObserverInterface>& observer, 
		std::set<FrameworkEvent::Type> events) = 0;
};
