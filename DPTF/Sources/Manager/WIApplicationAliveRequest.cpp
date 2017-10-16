/******************************************************************************
** Copyright (c) 2013-2017 Intel Corporation All Rights Reserved
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

#include "WIApplicationAliveRequest.h"
#include "EsifServicesInterface.h"

WIApplicationAliveRequest::WIApplicationAliveRequest(DptfManagerInterface* dptfManager)
	: WorkItem(dptfManager, FrameworkEvent::DptfAppAliveRequest)
{
}

WIApplicationAliveRequest::~WIApplicationAliveRequest(void)
{
}

void WIApplicationAliveRequest::execute(void)
{
	try
	{
		writeWorkItemStartingInfoMessage();
		getEsifServices()->primitiveExecuteSetAsUInt32(SET_APP_ALIVE_RESPONSE, 0);
	}
	catch (const std::exception& ex)
	{
		writeWorkItemWarningMessage(ex, "WIApplicationAliveRequest::execute");
	}
}
