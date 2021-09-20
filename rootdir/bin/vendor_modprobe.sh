#! /vendor/bin/sh
#=============================================================================
# Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
#=============================================================================

MODULES_PATH="/vendor/lib/modules/"

MODPROBE="/vendor/bin/modprobe"
MODULES=`${MODPROBE} -d ${MODULES_PATH} -l`

# Iterate over module list and modprobe them in background.
for MODULE in ${MODULES}; do
        if [ ${MODULE} == "qca_cld3_wlan" ]; then
			echo "Xiaomi Wifi:" ${MODULE}
        elif [ ${MODULE} == "cnss2" ]; then
			echo "Xiaomi Wifi:" ${MODULE}
        else
		${MODPROBE} -a -b -d ${MODULES_PATH} ${MODULE} &
        fi
done

# Wait until all the modprobes are finished
wait
