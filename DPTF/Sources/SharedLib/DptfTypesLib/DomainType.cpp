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

#include "DomainType.h"
#include "esif_sdk_domain_type.h"

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
            case Power:
                return "Power";
            case Thermistor:
                return "Thermistor";
            case Infrared:
                return "Infrared";
            case Virtual:
                return "Virtual";
            case Ambient:
                return "Ambient";
            case DSx:
                return "DSx";
            case Rfem:
                return "Rfem";
            case M2Cnv:
                return "M2Cnv";
            case SocCnv:
                return "SocCnv";
            case IVCam:
                return "IVCam";
            case All:
                return "All";
            case Invalid:
                return "NA";
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
        case ESIF_DOMAIN_TYPE_AHCI:
            return DomainType::Storage;
        case ESIF_DOMAIN_TYPE_MULTIFUNCTION:
            return DomainType::MultiFunction;
        case ESIF_DOMAIN_TYPE_DISPLAY:
            return DomainType::Display;
        case ESIF_DOMAIN_TYPE_BATTERYCHARGER:
            return DomainType::Charger;
        case ESIF_DOMAIN_TYPE_BATTERY:
            return DomainType::Battery;
        case ESIF_DOMAIN_TYPE_AUDIO:
            return DomainType::Audio;
        case ESIF_DOMAIN_TYPE_OTHER:
            return DomainType::Other;
        case ESIF_DOMAIN_TYPE_WWAN:
            return DomainType::WWan;
        case ESIF_DOMAIN_TYPE_POWER:
            return DomainType::Power;
        case ESIF_DOMAIN_TYPE_THERMISTOR:
            return DomainType::Thermistor;
        case ESIF_DOMAIN_TYPE_INFRARED:
            return DomainType::Infrared;
        case ESIF_DOMAIN_TYPE_VIRTUAL:
            return DomainType::Virtual;
        case ESIF_DOMAIN_TYPE_AMBIENT:
            return DomainType::Ambient;
        case ESIF_DOMAIN_TYPE_DSX:
            return DomainType::DSx;
        case ESIF_DOMAIN_TYPE_RFEM:
            return DomainType::Rfem;
        case ESIF_DOMAIN_TYPE_M2CNV:
            return DomainType::M2Cnv;
        case ESIF_DOMAIN_TYPE_SOCCNV:
            return DomainType::SocCnv;
        case ESIF_DOMAIN_TYPE_IVCAM:
            return DomainType::IVCam;
        case ESIF_DOMAIN_TYPE_ALL:
            return DomainType::All;
        case ESIF_DOMAIN_TYPE_INVALID:
            return DomainType::Invalid;
        default: break;
    }

    throw dptf_exception("Received unknown esif_domain_type.");
}

esif_domain_type DptfDomainTypeToEsifDomainType(DomainType::Type dptfDomainType)
{
    switch (dptfDomainType)
    {
    case DomainType::Processor:
        return ESIF_DOMAIN_TYPE_PROCESSOR;
    case DomainType::Graphics:
        return ESIF_DOMAIN_TYPE_GRAPHICS;
    case DomainType::Memory:
        return ESIF_DOMAIN_TYPE_MEMORY;
    case DomainType::Temperature:
        return ESIF_DOMAIN_TYPE_TEMPERATURE;
    case DomainType::Fan:
        return ESIF_DOMAIN_TYPE_FAN;
    case DomainType::Chipset:
        return ESIF_DOMAIN_TYPE_CHIPSET;
    case DomainType::Ethernet:
        return ESIF_DOMAIN_TYPE_ETHERNET;
    case DomainType::Wireless:
        return ESIF_DOMAIN_TYPE_WIRELESS;
    case DomainType::Storage:
        return ESIF_DOMAIN_TYPE_AHCI;
    case DomainType::MultiFunction:
        return ESIF_DOMAIN_TYPE_MULTIFUNCTION;
    case DomainType::Display:
        return ESIF_DOMAIN_TYPE_DISPLAY;
    case DomainType::Charger:
        return ESIF_DOMAIN_TYPE_BATTERYCHARGER;
    case DomainType::Battery:
        return ESIF_DOMAIN_TYPE_BATTERY;
    case DomainType::Audio:
        return ESIF_DOMAIN_TYPE_AUDIO;
    case DomainType::Other:
        return ESIF_DOMAIN_TYPE_OTHER;
    case DomainType::WWan:
        return ESIF_DOMAIN_TYPE_WWAN;
    case DomainType::Power:
        return ESIF_DOMAIN_TYPE_POWER;
    case DomainType::Thermistor:
        return ESIF_DOMAIN_TYPE_THERMISTOR;
    case DomainType::Infrared:
        return ESIF_DOMAIN_TYPE_INFRARED;
    case DomainType::Virtual:
        return ESIF_DOMAIN_TYPE_VIRTUAL;
    case DomainType::Ambient:
        return ESIF_DOMAIN_TYPE_AMBIENT;
    case DomainType::DSx:
        return ESIF_DOMAIN_TYPE_DSX;
    case DomainType::Rfem:
        return ESIF_DOMAIN_TYPE_RFEM;
    case DomainType::M2Cnv:
        return ESIF_DOMAIN_TYPE_M2CNV;
    case DomainType::SocCnv:
        return ESIF_DOMAIN_TYPE_SOCCNV;
    case DomainType::IVCam:
        return ESIF_DOMAIN_TYPE_IVCAM;
    case DomainType::All:
        return ESIF_DOMAIN_TYPE_ALL;
    case DomainType::Invalid:
        return ESIF_DOMAIN_TYPE_INVALID;
    default: break;
    }

    throw dptf_exception("Received unknown Domain::Type");
}
