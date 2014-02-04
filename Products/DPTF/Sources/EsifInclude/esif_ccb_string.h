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

#ifndef _ESIF_CCB_STRING_H_
#define _ESIF_CCB_STRING_H_

#include "esif.h"
#include "esif_ccb.h"

/*******************************************************************************
** Kernel
*******************************************************************************/

#ifdef ESIF_ATTR_KERNEL

    static ESIF_INLINE int
    esif_acpi_get_strlen(const u16 type, const u32 is_unicode, u16 str_len)
    {
        if (is_unicode)
            return str_len / 2;  /* All Unicode shall contain null-terminated code */

    #ifdef ESIF_ATTR_OS_LINUX
        if (type == ACPI_TYPE_STRING)
            return str_len + 1;  /* Linux ascii doesn't have null-terminated byte, so append one */
        else
            return str_len;      /* Buffer type, length should be as it is */
    #endif

    #ifdef ESIF_ATTR_OS_WINDOWS
        UNREFERENCED_PARAMETER(type);
        return str_len;          /* Windows ascii ALWAYS contain null-terminated byte */
    #endif
    }

    static ESIF_INLINE int
    esif_ccb_is_unicode(const u32 acpi_method)
    {
        /*
        * 0) Rules applying to both Windows and Linux.
        * 1) Unicode only exists in ACPI buffer, but not in ACPI string.
        * 2) The only known unicode method is '_STR', need to convert it to ascii.
        * 3) Others such as GUID in 'IDSP' are binary (hex), no need to convert.
        */
        switch (acpi_method) {
        case 'RTS_':  /* 0x5254535f: _STR */
            return 1;
        default:
           return 0;
        }
    }

    static ESIF_INLINE void
    esif_ccb_uni2ascii(char *string_ptr, int buflen, u8* unicode_ptr, int unicode_size)
    {
    #ifdef ESIF_ATTR_OS_LINUX
        int i;
        __le16* le16unicode_ptr = (__le16*) unicode_ptr;

        if (buflen <= 0)        /* never happens, but... */
            return;
        --buflen;               /* space for null */

        for (i = 0; i < unicode_size; i++) {
            if (i >= buflen)
                break;
            string_ptr[i] = (char)(le16_to_cpu(le16unicode_ptr[i]));
        }
        string_ptr[i] = 0x00;
    #endif
    #ifdef ESIF_ATTR_OS_WINDOWS
        UNICODE_STRING us_val;
        ANSI_STRING ansi_val;

        UNREFERENCED_PARAMETER(unicode_size);

        if (buflen <= 0)
            return;

        RtlInitUnicodeString(&us_val, (PCWSTR) unicode_ptr);
        RtlUnicodeStringToAnsiString(&ansi_val, &us_val, TRUE /* Allocate ANSI String */);
        if (NULL == ansi_val.Buffer)
            return;

        buflen = min(ansi_val.Length, buflen);
        esif_ccb_memcpy(string_ptr, ansi_val.Buffer, buflen);
        string_ptr[buflen - 1];

        RtlFreeAnsiString(&ansi_val);
    #endif
    }

    #ifdef ESIF_ATTR_OS_WINDOWS
        #include <ntstrsafe.h>
        #define esif_ccb_sprintf(siz, str, fmt, ...)             RtlStringCbPrintfA((NTSTRSAFE_PSTR)str, (size_t) siz, fmt, __VA_ARGS__)
        #define esif_acpi_memcpy(out, in, len)                   esif_ccb_memcpy(out, &in, len)
        #define esif_acpi_uni2ascii(out, out_len, in, in_len)    esif_ccb_uni2ascii((char *)out, out_len, (u8 *)&in, in_len)
    #endif /* ESIF_ATTR_OS_WINDOWS */

    #ifdef ESIF_ATTR_OS_LINUX
        #define esif_ccb_sprintf(siz, str, fmt, arg...)          sprintf(str, fmt, ##arg)
        #define esif_acpi_memcpy(out, in, len)                   esif_ccb_memcpy(out, in, len)
        #define esif_acpi_uni2ascii(out, out_len, in, in_len)    esif_ccb_uni2ascii(out, out_len, in, in_len)
    #endif

#else

/*******************************************************************************
** User
*******************************************************************************/

#ifdef ESIF_ATTR_OS_WINDOWS
#define esif_ccb_sprintf(siz, str, fmt, ...)    sprintf_s(str, (size_t) siz, fmt, __VA_ARGS__)
#define esif_ccb_vsprintf(siz, str, fmt, ...)   vsnprintf_s(str, (size_t) siz, _TRUNCATE, fmt, __VA_ARGS__)
#define esif_ccb_sscanf(str, fmt, var, siz)     sscanf_s(str, fmt, var, siz)
#define esif_ccb_strtok(str, sep, ctxt)         strtok_s(str, sep, ctxt)
#define esif_ccb_strcmp(s1, s2)                 strcmp(s1, s2)
#define esif_ccb_stricmp(s1, s2)                _stricmp(s1, s2)
#define esif_ccb_strncmp(s1, s2, count)         strncmp(s1, s2, count)
#define esif_ccb_strnicmp(s1, s2, cnt)          _strnicmp(s1, s2, cnt)
#define esif_ccb_strstr(str, sub)               strstr(str, sub)
#define esif_ccb_strlen(str, siz)               strnlen_s(str, siz)
#define esif_ccb_wcslen(str, siz)               wcsnlen_s(str, siz)
#define esif_ccb_strdup_notrace(str)            _strdup(str)
#define esif_ccb_strupr(s, count)               _strupr_s(s, count)
#define esif_ccb_strlwr(s, count)               _strlwr_s(s, count)
#define esif_ccb_vscprintf(fmt, args)           _vscprintf(fmt, args)
#else
#include <ctype.h>
#include <stdarg.h>
#define esif_ccb_sprintf(siz, str, fmt, arg...) sprintf(str, fmt, ##arg)
#define esif_ccb_vsprintf(siz, str, fmt,arg...) vsprintf(str, fmt, ##arg)
#define esif_ccb_sscanf(str, fmt, var, siz)     sscanf(str, fmt, var)
#define esif_ccb_strtok(str, sep, ctxt)         strtok(str, sep)
#define esif_ccb_strcmp(s1, s2)                 strcmp(s1, s2)
#define esif_ccb_stricmp(s1, s2)                strcasecmp(s1, s2)
#define esif_ccb_strncmp(s1, s2, count)         strncmp(s1, s2, count)
#define esif_ccb_strnicmp(s1, s2, cnt)          strncasecmp(s1, s2, cnt)
#define esif_ccb_strstr(str, sub)               strstr(str, sub)
#define esif_ccb_strlen(str, siz)               strlen(str)
#define esif_ccb_strdup_notrace(str)            strdup(str)

// Linux _strupr_s() equlivalent
static ESIF_INLINE void esif_ccb_strupr(char *s, size_t count)
{
    for ( ; *s && count; s++, count--)
        *s = toupper(*s);
}
// Linux _strlwr_s() equlivalent
static ESIF_INLINE void esif_ccb_strlwr(char *s, size_t count)
{
    for ( ; s && *s && count; s++, count--)
        *s = tolower(*s);
}
// Linux _vscprintf() equivalient
static ESIF_INLINE int esif_ccb_vscprintf(const char *format, va_list args)
{
    int retval;
    va_list argcopy;
    va_copy(argcopy,args);
    retval = vsnprintf(0, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}
#endif

#ifdef ESIF_TRACE_MALLOC
static ESIF_INLINE char *esif_ccb_strdup_trace(char *str)
{
    char *mem_ptr = esif_ccb_strdup_notrace(str);
    if (NULL != mem_ptr) atomic_inc(&g_user_memstats.allocs);
    return mem_ptr;
}
# define esif_ccb_strdup(str)   esif_ccb_strdup_trace(str)
#else
# define esif_ccb_strdup(str)   esif_ccb_strdup_notrace(str)
#endif
#endif /* ESIF_ATTR_USER */


/*******************************************************************************
** Kernel/User Agnostic
*******************************************************************************/

#ifdef ESIF_ATTR_OS_WINDOWS
#define esif_ccb_strcpy(dst, src, siz) strncpy_s(dst, siz, src, _TRUNCATE)
#define esif_ccb_strcat(dst, src, siz) strcat_s(dst, siz, src)
#else
//#define esif_ccb_strcpy(dst, src, siz) { strncpy(dst, src, siz); if (siz) ((char*)dst)[(siz)-1]=0; }
// Linux strncpy_s equivalent
static ESIF_INLINE void esif_ccb_strcpy(char *dst, const char *src, size_t siz)
{
    strncpy(dst, src, siz);
    if (siz) dst[siz-1]=0;
}
#define esif_ccb_strcat(dst, src, size) strcat(dst, src)
#endif

#define esif_ccb_max(a,b)            ((a) >= (b) ? (a) : (b))
#define esif_ccb_min(a,b)            ((a) <= (b) ? (a) : (b))


/*******************************************************************************
** GUID - Requires 32 Byte Buffer
*******************************************************************************/
#ifdef ESIF_ATTR_USER
#define ESIF_GUID_PRINT_SIZE 64
static ESIF_INLINE esif_string esif_guid_print(esif_guid_t *guid, esif_string buf)
{
    u8 *ptr = (u8*)guid;
    esif_ccb_sprintf(64, buf, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X", 
        *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5), *(ptr+6), *(ptr+7) ,
        *(ptr+8), *(ptr+9), *(ptr+10), *(ptr+11), *(ptr+12), *(ptr+13), *(ptr+14), *(ptr+15)
        );
    return buf;
}
#endif

#endif /* _ESIF_CCB_STRING_H_ */