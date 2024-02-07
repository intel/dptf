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

#pragma once
#include "WorkItem.h"

class WIDptfExtendedWorkloadPredictionEventRegistrationChanged : public WorkItem
{
public:
	WIDptfExtendedWorkloadPredictionEventRegistrationChanged(
		DptfManagerInterface* dptfManager,
		UInt32 consumerCount);
	WIDptfExtendedWorkloadPredictionEventRegistrationChanged(
		const WIDptfExtendedWorkloadPredictionEventRegistrationChanged& other) = delete;
	WIDptfExtendedWorkloadPredictionEventRegistrationChanged(
		WIDptfExtendedWorkloadPredictionEventRegistrationChanged&& other) noexcept = delete;
	WIDptfExtendedWorkloadPredictionEventRegistrationChanged& operator=(
		const WIDptfExtendedWorkloadPredictionEventRegistrationChanged& other) = delete;
	WIDptfExtendedWorkloadPredictionEventRegistrationChanged& operator=(
		WIDptfExtendedWorkloadPredictionEventRegistrationChanged&& other) noexcept = delete;
	~WIDptfExtendedWorkloadPredictionEventRegistrationChanged() override = default;

	void onExecute() final;

private:
	UInt32 m_consumerCount;
};
