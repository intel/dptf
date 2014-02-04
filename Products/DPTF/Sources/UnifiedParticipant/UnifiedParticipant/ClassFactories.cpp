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

#include "ClassFactories.h"

ClassFactories::ClassFactories(void) :
    domainActiveControlFactory(nullptr),
    domainConfigTdpControlFactory(nullptr),
    domainCoreControlFactory(nullptr),
    domainDisplayControlFactory(nullptr),
    domainPerformanceControlFactory(nullptr),
    domainPixelClockControlFactory(nullptr),
    domainPixelClockStatusFactory(nullptr),
    domainPowerControlFactory(nullptr),
    domainPowerStatusFactory(nullptr),
    domainPriorityFactory(nullptr),
    domainRfProfileControlFactory(nullptr),
    domainRfProfileStatusFactory(nullptr),
    domainTemperatureFactory(nullptr),
    domainUtilizationFactory(nullptr),
    participantGetSpecificInfoFactory(nullptr),
    participantSetSpecificInfoFactory(nullptr)
{
}

void ClassFactories::deleteAllFactories(void)
{
    DELETE_MEMORY_TC(domainActiveControlFactory);
    DELETE_MEMORY_TC(domainConfigTdpControlFactory);
    DELETE_MEMORY_TC(domainCoreControlFactory);
    DELETE_MEMORY_TC(domainDisplayControlFactory);
    DELETE_MEMORY_TC(domainPerformanceControlFactory);
    DELETE_MEMORY_TC(domainPixelClockControlFactory);
    DELETE_MEMORY_TC(domainPixelClockStatusFactory);
    DELETE_MEMORY_TC(domainPowerControlFactory);
    DELETE_MEMORY_TC(domainPowerStatusFactory);
    DELETE_MEMORY_TC(domainPriorityFactory);
    DELETE_MEMORY_TC(domainRfProfileControlFactory);
    DELETE_MEMORY_TC(domainRfProfileStatusFactory);
    DELETE_MEMORY_TC(domainTemperatureFactory);
    DELETE_MEMORY_TC(domainUtilizationFactory);
    DELETE_MEMORY_TC(participantGetSpecificInfoFactory);
    DELETE_MEMORY_TC(participantSetSpecificInfoFactory);
}