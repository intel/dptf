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

#include "EsifLibrary.h"
#include "Dptf.h"

EsifLibrary::EsifLibrary(void) :
    m_fileName(""), m_libraryLoaded(false), m_library(nullptr)
{
}

EsifLibrary::EsifLibrary(const std::string& fileName) :
    m_fileName(fileName), m_libraryLoaded(false), m_library(nullptr)
{
}

EsifLibrary::~EsifLibrary(void)
{
    if (m_libraryLoaded == true)
    {
        esif_ccb_library_unload(m_library);
        m_libraryLoaded = false;
    }
}

void EsifLibrary::setFileName(const std::string& fileName)
{
    if (m_libraryLoaded == true)
    {
        throw dptf_exception("Attempted to set file name while library loaded.");
    }

    m_fileName = fileName;
}

void EsifLibrary::load(void)
{
    if (m_libraryLoaded == false)
    {
        esif_string esifString = const_cast<esif_string>(m_fileName.c_str());
        m_library = esif_ccb_library_load(esifString);

        if (m_library == nullptr)
        {
            std::stringstream message;
            message << "Library failed to load: " << m_fileName;
            throw dptf_exception(message.str());
        }

        m_libraryLoaded = true;
    }
}

void EsifLibrary::unload(void)
{
    if (m_libraryLoaded == true)
    {
        esif_ccb_library_unload(m_library);
        m_libraryLoaded = false;
    }
}

void* EsifLibrary::getFunctionPtr(std::string functionName)
{
    if (m_libraryLoaded == false)
    {
        throw dptf_exception("Library not loaded.");
    }

    esif_string esifString = const_cast<esif_string>(functionName.c_str());

    void* funcPtr = esif_ccb_library_get_func(m_library, esifString);
    if (funcPtr == nullptr)
    {
        std::stringstream message;
        message << "Could not get pointer to function: " << functionName;
        throw dptf_exception(message.str());
    }

    return funcPtr;
}