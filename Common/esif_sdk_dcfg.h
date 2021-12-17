/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#include "esif_ccb.h"

#ifdef __cplusplus
extern "C" {
#endif

// DCFG Bitmask Options. Default Values must always be 0 for backwards compatibility
typedef union DCfgOptions_u {
	struct {
		UInt32 GenericUIAccessControl : 1;			// 0:0 - 0 = Enable [default], 1 = Disable access
		UInt32 Unused : 1;							// 1:1 - 0 = Unused bit [Formerly RestrictedUIAccessControl]
		UInt32 ShellAccessControl : 1;				// 2:2 - 0 = Enable [default], 1 = Disable access
		UInt32 EnvMonitoringReportControl : 1;		// 3:3 - 0 = Report is allowed [default], 1 = No environmental monitoring report to Microsoft
		UInt32 ThermalMitigationReportControl : 1;	// 4:4 - 0 = No mitigation report to Microsoft [default], 1 = Report is allowed
		UInt32 ThermalPolicyReportControl : 1;		// 5:5 - 0 = No thermal policy report to Microsoft [default], 1 = Report is allowed
		UInt32 TcsAccessControl : 1;				// 6:6 - 0 = Enable [default], 1 = Disable Telemetry Collection Service App
		UInt32 CstAccessControl : 1;				// 7:7 - 0 = Enable [default], 1 = Disable Context Sensing Technology App
		UInt32 DttAccessControl : 1;				// 8:8 - 0 = Enable [default], 1 = Disable Dynamic Tuning Technology App
		UInt32 Reserved : 23;						// 31:9  Reserved
	} opt;
	UInt32 asU32;
} DCfgOptions;

#ifdef __cplusplus
}
#endif
