/******************************************************************************
** Copyright (c) 2013-2020 Intel Corporation All Rights Reserved
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

#include "esif_sdk_data.h"
#include <string>

class TableObjectField final
{
public:
	TableObjectField(std::string name, std::string label, esif_data_type dataType)
	{
		m_fieldName = name;
		m_fieldLabel = label;
		m_fieldDataType = dataType;
	}

	~TableObjectField()
	{
	}

	std::string m_fieldName;
	std::string m_fieldLabel;
	esif_data_type m_fieldDataType;
};
