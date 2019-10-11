/******************************************************************************
** Copyright (c) 2013-2019 Intel Corporation All Rights Reserved
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
#include <string>
#include "DptfBuffer.h"

class dptf_export IFileIO
{
public:
	virtual ~IFileIO(void){};
	virtual void writeData(const std::string& filePath, const std::string& data) = 0;
	virtual void writeData(const std::string& filePath, const DptfBuffer& data) = 0;
	static Bool fileNameHasIllegalChars(const std::string& fileName);
	static std::string generatePathWithTrailingSeparator(const std::string& folderPath);
};

class dptf_export FileIO : public IFileIO
{
public:
	FileIO();
	virtual ~FileIO();
	virtual void writeData(const std::string& filePath, const std::string& data) override;
	virtual void writeData(const std::string& filePath, const DptfBuffer& data) override;

private:
	void throwIfFileNotOpened(const std::fstream& fp, const std::string& filePath);
};
