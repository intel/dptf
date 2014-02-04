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

#include "DomainType.h"
#include "esif_domain_type.h"
#include "DptfExceptions.h"

namespace DomainType
{
    std::string ToString(DomainType::Type type)
    {
        switch (type)
        {
            case Processor:
                return "Processor";
            case Graphics:
                return "Graphics";
            case Memory:
                return "Memory";
            case Temperature:
                return "Temperature";
            case Fan:
                return "Fan";
            case Chipset:
                return "Chipset";
            case Ethernet:
                return "Ethernet";
            case Wireless:
                return "Wireless";
            case Storage:
                return "Storage";
            case MultiFunction:
                return "MultiFunction";
            case Display:
                return "Display";
            case Charger:
                return "Charger";
            case Battery:
                return "Battery";
            case Audio:
                return "Audio";
            case Other:
                return "Other";
            case WWan:
                return "WWan";
            case WGig:
                return "WGig";
            case Power:
                return "Power";
            case Thermistor:
                return "Thermistor";
            case Infrared:
                return "Infrared";
            case WirelessRfem:
                return "WirelessRfem";
            case Virtual:
                return "Virtual";
            case Ambient:
                return "Ambient";
            default:
                throw dptf_exception("DomainType::Type is invalid.");
        }
    }
}

DomainType::Type EsifDomainTypeToDptfDomainType(esif_domain_type esifDomainType)
{
    switch (esifDomainType)
    {
        case ESIF_DOMAIN_TYPE_PROCESSOR:
            return DomainType::Processor;
        case ESIF_DOMAIN_TYPE_GRAPHICS:
            return DomainType::Graphics;
        case ESIF_DOMAIN_TYPE_MEMORY:
            return DomainType::Memory;
        case ESIF_DOMAIN_TYPE_TEMPERATURE:
            return DomainType::Temperature;
        case ESIF_DOMAIN_TYPE_FAN:
            return DomainType::Fan;
        case ESIF_DOMAIN_TYPE_CHIPSET:
            return DomainType::Chipset;
        case ESIF_DOMAIN_TYPE_ETHERNET:
            return DomainType::Ethernet;
        case ESIF_DOMAIN_TYPE_WIRELESS:
            return DomainType::Wireless;
        case ESIF_DOMAIN_TYPE_STORAGE:
            return DomainType::Storage;
        case ESIF_DOMAIN_TYPE_MULTIFUNCTION:
            return DomainType::MultiFunction;
        case ESIF_DOMAIN_TYPE_DISPLAY:
            return DomainType::Display;
        case ESIF_DOMAIN_TYPE_CHARGER:
            return DomainType::Charger;
        case ESIF_DOMAIN_TYPE_BATTERY:
            return DomainType::Battery;
        case ESIF_DOMAIN_TYPE_AUDIO:
            return DomainType::Audio;
        case ESIF_DOMAIN_TYPE_OTHER:
            return DomainType::Other;
        case ESIF_DOMAIN_TYPE_WWAN:
            return DomainType::WWan;
        case ESIF_DOMAIN_TYPE_WGIG:
            return DomainType::WGig;
        case ESIF_DOMAIN_TYPE_POWER:
            return DomainType::Power;
        case ESIF_DOMAIN_TYPE_THERMISTOR:
            return DomainType::Thermistor;
        case ESIF_DOMAIN_TYPE_INFRARED:
            return DomainType::Infrared;
        case ESIF_DOMAIN_TYPE_WIRELESSRFEM:
            return DomainType::WirelessRfem;
        case ESIF_DOMAIN_TYPE_VIRTUAL:
            return DomainType::Virtual;
        case ESIF_DOMAIN_TYPE_AMBIENT:
            return DomainType::Ambient;
        default: break;
    }

    throw dptf_exception("Received unknown esif_domain_type.");
}