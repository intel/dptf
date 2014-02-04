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

#include "LpmMode.h"
#include "DptfExceptions.h"

namespace LpmMode
{
    std::string ToString(LpmMode::Type type)
    {
        switch (type)
        {
            case Disabled:
                return "Disabled";
            case Standard:
                return "Standard";
            case AppSpecific:
                return "App Specific";
            default:
                return "Standard/LpmSet";
        }
    }

    std::string ToString(LpmMode::Boss boss)
    {
        switch (boss)
        {
            case Bios:
                return "Bios";
            case Dppe:
                return "OS Controlled";
            default:
                throw dptf_exception("Invalid Lpm Mode Boss");
        }
    }
}