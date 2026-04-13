cmake_minimum_required(VERSION 3.16)

project(afehal${PID})

message("=============== compile library ${PROJECT_NAME} ===============")
if (DEFINED __AFE_HAL_TEE_BUILD__)
	add_compile_definitions(__AFE_HAL_TEE_BUILD__)
	SET(CMAKE_SYSTEM_NAME Generic)
	SET(THREADX_TOOLCHAIN "gnu")
	if (DEFINED APP_ARCH OR DEFINED __TUI__)
		SET(CMAKE_SYSTEM_PROCESSOR ${APP_ARCH})
		SET(VFP_FLAGS "-march=${APP_ARCH}")
		add_compile_definitions(__TUI__)
	elseif (DEFINED APP_CPU OR DEFINED __SENSORHUB__)
		SET(CMAKE_SYSTEM_PROCESSOR ${APP_CPU})
		SET(VFP_FLAGS "-mcpu=${APP_CPU}")
		add_compile_definitions(__SENSORHUB__)
	endif ()

	SET(VFP_FLAGS "${VFP_FLAGS} -mfloat-abi=${APP_ABI} -mfpu=${APP_FPU} -mthumb")
	message("vfp_flags = ${VFP_FLAGS}")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -c")
	INCLUDE(${CMAKE_CURRENT_LIST_DIR}/build/arm-none-eabi.cmake)
endif()

add_compile_definitions(VID="${PID}")

set(CMAKE_CXX_STANDARD 20)
set(LIBRARY_OUTPUT_PATH${PROJECT_SOURCE_DIR}/obj)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-command-lineargument")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector --param sspbuffer-size=4")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Qunused-arguments")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-short-enums")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORTIFY_SOURCE=2")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -staticlibstdc++ -static-libgcc")
#
if (DEFINED ASAN AND ASAN STREQUAL "ENABLE")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -shared-libsan -fsanitize=hwaddress -fno-emulated-tls -mllvm -hwasan-globals=0 -fno-lto -fno-whole-program-vtables")

# ����??C++������??�¡���??

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -shared-libsan -fsanitize=hwaddress -fno-emulated-tls -mllvm -hwasan-globals=0 -fno-lto -fno-whole-program-vtables")

# ����??����?��?�¡���??

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -shared-libsan -fsanitize=hwaddress")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -shared-libsan -fsanitize=hwaddress")
endif()
message("C compiler ${CMAKE_C_COMPILER}")
message("CPP compiler ${CMAKE_CXX_COMPILER}")
set(afehal ${PROJECT_SOURCE_DIR}/src)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPENHARMONY -DPROJECT_ID_1=\\\"W427CF1300\\\" -DPROJECT_ID_2=\\\"W427CF3200\\\"")

if("${PID}" STREQUAL "W427CF1300")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPENHARMONY -DM2 -DPROJECT_ID=\\\"${PID}\\\"")
elseif("${PID}" STREQUAL "W427CF3200")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPENHARMONY -DDPT -DPROJECT_ID=\\\"${PID}\\\"")
else()
    message(FATAL_ERROR "Unsupported PID value: ${PID}")
endif()

include_directories(
	${PROJECT_SOURCE_DIR}/inc
)

file(GLOB AFEHAL_SRC
	${afehal}/*.c
)

if (DEFINED __AFE_HAL_TEE_BUILD__)
	add_library(${PROJECT_NAME} STATIC ${AFEHAL_SRC})
else ()
	add_library(${PROJECT_NAME} SHARED ${AFEHAL_SRC})
	target_link_options(${PROJECT_NAME} PUBLIC "-rdynamic")
	target_link_options(${PROJECT_NAME} PUBLIC "-s")
	target_link_options(${PROJECT_NAME} PUBLIC "-Wl,-Bsymbolic")
	target_link_options(${PROJECT_NAME} PUBLIC "-Wl,--no-undefined")
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES 
	MODULE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs/${ANDROID_ABI}
	TAGS optional
)

target_link_libraries(${PROJECT_NAME} -static-libstdc++ ${LIB_LOG})
