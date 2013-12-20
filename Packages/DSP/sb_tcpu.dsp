################################################################################
## Copyright (c) 2013 Intel Corporation All Rights Reserved
##
## Licensed under the Apache License, Version 2.0 (the "License"); you may not
## use this file except in compliance with the License.
##
## You may obtain a copy of the License at
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS"BASIS, WITHOUT
## WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##
## See the License for the specific language governing permissions and
## limitations under the License.
##
################################################################################
#
# Eco-System Independent Framework v1 (ESIF)
# ESIF Device Support Package (DSP)
#
# DSP Package: sb_tcpu
#      Format: DSP Compiler
#     Version: 1.0
#        HMAC: 0000000000000000000000000000000000000000
#
# =====                            ======
# ===== AUTO GENERATED DO NOT EDIT ======
# =====                            ======
#
#
# H=Name, VerMaj, VerMin, Description, Type, BusEnum, acpiDevice, acpiUID, acpiType, acpiScope, pciVendorID, pciDeviceID, pciBus, pciDevice, pciFunction, pciRevision, pciClass, pciSubClass, pciProgIF
# T=actionType, tempXform, tempC1, tempC2, powerXform, timeXform
# E=alias, eventKey, eventType, eventGroup, eventDataType, eventGUID
# D=name, description, qualifier, domainType, priority
# C=capability, capabilityByte Array
# P=Primitive, Qualifier, Instance, Operation, RequestType, ReturnType
# A=ActionPriority, ActionType, IsKernel, p1, p2, p3, p4, p5
#
H,"sb_tcpu",1,0,"ACPI Processor Device","6A77E67D-B0EB-4F86-8EAF-DCBED150A62D",0,"INT3401","","","","","","","","","","","",""
T,1,6,,,1,8
T,4,5,,,0,8
T,22,10,90,,12,8
T,29,6,,,1,8
T,34,11,90,,2,8
#
E,"CORE",0x17C83405-B08B-4FB4-924B-F54F5DDB37E2,8,4,24,0x8487D740-62F7-4030-BE1A-C201377E0C18
E,"PC",0x6A8C5692-C17D-4DB7-B6FE-6BED26D04C16,15,4,24,0x98077FF3-AD61-4E50-AFEE-51D0CE8DE396
E,"PCAP",0x82,11,3,24,0x9091810C-F301-44D6-B2B5-B301812E4D08
E,"PCON",0xD0241F4B-86AE-4561-BF70-E13D011DF646,12,4,24,0xD8B5EA17-5486-40FC-A0C6-2AE92AEB3775
E,"PWCA",0x83,13,3,24,0x82C438DD-673B-46A6-995F-24CAF4644DCF
E,"SIC",0x91,17,3,24,0x75494A00-417C-4E51-9FAB-FBDD965577D4
E,"TTC",0xAB7563E4-C6C0-4D06-9838-4314AB83F9B0,16,4,24,0x43CDD7D8-C96D-4EE7-9A4A-7EC5C2EE1B6E
#
D,"PKG","Package Domain","D0","CC8FEBAE-226C-4EB5-B6CF-498516BA5288",0,9
C,0x21C0 ,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x1,0x0
#
# GET_DEVICE_HARDWARE_ID
P,53,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_HID",,,,
# GET_DEVICE_STATUS
P,88,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_STA",,,,
# GET_DEVICE_UNIQUE_ID
P,67,255,1,24,"<schema></schema>",8,"<schema></schema>"
A,0,4,True,"_UID",,,,
# GET_NOTIFICATION_TEMP_THRESHOLD
P,54,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,4,True,"_NTT",,,,
# GET_PROC_PERF_THROTTLE_CONTROL
P,58,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"_PTC",,,,
# GET_PROC_TJMAX
P,20,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,22,True,0x1a2,23,16,,
# GET_PROC_TURBO_STATE
P,218,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,22,True,0x199,32,32,0x1,
# GET_RAPL_ENERGY
P,128,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,22,True,0x611,31,0,,
# GET_RAPL_ENERGY_UNIT
P,123,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,22,True,0x606,12,8,,
# GET_RAPL_POWER
P,35,255,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,29,True,"GRP0",,,,
# GET_RAPL_POWER_CONTROL_CAPABILITIES
P,75,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"PPCC",,,,
# GET_RAPL_POWER_LIMIT
P,38,0,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,22,True,0x610,14,0,,
# GET_RAPL_POWER_LIMIT_ENABLE
P,126,0,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,22,True,0x610,15,15,,
# GET_RAPL_POWER_UNIT
P,124,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,22,True,0x606,3,0,,
# GET_RAPL_TIME_UNIT
P,122,255,1,24,"<schema></schema>",31,"<schema></schema>"
A,0,22,True,0x606,19,16,,
# GET_RAPL_TIME_WINDOW
P,39,0,1,24,"<schema></schema>",31,"<schema></schema>"
A,0,22,True,0x610,23,17,,
# GET_TEMPERATURE
P,14,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
A,1,29,True,"MAXT",0x3144,0x3244,,
# GET_TEMPERATURE_THRESHOLD_COUNT
P,72,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,1,True,2,,,,
# GET_TEMPERATURE_THRESHOLD_HYSTERESIS
P,15,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,1,True,2,,,,
# GET_TEMPERATURE_THRESHOLDS
P,143,0,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,29,True,"GTT0",,,,
# GET_TEMPERATURE_THRESHOLDS
P,143,1,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,29,True,"GTT1",,,,
# GET_TRIP_POINT_CRITICAL
P,13,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_crt",,,
A,1,4,True,"_CRT",,,,
# GET_TRIP_POINT_HOT
P,12,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_hot",,,
A,1,4,True,"_HOT",,,,
# GET_TRIP_POINT_PASSIVE
P,11,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_psv",,,
A,1,4,True,"_PSV",,,,
# GET_TRIP_POINT_WARM
P,177,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_cr3",,,
A,1,4,True,"_CR3",,,,
# SET_COOLING_POLICY
P,81,255,2,7,"<schema></schema>",24,"<schema></schema>"
A,0,4,True,"_SCP",,,,
# SET_DEVICE_TEMPERATURE_INDICATION
P,51,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,4,True,"_DTI",,,,
# SET_PROC_TURBO_STATE
P,100,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,22,True,0x199,32,32,0x1,
# SET_RAPL_POWER_LIMIT
P,130,0,2,26,"<schema></schema>",24,"<schema></schema>"
A,0,22,True,0x610,14,0,,
# SET_RAPL_POWER_LIMIT_ENABLE
P,222,0,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,22,True,0x610,15,15,,
# SET_RAPL_TIME_WINDOW
P,127,0,2,31,"<schema></schema>",24,"<schema></schema>"
A,0,22,True,0x610,23,17,,
# SET_TEMPERATURE
P,241,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
# SET_TEMPERATURE_THRESHOLDS
P,47,0,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,29,True,"STT0",,,,
# SET_TEMPERATURE_THRESHOLDS
P,47,1,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,29,True,"STT1",,,,
# SET_TRIP_POINT_CRITICAL
P,203,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_crt",,,
# SET_TRIP_POINT_HOT
P,204,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_hot",,,
# SET_TRIP_POINT_PASSIVE
P,206,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_psv",,,
# SET_TRIP_POINT_WARM
P,205,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_cr3",,,
#
D,"CPU","IA Domain","D1","B8F4BD9B-D943-490B-877A-B334DDB9265C",1,0
C,0x1B4 ,0x0,0x0,0x1,0x0,0x1,0x2,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0
#
# GET_DOMAIN_PRIORITY
P,178,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,1,True,9,,,,
# GET_PROC_APPLICATION_EXCLUDE_LIST
P,68,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"AEXL",,,,
# GET_PROC_CURRENT_LOGICAL_PROCESSOR_OFFLINING
P,69,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"CLPO",,,,
# GET_PROC_PERF_PRESENT_CAPABILITY
P,56,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_PPC",,,,
# GET_PROC_PERF_PSTATE_DEPTH_LIMIT
P,55,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_PDL",,,,
# GET_PROC_PERF_SUPPORT_STATES
P,95,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"_PSS",,,,
# GET_PROC_PERF_THROTTLE_PRESENT_CAPABILITY
P,62,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_TPC",,,,
# GET_PROC_PERF_TSTATE_DEPTH_LIMIT
P,61,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_TDL",,,,
# GET_RAPL_ENERGY
P,128,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,22,True,0x639,31,0,,
# GET_RAPL_POWER
P,35,255,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,29,True,"GRP1",,,,
# GET_TEMPERATURE
P,14,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
A,1,22,True,0x19c,31,0,,
# GET_TSTATE_CURRENT
P,16,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,22,True,0x19a,4,0,0x1,
# GET_TSTATES
P,65,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"_TSS",,,,
# SET_ACTIVE_CORE_LIMIT
P,153,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,4,True,"SPUR",,,,
# SET_PERF_PRESENT_CAPABILITY
P,82,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,4,True,"SPPC",,,,
# SET_TEMPERATURE
P,241,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
# SET_TSTATE_CURRENT
P,147,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,22,True,0x19a,4,0,0x1,
#
D,"GFX","Graphics Domain","D2","9FFAF11B-0461-406A-BAFE-34407D6EFE2B",2,1
C,0x1B0 ,0x0,0x0,0x0,0x0,0x1,0x2,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0
#
# GET_DOMAIN_PRIORITY
P,178,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,1,True,13,,,,
# GET_PERF_SUPPORT_STATES
P,137,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,36,True,,,,,
# GET_RAPL_POWER
P,35,255,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,29,True,"SUBP",0x3044,0x3144,,
# GET_TEMPERATURE
P,14,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
A,1,29,True,"MAXT",0x3344,0x3444,,
# SET_PERF_PRESENT_CAPABILITY
P,82,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,36,True,,,,,
# SET_TEMPERATURE
P,241,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
#
D,"DTS0","Die Temp Sensor 0","D3","4B13ADC4-167B-4168-8B4C-0CFE7D4D5EE1",3,3
C,0x0 ,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
#
# GET_TEMPERATURE
P,14,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
A,1,34,True,0x4,0xb1,7,0,
# SET_TEMPERATURE
P,241,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
#
D,"DTS1","Die Temp Sensor 1","D4","9E761824-B517-4604-B20B-53A6427C2B9E",4,3
C,0x0 ,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
#
# GET_TEMPERATURE
P,14,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
A,1,34,True,0x4,0xb1,15,8,
# SET_TEMPERATURE
P,241,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
