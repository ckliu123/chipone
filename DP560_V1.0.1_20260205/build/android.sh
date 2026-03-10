set -e

for tools in $1; do
	eval "${tools%=*}=\${tools#*=}"
done
export CUR_PATH=$(pwd)
export APP_ABI="arm64-v8a"
export STL_LINKER="c++_static"
export APP_PLATFORM="android-28"

OUT_DIR="${CUR_PATH}/out/android"

if [ ! -d "${OUT_DIR}" ]; then
	echo $(mkdir -p ${OUT_DIR})
fi

echo " "
cd ${OUT_DIR}
echo "----------------------------Android-----------------------"
CMAKE=${TOOLS_PATH}/cmake-3.27.6-windows-x86_64/bin/cmake

${CMAKE} ${CUR_PATH} \
		-DCMAKE_TOOLCHAIN_FILE=${TOOLS_PATH}/android-ndk-r23b/build/cmake/android.toolchain.cmake \
		-DCMAKE_MAKE_PROGRAM=${TOOLS_PATH}/ninja.exe \
		-G "${MAKE_TOOLS}" \
		-DANDROID_ABI=${APP_ABI} \
		-DANDROID_STL=${STL_LINKER} \
		-DANDROID_NDK=${TOOLS_PATH}/android-ndk-r23b \
		-DANDROID_PLATFORM=${APP_PLATFORM} \
		-DLIB_LOG="log" \
		-DPID=${PID} \
		
${TOOLS_PATH}/ninja.exe
