#!/bin/bash

set -e

CUR_PATH=$(cd $(dirname $0); pwd)
HOST_OS="windows"
HOST_ARCH="x86_64"
MAKE_TOOLS="Ninja"
EXT_NAME=""
PID="B560D81100"
HALVER='1.0.1'
TOOLS_PATH=${CUR_PATH}/../../tools
CUR_DATE=$(date +"%Y%m%d_%H%M%S")
 


echo $current_date

if ( echo ${CUR_PATH}/ | grep -q "\/build\/" ); then
	CUR_PATH=${CUR_PATH}/..
else
	echo $CUR_PATH
fi





#TOOLS="HARNOMY=${TOOLS_PATH}/ohos-sdk/${HOST_OS}/native "
#TOOLS+="CMAKE=${TOOLS_PATH}/cmake-3.27.6-windows-x86_64/bin/cmake "
#TOOLS+="CMAKE=${TOOLS_PATH}/ohos-sdk/${HOST_OS}/native/build-tools/cmake/bin/cmake "
TOOLS="HARNOMY=${TOOLS_PATH}/ohos-sdk/${HOST_OS}/nativeTianWang "
TOOLS+="CMAKE=${TOOLS_PATH}/ohos-sdk/${HOST_OS}/nativeTianWang/build-tools/cmake/bin/cmake "
#TOOLS+="MAKE_TOOLS_PATH=${TOOLS_PATH}/ohos-sdk/${HOST_OS}/native/build-tools/cmake/bin/$(echo "$MAKE_TOOLS" | tr '[:upper:]' '[:lower:]') "
TOOLS+="MAKE_TOOLS_PATH=${TOOLS_PATH}/ohos-sdk/${HOST_OS}/nativeTianWang/build-tools/cmake/bin/$(echo "$MAKE_TOOLS" | tr '[:upper:]' '[:lower:]') "




TOOLS+="MAKE_TOOLS=${MAKE_TOOLS} "
TOOLS+="HOST_OS=${HOST_OS} "
TOOLS+="HOST_ARCH=${HOST_ARCH} "
TOOLS+="EXT_NAME=${EXT_NAME} "
TOOLS+="PID=${PID} "
TOOLS+="TOOLS_PATH=${TOOLS_PATH} "

# 判断是否有参数 $1
if [ -n "$1" ]; then
    echo "Start Compiling << $1 >> afehal..."
	PARA=$1
else
    echo "Start Compiling All(ohos、asan、android) afehal..."
	PARA="all"
fi

build_afehal(){
	local input_value="$1"
	cd ${CUR_PATH}
	./build/${input_value}.sh "${TOOLS}"
	cd ${CUR_PATH}/out/${input_value}
	find . -type f ! -name "*.so" -exec rm -f {} +;find . -mindepth 1 -type d -exec rm -rf {} +;
}

case ${PARA} in
#	harmony)
#		build_afehal "harmony"
#		;;
#	asan)
#		build_afehal "asan"
#		;;
	android)
		build_afehal "android"
		;;
	all)
		build_afehal "ohos"
		build_afehal "asan"
		build_afehal "android"
		;;
	*)
		echo "Invalid parameter: $1"
        echo "Usage: $0 {harmony|asan|android|all}"
        exit 1
        ;;
esac

cd ${CUR_PATH}/out
echo ""
cp android/libafehal${PID}.so . && 7z a -tzip -bso0 ${PID}_OHOS_${HALVER}.zip android/libafehal${PID}.so && rm libafehal${PID}.so
echo "-- Package ${PID}_DHOS_${HALVER}.zip done --"
echo ""
cp ohos/libafehal${PID}.so . && 7z a -tzip -bso0 ${PID}_OHOS_${HALVER}.zip ohos/libafehal${PID}.so && rm libafehal${PID}.so
cp asan/libafehal${PID}.so . && 7z a -tzip -bso0 ${PID}_OHOS_${HALVER}.zip asan/libafehal${PID}.so && rm libafehal${PID}.so
echo "-- Package ${PID}_OHOS_${HALVER}.zip done --"
echo ""
# TARGET="$(find -name "libafehal*" -type f \( -name "*.a" -o -name "*.so" \))"




