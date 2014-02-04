/******************************************************************************
** Copyright (c) 2014 Intel Corporation All Rights Reserved
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

#include "WorkItem.h"
#include "esif_uf_app_iface.h"

class WIDptfGetStatus : public WorkItem
{
public:

    WIDptfGetStatus(DptfManager* dptfManager, const eAppStatusCommand command,
        const UInt32 appStatusIn, esif::EsifDataPtr appStatusOut, eEsifError* returnCode);
    virtual ~WIDptfGetStatus(void);

    virtual void execute(void) override final;

private:

    const eAppStatusCommand m_command;
    const UInt32 m_appStatusIn;
    esif::EsifDataPtr m_appStatusOut;
    eEsifError* m_returnCode;
};