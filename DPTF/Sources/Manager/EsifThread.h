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

#include "Dptf.h"
#include "esif_uf_ccb_thread.h"

// Prototype for work_func_t
// void* (*work_func_t)(void*);

class EsifThread
{
public:

    // creates the thread
    EsifThread(work_func_t function, void* contextPtr);

    // destroys the thread
    ~EsifThread(void);

private:

    // hide the copy constructor and assignment operator.
    EsifThread(const EsifThread& rhs);
    EsifThread& operator=(const EsifThread& rhs);

    work_func_t m_function;
    void* m_argument;
    esif_thread_t m_thread;
};