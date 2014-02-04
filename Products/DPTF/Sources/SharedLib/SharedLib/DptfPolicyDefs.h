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

//
// The main unit of work done by policy engines. This is the piece of data that
// DPTF Framework creates and exposes to the Policy Engines. Internally, the
// framework may have more data stored for the work items. An example internal data
// could be policy context that identifies who created a PolicyEventPolicy type of
// work item.
//
// All entries within the POLICY_WORKITEM structure are considered read only to the
// policy engine. Policy Engine can set the member values when it creates the work
// item using ::CreateWorkItem interface function. Policy Engine can
// read the member values inside the Execute implementation when the work item is
// dispatched for execution by the policy.
//
typedef struct _PolicyWorkItem
{
    unsigned long eventCode;
    unsigned long param1;
    void* param2;
    unsigned long participantIndex;
} PolicyWorkItem;