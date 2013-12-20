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

CC := x86_64-cros-linux-gnu-gcc
ESIF_SOURCES := ../../..
ESIF_UF_SOURCES := $(ESIF_SOURCES)/ESIF_UF/Sources
ESIF_CM_SOURCES := $(ESIF_SOURCES)/ESIF_CM/Sources
ESIF_LF_SOURCES := $(ESIF_SOURCES)/ESIF_LF/Sources
ESIF_WS_SOURCES := $(ESIF_SOURCES)/ESIF_WS/Sources
ESIF_LIB_SOURCES := $(ESIF_SOURCES)/ESIF_LIB/Sources

LIB := -lpthread -ldl -lrt 
INC := -I$(ESIF_UF_SOURCES) -I$(ESIF_CM_SOURCES) -I$(ESIF_LF_SOURCES) -I$(ESIF_WS_SOURCES) -I$(ESIF_LIB_SOURCES)

CC_OPTIONS := -o9 -Wno-multichar
CC_OPTIONS += -DESIF_ATTR_USER 
CC_OPTIONS += -DESIF_ATTR_OS_CHROME 
CC_OPTIONS += -DESIF_ATTR_64BIT 
CC_OPTIONS += $(INC) 

###############################################################################
# EISF_UF 
###############################################################################

# OS Specific
OBJ := $(ESIF_UF_SOURCES)/lin/main.o
OBJ += $(ESIF_LF_SOURCES)/lin/esif_ipc_os_lin.o

# Common Source 
OBJ += $(ESIF_CM_SOURCES)/esif_debug.o
OBJ += $(ESIF_CM_SOURCES)/esif_ipc.o
OBJ += $(ESIF_CM_SOURCES)/esif_hash_table.o
OBJ += $(ESIF_CM_SOURCES)/esif_link_list.o

OBJ += $(ESIF_UF_SOURCES)/esif_uf.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_action.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_action_config.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_action_constant.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_action_system.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_actmgr.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_app.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_appmgr.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_cnjmgr.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_conjure.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_dsp.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_dspmgr.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_event.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_ipc.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_pm.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_primitive.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_rest.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_service.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_shell.o
OBJ += $(ESIF_UF_SOURCES)/esif_uf_test.o

OBJ += $(ESIF_LIB_SOURCES)/esif_lib_databank.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_datacache.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_datavault.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_eqlcmd.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_eqlparser.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_eqlprovider.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_esifdata.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_esifdatalist.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_iostream.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_istring.o
OBJ += $(ESIF_LIB_SOURCES)/esif_lib_istringlist.o

OBJ += $(ESIF_WS_SOURCES)/esif_ws_algo.o
OBJ += $(ESIF_WS_SOURCES)/esif_ws_cgi.o
OBJ += $(ESIF_WS_SOURCES)/esif_ws_http.o
OBJ += $(ESIF_WS_SOURCES)/esif_ws_server.o
OBJ += $(ESIF_WS_SOURCES)/esif_ws_socket.o


###############################################################################
# BUILD 
###############################################################################

%.o: %.c $(DEPS)
	$(CC) -c -fPIC $(CC_OPTIONS) -o $@ $<

all: esif_uf

esif_uf: $(OBJ)
	$(CC) $(CC_OPTIONS) -o $@ $^ $(LIB) 

clean:
	rm -f $(OBJ) esif_uf 
