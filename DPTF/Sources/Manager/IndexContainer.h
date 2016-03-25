/******************************************************************************
** Copyright (c) 2013-2016 Intel Corporation All Rights Reserved
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
#include "EsifMutex.h"
#include "IndexContainerInterface.h"

class IndexContainer : public IndexContainerInterface
{
public:

    IndexContainer(UIntN initialCount);
    ~IndexContainer(void);

    virtual IndexStructPtr getIndexPtr(UIntN index) override;
    virtual UIntN getIndex(IndexStructPtr indexStructPtr) override;

private:

    // hide the copy constructor and assignment operator.
    IndexContainer(const IndexContainer& rhs);
    IndexContainer& operator=(const IndexContainer& rhs);

    EsifMutex m_mutex;

    std::vector<IndexStructPtr> m_vectorIndexStructPtr;
};