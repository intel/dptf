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
# DSP Package: sb_tamb
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
H,"sb_tamb",1,0,"Ambient Temperature Sensor","18A7A6C8-2B10-419B-9579-084C516F71C8",0,"INT3409","","","","","","","","","","","",""
T,1,6,,,2,8
T,4,5,,,3,8
T,29,6,,,2,8
#
E,"TTC",0x90,16,3,24,0x43CDD7D8-C96D-4EE7-9A4A-7EC5C2EE1B6E
#
D,"TMPC","Cold Temperature","D0","CBF064F4-ED4D-4D31-8711-A33096882FA1",1,22
C,0x2100 ,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x1,0x0
#
# GET_AMBIENT_CAPICITANCE_COEF
P,243,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"CPRM",,,,
# GET_AMBIENT_NOTIFICATION_THRESHOLD
P,244,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,4,True,"AMBT",,,,
# GET_DEVICE_DESCRIPTION
P,60,255,1,24,"<schema></schema>",9,"<schema></schema>"
A,0,4,True,"_STR",,,,
# GET_DEVICE_HARDWARE_ID
P,53,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_HID",,,,
# GET_DEVICE_STATUS
P,88,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,4,True,"_STA",,,,
# GET_DEVICE_UNIQUE_ID
P,67,255,1,24,"<schema></schema>",8,"<schema></schema>"
A,0,4,True,"_UID",,,,
# GET_PARTICIPANT_TYPE
P,139,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,1,True,0x16,,,,
# GET_TEMPERATURE
P,14,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,4,True,"TMPC",,,,
# GET_TEMPERATURE_THRESHOLD_COUNT
P,72,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,1,True,2,,,,
A,1,4,True,"PATC",,,,
# GET_TEMPERATURE_THRESHOLD_HYSTERESIS
P,15,255,1,24,"<schema></schema>",3,"<schema></schema>"
A,0,1,True,0,,,,
# GET_TEMPERATURE_THRESHOLDS
P,143,0,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,29,True,"GTT0",,,,
# GET_TEMPERATURE_THRESHOLDS
P,143,1,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,29,True,"GTT1",,,,
# SET_AMBIENT_TEMPERATURE_INDICATION
P,242,255,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,4,True,"_ATI",,,,
# SET_TEMPERATURE_THRESHOLDS
P,47,0,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,29,True,"STT0",,,,
A,1,4,True,"PAT0",,,,
# SET_TEMPERATURE_THRESHOLDS
P,47,1,2,6,"<schema></schema>",24,"<schema></schema>"
A,0,29,True,"STT1",,,,
A,1,4,True,"PAT1",,,,
#
D,"TMPH","Hot Temperature","D1","A6E6DBB3-48B1-41C4-A7E8-8113F7187E31",2,22
C,0x100 ,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0
#
# GET_TEMPERATURE
P,14,255,1,24,"<schema></schema>",6,"<schema></schema>"
A,0,4,True,"TMPH",,,,
