#!../../bin/linux-x86_64/armcontrol

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

drvAsynSerialPortConfigure("RS232B1","/dev/armCTST")
asynSetOption("RS232B1", 0, "baud", "115200")
asynSetOption("RS232B1", 0, "bits", "8")
asynSetOption("RS232B1", 0, "parity", "none")
asynSetOption("RS232B1", 0, "stop", "1")
##asynSetOption("RS232B1", 0, "clocal", "Y")
#asynSetOption("RS485", 0, "crtscts", "N")

dbLoadRecords("db/armcontrol.db", "P=UUU:,R=ARM:,BUS=RS232B1")

var streamError 1
# var streamDebug 1
#streamSetLogfile("stream_logfile.txt")



cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncxxx,"user=bernardo"
