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
# DSP Package: sb_b0_d4_f0
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
H,"sb_b0_d4_f0",1,0,"Processor Device","A66435C1-2DE9-4A94-B834-6B51C8C187FF",1,"","","","","0x8086","0x0c03","0x0","0x4","0x0","","","",""
T,1,6,,,1,8
T,2,6,,,3,8
T,4,5,,,0,8
T,22,7,100,,3,8
T,29,6,,,1,8
#
E,"CORE",0x17C83405-B08B-4FB4-924B-F54F5DDB37E2,8,4,24,0x8487D740-62F7-4030-BE1A-C201377E0C18
E,"CTDP",0x84,7,3,24,0x68D16E98-2C89-4A3D-95C7-5DEEAA4FD73F
E,"PC",0x6A8C5692-C17D-4DB7-B6FE-6BED26D04C16,15,4,24,0x98077FF3-AD61-4E50-AFEE-51D0CE8DE396
E,"PCAP",0x82,11,3,24,0x9091810C-F301-44D6-B2B5-B301812E4D08
E,"PCON",0xD0241F4B-86AE-4561-BF70-E13D011DF646,12,4,24,0xD8B5EA17-5486-40FC-A0C6-2AE92AEB3775
E,"PWCA",0x83,13,3,24,0x82C438DD-673B-46A6-995F-24CAF4644DCF
E,"SIC",0x91,17,3,24,0x75494A00-417C-4E51-9FAB-FBDD965577D4
E,"TTC",0xAB7563E4-C6C0-4D06-9838-4314AB83F9B0,16,4,24,0x43CDD7D8-C96D-4EE7-9A4A-7EC5C2EE1B6E
#
D,"PKG","Package Domain","D0","34C1E485-4F17-4726-ACB9-FECEF81F2AE6",0,9
C,0x61C2 ,0x0,0x1,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x1,0x1
#
# GET_DEVICE_ADDRESS_ON_PARENT_BUS
P,49,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_ADR",,,,
# GET_DEVICE_STATUS
P,88,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_STA",,,,
# GET_NOTIFICATION_TEMP_THRESHOLD
P,54,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,4,True,"_NTT",,,,
# GET_PROC_APPLICATION_EXCLUDE_LIST
P,68,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"AEXL",,,,
# GET_PROC_CTDP_CAPABILITY
P,79,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"TDPC",,,,
# GET_PROC_CTDP_CURRENT_SETTING
P,26,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x5f50,1,0,,
A,1,22,True,0x64b,1,0,,
# GET_PROC_CTDP_LOCK_STATUS
P,27,255,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x5f50,31,31,,
A,1,22,True,0x64b,31,31,,
# GET_PROC_CTDP_POINT_LIST
P,80,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"TDPL",,,,
# GET_PROC_CTDP_SUPPORT_CHECK
P,25,255,1,24,"<schema></schema>",1,"<schema></schema>"
A,0,2,True,0x595c,2,1,,
A,1,22,True,0xce,34,33,,
# GET_PROC_CTDP_TAR_LOCK_STATUS
P,28,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x5f54,31,31,,
A,1,22,True,0x64c,31,31,,
# GET_PROC_MFM_LPM_SUPPORTED
P,207,255,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x595c,0,0,,
A,1,22,True,0xce,32,32,,
# GET_PROC_MFM_MAX_EFFICIENCY_RATIO
P,209,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x595c,15,8,,
A,1,22,True,0xce,47,40,,
# GET_PROC_MFM_MIN_SUPPORTED_RATIO
P,208,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x595c,23,16,,
A,1,22,True,0xce,55,48,,
# GET_PROC_PERF_THROTTLE_CONTROL
P,58,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"_PTC",,,,
# GET_PROC_THERMAL_DESIGN_POWER
P,21,255,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,2,True,0x5930,14,0,,
A,1,22,True,0x614,14,0,,
# GET_PROC_TJMAX
P,20,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x599c,23,16,,
A,1,22,True,0x1a2,23,16,,
# GET_PROC_TURBO_ACTIVATION_RATIO
P,219,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x5f54,7,0,,
# GET_PROC_TURBO_POWER_STATE
P,97,255,1,24,"<schema></schema>",27,"<schema></schema>"
A,1,22,True,0xce,29,29,,
# GET_PROC_TURBO_STATE
P,218,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,22,True,0x199,32,32,0x1,
# GET_RAPL_ENERGY
P,128,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x593c,31,0,,
A,1,22,True,0x611,31,0,,
# GET_RAPL_ENERGY_UNIT
P,123,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x5938,12,8,,
A,1,22,True,0x606,12,8,,
# GET_RAPL_POWER
P,35,255,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,29,True,"GRP0",,,,
# GET_RAPL_POWER_CONTROL_CAPABILITIES
P,75,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"PPCC",,,,
# GET_RAPL_POWER_LIMIT
P,38,0,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,2,True,0x59a0,14,0,,
A,1,22,True,0x610,14,0,,
# GET_RAPL_POWER_LIMIT
P,38,1,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,2,True,0x59a4,14,0,,
A,1,22,True,0x610,46,32,,
# GET_RAPL_POWER_LIMIT_ENABLE
P,126,0,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x59a0,15,15,,
A,1,22,True,0x610,15,15,,
# GET_RAPL_POWER_LIMIT_ENABLE
P,126,1,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x59a4,15,15,,
A,1,22,True,0x610,47,47,,
# GET_RAPL_POWER_LIMIT_LOCK
P,172,255,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x59a4,31,31,,
A,1,22,True,0x610,63,63,,
# GET_RAPL_POWER_MAX
P,23,255,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,2,True,0x5934,14,0,,
A,1,22,True,0x614,46,32,,
# GET_RAPL_POWER_MAX_TIME_WINDOW
P,24,255,1,24,"<schema></schema>",31,"<schema></schema>"
A,0,2,True,0x5934,22,16,,
A,1,22,True,0x614,54,48,,
# GET_RAPL_POWER_MIN
P,22,255,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,2,True,0x5930,30,16,,
A,1,22,True,0x614,30,16,,
# GET_RAPL_POWER_UNIT
P,124,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x5938,3,0,,
A,1,22,True,0x606,3,0,,
# GET_RAPL_TIME_UNIT
P,122,255,1,24,"<schema></schema>",31,"<schema></schema>"
A,0,2,True,0x5938,19,16,,
A,1,22,True,0x606,19,16,,
# GET_RAPL_TIME_WINDOW
P,39,0,1,24,"<schema></schema>",31,"<schema></schema>"
A,0,2,True,0x59a0,23,17,,
A,1,22,True,0x610,23,17,,
# GET_RAPL_TIME_WINDOW
P,39,1,1,24,"<schema></schema>",31,"<schema></schema>"
A,0,2,True,0x59a0,55,49,,
A,1,22,True,0x610,55,49,,
# GET_TEMPERATURE
P,14,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
A,1,2,True,0x5978,7,0,,
A,2,22,True,0x1b1,22,16,,
# GET_TEMPERATURE_THRESHOLD_COUNT
P,72,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,1,True,2,,,,
# GET_TEMPERATURE_THRESHOLD_HYSTERESIS
P,15,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,1,True,0,,,,
# GET_TEMPERATURE_THRESHOLDS
P,143,0,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,29,True,"GTT0",,,,
# GET_TEMPERATURE_THRESHOLDS
P,143,1,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,29,True,"GTT1",,,,
# GET_TRIP_POINT_ACTIVE
P,1,0,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac0",,,
A,1,4,True,"_AC0",,,,
# GET_TRIP_POINT_ACTIVE
P,1,1,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac1",,,
A,1,4,True,"_AC1",,,,
# GET_TRIP_POINT_ACTIVE
P,1,2,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac2",,,
A,1,4,True,"_AC2",,,,
# GET_TRIP_POINT_ACTIVE
P,1,3,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac3",,,
A,1,4,True,"_AC3",,,,
# GET_TRIP_POINT_ACTIVE
P,1,4,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac4",,,
A,1,4,True,"_AC4",,,,
# GET_TRIP_POINT_ACTIVE
P,1,5,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac5",,,
A,1,4,True,"_AC5",,,,
# GET_TRIP_POINT_ACTIVE
P,1,6,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac6",,,
A,1,4,True,"_AC6",,,,
# GET_TRIP_POINT_ACTIVE
P,1,7,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac7",,,
A,1,4,True,"_AC7",,,,
# GET_TRIP_POINT_ACTIVE
P,1,8,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac8",,,
A,1,4,True,"_AC8",,,,
# GET_TRIP_POINT_ACTIVE
P,1,9,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac9",,,
A,1,4,True,"_AC9",,,,
# GET_TRIP_POINT_CRITICAL
P,13,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_crt",,,
A,1,4,True,"_CRT",,,,
# GET_TRIP_POINT_HOT
P,12,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_hot",,,
A,1,4,True,"_HOT",,,,
# GET_TRIP_POINT_PASSIVE
P,11,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_psv",,,
A,1,4,True,"_PSV",,,,
# GET_TRIP_POINT_WARM
P,177,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_cr3",,,
A,1,4,True,"_CR3",,,,
# SET_COOLING_POLICY
P,81,255,2,7,"<schema></schema>",24,"<schema></schema>"
A,0,4,True,"_SCP",,,,
# SET_CTDP_POINT
P,83,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,4,True,"STDP",,,,
# SET_DEVICE_TEMPERATURE_INDICATION
P,51,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,4,True,"_DTI",,,,
# SET_PROC_CTDP_CONTROL
P,221,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x5f50,1,0,,
# SET_PROC_FIVR_RFI_TUNING
P,211,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,1,22,True,0xe3,15,0,,
# SET_PROC_TURBO_ACTIVATION_RATIO
P,220,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x5f54,7,0,,
# SET_PROC_TURBO_STATE
P,100,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,22,True,0x199,32,32,0x1,
# SET_RAPL_POWER_LIMIT
P,130,0,2,26,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59a0,14,0,,
A,1,22,True,0x610,14,0,,
# SET_RAPL_POWER_LIMIT
P,130,1,2,26,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59a4,14,0,,
A,1,22,True,0x610,46,32,,
# SET_RAPL_POWER_LIMIT_ENABLE
P,222,0,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59a0,15,15,,
A,1,22,True,0x610,15,15,,
# SET_RAPL_POWER_LIMIT_ENABLE
P,222,1,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59a4,15,15,,
A,1,22,True,0x610,47,47,,
# SET_RAPL_TIME_WINDOW
P,127,0,2,31,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59a0,23,17,,
A,1,22,True,0x610,23,17,,
# SET_RAPL_TIME_WINDOW
P,127,1,2,31,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59a4,23,17,,
A,1,22,True,0x610,55,49,,
# SET_TEMPERATURE
P,241,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
# SET_TEMPERATURE_THRESHOLDS
P,47,0,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,29,True,"STT0",,,,
A,1,2,True,0x5820,14,8,,
# SET_TEMPERATURE_THRESHOLDS
P,47,1,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,29,True,"STT1",,,,
A,1,2,True,0x5820,22,16,,
# SET_TRIP_POINT_ACTIVE
P,202,0,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac0",,,
# SET_TRIP_POINT_ACTIVE
P,202,1,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac1",,,
# SET_TRIP_POINT_ACTIVE
P,202,2,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac2",,,
# SET_TRIP_POINT_ACTIVE
P,202,3,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac3",,,
# SET_TRIP_POINT_ACTIVE
P,202,4,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac4",,,
# SET_TRIP_POINT_ACTIVE
P,202,5,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac5",,,
# SET_TRIP_POINT_ACTIVE
P,202,6,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac6",,,
# SET_TRIP_POINT_ACTIVE
P,202,7,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac7",,,
# SET_TRIP_POINT_ACTIVE
P,202,8,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac8",,,
# SET_TRIP_POINT_ACTIVE
P,202,9,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_ac9",,,
# SET_TRIP_POINT_CRITICAL
P,203,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_crt",,,
# SET_TRIP_POINT_HOT
P,204,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_hot",,,
# SET_TRIP_POINT_PASSIVE
P,206,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_psv",,,
# SET_TRIP_POINT_WARM
P,205,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_cr3",,,
#
D,"CPU","IA Domain","D1","53071E9E-3E87-46ED-A397-49CA8651D989",1,0
C,0x1B4 ,0x0,0x0,0x1,0x0,0x1,0x2,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0
#
# GET_DOMAIN_PRIORITY
P,178,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x5920,4,0,,
A,1,22,True,0x63a,4,0,,
# GET_PROC_CURRENT_LOGICAL_PROCESSOR_OFFLINING
P,69,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,4,True,"CLPO",,,,
# GET_PROC_LOGICAL_PROCESSOR_AFFINITY
P,228,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,29,True,"GAFF",,,,
# GET_PROC_LOGICAL_PROCESSOR_COUNT
P,101,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,1,22,True,0x35,15,0,,
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
A,0,2,True,0x5928,31,0,,
A,1,22,True,0x639,31,0,,
# GET_RAPL_POWER
P,35,255,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,29,True,"GRP1",,,,
# GET_RAPL_POWER_LIMIT
P,38,0,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,2,True,0x59a8,14,0,,
A,1,22,True,0x638,14,0,,
# GET_RAPL_POWER_LIMIT_ENABLE
P,126,0,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x59a8,15,15,,
A,1,22,True,0x638,15,15,,
# GET_RAPL_POWER_LIMIT_LOCK
P,172,255,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x59a8,31,31,,
A,1,22,True,0x638,31,31,,
# GET_RAPL_TIME_WINDOW
P,39,0,1,24,"<schema></schema>",31,"<schema></schema>"
A,0,2,True,0x59a8,23,17,,
A,1,22,True,0x638,23,17,,
# GET_TEMPERATURE
P,14,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
A,1,2,True,0x597c,7,0,,
# GET_TRIP_POINT_WARM
P,177,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/trippoint/_cr3",,,
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
# SET_PROC_LOGICAL_PROCESSOR_AFFINITY
P,229,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,4,True,"SPUR",,,,
A,1,29,True,"SAFF",,,,
# SET_RAPL_POWER_LIMIT
P,130,0,2,26,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59a8,14,0,,
A,1,22,True,0x638,14,0,,
# SET_RAPL_POWER_LIMIT_ENABLE
P,222,0,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59a8,15,15,,
A,1,22,True,0x638,15,15,,
# SET_RAPL_TIME_WINDOW
P,127,0,2,31,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59a8,23,17,,
A,1,22,True,0x638,23,17,,
# SET_TEMPERATURE
P,241,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
# SET_TSTATE_CURRENT
P,147,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,22,True,0x19a,4,0,0x1,
#
D,"GFX","Graphics Domain","D2","73558331-4753-49B0-9B2D-EF26B10C4DB6",2,1
C,0x1B0 ,0x0,0x0,0x0,0x0,0x1,0x3,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0
#
# GET_DOMAIN_PRIORITY
P,178,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x5924,4,0,,
A,1,22,True,0x642,4,0,,
# GET_PERF_SUPPORT_STATES
P,137,255,1,24,"<schema></schema>",7,"<schema></schema>"
A,0,36,True,,,,,
# GET_RAPL_ENERGY
P,128,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x592c,31,0,,
A,1,22,True,0x641,31,0,,
# GET_RAPL_POWER
P,35,255,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,29,True,"GRP2",,,,
# GET_RAPL_POWER_LIMIT
P,38,0,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,2,True,0x59ac,14,0,,
A,1,22,True,0x640,14,0,,
# GET_RAPL_POWER_LIMIT_ENABLE
P,126,0,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x59ac,15,15,,
A,1,22,True,0x640,15,15,,
# GET_RAPL_POWER_LIMIT_LOCK
P,172,255,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x59ac,31,31,,
A,1,22,True,0x640,31,31,,
# GET_RAPL_TIME_WINDOW
P,39,0,1,24,"<schema></schema>",31,"<schema></schema>"
A,0,2,True,0x59ac,23,17,,
A,1,22,True,0x640,23,17,,
# GET_TEMPERATURE
P,14,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
A,1,2,True,0x5980,7,0,,
# SET_PERF_PRESENT_CAPABILITY
P,82,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,36,True,,,,,
# SET_RAPL_POWER_LIMIT
P,130,0,2,26,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59ac,14,0,,
A,1,22,True,0x640,14,0,,
# SET_RAPL_POWER_LIMIT_ENABLE
P,222,0,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59ac,15,15,,
A,1,22,True,0x640,15,15,,
# SET_RAPL_TIME_WINDOW
P,127,0,2,31,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x59ac,23,17,,
A,1,22,True,0x640,23,17,,
# SET_TEMPERATURE
P,241,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
