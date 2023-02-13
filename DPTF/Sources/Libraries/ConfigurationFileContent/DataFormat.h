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

#pragma once
#include <vector>

typedef struct DttConfigurationHeader_s
{
	unsigned short signature; // File Signature: 0xD07F
	unsigned short version; // File Format Version: 0, 1, 2
	unsigned short flags; // Global Payload Flags v2.  bit0: 0 - not compressed, 1 - compressed
	unsigned short payload_offset; // The file location of the payload
	unsigned int payload_checksum; // Simple checksum
	unsigned int payload_size; // Payload Size
} DttConfigurationHeaderV0;

namespace DttConfigurationHeaderFlagMasks
{
	static unsigned short compression = 0x0001;
}

namespace DttConfigurationHeaderSignature
{
	static unsigned short value = 0xD07F;
}

class DataFormat
{
public:
	static unsigned int calculateChecksum(const std::vector<unsigned char>& payload);
};

