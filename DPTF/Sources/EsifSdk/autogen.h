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

#ifndef _ESIF_AUTOGEN_H_
#define _ESIF_AUTOGEN_H_

#include "esif.h"
#include <string.h>

/*
** Vendor Strings
*/
static ESIF_INLINE esif_string esif_vendor_str(u32 vendor_id)
{
#define CREATE_VENDOR(v, vs) case v: str = (esif_string) vs; break;
esif_string str = (esif_string) ESIF_NOT_AVAILABLE;
switch(vendor_id) {
CREATE_VENDOR(0x8086, "Intel Corp")
}
return str;
}

///*
//** PCI Devices
//*/
//
//#define ENUM_ESIF_PCI_DEVICE_ID(ENUM) \
//    ENUM##_VAL(ESIF_PCI_DEVICE_ID_SNB, 0x0103) \
//    ENUM##_VAL(ESIF_PCI_DEVICE_ID_IVB, 0x0153) \
//    ENUM##_VAL(ESIF_PCI_DEVICE_ID_HSW_ULT, 0x0a03) \
//    ENUM##_VAL(ESIF_PCI_DEVICE_ID_HSW, 0x0c03) \
//    ENUM##_VAL(ESIF_PCI_DEVICE_ID_BDW, 0x1603) \
//    ENUM##_VAL(ESIF_PCI_DEVICE_ID_SKL, 0x1903) \
//    ENUM##_VAL(ESIF_PCI_DEVICE_ID_CPT, 0x1c24) \
//    ENUM##_VAL(ESIF_PCI_DEVICE_ID_PPT, 0x1e24) \
//    ENUM##_VAL(ESIF_PCI_DEVICE_ID_LPT, 0x8c24) \
//    ENUM##_VAL(ESIF_PCI_DEVICE_ID_LPT_LP, 0x9c24) \
//    ENUM##_VAL(ESIF_PCI_DEVICE_ID_WCP, 0x9ca4) \
//
//enum esif_pci_device_id {
//#ifdef ESIF_ATTR_KERNEL
//    ESIF_PCI_DEVICE_ID_SNB     = 0x0103,
//    ESIF_PCI_DEVICE_ID_IVB     = 0x0153,
//    ESIF_PCI_DEVICE_ID_HSW_ULT = 0x0a03,
//    ESIF_PCI_DEVICE_ID_HSW     = 0x0c03,
//    ESIF_PCI_DEVICE_ID_BDW     = 0x1603,
//    ESIF_PCI_DEVICE_ID_SKL     = 0x1903,
//    ESIF_PCI_DEVICE_ID_CPT     = 0x1c24,
//    ESIF_PCI_DEVICE_ID_PPT     = 0x1e24,
//    ESIF_PCI_DEVICE_ID_LPT     = 0x8c24,
//    ESIF_PCI_DEVICE_ID_LPT_LP  = 0x9c24,
//    ESIF_PCI_DEVICE_ID_WCP     = 0x9ca4,
//#else
//    ENUM_ESIF_PCI_DEVICE_ID(ENUMDECL)
//#endif
//};
//
//// Implement these if they are needed
//extern enum esif_pci_device_id esif_pci_device_id_string2enum(esif_string str);
//extern esif_string esif_pci_device_id_enum2string(enum esif_pci_device_id type);
//
//
//static ESIF_INLINE esif_string esif_device_str(u32 device_id)
//{
//#define CREATE_DEVICE(d, ds) case d: str = (esif_string) ds; break;
//esif_string str = (esif_string) ESIF_NOT_AVAILABLE;
//switch(device_id) {
//    CREATE_DEVICE(ESIF_PCI_DEVICE_ID_SNB, "DPTF Participant for 2nd Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)")
//    CREATE_DEVICE(ESIF_PCI_DEVICE_ID_IVB, "DPTF Participant for 3nd Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)")
//    CREATE_DEVICE(ESIF_PCI_DEVICE_ID_HSW_ULT, "DPTF Participant for 4th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)")
//    CREATE_DEVICE(ESIF_PCI_DEVICE_ID_HSW, "DPTF Participant for 4th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)")
//    CREATE_DEVICE(ESIF_PCI_DEVICE_ID_BDW, "DPTF Participant for 5th Generation Intel Core i7/i5/i3 Mobile Processors(DPTF CPU)")
//    CREATE_DEVICE(ESIF_PCI_DEVICE_ID_CPT, "Cougar Point(DPTF PCH)")
//    CREATE_DEVICE(ESIF_PCI_DEVICE_ID_PPT, "Panther Point(DPTF PCH)")
//    CREATE_DEVICE(ESIF_PCI_DEVICE_ID_LPT, "Lynx Point(DPTF PCH)")
//    CREATE_DEVICE(ESIF_PCI_DEVICE_ID_LPT_LP, "Lynx Point Low Power(DPTF PCH)")
//    CREATE_DEVICE(ESIF_PCI_DEVICE_ID_WCP, "Wild Cat Point (DPTF PCH)")
//}
//return str;
//}
//
//#ifdef ESIF_ATTR_OS_LINUX_DRIVER
//const struct pci_device_id esif_pci_cpu_ids[] = {
//{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_SNB) },
//{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_IVB) },
//{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_HSW_ULT) },
//{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_HSW) },
//{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_BDW) },
//{ 0, }
//};
//
//const struct pci_device_id esif_pci_pch_ids[] = {
//{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_CPT) },
//{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_PPT) },
//{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_LPT) },
//{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_LPT_LP) },
//{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, ESIF_PCI_DEVICE_ID_WCP) },
//{ 0, }
//};
//#endif
//
//static ESIF_INLINE esif_string esif_pci_class_str(u8 class_id)
//{
//#define CREATE_PCI_CLASS(c, cs) case c: str = (esif_string) cs; break;
//esif_string str = (esif_string) ESIF_NOT_AVAILABLE;
//switch(class_id) {
//    CREATE_PCI_CLASS(0x00, "NA")
//    CREATE_PCI_CLASS(0x01, "Mass Storage Controller")
//    CREATE_PCI_CLASS(0x02, "Network Controller")
//    CREATE_PCI_CLASS(0x03, "Display Controller")
//    CREATE_PCI_CLASS(0x04, "Multimedia Device")
//    CREATE_PCI_CLASS(0x05, "Memory Controller")
//    CREATE_PCI_CLASS(0x06, "Bridge Device")
//    CREATE_PCI_CLASS(0x07, "Simple Communications Controller")
//    CREATE_PCI_CLASS(0x08, "Base System Peripherals")
//    CREATE_PCI_CLASS(0x09, "Input Devices")
//    CREATE_PCI_CLASS(0x0a, "Docking Stations")
//    CREATE_PCI_CLASS(0x0b, "Processors")
//    CREATE_PCI_CLASS(0x0c, "Serial Bus Controllers")
//    CREATE_PCI_CLASS(0x0d, "Wireless Controllers")
//    CREATE_PCI_CLASS(0x0e, "Intelligent I/O Controllers")
//    CREATE_PCI_CLASS(0x0f, "Satelite Communication Controllers")
//    CREATE_PCI_CLASS(0x10, "Encryption/Decryption Controllers")
//    CREATE_PCI_CLASS(0x11, "Data Acquisition and Signal Processing Controllers")
//    CREATE_PCI_CLASS(0xff, "Misc")
//}
//return str;
//}
//
///*
//** ACPI Devices
//*/
//
//#define ESIF_ACPI_DEVICE_INT3400 "INT3400"
//#define ESIF_ACPI_DEVICE_INT3401 "INT3401"
//#define ESIF_ACPI_DEVICE_INT3402 "INT3402"
//#define ESIF_ACPI_DEVICE_INT3403 "INT3403"
//#define ESIF_ACPI_DEVICE_INT3404 "INT3404"
//#define ESIF_ACPI_DEVICE_INT3405 "INT3405"
//#define ESIF_ACPI_DEVICE_INT3406 "INT3406"
//#define ESIF_ACPI_DEVICE_INT3407 "INT3407"
//#define ESIF_ACPI_DEVICE_INT3408 "INT3408"
//#define ESIF_ACPI_DEVICE_INT3409 "INT3409"
//#define ESIF_ACPI_DEVICE_INT340A "INT340A"
//#define ESIF_ACPI_DEVICE_INT340B "INT340B"
//
//static ESIF_INLINE esif_string esif_acpi_device_str(esif_string acpi_device)
//{
//#define CREATE_ACPI_DEVICE(d, ds) if(!strncmp(acpi_device, d, 7)) str = (esif_string) ds;
//esif_string str = (esif_string) ESIF_NOT_AVAILABLE;
//if(NULL == acpi_device)
//  return str;
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT3400, "DPTF Zone")
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT3401, "DPTF Processor Device")
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT3402, "DPTF Memory Device")
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT3403, "DPTF Generic Device")
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT3404, "DPTF Fan Device")
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT3405, "DPTF Depricated")
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT3406, "DPTF Display Device")
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT3407, "DPTF Charger Device")
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT3408, "DPTF Wireless Device")
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT3409, "DPTF Ambient Temperature Sensor")
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT340A, "DPTF Storage Device")
//CREATE_ACPI_DEVICE(ESIF_ACPI_DEVICE_INT340B, "DPTF Perceptual Computing Camera")
//return str;
//}
//
//#ifdef ESIF_ATTR_OS_LINUX_DRIVER
//const struct acpi_device_id esif_acpi_ids[] = {
//{ ESIF_ACPI_DEVICE_INT3400, 0 },
//{ ESIF_ACPI_DEVICE_INT3401, 0 },
//{ ESIF_ACPI_DEVICE_INT3402, 0 },
//{ ESIF_ACPI_DEVICE_INT3403, 0 },
//{ ESIF_ACPI_DEVICE_INT3404, 0 },
//{ ESIF_ACPI_DEVICE_INT3405, 0 },
//{ ESIF_ACPI_DEVICE_INT3406, 0 },
//{ ESIF_ACPI_DEVICE_INT3407, 0 },
//{ ESIF_ACPI_DEVICE_INT3408, 0 },
//{ ESIF_ACPI_DEVICE_INT3409, 0 },
//{ ESIF_ACPI_DEVICE_INT340A, 0 },
//{ ESIF_ACPI_DEVICE_INT340B, 0 },
//{ "", 0 },
//};
//#endif

//#define ESIF_PRODUCT "Dynamic Platform Thermal Framework"
//#define ESIF_COPYRIGHT "(c) 2012 - 2013 Intel Corporation"
//#define ESIF_COMPANY "Intel Corporation"
//#define ESIF_LOWER_FRAMEWORK "ESIF Lower Framework"
//#define ESIF_UPPER_FRAMEWORK "ESIF Upper Framework"
//#define ESIF_AUTHOR "doug.hegge@intel.com"
//
//#define ESIF_PARTICIPANT_DPTF_NAME "DPTFZ"
//#define ESIF_PARTICIPANT_DPTF_DESC "DPTF Zone"
//#define ESIF_PARTICIPANT_DPTF_CLASS_GUID {0x23, 0xa9, 0xe1, 0x5f, 0x0b, 0xa4, 0x46, 0x9c, 0xb1, 0x6f, 0x1c, 0x46, 0x79, 0x75, 0x4f, 0x80 }
//#define ESIF_PARTICIPANT_ACPI_NAME "ACPI"
//#define ESIF_PARTICIPANT_ACPI_DESC "DPTF ACPI Device"
//#define ESIF_PARTICIPANT_ACPI_CLASS_GUID {0xd9, 0xd5, 0xbe, 0x64, 0x3c, 0xe2, 0x49, 0x27, 0x95, 0xa3, 0xee, 0xe4, 0xc, 0x9a, 0x58, 0x3b}
//#define ESIF_PARTICIPANT_CPU_NAME "TCPU"
//#define ESIF_PARTICIPANT_CPU_DESC "DPTF CPU Device"
//#define ESIF_PARTICIPANT_CPU_CLASS_GUID {0x53, 0x4a, 0x09, 0x8f, 0x5e, 0x42, 0x4c, 0x64, 0xbe, 0xb3, 0x91, 0x7a, 0xb3, 0x7c, 0x5d, 0xa5}
//#define ESIF_PARTICIPANT_PCH_NAME "TPCH"
//#define ESIF_PARTICIPANT_PCH_DESC "DPTF PCH Device"
//#define ESIF_PARTICIPANT_PCH_CLASS_GUID {0x33, 0xab, 0xb9, 0xb2, 0xe6, 0x86, 0x43, 0xb8, 0xb0, 0xdf, 0x3e, 0x91, 0x53, 0x90, 0xcb, 0xe5}
//#define ESIF_PARTICIPANT_PLAT_NAME "PLAT"
//#define ESIF_PARTICIPANT_PLAT_DESC "DPTF Platform Device"
//#define ESIF_PARTICIPANT_PLAT_CLASS_GUID {0xd4, 0x08, 0x04, 0xf4, 0xdc, 0x73, 0x4b, 0x33, 0xb2, 0x8c, 0x19, 0x5a, 0xe9, 0x8f, 0x29, 0xe}
//
//#define ENUM_ESIF_ALGORITHM_TYPE(ENUM) \
//    ENUM##_VAL(ESIF_ALGORITHM_TYPE_POWER_DECIW,0) \
//    ENUM##_VAL(ESIF_ALGORITHM_TYPE_POWER_MILLIW,1) \
//    ENUM##_VAL(ESIF_ALGORITHM_TYPE_POWER_NONE,2) \
//    ENUM##_VAL(ESIF_ALGORITHM_TYPE_POWER_OCTAW,3) \
//    ENUM##_VAL(ESIF_ALGORITHM_TYPE_TEMP_50DEGOFF,4) \
//    ENUM##_VAL(ESIF_ALGORITHM_TYPE_TEMP_DECIK,5) \
//    ENUM##_VAL(ESIF_ALGORITHM_TYPE_TEMP_NONE,6) \
//    ENUM##_VAL(ESIF_ALGORITHM_TYPE_TEMP_TJMAX,7) \
//    ENUM##_VAL(ESIF_ALGORITHM_TYPE_TIMER_NONE,8) \
//    ENUM##_VAL(ESIF_ALGORITHM_TYPE_TEMP_MILLIC,9) \
//
//enum esif_algorithm_type {
//#ifdef ESIF_ATTR_KERNEL
//    ESIF_ALGORITHM_TYPE_POWER_DECIW=0,
//    ESIF_ALGORITHM_TYPE_POWER_MILLIW=1,
//    ESIF_ALGORITHM_TYPE_POWER_NONE=2,
//    ESIF_ALGORITHM_TYPE_POWER_OCTAW=3,
//    ESIF_ALGORITHM_TYPE_TEMP_50DEGOFF=4,
//    ESIF_ALGORITHM_TYPE_TEMP_DECIK=5,
//    ESIF_ALGORITHM_TYPE_TEMP_NONE=6,
//    ESIF_ALGORITHM_TYPE_TEMP_TJMAX=7,
//    ESIF_ALGORITHM_TYPE_TIMER_NONE=8,
//    ESIF_ALGORITHM_TYPE_TEMP_MILLIC=9
//#else
//    ENUM_ESIF_ALGORITHM_TYPE(ENUMDECL)
//#endif
//};
//
//// Implement these if they are needed
//extern enum esif_algorithm_type esif_algorithm_type_string2enum(esif_string str);
//extern esif_string esif_algorithm_type_enum2string(enum esif_algorithm_type type);
//
//static ESIF_INLINE esif_string esif_algorithm_type_str(enum esif_algorithm_type type)
//{
//#define CREATE_ALGORITHM_TYPE(at) case at: str = (esif_string) #at; break;
//#define CREATE_ALGORITHM_TYPE_VAL(at,VAL) case at: str = (esif_string) #at; break;
//    esif_string str = (esif_string) ESIF_NOT_AVAILABLE;
//    switch(type) {
//#ifdef ESIF_ATTR_KERNEL
//        CREATE_ALGORITHM_TYPE(ESIF_ALGORITHM_TYPE_POWER_DECIW)
//        CREATE_ALGORITHM_TYPE(ESIF_ALGORITHM_TYPE_POWER_MILLIW)
//        CREATE_ALGORITHM_TYPE(ESIF_ALGORITHM_TYPE_POWER_NONE)
//        CREATE_ALGORITHM_TYPE(ESIF_ALGORITHM_TYPE_POWER_OCTAW)
//        CREATE_ALGORITHM_TYPE(ESIF_ALGORITHM_TYPE_TEMP_50DEGOFF)
//        CREATE_ALGORITHM_TYPE(ESIF_ALGORITHM_TYPE_TEMP_DECIK)
//        CREATE_ALGORITHM_TYPE(ESIF_ALGORITHM_TYPE_TEMP_NONE)
//        CREATE_ALGORITHM_TYPE(ESIF_ALGORITHM_TYPE_TEMP_TJMAX)
//        CREATE_ALGORITHM_TYPE(ESIF_ALGORITHM_TYPE_TIMER_NONE)
//        CREATE_ALGORITHM_TYPE(ESIF_ALGORITHM_TYPE_TEMP_MILLIC)
//#else
//        ENUM_ESIF_ALGORITHM_TYPE(CREATE_ALGORITHM_TYPE)
//#endif
//    }
//    return str;
//}
//
////#ifdef ESIF_ATTR_USER
////typedef enum esif_action_type eEsifActionType;
////#endif
//
///* Todo Move to Autogen.h */
//enum esif_event_group {
//    ESIF_EVENT_GROUP_DPTF   = 0,
//    ESIF_EVENT_GROUP_POWER  = 1,
//    ESIF_EVENT_GROUP_SENSOR = 2,
//    ESIF_EVENT_GROUP_ACPI   = 3,
//    ESIF_EVENT_GROUP_CODE   = 4
//};
//
///* Enumeration String */
//static ESIF_INLINE esif_string
//    esif_event_group_enum_str(enum esif_event_group group)
//{
//#define CREATE_EV_GROUP_ENUM(eg) case eg: str = #eg; break;
//    esif_string str = ESIF_NOT_AVAILABLE;
//    switch(group) {
//        CREATE_EV_GROUP_ENUM(ESIF_EVENT_GROUP_DPTF)
//        CREATE_EV_GROUP_ENUM(ESIF_EVENT_GROUP_POWER)
//        CREATE_EV_GROUP_ENUM(ESIF_EVENT_GROUP_SENSOR)
//        CREATE_EV_GROUP_ENUM(ESIF_EVENT_GROUP_ACPI)
//        CREATE_EV_GROUP_ENUM(ESIF_EVENT_GROUP_CODE)
//    }
//    return str;
//}

/*
 * Event Name:  CONNECTED_STANDBY_ENTRY
 * Event GUID:  FD34F756-F7B6-47DD-B3D5-0011A34E4337
 * Event Desc:  Enter connected standby
 */
#define CONNECTED_STANDBY_ENTRY {0xFD, 0x34, 0xF7, 0x56, 0xF7, 0xB6, 0x47, 0xDD, \
    0xB3, 0xD5, 0x00, 0x11, 0xA3, 0x4E, 0x43, 0x37}

/*
 * Event Name:  CONNECTED_STANDBY_EXIT
 * Event GUID:  9604508D-F4AA-4716-83D9-6EE951EBBEA9
 * Event Desc:  Exit connected standby
 */
#define CONNECTED_STANDBY_EXIT {0x96, 0x04, 0x50, 0x8D, 0xF4, 0xAA, 0x47, 0x16, \
    0x83, 0xD9, 0x6E, 0xE9, 0x51, 0xEB, 0xBE, 0xA9}

/*
 * Event Name:  ACTIVE_RELATIONSHIP_CHANGED
 * Event GUID:  C7C5FC88-8AAC-42C2-8B51-21777033E75D
 * Event Desc:  Active relationship table changed
 */
#define ACTIVE_RELATIONSHIP_CHANGED {0xC7, 0xC5, 0xFC, 0x88, 0x8A, 0xAC, 0x42, \
    0xC2, 0x8B, 0x51, 0x21, 0x77, 0x70, 0x33, \
    0xE7, 0x5D}

/*
 * Event Name:  THERMAL_RELATIONSHIP_CHANGED
 * Event GUID:  7E99E90E-0A22-4EEC-AD6C-908DEB21E9A9
 * Event Desc:  Thermal relationship table changed
 */
#define THERMAL_RELATIONSHIP_CHANGED {0x7E, 0x99, 0xE9, 0x0E, 0x0A, 0x22, 0x4E, \
    0xEC, 0xAD, 0x6C, 0x90, 0x8D, 0xEB, 0x21, \
    0xE9, 0xA9}

/*
 * Event Name:  FOREGROUND_CHANGED
 * Event GUID:  88E419E3-609B-4BDA-9A17-83DE899831FD
 * Event Desc:  Foreground application changed
 */
#define FOREGROUND_CHANGED {0x88, 0xE4, 0x19, 0xE3, 0x60, 0x9B, 0x4B, 0xDA, \
    0x9A, 0x17, 0x83, 0xDE, 0x89, 0x98, 0x31, 0xFD}

/*
 * Event Name:  SUSPEND
 * Event GUID:  547F7465-D98A-40FD-AC12-E6D20F7B091B
 * Event Desc:  Suspend Upper Framework
 */
#define SUSPEND {0x54, 0x7F, 0x74, 0x65, 0xD9, 0x8A, 0x40, 0xFD, 0xAC, 0x12, \
    0xE6, 0xD2, 0x0F, 0x7B, 0x09, 0x1B}

/*
 * Event Name:  RESUME
 * Event GUID:  AB3E045F-6B51-4EC5-9330-EAB70836F02F
 * Event Desc:  Resume Upper Framework
 */
#define RESUME {0xAB, 0x3E, 0x04, 0x5F, 0x6B, 0x51, 0x4E, 0xC5, 0x93, 0x30, \
    0xEA, 0xB7, 0x08, 0x36, 0xF0, 0x2F}

/*
 * Event Name:  CTDP_CAPABILITY_CHANGED
 * Event GUID:  68D16E98-2C89-4A3D-95C7-5DEEAA4FD73F
 * Event Desc:  Config TDP Capability changed (Configurable TDP)
 */
#define CTDP_CAPABILITY_CHANGED {0x68, 0xD1, 0x6E, 0x98, 0x2C, 0x89, 0x4A, 0x3D, \
    0x95, 0xC7, 0x5D, 0xEE, 0xAA, 0x4F, 0xD7, 0x3F}

/*
 * Event Name:  CORE_CAPABILITY_CHANGED
 * Event GUID:  8487D740-62F7-4030-BE1A-C201377E0C18
 * Event Desc:  For future use
 */
#define CORE_CAPABILITY_CHANGED {0x84, 0x87, 0xD7, 0x40, 0x62, 0xF7, 0x40, 0x30, \
    0xBE, 0x1A, 0xC2, 0x01, 0x37, 0x7E, 0x0C, 0x18}

/*
 * Event Name:  DISPLAY_CAPABILITY_CHANGED
 * Event GUID:  F1CDA338-0F3C-4F8D-A1D9-8033E672F672
 * Event Desc:  Display control upper/lower limits changed.
 */
#define DISPLAY_CAPABILITY_CHANGED {0xF1, 0xCD, 0xA3, 0x38, 0x0F, 0x3C, 0x4F, \
    0x8D, 0xA1, 0xD9, 0x80, 0x33, 0xE6, 0x72, \
    0xF6, 0x72}

/*
 * Event Name:  DISPLAY_STATUS_CHANGED
 * Event GUID:  BDB4F356-CF69-4152-99A9-1DCE4972AB9D
 * Event Desc:  Current Display brightness status has changed due to a user or
 * other override
 */
#define DISPLAY_STATUS_CHANGED {0xBD, 0xB4, 0xF3, 0x56, 0xCF, 0x69, 0x41, 0x52, \
    0x99, 0xA9, 0x1D, 0xCE, 0x49, 0x72, 0xAB, 0x9D}

/*
 * Event Name:  PERF_CAPABILITY_CHANGED
 * Event GUID:  9091810C-F301-44D6-B2B5-B301812E4D08
 * Event Desc:  Performance Control Upper/Lower Limits Changed
 */
#define PERF_CAPABILITY_CHANGED {0x90, 0x91, 0x81, 0x0C, 0xF3, 0x01, 0x44, 0xD6, \
    0xB2, 0xB5, 0xB3, 0x01, 0x81, 0x2E, 0x4D, 0x08}

/*
 * Event Name:  PERF_CONTROL_CHANGED
 * Event GUID:  D8B5EA17-5486-40FC-A0C6-2AE92AEB3775
 * Event Desc:  For future use
 */
#define PERF_CONTROL_CHANGED {0xD8, 0xB5, 0xEA, 0x17, 0x54, 0x86, 0x40, 0xFC, \
    0xA0, 0xC6, 0x2A, 0xE9, 0x2A, 0xEB, 0x37, 0x75}

/*
 * Event Name:  POWER_CAPABILITY_CHANGED
 * Event GUID:  82C438DD-673B-46A6-995F-24CAF4644DCF
 * Event Desc:  Power Control Capability Changed
 */
#define POWER_CAPABILITY_CHANGED {0x82, 0xC4, 0x38, 0xDD, 0x67, 0x3B, 0x46, \
    0xA6, 0x99, 0x5F, 0x24, 0xCA, 0xF4, 0x64, \
    0x4D, 0xCF}

/*
 * Event Name:  POWER_THRESHOLD_CROSSED
 * Event GUID:  68138891-C225-438A-8F43-04B071CBF4E3
 * Event Desc:  Programmable Threshold Power Event
 */
#define POWER_THRESHOLD_CROSSED {0x68, 0x13, 0x88, 0x91, 0xC2, 0x25, 0x43, 0x8A, \
    0x8F, 0x43, 0x04, 0xB0, 0x71, 0xCB, 0xF4, 0xE3}

/*
 * Event Name:  PRIORITY_CHANGED
 * Event GUID:  98077FF3-AD61-4E50-AFEE-51D0CE8DE396
 * Event Desc:  Domain priority has changed
 */
#define PRIORITY_CHANGED {0x98, 0x07, 0x7F, 0xF3, 0xAD, 0x61, 0x4E, 0x50, 0xAF, \
    0xEE, 0x51, 0xD0, 0xCE, 0x8D, 0xE3, 0x96}

/*
 * Event Name:  TEMP_THRESHOLD_CROSSED
 * Event GUID:  43CDD7D8-C96D-4EE7-9A4A-7EC5C2EE1B6E
 * Event Desc:  Temperature Threshold Changed
 */
#define TEMP_THRESHOLD_CROSSED {0x43, 0xCD, 0xD7, 0xD8, 0xC9, 0x6D, 0x4E, 0xE7, \
    0x9A, 0x4A, 0x7E, 0xC5, 0xC2, 0xEE, 0x1B, 0x6E}

/*
 * Event Name:  SPEC_INFO_CHANGED
 * Event GUID:  75494A00-417C-4E51-9FAB-FBDD965577D4
 * Event Desc:  Participant Specific Information Changed
 */
#define SPEC_INFO_CHANGED {0x75, 0x49, 0x4A, 0x00, 0x41, 0x7C, 0x4E, 0x51, 0x9F, \
    0xAB, 0xFB, 0xDD, 0x96, 0x55, 0x77, 0xD4}

/*
 * Event Name:  CREATE
 * Event GUID:  F3B70F0B-79BC-4414-BCF7-6389658E9FAB
 * Event Desc:  Create Upper Framework (Participant)
 */
#define CREATE {0xF3, 0xB7, 0x0F, 0x0B, 0x79, 0xBC, 0x44, 0x14, 0xBC, 0xF7, \
    0x63, 0x89, 0x65, 0x8E, 0x9F, 0xAB}

/*
 * Event Name:  DESTROY
 * Event GUID:  D58C6702-BB4E-4F82-87F9-E527FEBBBEE1
 * Event Desc:  Destroy Upper Framework (Participant)
 */
#define DESTROY {0xD5, 0x8C, 0x67, 0x02, 0xBB, 0x4E, 0x4F, 0x82, 0x87, 0xF9, \
    0xE5, 0x27, 0xFE, 0xBB, 0xBE, 0xE1}

/*
 * Event Name:  SHUTDOWN
 * Event GUID:  DE7CA990-66C6-4F0F-B78F-E2774E3CE790
 * Event Desc:  Shutdown Upper Framework (Participant)
 */
#define SHUTDOWN {0xDE, 0x7C, 0xA9, 0x90, 0x66, 0xC6, 0x4F, 0x0F, 0xB7, 0x8F, \
    0xE2, 0x77, 0x4E, 0x3C, 0xE7, 0x90}

/*
 * Event Name:  ACPI
 * Event GUID:  722610FF-EDA3-4FED-BEAE-B70290011287
 * Event Desc:  ACPI Notify To ESIF Event Translation
 */
#define ACPI {0x72, 0x26, 0x10, 0xFF, 0xED, 0xA3, 0x4F, 0xED, 0xBE, 0xAE, 0xB7, \
    0x02, 0x90, 0x01, 0x12, 0x87}

/*
 * Event Name:  COOLING_MODE_ACOUSTIC_LIMIT_CHANGED
 * Event GUID:  0CB6C2E2-3242-40FC-845F-17F824FB857E
 * Event Desc:  Cooling mode Acoustic Limit Changed
 */
#define COOLING_MODE_ACOUSTIC_LIMIT_CHANGED {0x0C, 0xB6, 0xC2, 0xE2, 0x32, 0x42, \
    0x40, 0xFC, 0x84, 0x5F, 0x17, 0xF8, \
    0x24, 0xFB, 0x85, 0x7E}

/*
 * Event Name:  COOLING_MODE_POWER_LIMIT_CHANGED
 * Event GUID:  DBF7B2CF-3B16-4773-9CA9-DD74FF91D6BF
 * Event Desc:  Cooling Mode Power Limit Changed
 */
#define COOLING_MODE_POWER_LIMIT_CHANGED {0xDB, 0xF7, 0xB2, 0xCF, 0x3B, 0x16, \
    0x47, 0x73, 0x9C, 0xA9, 0xDD, 0x74, \
    0xFF, 0x91, 0xD6, 0xBF}

/*
 * Event Name:  OS_LPM_MODE_CHANGED
 * Event GUID:  5569447B-6E8F-4FE2-94DE-C31DA011ECF7
 * Event Desc:  OS LPM Changed
 */
#define OS_LPM_MODE_CHANGED {0x55, 0x69, 0x44, 0x7B, 0x6E, 0x8F, 0x4F, 0xE2, \
    0x94, 0xDE, 0xC3, 0x1D, 0xA0, 0x11, 0xEC, 0xF7}

/*
 * Event Name:  PASSIVE_TABLE_CHANGED
 * Event GUID:  661C68E1-B73E-4D02-859B-F1C1505F90D1
 * Event Desc:  PSV Object Changed
 */
#define PASSIVE_TABLE_CHANGED {0x66, 0x1C, 0x68, 0xE1, 0xB7, 0x3E, 0x4D, 0x02, \
    0x85, 0x9B, 0xF1, 0xC1, 0x50, 0x5F, 0x90, 0xD1}

/*
 * Event Name:  SENSOR_ORIENTATION_CHANGED
 * Event GUID:  019C3571-3560-4EC6-BDED-884F6125B5F9
 * Event Desc:  Sensor Orientation Changed
 */
#define SENSOR_ORIENTATION_CHANGED {0x01, 0x9C, 0x35, 0x71, 0x35, 0x60, 0x4E, \
    0xC6, 0xBD, 0xED, 0x88, 0x4F, 0x61, 0x25, \
    0xB5, 0xF9}

/*
 * Event Name:  SENSOR_SPATIAL_ORIENTATION_CHANGED
 * Event GUID:  164B8FD7-C165-4C86-8E9B-4464B6EEC015
 * Event Desc:  Sensor Spatial Orientation Changed
 */
#define SENSOR_SPATIAL_ORIENTATION_CHANGED {0x16, 0x4B, 0x8F, 0xD7, 0xC1, 0x65, \
    0x4C, 0x86, 0x8E, 0x9B, 0x44, 0x64, \
    0xB6, 0xEE, 0xC0, 0x15}

/*
 * Event Name:  SENSOR_PROXIMITY_CHANGED
 * Event GUID:  C7C83E34-519B-4650-A8B2-640E31F5BB0A
 * Event Desc:  Sensor Proximity Changed
 */
#define SENSOR_PROXIMITY_CHANGED {0xC7, 0xC8, 0x3E, 0x34, 0x51, 0x9B, 0x46, \
    0x50, 0xA8, 0xB2, 0x64, 0x0E, 0x31, 0xF5, \
    0xBB, 0x0A}

/*
 * Event Name:  SYSTEM_COOLING_POLICY_CHANGED
 * Event GUID:  5C7D591E-2EA8-4DA1-85A4-476191404650
 * Event Desc:  System Cooling Policy
 */
#define SYSTEM_COOLING_POLICY_CHANGED {0x5C, 0x7D, 0x59, 0x1E, 0x2E, 0xA8, 0x4D, \
    0xA1, 0x85, 0xA4, 0x47, 0x61, 0x91, 0x40, \
    0x46, 0x50}

/*
 * Event Name:  LPM_MODE_CHANGED
 * Event GUID:  DDADD3BF-2385-4E3B-B242-2793B81293AA
 * Event Desc:  Non OS LPM Changed
 */
#define LPM_MODE_CHANGED {0xDD, 0xAD, 0xD3, 0xBF, 0x23, 0x85, 0x4E, 0x3B, 0xB2, \
    0x42, 0x27, 0x93, 0xB8, 0x12, 0x93, 0xAA}

/*
 * Event Name:  OS_CTDP_CAPABILITY_CHANGED
 * Event GUID:  07029cd8-4664-4698-95d8-43b2e9666596
 * Event Desc:  OS CTDP Capability Changed
 */
#define OS_CTDP_CAPABILITY_CHANGED {0x07, 0x02, 0x9c, 0xd8, 0x46, 0x64, 0x46, \
    0x98, 0x95, 0xd8, 0x43, 0xb2, 0xe9, 0x66, \
    0x65, 0x96}

// Event Name:  RF_PROFILE_CHANGED
// Event GUID:  C13C9EAF-9F51-4027-BFE2-E278152D238B
// Event Desc:  RF Profile Changed
#define RF_PROFILE_CHANGED {0xC1, 0x3C, 0x9E, 0xAF, 0x9F, 0x51, 0x40, 0x27, \
    0xBF, 0xE2, 0xE2, 0x78, 0x15, 0x2D, 0x23, 0x8B}

// Event Name:  RF_CONNECTION_STATUS_CHANGED
// Event GUID:  127FB178-2FF0-4286-8A3F-9161B6E87D57
// Event Desc:  RF Connection Status Changed
#define RF_CONNECTION_STATUS_CHANGED {0x12, 0x7F, 0xB1, 0x78, 0x2F, 0xF0, 0x42, 0x86, \
    0x8A, 0x3F, 0x91, 0x61, 0xB6, 0xE8, 0x7D, 0x57}

// Event Name:  LOG_VERBOSITY_CHANGED
// Event GUID:  F77BD545-C448-4B5F-99C8-D0BA02968665
// Event Desc:  Log Verbosity Changed
#define LOG_VERBOSITY_CHANGED {0xF7, 0x7B, 0xD5, 0x45, 0xC4, 0x48, 0x4B, 0x5F, \
    0x99, 0xC8, 0xD0, 0xBA, 0x02, 0x96, 0x86, 0x65}

#endif /* _ESIF_AUTOGEN_H_ */
