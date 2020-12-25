#!/usr/bin/env bash

read -d '' usage << EOF
USAGE

    . configure <options>

OPTIONS

    -c <mode>

        Available modes: 1, 2, 3, and 4.

            1. '3', 'all'

            2. '3', 'diagonal'

            3. 'random', 'all'

            4. 'random', diagonal'

    -f

        Set fullscreen mode.
EOF

if [[ $# -eq 0 ]]; then
    IFS=$'\n'
    echo "${usage[*]}"
fi

CFLAGS=""
while [[ $# -gt 0 ]]; do
    case "$1" in
    "-c")
        case "$2" in
        "1")
            CFLAGS+=" -DRPS_INIT_TRI -DRPS_CELL_ALL"
            ;;
        "2")
            CFLAGS+=" -DRPS_INIT_TRI -DRPS_CELL_DIAG"
            ;;
        "3")
            CFLAGS+=" -DRPS_INIT_RAND -DRPS_CELL_ALL"
            ;;
        "4")
            CFLAGS+=" -DRPS_INIT_RAND -DRPS_CELL_DIAG"
            ;;
        *)
            echo "[ERROR] Invalid option..."
            break;
        esac
        shift
        shift
        ;;
    "-f")
        CFLAGS+=" -DRPS_FULLSCREEN"
        shift
        ;;
    *)
        echo "[ERROR] Invalid option..."
        break
    esac
done

export CFLAGS
export OMP_NUM_THREADS=4
