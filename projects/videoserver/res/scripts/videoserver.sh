#!/bin/sh
# Copyright (c) 2015 Manuel Freiholz
# All rights reserved

current_dir=`dirname "$0"`
binary_name=`basename $0 | sed s,\.sh$,,`
cd "${current_dir}"

case "$1" in
  start)
    # Check whether the script is executed as root.
    if [ "${UID}" = "0" ]; then
      echo "WARNING! For security reasons we adrive: DO NOT RUN THE SERVER AS ROOT"
    fi
    # Start server as background-process.
    echo "Starting the Conference Server"
    shift 1
    export LD_LIBRARY_PATH="${current_dir}"
    "./ld.so" "./${binary_name}" "$@" &
    echo $! > ${binary_name}.pid
    ;;

  stop)
    echo "Stopping the VideoServer"
    if [ -e ${binary_name}.pid ]; then
      # Execute the KILL command (does not kill immediately!).
      if ( kill -TERM $(cat ${binary_name}.pid) 2> /dev/null ); then
        # Wait and check until the process has been killed.
        c=1
        while [ "$c" -le 300 ]; do
          if ( kill -0 $(cat ${binary_name}.pid) 2> /dev/null ); then
            echo -n "."
            sleep 1
          else
            break
          fi
        done
      fi
      if ( kill -0 $(cat ${binary_name}.pid) 2> /dev/null ); then
        echo "Server is not shutting down cleanly - FORCE KILL"
        kill -KILL $(cat ${binary_name}.pid)
      else
        echo "done"
      fi
      rm ${binary_name}.pid
    else
      echo "No server running (${binary_name}.pid is missing)"
      exit 1
    fi
    ;;

  *)
    echo "Usage: ${0} {start|stop}"
    exit 1
    ;;
esac
