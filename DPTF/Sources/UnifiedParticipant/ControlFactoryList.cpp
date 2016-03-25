/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "DomainConfigTdpControlFactory.h"
#include "DomainCoreControlFactory.h"
#include "DomainDisplayControlFactory.h"
#include "DomainPerformanceControlFactory.h"
#include "DomainPixelClockControlFactory.h"
#include "DomainPixelClockStatusFactory.h"
#include "DomainPowerControlFactory.h"
#include "DomainPowerStatusFactory.h"
#include "DomainPriorityFactory.h"
#include "DomainRfProfileControlFactory.h"
#include "DomainRfProfileStatusFactory.h"
#include "DomainTemperatureFactory.h"
#include "DomainUtilizationFactory.h"
#include "DomainHardwareDutyCycleControlFactory.h"
#include "ParticipantGetSpecificInfoFactory.h"
#include "ParticipantSetSpecificInfoFactory.h"
#include "DomainPlatformPowerControlFactory.h"
#include "DomainPlatformPowerStatusFactory.h"
using namespace std;

ControlFactoryList::ControlFactoryList(void)
{
    for (ControlFactoryType::Type factoryType = ControlFactoryType::FIRST;
        factoryType < ControlFactoryType::MAX; 
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
        throw dptf_exception("Control factory type \"" + 
            ControlFactoryType::ToString(factoryType) + 
            "\" does not exist.");
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
        return shared_ptr<ControlFactoryInterface>(new DomainActiveControlFactory());
    case ControlFactoryType::ConfigTdp:
        return shared_ptr<ControlFactoryInterface>(new DomainConfigTdpControlFactory());
    case ControlFactoryType::Core:
        return shared_ptr<ControlFactoryInterface>(new DomainCoreControlFactory());
    case ControlFactoryType::Display:
        return shared_ptr<ControlFactoryInterface>(new DomainDisplayControlFactory());
    case ControlFactoryType::Performance:
        return shared_ptr<ControlFactoryInterface>(new DomainPerformanceControlFactory());
    case ControlFactoryType::PixelClockControl:
        return shared_ptr<ControlFactoryInterface>(new DomainPixelClockControlFactory());
    case ControlFactoryType::PixelClockStatus:
        return shared_ptr<ControlFactoryInterface>(new DomainPixelClockStatusFactory());
    case ControlFactoryType::PowerControl:
        return shared_ptr<ControlFactoryInterface>(new DomainPowerControlFactory());
    case ControlFactoryType::PowerStatus:
        return shared_ptr<ControlFactoryInterface>(new DomainPowerStatusFactory());
    case ControlFactoryType::Priority:
        return shared_ptr<ControlFactoryInterface>(new DomainPriorityFactory());
    case ControlFactoryType::RfProfileControl:
        return shared_ptr<ControlFactoryInterface>(new DomainRfProfileControlFactory());
    case ControlFactoryType::RfProfileStatus:
        return shared_ptr<ControlFactoryInterface>(new DomainRfProfileStatusFactory());
    case ControlFactoryType::Temperature:
        return shared_ptr<ControlFactoryInterface>(new DomainTemperatureFactory());
    case ControlFactoryType::Utilization:
        return shared_ptr<ControlFactoryInterface>(new DomainUtilizationFactory());
    case ControlFactoryType::HardwareDutyCycle:
        return shared_ptr<ControlFactoryInterface>(new DomainHardwareDutyCycleControlFactory());
    case ControlFactoryType::GetSpecificInfo:
        return shared_ptr<ControlFactoryInterface>(new ParticipantGetSpecificInfoFactory());
    case ControlFactoryType::SetSpecificInfo:
        return shared_ptr<ControlFactoryInterface>(new ParticipantSetSpecificInfoFactory());
    case ControlFactoryType::PlatformPower:
        return shared_ptr<ControlFactoryInterface>(new DomainPlatformPowerControlFactory());
    case ControlFactoryType::PlatformPowerStatus:
        return shared_ptr<ControlFactoryInterface>(new DomainPlatformPowerStatusFactory());
    default:
        throw dptf_exception("Cannot make control factory for invalid control factory type.");
    }
}