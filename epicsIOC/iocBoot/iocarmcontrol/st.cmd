#!../../bin/linux-x86_64/armcontrol
#
#!../../bin/darwin-x86/armcontrol

#- You may have to change armcontrol to something else
#- everywhere it appears in this file

< envPaths

epicsEnvSet( "STREAM_PROTOCOL_PATH", "$(TOP)/db" )

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/armcontrol.dbd"
armcontrol_registerRecordDeviceDriver pdbbase

## Load record instances
#dbLoadRecords("db/xxx.db","user=bernardo")

#drvAsynSerialPortConfigure("RS232B1","/dev/cu.usbmodem14201", 0, 0)
#drvAsynSerialPortConfigure("RS232B1","/dev/SLAB_USBtoUART", 0, 0)
# For virtual serial port do:
# socat -d -d -v pty,rawer,link=Vport EXEC:./echo.sh,pty,rawer
#drvAsynSerialPortConfigure("RS232B1","/dev/ttys001")

drvAsynSerialPortConfigure("RS232B1","/dev/armSTDT")
asynSetOption("RS232B1", 0, "baud", "115200")
asynSetOption("RS232B1", 0, "bits", "8")
asynSetOption("RS232B1", 0, "parity", "none")
asynSetOption("RS232B1", 0, "stop", "1")
# ESP32 boards reboots on serial connect
#asynSetOption("RS232B1", 0, "clocal", "N")
asynSetOption("RS232B1", 0, "crtscts", "Y")
dbLoadRecords("db/armcontrol.db", "P=Esther:,R=ARM:,A=1,BUS=RS232B1")

drvAsynSerialPortConfigure("RS232B2","/dev/armSTDT")
asynSetOption("RS232B2", 0, "baud", "115200")
asynSetOption("RS232B2", 0, "bits", "8")
asynSetOption("RS232B2", 0, "parity", "none")
asynSetOption("RS232B2", 0, "stop", "1")

dbLoadRecords("db/armcontrol.db", "P=Esther:,R=ARM:,A=2,BUS=RS232B2")

var streamError 1
# var streamDebug 1
#streamSetLogfile("stream_logfile.txt")

cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncxxx,"user=bernardo"
