#!/bin/bash

set -e
LOCAL_PATH=`pwd`
FDKAACPATH=$LOCAL_PATH/fdk-aac-0.1.5
if [ ! -d "$FDKAACPATH" ]; then
    wget https://nchc.dl.sourceforge.net/project/opencore-amr/fdk-aac/fdk-aac-0.1.5.tar.gz
	tar -xf fdk-aac-0.1.5.tar.gz
    ln -s fdk-aac-0.1.5 fdk
fi

