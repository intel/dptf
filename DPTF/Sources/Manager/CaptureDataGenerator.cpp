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

#include "CaptureDataGenerator.h"
#include "EsifServicesInterface.h"
#include <string>

using namespace std;

CaptureDataGenerator::CaptureDataGenerator(DptfManagerInterface* dptfManager)
	: m_dptfManager(dptfManager)
{
}

void CaptureDataGenerator::logMessage(const std::string& message) const
{
	m_dptfManager->getEsifServices()->writeMessageWarning(
		"DTT Capture Command: "s + std::string{message}, MessageCategory::Default);
}
