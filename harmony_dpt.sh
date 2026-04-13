set -e

CUR_PATH=$(pwd)
PID="W427CF3200"
OUT_DIR="${CUR_PATH}/../out/${PID}_harmony"
if [ ! -d "${OUT_DIR}" ]; then
	echo $(mkdir -p ${OUT_DIR})
fi


cd ${OUT_DIR}
echo "----------------------------harmony-----------------------"
CMAKE=${CUR_PATH}/../tools/cmake-3.27.6-windows-x86_64/bin/cmake
APP_ABI="arm64-v8a"
STL_LINKER="c++_static"

${CMAKE} ${CUR_PATH} \
		-DCMAKE_TOOLCHAIN_FILE=${CUR_PATH}/../tools/ohos-sdk/windows/native/build/cmake/ohos.toolchain.cmake \
		-DCMAKE_MAKE_PROGRAM=${CUR_PATH}/../tools/ninja.exe \
		-G 'Ninja' \
		-DOHOS_ARCH=${APP_ABI} \
		-DOHOS_STL=${STL_LINKER} \
		-DCMAKE_SYSTEM_NAME="OHOS" \
		-DLIB_LOG="libhilog_ndk.z.so" \
		-DOHOS_SDK_NATIVE=${CUR_PATH}/../tools/ohos-sdk/windows/native \
		-DPID=${PID} \
		
${CUR_PATH}/../tools/ninja.exe