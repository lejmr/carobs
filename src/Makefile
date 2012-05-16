#
# OMNeT++/OMNEST Makefile for carobs
#
# This file was generated with the command:
#  opp_makemake -f --deep
#

# Name of target to be created (-o option)
TARGET = carobs$(EXE_SUFFIX)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(TKENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(TKENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = -I. -Ilink -Imessages -Imodules

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS =

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/CoreNodeMAC.o \
    $O/routing.o \
    $O/payloadInterface.o \
    $O/SOAManager.o \
    $O/merge.o \
    $O/MACEntry.o \
    $O/routingTable.o \
    $O/aggregationPoolManager.o \
    $O/aggregationQueues.o \
    $O/SOA.o \
    $O/trainAssembler.o \
    $O/deMux.o \
    $O/SOAEntry.o \
    $O/generator.o \
    $O/MAC.o \
    $O/portManager.o \
    $O/cityGenerator.o \
    $O/sink.o \
    $O/link/fiberChannel.o \
    $O/messages/CAROBSCarHeader_m.o \
    $O/messages/schedulerUnit_m.o \
    $O/messages/trainContainer_m.o \
    $O/messages/Payload_m.o \
    $O/messages/CAROBSHeader_m.o \
    $O/messages/OpticalLayer_m.o \
    $O/messages/car_m.o \
    $O/messages/MACContainer_m.o

# Message files
MSGFILES = \
    messages/CAROBSCarHeader.msg \
    messages/schedulerUnit.msg \
    messages/trainContainer.msg \
    messages/Payload.msg \
    messages/CAROBSHeader.msg \
    messages/OpticalLayer.msg \
    messages/car.msg \
    messages/MACContainer.msg

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppmain$D $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS)  $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	$(CXX) $(LDFLAGS) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(CXX) -c $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

clean:
	-rm -rf $O
	-rm -f carobs carobs.exe libcarobs.so libcarobs.a libcarobs.dll libcarobs.dylib
	-rm -f ./*_m.cc ./*_m.h
	-rm -f link/*_m.cc link/*_m.h
	-rm -f messages/*_m.cc messages/*_m.h
	-rm -f modules/*_m.cc modules/*_m.h

cleanall: clean
	-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc link/*.cc messages/*.cc modules/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/CoreNodeMAC.o: CoreNodeMAC.cc \
	CoreNodeMAC.h \
	SOA.h \
	SOAEntry.h \
	SOAManager.h \
	aggregationQueues.h \
	messages/CAROBSCarHeader_m.h \
	messages/CAROBSHeader_m.h \
	messages/MACContainer_m.h \
	messages/OpticalLayer_m.h \
	messages/Payload_m.h \
	messages/car_m.h \
	messages/schedulerUnit_m.h \
	routing.h
$O/MAC.o: MAC.cc \
	MAC.h \
	MACEntry.h \
	messages/CAROBSHeader_m.h \
	messages/MACContainer_m.h \
	messages/OpticalLayer_m.h \
	messages/car_m.h \
	messages/schedulerUnit_m.h \
	routing.h
$O/MACEntry.o: MACEntry.cc \
	MACEntry.h
$O/SOA.o: SOA.cc \
	CoreNodeMAC.h \
	SOA.h \
	SOAEntry.h \
	SOAManager.h \
	aggregationQueues.h \
	messages/CAROBSCarHeader_m.h \
	messages/CAROBSHeader_m.h \
	messages/MACContainer_m.h \
	messages/OpticalLayer_m.h \
	messages/Payload_m.h \
	messages/car_m.h \
	messages/schedulerUnit_m.h \
	routing.h
$O/SOAEntry.o: SOAEntry.cc \
	SOAEntry.h
$O/SOAManager.o: SOAManager.cc \
	CoreNodeMAC.h \
	SOA.h \
	SOAEntry.h \
	SOAManager.h \
	aggregationQueues.h \
	messages/CAROBSCarHeader_m.h \
	messages/CAROBSHeader_m.h \
	messages/MACContainer_m.h \
	messages/OpticalLayer_m.h \
	messages/Payload_m.h \
	messages/car_m.h \
	messages/schedulerUnit_m.h \
	routing.h
$O/aggregationPoolManager.o: aggregationPoolManager.cc \
	aggregationPoolManager.h \
	aggregationQueues.h \
	messages/Payload_m.h \
	messages/car_m.h \
	routing.h
$O/aggregationQueues.o: aggregationQueues.cc \
	aggregationPoolManager.h \
	aggregationQueues.h \
	messages/Payload_m.h \
	messages/car_m.h \
	routing.h
$O/cityGenerator.o: cityGenerator.cc \
	cityGenerator.h \
	messages/Payload_m.h
$O/deMux.o: deMux.cc \
	deMux.h \
	messages/OpticalLayer_m.h
$O/generator.o: generator.cc \
	generator.h \
	messages/Payload_m.h \
	routingTable.h
$O/merge.o: merge.cc \
	merge.h \
	messages/OpticalLayer_m.h \
	messages/Payload_m.h \
	messages/car_m.h
$O/payloadInterface.o: payloadInterface.cc \
	messages/Payload_m.h \
	payloadInterface.h \
	portManager.h
$O/portManager.o: portManager.cc \
	portManager.h
$O/routing.o: routing.cc \
	routing.h
$O/routingTable.o: routingTable.cc \
	routingTable.h
$O/sink.o: sink.cc \
	messages/Payload_m.h \
	sink.h
$O/trainAssembler.o: trainAssembler.cc \
	messages/CAROBSCarHeader_m.h \
	messages/CAROBSHeader_m.h \
	messages/MACContainer_m.h \
	messages/Payload_m.h \
	messages/car_m.h \
	messages/schedulerUnit_m.h \
	routing.h \
	trainAssembler.h
$O/link/fiberChannel.o: link/fiberChannel.cc \
	link/fiberChannel.h \
	messages/OpticalLayer_m.h
$O/messages/CAROBSCarHeader_m.o: messages/CAROBSCarHeader_m.cc \
	messages/CAROBSCarHeader_m.h
$O/messages/CAROBSHeader_m.o: messages/CAROBSHeader_m.cc \
	messages/CAROBSHeader_m.h
$O/messages/MACContainer_m.o: messages/MACContainer_m.cc \
	messages/MACContainer_m.h
$O/messages/OpticalLayer_m.o: messages/OpticalLayer_m.cc \
	messages/OpticalLayer_m.h
$O/messages/Payload_m.o: messages/Payload_m.cc \
	messages/Payload_m.h
$O/messages/car_m.o: messages/car_m.cc \
	messages/car_m.h
$O/messages/schedulerUnit_m.o: messages/schedulerUnit_m.cc \
	messages/schedulerUnit_m.h
$O/messages/trainContainer_m.o: messages/trainContainer_m.cc \
	messages/trainContainer_m.h

