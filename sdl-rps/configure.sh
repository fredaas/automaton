#!/usr/bin/env bash
#
# SYNTAX
#   source configure.sh [options]
#
# OPTIONS
#   -c
#     Combination of initialization and cell selection.
#
#   -f
#     Runs the program in full screen mode.
#

FALLBACK=0

# Fallback, if no arguments are given
if [[ $# -eq 0 ]];
then
    FALLBACK=1 
else
    CFLAGS=""
fi

while [[ $# -gt 0 ]];
do
    case "$1" in
    "-c")
        case "$2" in
        "tri-all")
            CFLAGS+=" -DRPS_INIT_TRI -DRPS_CELL_ALL"
            ;;
        "tri-diag")
            CFLAGS+=" -DRPS_INIT_TRI -DRPS_CELL_DIAG"
            ;;
        "rand-all")
            CFLAGS+=" -DRPS_INIT_RAND -DRPS_CELL_ALL"
            ;;
        "rand-diag")
            CFLAGS+=" -DRPS_INIT_RAND -DRPS_CELL_DIAG"
            ;;
        *)
            FALLBACK=1
        esac
        shift
        shift
        ;;
    "-f")
        CFLAGS+=" -DRPS_FULLSCREEN"
        shift
        ;;
    *)
        FALLBACK=1
        break
    esac
done

if [[ $FALLBACK -eq 1 ]]; 
then
    echo "Invalid option. Using fall-back settings..."
    CFLAGS=" -DRPS_INIT_TRI -DRPS_CELL_ALL"
fi

export CFLAGS
export OMP_NUM_THREADS=8
