set -e

# -------------------
HAL_VER="V1.0.1"
PATH_TOOLS=/d/Project/tools
PID="P520D81300"
# -------------------

CUR_PATH=$(pwd)
OUT_DIR="${CUR_PATH}/out/${PID}_harmony"
if [ ! -d "${OUT_DIR}" ]; then
	echo $(mkdir -p ${OUT_DIR})
fi


cd ${OUT_DIR}
echo "----------------------------harmony-----------------------"
CMAKE=${PATH_TOOLS}/cmake-3.27.6-windows-x86_64/bin/cmake
APP_ABI="arm64-v8a"
STL_LINKER="c++_static"

${CMAKE} ${CUR_PATH} \
		-DCMAKE_TOOLCHAIN_FILE=${PATH_TOOLS}/ohos-sdk/windows/native/build/cmake/ohos.toolchain.cmake \
		-DCMAKE_MAKE_PROGRAM=${PATH_TOOLS}/ninja.exe \
		-G 'Ninja' \
		-DOHOS_ARCH=${APP_ABI} \
		-DOHOS_STL=${STL_LINKER} \
		-DCMAKE_SYSTEM_NAME="OHOS" \
		-DLIB_LOG="libhilog_ndk.z.so" \
		-DOHOS_SDK_NATIVE=${PATH_TOOLS}/ohos-sdk/windows/native \
		-DPID=${PID} \
		
${PATH_TOOLS}/ninja.exe
mkdir -p ${CUR_PATH}/${PID}_afehal_${HAL_VER}
cp ./libafehal*.so ${CUR_PATH}/${PID}_afehal_${HAL_VER}
cp ${CUR_PATH}/push_so_hdc.bat ${CUR_PATH}/${PID}_afehal_${HAL_VER}
rm -rf ${OUT_DIR}