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

#include "EsifDataVoid.h"

EsifDataVoid::EsifDataVoid(void)
{
	m_esifData.type = esif_data_type::ESIF_DATA_VOID;
	m_esifData.buf_ptr = nullptr;
	m_esifData.buf_len = 0;
	m_esifData.data_len = 0;
}

EsifDataVoid::operator EsifDataPtr(void)
{
	return &m_esifData;
}
