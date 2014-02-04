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

#include "BusType.h"
#include "DptfExceptions.h"

namespace BusType
{
    std::string ToString(BusType::Type type)
    {
        switch (type)
        {
            case BusType::Acpi:
                return "ACPI";
            case BusType::Pci:
                return "PCI";
            case BusType::Plat:
                return "Platform";
            case BusType::Conjure:
                return "Conjure";
            case BusType::None:
                return "None";
            default:
                throw dptf_exception("BusType::Type is invalid.");
        }
    }
}

BusType::Type EsifParticipantEnumToBusType(esif_participant_enum esifParticipantEnum)
{
    switch (esifParticipantEnum)
    {
        case ESIF_PARTICIPANT_ENUM_ACPI:
            return BusType::Type::Acpi;
        case ESIF_PARTICIPANT_ENUM_PCI:
            return BusType::Type::Pci;
        case ESIF_PARTICIPANT_ENUM_PLAT:
            return BusType::Type::Plat;
        case ESIF_PARTICIPANT_ENUM_CONJURE:
            return BusType::Type::Conjure;
        default:
            throw dptf_exception("Received unexpected esif_participant_enum.");
    }
}