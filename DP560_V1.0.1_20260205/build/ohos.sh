set -e

for tools in $1; do
	eval "${tools%=*}=\${tools#*=}"
done
export CUR_PATH=$(pwd)
export PREBUILT_PATH=${CUR_PATH}/../tools
export APP_ABI="arm64-v8a"
export STL_LINKER="c++_static"
OUT_DIR="${CUR_PATH}/out/ohos"
if [ ! -d "${OUT_DIR}" ]; then
	echo $(mkdir -p ${OUT_DIR})
fi

echo " "
cd ${OUT_DIR}
echo "----------------------------harmony-----------------------"
CMAKE=${TOOLS_PATH}/cmake-3.27.6-windows-x86_64/bin/cmake

${CMAKE} ${CUR_PATH} \
-DCMAKE_TOOLCHAIN_FILE=${HARNOMY}/build/cmake/ohos.toolchain.cmake \
-DCMAKE_MAKE_PROGRAM=${MAKE_TOOLS_PATH} \
-G "${MAKE_TOOLS}" \
-DOHOS_ARCH=${APP_ABI} \
-DOHOS_STL=${STL_LINKER} \
-DCMAKE_SYSTEM_NAME="OHOS" \
-DLIB_LOG="libhilog_ndk.z.so" \
-DOHOS_SDK_NATIVE=${HARNOMY} \
-DPID=${PID} \
-DHM_EN='ENABLE' \
		
${MAKE_TOOLS_PATH}
