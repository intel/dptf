/******************************************************************************
** Copyright (c) 2013-2023 Intel Corporation All Rights Reserved
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

#include "DdrfChannelBandPackage.h"

namespace DdrfChannelBandPackage
{
	std::string toString(DdrfChannelBandPackage::WifiBand wifiGhzBandValue)
	{
		switch (wifiGhzBandValue)
		{
		case WIFI_BAND_UNSUPPORTED:
			return "WIFI_BAND_UNSUPPORTED";
		case WIFI_BAND_2_4:
			return "WIFI_BAND_2_4";
		case WIFI_BAND_5_2:
			return "WIFI_BAND_5_2";
		case WIFI_BAND_6:
			return "WIFI_BAND_6";
		case WIFI_BAND_MAX:
			return "WIFI_BAND_MAX";
		default:
			return "Unspecified Band Value";
		}
	}
}
