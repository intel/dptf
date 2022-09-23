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

#include "esif_ccb.h"
#include "esif_ccb_rc.h"
#include <string.h>
#include <stdlib.h>

// Current IPF SDK Version: Major.Minor.Revision where same Major.Minor compatible with other Revisions, but not newer Major.Minor
#define IPF_SDK_VERSION		"1.0.11100"

typedef u64 ipfsdk_version_t;	// IPF SDK Encoded Version Number

// IPF SDK Encoded Version Number Helper Macros
#define IPFSDK_VERSION(major, minor, revision)	((((ipfsdk_version_t)(major) & 0xffff) << 32) | (((ipfsdk_version_t)(minor) & 0xffff) << 16) | ((ipfsdk_version_t)(revision) & 0xffff))
#define IPFSDK_GETMAJOR(ver)					((u32)(((ver) & 0x0000ffff00000000) >> 32))
#define IPFSDK_GETMINOR(ver)					(((u32)(ver) & 0xffff0000) >> 16)
#define IPFSDK_GETRELEASE(ver)					((u32)(((ver) & 0x0000ffffffff0000) >> 16))
#define IPFSDK_GETREVISION(ver)					((u32)(ver) & 0x0000ffff)

// Convert an IPF SDK Version string to an Encoded Version Number that can be directly compared with another
static ESIF_INLINE ipfsdk_version_t IpfSdk_VersionFromString(const char *str) {
	ipfsdk_version_t ver = 0;
	if (str) {
		const char *dot = strchr(str, '.');
		const char *dotdot = (dot ? strchr(dot + 1, '.') : NULL);
		u32 major = atoi(str);
		u32 minor = (dot ? atoi(dot + 1) : 0);
		u32 revision = (dotdot ? atoi(dotdot + 1) : 0);
		ver = IPFSDK_VERSION(major, minor, revision);
	}
	return ver;
}
