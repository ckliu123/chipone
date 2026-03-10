cmake_minimum_required(VERSION 3.16)
set (CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
project(afehal${PID})
message("=============== compile library ${PROJECT_NAME} ===============")

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector --param ssp-buffer-size=4")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-command-line-argument")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-short-enums")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORTIFY_SOURCE=2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-label")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-braces")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
set(CMAKE_SHARED_LIBRARY_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")

if (DEFINED HM_EN AND HM_EN STREQUAL "ENABLE") #harmony 
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable")
endif()

if (DEFINED ASAN AND ASAN STREQUAL "ENABLE") #harmony_asan
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -shared-libsan -fsanitize=hwaddress -fno-emulated-tls -mllvm -hwasan-globals=0 -fno-lto -fno-whole-program-vtables")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -shared-libsan -fsanitize=hwaddress -fno-emulated-tls -mllvm -hwasan-globals=0 -fno-lto -fno-whole-program-vtables")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -shared-libsan -fsanitize=hwaddress")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -shared-libsan -fsanitize=hwaddress")
endif()
message("C compiler ${CMAKE_C_COMPILER}")
message("CPP compiler ${CMAKE_CXX_COMPILER}")
set(afehal ${PROJECT_SOURCE_DIR}/src)

include_directories(
	./inc
	./inc/thp
)
file(GLOB_RECURSE AFEHAL_SRC "${afehal}/*.c")

add_library(
	${PROJECT_NAME}
	SHARED
	${AFEHAL_SRC}
)

target_link_options(${PROJECT_NAME} PUBLIC "-rdynamic")
target_link_options(${PROJECT_NAME} PUBLIC "-s")
target_link_options(${PROJECT_NAME} PUBLIC "-Wl,-Bsymbolic")
target_link_options(${PROJECT_NAME} PUBLIC "-Wl,--no-undefined")

target_link_libraries(${PROJECT_NAME} -static-libstdc++ ${LIB_LOG})
