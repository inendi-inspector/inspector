#!/bin/bash

inendi-lsinit &> /dev/null

export OCL_ICD_VENDORS=/etc/opencl_vendors
mkdir -p $OCL_ICD_VENDORS

NVIDIA_VERSION_NAME=`ls /usr/lib/GL|grep "nvidia-*"|sed -e "s/nvidia-//"`
NVIDIA_VERSION=`echo $NVIDIA_VERSION_NAME | sed -rn 's/([[:digit:]]+)-([[:digit:]]+)/\1\.\2/p'`

echo "/app/lib/libpocl.so" > $OCL_ICD_VENDORS/pocl.icd
if [ -n "$NVIDIA_VERSION" ]; then
	echo "/usr/lib/GL/nvidia-$NVIDIA_VERSION_NAME/lib/libnvidia-opencl.so.$NVIDIA_VERSION" > $OCL_ICD_VENDORS/nvidia.icd
	NVIDIA_EXTRA_LIBS_PATH=$OCL_ICD_VENDORS/lib
	mkdir $NVIDIA_EXTRA_LIBS_PATH
	ln -s /usr/lib/GL/nvidia-$NVIDIA_VERSION_NAME/lib/libnvidia-ptxjitcompiler.so.$NVIDIA_VERSION $NVIDIA_EXTRA_LIBS_PATH/libnvidia-ptxjitcompiler.so.1
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$NVIDIA_EXTRA_LIBS_PATH
fi

eval $@