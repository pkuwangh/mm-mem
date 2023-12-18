#!/bin/bash

infoMsg() {
    echo -e "\033[32;1m${1}\033[0m"
}

warnMsg() {
    echo -e "\033[31;1m${1}\033[0m"
}

checkMsg() {
    echo -e "\033[33;4m${1}\033[0m"
}

alias lt='ls -lhrt'

CM_SHARED_BOOST_PATH="/cm/shared/apps/boost/current"
CM_LOCAL_BOOST_PATH="/cm/local/apps/boost/current"

if [ -d $CM_SHARED_BOOST_PATH ]; then
    BOOST_PATH=$CM_SHARED_BOOST_PATH
elif [ -d $CM_LOCAL_BOOST_PATH ]; then
    BOOST_PATH=$CM_LOCAL_BOOST_PATH
else
    warnMsg "boost not found in shared place; try `module load boost`?"
    return
fi
infoMsg "Found boost at ${BOOST_PATH}/"

CUDA_PATH="/cm/shared/apps/cuda-latest/toolkit/current"
infoMsg "Found cuda at ${CUDA_PATH}/"

export BOOST_ROOT="${BOOST_PATH}"
export BOOST_INCLUDEDIR="${BOOST_PATH}/include"
export BOOST_LIBRARYDIR="${BOOST_PATH}/lib64"
export CUDACXX="${CUDA_PATH}/bin/nvcc"

export CPATH="${BOOST_PATH}/include:${CUDA_PATH}/include"
export LIBRARY_PATH="${BOOST_PATH}/lib64:${CUDA_PATH}/lib64"
export LD_LIBRARY_PATH="${BOOST_PATH}/lib64:${CUDA_PATH}/lib64"

