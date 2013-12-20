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
# DSP Package: sb_tmem
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
H,"sb_tmem",1,0,"DRAM Memory Device","944974FB-B86D-4063-991B-4D6DD736AA2D",0,"INT3402","","","","","","","","","","","",""
T,1,6,,,1,8
T,2,6,,,3,8
T,4,5,,,0,8
T,22,6,,,3,8
T,29,6,,,1,8
#
E,"PWCA",0x83,13,3,24,0x82C438DD-673B-46A6-995F-24CAF4644DCF
E,"SIC",0x91,17,3,24,0x75494A00-417C-4E51-9FAB-FBDD965577D4
E,"TTC",0x90,16,3,24,0x43CDD7D8-C96D-4EE7-9A4A-7EC5C2EE1B6E
#
D,"DFL","Default Domain","D0","E7F783EE-19E5-4219-96CF-E9E3A26D449F",0,2
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
# GET_RAPL_ENERGY
P,128,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,2,True,0x58e8,31,0,,
A,1,22,True,0x619,31,0,,
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
A,0,2,True,0x58e0,14,0,,
# GET_RAPL_POWER_LIMIT
P,38,1,1,24,"<schema></schema>",26,"<schema></schema>"
A,0,2,True,0x58e4,14,0,,
# GET_RAPL_POWER_LIMIT_ENABLE
P,126,0,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x58e0,15,15,,
A,1,22,True,0x618,15,15,,
# GET_RAPL_POWER_LIMIT_ENABLE
P,126,1,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x58e4,15,15,,
A,1,22,True,0x618,47,47,,
# GET_RAPL_POWER_LIMIT_LOCK
P,172,255,1,24,"<schema></schema>",27,"<schema></schema>"
A,0,2,True,0x58e4,31,31,,
A,1,22,True,0x618,63,63,,
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
A,0,2,True,0x58e0,23,17,,
A,1,22,True,0x618,23,17,,
# GET_RAPL_TIME_WINDOW
P,39,1,1,24,"<schema></schema>",31,"<schema></schema>"
A,0,2,True,0x58e4,23,17,,
A,1,22,True,0x618,55,49,,
# GET_TEMPERATURE
P,14,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
A,1,4,True,"_TMP",,,,
A,2,2,True,0x58b8,7,0,,
# GET_TEMPERATURE_THRESHOLD_COUNT
P,72,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"PATC",,,,
A,1,1,True,2,,,,
# GET_TEMPERATURE_THRESHOLD_HYSTERESIS
P,15,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/gtsh",,,
A,1,4,True,"GTSH",,,,
A,2,1,True,2,,,,
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
# SET_DEVICE_TEMPERATURE_INDICATION
P,51,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,4,True,"_DTI",,,,
# SET_RAPL_POWER_LIMIT
P,130,0,2,26,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x58e0,14,0,,
A,1,22,True,0x618,14,0,,
# SET_RAPL_POWER_LIMIT
P,130,1,2,26,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x58e4,14,0,,
A,1,22,True,0x618,46,32,,
# SET_RAPL_POWER_LIMIT_ENABLE
P,222,0,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x58e0,15,15,,
A,1,22,True,0x618,15,15,,
# SET_RAPL_POWER_LIMIT_ENABLE
P,222,1,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x58e4,15,15,,
A,1,22,True,0618,47,47,,
# SET_RAPL_TIME_WINDOW
P,127,0,2,31,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x58e0,23,17,,
A,1,22,True,0x618,23,17,,
# SET_RAPL_TIME_WINDOW
P,127,1,2,31,"<schema></schema>",24,"<schema></schema>"
A,0,2,True,0x58e4,23,17,,
A,1,22,True,0x618,55,49,,
# SET_TEMPERATURE
P,241,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/_tmp",,,
# SET_TEMPERATURE_THRESHOLD_HYSTERESIS
P,232,255,2,3,"<schema></schema>",24,"<schema></schema>"
A,0,20,False,"@ESIF","/participants/%nm%/gtsh",,,
# SET_TEMPERATURE_THRESHOLDS
P,47,0,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,29,True,"STT0",,,,
A,1,4,True,"PAT0",,,,
# SET_TEMPERATURE_THRESHOLDS
P,47,1,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,29,True,"STT1",,,,
A,1,4,True,"PAT1",,,,
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
