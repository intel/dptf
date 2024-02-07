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
#include "esif_ccb_library.h"

class EsifLibrary
{
public:
	EsifLibrary(void);
	EsifLibrary(const std::string& fileName);
	~EsifLibrary(void);

	void setFileName(const std::string& fileName);

	void load(void);
	void unload(void);
	void dropSymLink(void);

	void* getFunctionPtr(std::string functionName);
	std::string getLibDirectory(void) const;

private:
	// hide the copy constructor and assignment operator.
	EsifLibrary(const EsifLibrary& rhs);
	EsifLibrary& operator=(const EsifLibrary& rhs);

	std::string m_fileName;
	Bool m_libraryLoaded;
	esif_lib_t m_library;
};
