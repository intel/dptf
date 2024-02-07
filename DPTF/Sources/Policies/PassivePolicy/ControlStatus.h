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
#include "DomainProxy.h"

// contains generic control status information.
// all controls report their status in the form of name, min, max, current.
class dptf_export ControlStatus
{
public:
	ControlStatus(const std::string& name, UIntN min, UIntN max, UIntN current);

	std::shared_ptr<XmlNode> getXml();

private:
	std::string m_name;
	UIntN m_min;
	UIntN m_max;
	UIntN m_current;
};
