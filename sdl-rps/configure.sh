#!/usr/bin/env bash
#
# SYNTAX
#
#   source configure.sh [options]
#
# OPTIONS
#
#   -c
#
#     Combination of initialization and cell selection.
#

if [[ $# -eq 0 ]]; then
    echo "[INFO] Using default settings"
    export CFLAGS=" -DRPS_INIT_TRI -DRPS_CELL_ALL"
    return 0
fi

while [[ $# -gt 0 ]];
do
    case "$1" in
    "-c")
        case "$2" in
        "tri-all")
            export CFLAGS=" -DRPS_INIT_TRI -DRPS_CELL_ALL"
            ;;
        "tri-diag")
            export CFLAGS=" -DRPS_INIT_TRI -DRPS_CELL_DIAG"
            ;;
        "rand-all")
            export CFLAGS=" -DRPS_INIT_RAND -DRPS_CELL_ALL"
            ;;
        "rand-diag")
            export CFLAGS=" -DRPS_INIT_RAND -DRPS_CELL_DIAG"
            ;;
        *)
            echo "[ERROR] Invalid option"
            break
        esac
        shift
        shift
        ;;
    *)
        echo "[ERROR] Invalid option"
        break;
    esac
done
