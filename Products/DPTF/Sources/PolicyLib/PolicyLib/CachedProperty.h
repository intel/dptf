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
#include "PolicyServicesInterfaceContainer.h"

// base class for properties that desire caching.  maintains cache validity and provides a common interface for 
// refreshing the data.
class dptf_export CachedProperty
{
public:

    CachedProperty();
    ~CachedProperty();

    void refresh();
    void invalidate();

protected:

    Bool isCacheValid();
    virtual void refreshData() = 0;

private:

    Bool m_cacheIsValid;
};