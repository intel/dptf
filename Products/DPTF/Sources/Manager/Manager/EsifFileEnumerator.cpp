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

#include "EsifFileEnumerator.h"
#include "esif_ccb_memory.h"
#include "DptfExceptions.h"

EsifFileEnumerator::EsifFileEnumerator(std::string path, std::string pattern) : m_path(path), m_pattern(pattern),
    m_fileHandle(INVALID_HANDLE_VALUE)
{
    esif_ccb_memset(m_file.filename, 0, MAX_PATH);
}

EsifFileEnumerator::~EsifFileEnumerator(void)
{
    if (m_fileHandle != INVALID_HANDLE_VALUE)
    {
        esif_ccb_file_enum_close(m_fileHandle);
    }
}

std::string EsifFileEnumerator::getFirstFile(void)
{
    esif_string path = const_cast<esif_string>(m_path.c_str());
    esif_string pattern = const_cast<esif_string>(m_pattern.c_str());

    m_fileHandle = esif_ccb_file_enum_first(path, pattern, &m_file);

    if (m_fileHandle == INVALID_HANDLE_VALUE)
    {
        throw dptf_exception("Error while trying to find first file.");
    }

    return std::string(m_file.filename);
}

std::string EsifFileEnumerator::getNextFile(void)
{
    if (m_fileHandle == INVALID_HANDLE_VALUE)
    {
        throw dptf_exception("File handle not initialized.");
    }

    esif_string pattern = const_cast<esif_string>(m_pattern.c_str());

    esif_ccb_file* rc = esif_ccb_file_enum_next(m_fileHandle, pattern, &m_file);

    if (rc == nullptr)
    {
        return std::string();
    }
    else
    {
        return std::string(m_file.filename);
    }
}