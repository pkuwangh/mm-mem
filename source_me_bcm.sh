#!/bin/bash

# alias
alias lt='ls -lhrt'

# utils
infoMsg() {
    echo -e "\033[32;1m${1}\033[0m"
}

warnMsg() {
    echo -e "\033[31;1m${1}\033[0m"
}

checkMsg() {
    echo -e "\033[33;4m${1}\033[0m"
}

checkMsg "Before ..."
echo $CPATH
echo $LIBRARY_PATH
echo $LD_LIBRARY_PATH

# boost
if ! [[ "${LD_LIBRARY_PATH}" =~ "boost" ]]; then
    CM_SHARED_BOOST_PATH="/cm/shared/apps/boost/current"
    CM_LOCAL_BOOST_PATH="/cm/local/apps/boost/current"
    if [ -d $CM_SHARED_BOOST_PATH ]; then
        CM_BOOST_PATH=$CM_SHARED_BOOST_PATH
    elif [ -d $CM_LOCAL_BOOST_PATH ]; then
        CM_BOOST_PATH=$CM_LOCAL_BOOST_PATH
    else
        warnMsg "boost not found in shared place; try 'module load boost'?"
    fi
    if [ -n "${CM_BOOST_PATH}" ]; then
        infoMsg "Found boost at ${CM_BOOST_PATH}/"
        # for cmake to use boost
        export BOOST_ROOT="${CM_BOOST_PATH}"
        export BOOST_INCLUDEDIR="${CM_BOOST_PATH}/include"
        export BOOST_LIBRARYDIR="${CM_BOOST_PATH}/lib64"
        # system paths
        export CPATH="${CM_BOOST_PATH}/include:${CPATH}"
        export LIBRARY_PATH="${CM_BOOST_PATH}/lib64:${LIBRARY_PATH}"
        export LD_LIBRARY_PATH="${CM_BOOST_PATH}/lib64:${LD_LIBRARY_PATH}"
    fi
fi

checkMsg "After ..."
echo $CPATH
echo $LIBRARY_PATH
echo $LD_LIBRARY_PATH
