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
#pragma once

#include "DomainActiveControlFactoryInterface.h"
#include "DomainConfigTdpControlFactoryInterface.h"
#include "DomainCoreControlFactoryInterface.h"
#include "DomainDisplayControlFactoryInterface.h"
#include "DomainPerformanceControlFactoryInterface.h"
#include "DomainPowerControlFactoryInterface.h"
#include "DomainPowerStatusFactoryInterface.h"
#include "DomainPriorityFactoryInterface.h"
#include "DomainTemperatureFactoryInterface.h"
#include "DomainUtilizationFactoryInterface.h"
#include "ParticipantGetSpecificInfoFactoryInterface.h"
#include "ParticipantSetSpecificInfoFactoryInterface.h"

class ClassFactories
{
public:

    ClassFactories(void);
    void setAllPointersNull(void);
    void deleteAllFactories(void);

    DomainActiveControlFactoryInterface* domainActiveControlFactory;
    DomainConfigTdpControlFactoryInterface* domainConfigTdpControlFactory;
    DomainCoreControlFactoryInterface* domainCoreControlFactory;
    DomainDisplayControlFactoryInterface* domainDisplayControlFactory;
    DomainPerformanceControlFactoryInterface* domainPerformanceControlFactory;
    DomainPowerControlFactoryInterface* domainPowerControlFactory;
    DomainPowerStatusFactoryInterface* domainPowerStatusFactory;
    DomainPriorityFactoryInterface* domainPriorityFactory;
    DomainTemperatureFactoryInterface* domainTemperatureFactory;
    DomainUtilizationFactoryInterface* domainUtilizationFactory;
    ParticipantGetSpecificInfoFactoryInterface* participantGetSpecificInfoFactory;
    ParticipantSetSpecificInfoFactoryInterface* participantSetSpecificInfoFactory;
};