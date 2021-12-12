#!/bin/sh
. $BR2_CONFIG 2>/dev/null

"$BR2_EXTERNAL_MYLINUX_PATH/utils/genqemush" "$BR2_CONFIG" "$BINARIES_DIR"
