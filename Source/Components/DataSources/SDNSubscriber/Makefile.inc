#############################################################
#
# Copyright 2015 F4E | European Joint Undertaking for ITER 
#  and the Development of Fusion Energy ('Fusion for Energy')
# 
# Licensed under the EUPL, Version 1.1 or - as soon they 
# will be approved by the European Commission - subsequent  
# versions of the EUPL (the "Licence"); 
# You may not use this work except in compliance with the 
# Licence. 
# You may obtain a copy of the Licence at: 
#  
# http://ec.europa.eu/idabc/eupl
#
# Unless required by applicable law or agreed to in 
# writing, software distributed under the Licence is 
# distributed on an "AS IS" basis, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
# express or implied. 
# See the Licence for the specific language governing 
# permissions and limitations under the Licence. 
#
#############################################################

OBJSX=SDNSubscriber.x

PACKAGE=Components/DataSources
ROOT_DIR=../../../../
MAKEDEFAULTDIR=$(MARTe2_DIR)/MakeDefaults
include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

INCLUDES += -I.
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L0Types
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L1Portability
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L2Objects
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L3Streams
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L4Configuration
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L4Messages
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L5GAMs

ifdef CODAC_ROOT
INCLUDES += -I$(CODAC_ROOT)/include
endif

ifdef SDN_CORE_INCLUDE_DIR
INCLUDES += -I$(SDN_CORE_INCLUDE_DIR)
endif

ifdef CODAC_ROOT
LIBRARIES += -L$(CODAC_ROOT)/lib -lccs-core
endif

ifdef SDN_CORE_LIBRARY_DIR
LIBRARIES += -L$(SDN_CORE_LIBRARY_DIR) -lsdn-core -lxml2
endif

# Override CPPFLAGS due to scoped enums
CPPFLAGS += -std=c++0x

all: $(OBJS)    \
    $(BUILD_DIR)/SDNSubscriber$(LIBEXT) \
    $(BUILD_DIR)/SDNSubscriber$(DLLEXT)
	echo  $(OBJS)

include depends.$(TARGET)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)
