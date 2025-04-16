#!/bin/bash



#######################################
#	NUNIX BUILD FILE v1.0	      #
#######################################

make clean
make all
qemu-system-x86_64 nunix.iso
