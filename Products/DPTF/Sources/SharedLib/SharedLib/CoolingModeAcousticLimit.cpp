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

#include "CoolingModeAcousticLimit.h"

namespace CoolingModeAcousticLimit
{
    std::string ToString(CoolingModeAcousticLimit::Type coolingModeAcousticLimitType)
    {
        switch (coolingModeAcousticLimitType)
        {
            case Level_1:
                return "Level 1";
            case Level_2:
                return "Level 2";
            case Level_3:
                return "Level 3";
            case Level_4:
                return "Level 4";
            case Level_5:
                return "Level 5";
            default:
                throw dptf_exception("CoolingModeAcousticLimit::Type is invalid");
        }
    }
}