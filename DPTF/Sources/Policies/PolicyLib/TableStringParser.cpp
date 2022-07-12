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

#include "TableStringParser.h"

using namespace std;

std::string TableStringParser::getDeviceString(esif_data_variant& device)
{
	std::string deviceString(
		reinterpret_cast<const char*>(&(device)) + sizeof(union esif_data_variant), device.string.length);

	return deviceString;
}

std::string TableStringParser::getString(esif_data_variant& device, UInt32 length)
{
	std::string deviceString(
		reinterpret_cast<const char*>(&(device)) + sizeof(union esif_data_variant), length);

	return deviceString;
}
