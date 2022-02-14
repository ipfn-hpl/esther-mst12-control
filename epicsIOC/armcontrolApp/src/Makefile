TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = armcontrol
# armcontrol.dbd will be created and installed
DBD += armcontrol.dbd

# armcontrol.dbd will be made up from these files:
armcontrol_DBD += base.dbd
armcontrol_DBD += asyn.dbd
armcontrol_DBD += drvAsynSerialPort.dbd
armcontrol_DBD += stream.dbd

# Include dbd files from all support applications:
#armcontrol_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#armcontrol_LIBS += xxx
armcontrol_LIBS += asyn
armcontrol_LIBS += stream

# armcontrol_registerRecordDeviceDriver.cpp derives from armcontrol.dbd
armcontrol_SRCS += armcontrol_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
armcontrol_SRCS_DEFAULT += armcontrolMain.cpp
armcontrol_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#armcontrol_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
armcontrol_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

