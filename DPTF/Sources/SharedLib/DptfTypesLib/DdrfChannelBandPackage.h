/******************************************************************************
** Copyright (c) 2013-2022 Intel Corporation All Rights Reserved
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

#include "Dptf.h"

#define DVFS_POINTS_MAX   4
#define WIFI_BANDS_MAX    3
#define WIFI_CHANNELS_MAX 15

namespace DdrfChannelBandPackage {
	typedef enum WifiBand_e {
		WIFI_BAND_UNSUPPORTED = 0,
		WIFI_BAND_2_4,
		WIFI_BAND_5_2,
		WIFI_BAND_6,
		WIFI_BAND_MAX,
	}WifiBand, * WifiBandPtr;

	typedef struct WifiBandChannelInfo_s {
		WifiBand band;
		UInt32   numberOfChannels;
		UInt32   channels[WIFI_CHANNELS_MAX];
	} WifiBandChannelInfo, * WifiBandChannelInfoPtr;

	typedef struct DdrDvfsPointInfo_s {
		UInt32                frequencyRate;
		UInt32                numberOfBands;
		WifiBandChannelInfo   bandChannelInfo[WIFI_BANDS_MAX];
	} DdrDvfsPointInfo, * DdrDvfsPointInfoPtr;

	typedef struct WifiDdrRfi_s {
		UInt32                numberOfDvfsPoints;
		DdrDvfsPointInfo      dvfsPointsInfo[DVFS_POINTS_MAX];
	} WifiRfiDdr, * WifiDdrRfiPtr;

	std::string toString(DdrfChannelBandPackage::WifiBand wifiGhzBandValue);
}
