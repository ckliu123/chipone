set -e

CUR_PATH=$(pwd)
PID="W462CF2700"

#rm -rf ${CUR_PATH}/../out

OUT_DIR="${CUR_PATH}/../out/${PID}_android"

if [ ! -d "${OUT_DIR}" ]; then
	echo $(mkdir -p ${OUT_DIR})
fi

cd ${OUT_DIR}
echo "----------------------------Android-----------------------"
CMAKE=${CUR_PATH}/../tools/cmake-3.27.6-windows-x86_64/bin/cmake
APP_ABI="arm64-v8a"
STL_LINKER="c++_static"
APP_PLATFORM="android-28"

${CMAKE} ${CUR_PATH} \
		-DCMAKE_TOOLCHAIN_FILE=${CUR_PATH}/../tools/android-ndk-r23b/build/cmake/android.toolchain.cmake \
		-DCMAKE_MAKE_PROGRAM=${CUR_PATH}/../tools/ninja.exe \
		-G 'Ninja' \
		-DANDROID_ABI=${APP_ABI} \
		-DANDROID_STL=${STL_LINKER} \
		-DANDROID_NDK=${CUR_PATH}/../tools/android-ndk-r23b \
		-DANDROID_PLATFORM=${APP_PLATFORM} \
		-DLIB_LOG="log" \
		-DPID=${PID} \
		
${CUR_PATH}/../tools/ninja.exe
if [ -f "${OUT_DIR}/libafehal${PID}.so" ]; then
	cp "${OUT_DIR}/libafehal${PID}.so" ${CUR_PATH}/
fi