#!/bin/sh
# Copyright (c) 2015 Manuel Freiholz
# All rights reserved

USER="ts3video"
WORKDIR="/opt/ts3video"
CONFIG="${WORKDIR}/default.ini"

#echo $WORKDIR
#exit 0

case "$1" in
  start)
    shift 1
    su $USER -c "${WORKDIR}/start.sh start --config ${CONFIG}"
    ;;
  
  stop)
    shift 1
    su $USER -c "${WORKDIR}/start.sh stop"
    ;;
    
  *)
    echo "Usage: {start|stop}"
    exit 1
    ;;
esac
exit 0
