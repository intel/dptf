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

#include "PolicyServicesMessageLogging.h"
#include "EsifServices.h"
#include "ManagerMessage.h"

PolicyServicesMessageLogging::PolicyServicesMessageLogging(DptfManager* dptfManager, UIntN policyIndex) :
    PolicyServices(dptfManager, policyIndex)
{
}

void PolicyServicesMessageLogging::writeMessageFatal(const DptfMessage& message)
{
    throwIfNotWorkItemThread();

    ManagerMessage updatedMessage = ManagerMessage(getDptfManager(), message);
    updatedMessage.setPolicyIndex(getPolicyIndex());

    getEsifServices()->writeMessageFatal(updatedMessage);
}

void PolicyServicesMessageLogging::writeMessageError(const DptfMessage& message)
{
    throwIfNotWorkItemThread();

    ManagerMessage updatedMessage = ManagerMessage(getDptfManager(), message);
    updatedMessage.setPolicyIndex(getPolicyIndex());

    getEsifServices()->writeMessageError(updatedMessage);
}

void PolicyServicesMessageLogging::writeMessageWarning(const DptfMessage& message)
{
    throwIfNotWorkItemThread();

    ManagerMessage updatedMessage = ManagerMessage(getDptfManager(), message);
    updatedMessage.setPolicyIndex(getPolicyIndex());

    getEsifServices()->writeMessageWarning(updatedMessage);
}

void PolicyServicesMessageLogging::writeMessageInfo(const DptfMessage& message)
{
    throwIfNotWorkItemThread();

    ManagerMessage updatedMessage = ManagerMessage(getDptfManager(), message);
    updatedMessage.setPolicyIndex(getPolicyIndex());

    getEsifServices()->writeMessageInfo(updatedMessage);
}

void PolicyServicesMessageLogging::writeMessageDebug(const DptfMessage& message)
{
    throwIfNotWorkItemThread();

    ManagerMessage updatedMessage = ManagerMessage(getDptfManager(), message);
    updatedMessage.setPolicyIndex(getPolicyIndex());

    getEsifServices()->writeMessageDebug(updatedMessage);
}