/******************************************************************************
** Copyright (c) 2013-2024 Intel Corporation All Rights Reserved
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
	const std::map<DomainType::Type, std::string> names{{Processor, "Processor"},
														{Graphics, "Graphics"},
														{Memory, "Memory"},
														{Temperature, "Temperature"},
														{Fan, "Fan"},
														{Chipset, "Chipset"},
														{Ethernet, "Ethernet"},
														{Wireless, "Wireless"},
														{Storage, "Storage"},
														{MultiFunction, "MultiFunction"},
														{Display, "Display"},
														{Charger, "Charger"},
														{Battery, "Battery"},
														{Audio, "Audio"},
														{Other, "Other"},
														{WWan, "WWan"},
														{Power, "Power"},
														{Thermistor, "Thermistor"},
														{Infrared, "Infrared"},
														{Virtual, "Virtual"},
														{Ambient, "Ambient"},
														{DSx, "DSx"},
														{Rfem, "Rfem"},
														{M2Cnv, "M2Cnv"},
														{SocCnv, "SocCnv"},
														{IVCam, "IVCam"},
														{DgfxCore, "DgfxCore"},
														{DgfxMem, "DgfxMem"},
														{DgfxMcp, "DgfxMcp"},
														{Cam2D, "Cam2D"},
														{WwanAnalog, "WwanAnalog"},
														{WwanDigital, "WwanDigital"},
														{WwanRfim, "WwanRfim"},
														{IdgfxCore, "IdgfxCore"},
														{Idgfx2, "Idgfx2"},
														{Nvdgx, "Nvdgx"},
														{Pcie, "Pcie"},
														{Vpu, "Vpu"},
														{Ipu, "Ipu"},
														{All, "All"},
														{Invalid, Constants::NotAvailableString}};

	std::string toString(DomainType::Type type)
	{
		auto name = names.find(type);
		if (name != names.end())
		{
			return name->second;
		}
		else
		{
			throw dptf_exception("DomainType::Type is invalid.");
		}
	}

	DomainType::Type fromString(const std::string& typeName)
	{
		for (const auto &name : names)
		{
			if (name.second == typeName)
			{
				return name.first;
			}
		}
		throw dptf_exception("DomainType::Type is invalid.");
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
	case ESIF_DOMAIN_TYPE_CAM2D:
		return DomainType::Cam2D;
	case ESIF_DOMAIN_TYPE_DGFXCORE:
		return DomainType::DgfxCore;
	case ESIF_DOMAIN_TYPE_DGFXMEM:
		return DomainType::DgfxMem;
	case ESIF_DOMAIN_TYPE_DGFXMCP:
		return DomainType::DgfxMcp;
	case ESIF_DOMAIN_TYPE_WWANANALOG:
		return DomainType::WwanAnalog;
	case ESIF_DOMAIN_TYPE_WWANDIGITAL:
		return DomainType::WwanDigital;
	case ESIF_DOMAIN_TYPE_WWANRFIM:
		return DomainType::WwanRfim;
	case ESIF_DOMAIN_TYPE_IDGFXCORE:
		return DomainType::IdgfxCore;
	case ESIF_DOMAIN_TYPE_IDGFX2:
		return DomainType::Idgfx2;
	case ESIF_DOMAIN_TYPE_NVDGX:
		return DomainType::Nvdgx;
	case ESIF_DOMAIN_TYPE_PCIE:
		return DomainType::Pcie;
	case ESIF_DOMAIN_TYPE_VPU:
		return DomainType::Vpu;
	case ESIF_DOMAIN_TYPE_IPU:
		return DomainType::Ipu;
	case ESIF_DOMAIN_TYPE_MANAGER:
		return DomainType::Manager;
	case ESIF_DOMAIN_TYPE_ALL:
		return DomainType::All;
	case ESIF_DOMAIN_TYPE_INVALID:
		return DomainType::Invalid;
	default:
		break;
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
	case DomainType::Cam2D:
		return ESIF_DOMAIN_TYPE_CAM2D;
	case DomainType::DgfxCore:
		return ESIF_DOMAIN_TYPE_DGFXCORE;
	case DomainType::DgfxMem:
		return ESIF_DOMAIN_TYPE_DGFXMEM;
	case DomainType::DgfxMcp:
		return ESIF_DOMAIN_TYPE_DGFXMCP;
	case DomainType::WwanAnalog:
		return ESIF_DOMAIN_TYPE_WWANANALOG;
	case DomainType::WwanDigital:
		return ESIF_DOMAIN_TYPE_WWANDIGITAL;
	case DomainType::WwanRfim:
		return ESIF_DOMAIN_TYPE_WWANRFIM;
	case DomainType::IdgfxCore:
		return ESIF_DOMAIN_TYPE_IDGFXCORE;
	case DomainType::Idgfx2:
		return ESIF_DOMAIN_TYPE_IDGFX2;
	case DomainType::Nvdgx:
		return ESIF_DOMAIN_TYPE_NVDGX;
	case DomainType::Pcie:
		return ESIF_DOMAIN_TYPE_PCIE;
	case DomainType::Vpu:
		return ESIF_DOMAIN_TYPE_VPU;
	case DomainType::Ipu:
		return ESIF_DOMAIN_TYPE_IPU;
	case DomainType::Manager:
		return ESIF_DOMAIN_TYPE_MANAGER;
	case DomainType::All:
		return ESIF_DOMAIN_TYPE_ALL;
	case DomainType::Invalid:
		return ESIF_DOMAIN_TYPE_INVALID;
	default:
		break;
	}

	throw dptf_exception("Received unknown Domain::Type");
}
