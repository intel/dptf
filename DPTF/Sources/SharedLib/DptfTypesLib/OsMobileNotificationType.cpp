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

#include "OsMobileNotificationType.h"

namespace OsMobileNotificationType
{

    std::string ToString(Type type)
    {
        switch (type)
        {
        case OsMobileNotificationType::EmergencyCallMode:
            return "EmergencyCallMode";
        case OsMobileNotificationType::AirplaneMode:
            return "AirplaneMode";
        case OsMobileNotificationType::ServiceState:
            return "ServiceState";
        case OsMobileNotificationType::ActionRequestShutdown:
            return "ActionRequestShutdown";
        case OsMobileNotificationType::ConnectivityState:
            return "ConnectivityState";
        case OsMobileNotificationType::ScreenState:
            return "ScreenState";
        default:
            return "X";
        }
    }

}