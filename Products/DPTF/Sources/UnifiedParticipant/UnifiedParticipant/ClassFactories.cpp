/******************************************************************************
** Copyright (c) 2013 Intel Corporation All Rights Reserved
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

ClassFactories::ClassFactories(void)
{
    setAllPointersNull();
}

void ClassFactories::deleteAllFactories(void)
{
    delete domainActiveControlFactory;
    delete domainConfigTdpControlFactory;
    delete domainCoreControlFactory;
    delete domainDisplayControlFactory;
    delete domainPerformanceControlFactory;
    delete domainPowerControlFactory;
    delete domainPowerStatusFactory;
    delete domainPriorityFactory;
    delete domainTemperatureFactory;
    delete domainUtilizationFactory;
    delete participantGetSpecificInfoFactory;
    delete participantSetSpecificInfoFactory;
    setAllPointersNull();
}

void ClassFactories::setAllPointersNull(void)
{
    domainActiveControlFactory = nullptr;
    domainConfigTdpControlFactory = nullptr;
    domainCoreControlFactory = nullptr;
    domainDisplayControlFactory = nullptr;
    domainPerformanceControlFactory = nullptr;
    domainPowerControlFactory = nullptr;
    domainPowerStatusFactory = nullptr;
    domainPriorityFactory = nullptr;
    domainTemperatureFactory = nullptr;
    domainUtilizationFactory = nullptr;
    participantGetSpecificInfoFactory = nullptr;
    participantSetSpecificInfoFactory = nullptr;
}