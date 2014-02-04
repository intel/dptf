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

#include "RfProfileType.h"
#include "DptfExceptions.h"

namespace RfProfileType
{
    std::string ToString(RfProfileType::Type type)
    {
        switch (type)
        {
            case RfProfileType::Na:
                return "Na";
            case RfProfileType::Box:
                return "Box";
            default:
                throw dptf_exception("RfProfileType::Type is invalid.");
        }
    }
}