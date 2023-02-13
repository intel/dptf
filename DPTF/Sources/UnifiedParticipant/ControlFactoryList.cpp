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

#include "ControlFactoryList.h"
#include "DomainActiveControlFactory.h"
#include "DomainActivityStatusFactory.h"
#include "DomainCoreControlFactory.h"
#include "DomainDisplayControlFactory.h"
#include "DomainEnergyControlFactory.h"
#include "DomainPerformanceControlFactory.h"
#include "DomainPowerControlFactory.h"
#include "DomainPowerStatusFactory.h"
#include "DomainPriorityFactory.h"
#include "DomainRfProfileControlFactory.h"
#include "DomainRfProfileStatusFactory.h"
#include "DomainTemperatureFactory.h"
#include "DomainUtilizationFactory.h"
#include "ParticipantGetSpecificInfoFactory.h"
#include "ParticipantSetSpecificInfoFactory.h"
#include "DomainSystemPowerControlFactory.h"
#include "DomainPlatformPowerStatusFactory.h"
#include "DomainPeakPowerControlFactory.h"
#include "DomainProcessorControlFactory.h"
#include "DomainBatteryStatusFactory.h"
#include "DomainSocWorkloadClassificationFactory.h"
#include "DomainDynamicEppFactory.h"
using namespace std;

ControlFactoryList::ControlFactoryList(void)
	: m_factories()
{
	for (ControlFactoryType::Type factoryType = ControlFactoryType::FIRST; factoryType < ControlFactoryType::MAX;
		 factoryType = (ControlFactoryType::Type)((int)factoryType + 1))
	{
		try
		{
			pair<ControlFactoryType::Type, shared_ptr<ControlFactoryInterface>> record;
			record.first = factoryType;
			record.second = makeFactory(factoryType);
			m_factories.insert(record);
		}
		catch (...)
		{
		}
	}
}

ControlFactoryList::~ControlFactoryList(void)
{
}

std::shared_ptr<ControlFactoryInterface> ControlFactoryList::getFactory(ControlFactoryType::Type factoryType) const
{
	auto findResult = m_factories.find(factoryType);
	if (findResult == m_factories.end())
	{
		throw dptf_exception(
			"Control factory type \"" + ControlFactoryType::toString(factoryType) + "\" does not exist.");
	}
	else
	{
		return findResult->second;
	}
}

std::shared_ptr<ControlFactoryInterface> ControlFactoryList::makeFactory(ControlFactoryType::Type factoryType)
{
	switch (factoryType)
	{
	case ControlFactoryType::Active:
		return make_shared<DomainActiveControlFactory>();
	case ControlFactoryType::Core:
		return make_shared<DomainCoreControlFactory>();
	case ControlFactoryType::Display:
		return make_shared<DomainDisplayControlFactory>();
	case ControlFactoryType::EnergyControl:
		return make_shared<DomainEnergyControlFactory>();
	case ControlFactoryType::PeakPowerControl:
		return make_shared<DomainPeakPowerControlFactory>();
	case ControlFactoryType::Performance:
		return make_shared<DomainPerformanceControlFactory>();
	case ControlFactoryType::PowerControl:
		return make_shared<DomainPowerControlFactory>();
	case ControlFactoryType::PowerStatus:
		return make_shared<DomainPowerStatusFactory>();
	case ControlFactoryType::Priority:
		return make_shared<DomainPriorityFactory>();
	case ControlFactoryType::RfProfileControl:
		return make_shared<DomainRfProfileControlFactory>();
	case ControlFactoryType::RfProfileStatus:
		return make_shared<DomainRfProfileStatusFactory>();
	case ControlFactoryType::ProcessorControl:
		return make_shared<DomainProcessorControlFactory>();
	case ControlFactoryType::Temperature:
		return make_shared<DomainTemperatureFactory>();
	case ControlFactoryType::Utilization:
		return make_shared<DomainUtilizationFactory>();
	case ControlFactoryType::GetSpecificInfo:
		return make_shared<ParticipantGetSpecificInfoFactory>();
	case ControlFactoryType::SetSpecificInfo:
		return make_shared<ParticipantSetSpecificInfoFactory>();
	case ControlFactoryType::SystemPower:
		return make_shared<DomainSystemPowerControlFactory>();
	case ControlFactoryType::PlatformPowerStatus:
		return make_shared<DomainPlatformPowerStatusFactory>();
	case ControlFactoryType::ActivityStatus:
		return make_shared<DomainActivityStatusFactory>();
	case ControlFactoryType::BatteryStatus:
		return make_shared<DomainBatteryStatusFactory>();
	case ControlFactoryType::SocWorkloadClassification:
		return make_shared<DomainSocWorkloadClassificationFactory>();
	case ControlFactoryType::DynamicEpp:
		return make_shared<DomainDynamicEppFactory>();
	default:
		throw dptf_exception("Cannot make control factory for invalid control factory type.");
	}
}
