/******************************************************************************
** Copyright (c) 2013-2021 Intel Corporation All Rights Reserved
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

#ifndef _ESIF_UF_SYFS_LIN_
#define _ESIF_UF_SYFS_LIN_

#ifdef __cplusplus
extern "C" {
#endif

int SysfsSetString(const char *path, const char *filename, char *val);
int SysfsGetInt64Direct(int fd, Int64 *p64);
int SysfsGetString(char *path, char *filename, char *str, size_t buf_len);
int SysfsGetStringMultiline(const char *path, const char *filename, char *str);
int SysfsGetInt64(const char *path, const char *filename, Int64 *p64);
int SysfsSetInt64(char *path, char *filename, Int64 val);
eEsifError SysfsGetInt(const char *path, const char *filename, int *pInt);
eEsifError SysfsGetIntDirect(int fd, int *pInt);
eEsifError SysfsGetFloat(const char *path, const char *filename, float *pFloat);
eEsifError SysfsGetFileSize(const char *path, const char *fileName, size_t *fileSize);
eEsifError SysfsGetBinaryData(const char *path, const char *fileName, UInt8 *buffer, size_t bufferLength);

#define IIO_STR_LEN 24
#define MAX_SYSFS_FILENAME 256
#define MAX_SYSFS_SUFFIX 26
#define MAX_SYSFS_PATH (MAX_SYSFS_FILENAME * 2 + MAX_SYSFS_SUFFIX)
#define MAX_STR_LINE_LEN 64
#define MAX_SYSFS_STRING (4 * 1024)
#define MIN_INT64	((Int64) 0x8000000000000000)
#define MAX_INT64	((Int64) 0x7FFFFFFFFFFFFFFF)

#ifdef __cplusplus
}
#endif

#endif	// _ESIF_UF_SYSFS_LIN_

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

