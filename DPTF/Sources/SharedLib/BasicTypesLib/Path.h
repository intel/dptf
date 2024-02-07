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

#pragma once

#include "Dptf.h"

//
// represents OS configuration path
//
class Path final
{
public:
	Path() = default;
	Path(const std::string& basePath, const std::string& key);
	Path(const std::string& fullPath);

	std::string basePath;
	std::string key;
	std::string fullPath;

private:
	std::string extractBasePathIfValid(const std::string& fullPath);
	std::string extractKeyIfValid(const std::string& fullPath);
	void throwIfInvalidConfigPath(const std::string& configPath);
};