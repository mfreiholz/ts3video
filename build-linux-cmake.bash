#!/bin/bash
source build-linux-env.bash

# Run CMake
cd $BASEDIR
mkdir $OCS_BUILD_DIR_PATH
cd $OCS_BUILD_DIR_PATH
cmake -DCMAKE_INSTALL_PREFIX="$OCS_DEPLOY_DIR_PATH" -DCMAKE_BUILD_TYPE=Release -DIncludeOpenGLSupport=OFF -DIncludeAudioSupport=OFF ..
