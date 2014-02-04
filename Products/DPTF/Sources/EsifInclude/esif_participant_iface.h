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

#ifndef _ESIF_PARTICIPANT_IFACE_H_
#define _ESIF_PARTICIPANT_IFACE_H_

#include "esif.h"
#include "esif_uf_app_event_type.h"

/* Context Registration Functions */
#define ESIF_PARTICIPANT_VERSION 1

#ifdef ESIF_ATTR_KERNEL

/* Flags */
#define ESIF_FLAG_DPTFZ 0x1 /* Participant Is Actually A DPTF Zone */

/* Participant INTERFACE */
struct esif_participant_iface
{
    esif_ver_t                  version;                            /* Version                          */
    esif_guid_t                 class_guid;                         /* Class GUID                       */
    enum esif_participant_enum  enumerator;                         /* Device Enumerator If Any         */
    esif_flags_t                flags;                              /* Flags If Any                     */
    char                        name[ESIF_NAME_LEN];                /* Friendly Name                    */
    char                        desc[ESIF_DESC_LEN];                /* Description                      */
    char                        driver_name[ESIF_NAME_LEN];         /* Driver Name                      */
    char                        device_name[ESIF_NAME_LEN];         /* Device Name                      */
    char                        device_path[ESIF_PATH_LEN];         /* Device Path /sys/bus/platform... */

    /* EVENT Send Event From Driver To Framework */
    enum esif_rc  (*send_event)(struct esif_participant_iface *pi, enum esif_event_type type, u16 domain, struct esif_data *data);
    /* EVENT Receive Event From Framework To Driver */
    enum esif_rc  (*recv_event)(enum esif_event_type type, u16 domain, struct esif_data *data);

    /* ACPI */
    char                        acpi_device[ESIF_SCOPE_LEN];        /* Device INT340X                   */
    char                        acpi_scope[ESIF_SCOPE_LEN];         /* Scope/REGEX e.g. \_SB.PCI0.TPCH  */
    u32                         acpi_uid;                           /* Unique ID If Any                 */
    u32                         acpi_type;                          /* Participant Type If Any          */

    /* PCI */
    u32                         pci_vendor;                         /* PCI Vendor e.g. 0x8086 For Intel */
    u32                         pci_device;                         /* Device ID Unique To Vendor       */
    u8                          pci_bus;                            /* Bus This Device Resides On       */
    u8                          pci_bus_device;                     /* Device Number On Bus             */
    u8                          pci_function;                       /* Function Of Device               */
    u8                          pci_revision;                       /* Revision Of PCI Hardware Device  */
    u8                          pci_class;                          /* Class 3 bytes: (base class,subclass, prog-if) */
    u8                          pci_sub_class;                      /* Sub Class                                     */
    u8                          pci_prog_if;                        /* Program Interface                             */

    esif_device_t               device;                             /* Os Agnostic Driver Context */
    void __iomem                *mem_base;                          /* MMIO/MCHBAR Address        */
#ifdef ESIF_ATTR_OS_WINDOWS
    u32                         mem_size;                           /* MMIO/MCHBAR Size           */
#endif
    acpi_handle                 acpi_handle;                        /* ACPI Handle                */
    mbi_handle                  mbi_handle;                         /* MBI Handle                 */
};

#endif /* ESIF ATTR_KERNEL */
#ifdef ESIF_ATTR_USER

/* Conjure Participant INTERFACE */
typedef struct _t_EsifParticipantIface
{
    esif_ver_t                  version;                            /* Version                          */
    esif_guid_t                 class_guid;                         /* Class GUID                       */
    enum esif_participant_enum  enumerator;                         /* Device Enumerator If Any         */
    esif_flags_t                flags;                              /* Flags If Any                     */
    char                        name[ESIF_NAME_LEN];                /* Friendly Name                    */
    char                        desc[ESIF_DESC_LEN];                /* Description                      */
    char                        driver_name[ESIF_NAME_LEN];         /* Driver Name                      */
    char                        device_name[ESIF_NAME_LEN];         /* Device Name                      */
    char                        device_path[ESIF_PATH_LEN];         /* Device Path /sys/bus/platform... */
    char                        object_id[ESIF_SCOPE_LEN];          /* Scope/REGEX e.g. \_UF.CNJR.WIDI  */

    /* EVENT Send Event From Conjure To Framework */
    enum esif_rc  (*send_event)(struct _t_EsifParticipantIface* pi, enum esif_event_type type, void* data);
    /* EVENT Receive Event From Framework To Conjure */
    enum esif_rc  (*recv_event)(enum esif_event_type type, void* data);

} EsifParticipantIface, *EsifParticipantIfacePtr, **EsifParticipantIfacePtrLocation;

#endif /* ESIF_ATTR_USER */
#endif /* _ESIF_PARTICIPANT_IFACE_H_ */