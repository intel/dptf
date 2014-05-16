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
## distributed under the License is distributed on an "AS IS" BASIS, WITHOUT 
## WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## 
## See the License for the specific language governing permissions and 
## limitations under the License.
##
################################################################################

CC := gcc
ESIF_SOURCES := ../../..
ESIF_WS_SOURCES := $(ESIF_SOURCES)/ESIF_WS/Sources
ESIF_CM_SOURCES := $(ESIF_SOURCES)/ESIF_CM/Sources
ESIF_UF_SOURCES := $(ESIF_SOURCES)/ESIF_UF/Sources
ESIF_WS_SOURCES := $(ESIF_SOURCES)/ESIF_WS/Sources

INC := -I$(ESIF_CM_SOURCES) -I$(ESIF_UF_SOURCES) -I$(ESIF_WS_SOURCES)
LIB := -lsocket
#CC_OPTIONS := -Wall -ansi -pedantic -g $(INC)
CC_OPTIONS := -Wall -g $(INC) -DESIF_ATTR_USER -DESIF_ATTR_OS_LINUX -Wno-multichar


OBJ := $(ESIF_WS_SOURCES)/lin/main.o
OBJ += $(ESIF_WS_SOURCES)/esif_ws_server.o
OBJ += $(ESIF_WS_SOURCES)/esif_ws_algo.o 
OBJ += $(ESIF_WS_SOURCES)/esif_ws_socket.o 
OBJ += $(ESIF_WS_SOURCES)/esif_ws_http.o 
OBJ += $(ESIF_WS_SOURCES)/esif_ws_cgi.o 

%.o: %.c $(DEPS)
	$(CC) -c -fPIC $(CC_OPTIONS) -o $@ $^

all: esif_ws

esif_ws: $(OBJ)
	$(CC) $(CC_OPTIONS) -o $@ $^

clean:
	rm $(OBJ) esif_ws 
