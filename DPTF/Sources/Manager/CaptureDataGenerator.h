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
#include "DptfManagerInterface.h"
#include "XmlNode.h"

class dptf_export CaptureDataGenerator
{
public:
	CaptureDataGenerator(DptfManagerInterface* dptfManager);
	virtual ~CaptureDataGenerator() = default;
	virtual std::shared_ptr<XmlNode> generate() const = 0;

protected:
	virtual void logMessage(const std::string& message) const;
	DptfManagerInterface* m_dptfManager;
};
