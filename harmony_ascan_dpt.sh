set -e

CUR_PATH=$(pwd)
PID="W427CF3200"
OUT_DIR="${CUR_PATH}/../out/${PID}_harmony_ascan"
if [ ! -d "${OUT_DIR}" ]; then
	echo $(mkdir -p ${OUT_DIR})
fi


cd ${OUT_DIR}
echo "----------------------------ascan-----------------------"
CMAKE=${CUR_PATH}/../tools/ohos-sdk/windows/nativeTianWang/build-tools/cmake/bin/cmake
APP_ABI="arm64-v8a"
STL_LINKER="c++_static"

${CMAKE} ${CUR_PATH} \
		-DCMAKE_TOOLCHAIN_FILE=${CUR_PATH}/../tools/ohos-sdk/windows/nativeTianWang/build/cmake/ohos.toolchain.cmake \
		-DCMAKE_MAKE_PROGRAM=${CUR_PATH}/../tools/ohos-sdk/windows/nativeTianWang/build-tools/cmake/bin/ninja \
		-G 'Ninja' \
		-DOHOS_ARCH=${APP_ABI} \
		-DOHOS_STL=${STL_LINKER} \
		-DCMAKE_SYSTEM_NAME="OHOS" \
		-DLIB_LOG="libhilog_ndk.z.so" \
		-DOHOS_SDK_NATIVE=${CUR_PATH}/../tools/ohos-sdk/windows/nativeTianWang \
		-DPID=${PID} \
		-DASAN='ENABLE' \
		
${CUR_PATH}/../tools/ninja.exe