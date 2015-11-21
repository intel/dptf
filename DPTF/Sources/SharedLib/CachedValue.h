/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
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

template <typename T>
class dptf_export CachedValue
{
public:

    CachedValue();
    CachedValue(const T& value);
    ~CachedValue();

    Bool isValid() const;
    Bool isInvalid() const;
    const T& get() const;
    void set(const T& value);
    void invalidate();

private:

    Bool m_valid;
    T m_value;
};

template <typename T>
CachedValue<T>::CachedValue()
    : m_valid(false)
{
}

template <typename T>
CachedValue<T>::CachedValue(const T& value)
    : m_valid(true), m_value(value)
{
}

template <typename T>
CachedValue<T>::~CachedValue()
{
}

template <typename T>
void CachedValue<T>::invalidate()
{
    m_valid = false;
}

template <typename T>
void CachedValue<T>::set(const T& value)
{
    m_value = value;
    m_valid = true;
}

template <typename T>
const T& CachedValue<T>::get() const
{
    if (isInvalid())
    {
        throw dptf_exception("Cached value is not valid.");
    }
    return m_value;
}

template <typename T>
Bool CachedValue<T>::isValid() const
{
    return m_valid;
}

template <typename T>
Bool CachedValue<T>::isInvalid() const
{
    return !m_valid;
}