#!/bin/bash

set -e
LOCAL_PATH=`pwd`
X264PATH=$LOCAL_PATH/x264
if [ ! -d "$X264PATH" ]; then
    git clone http://git.videolan.org/git/x264.git
fi

