# cmake_minimum_required(VERSION <specify CMake version here>)
cmake_minimum_required(VERSION 3.19)
project(feeder_parse_history)

#Release Debug
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall")
add_compile_options(-std=c++17 -Wall -O2)

SET(ROOT_DIR "/home/work/sys/lib")

macro(SUBDIRLIST result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

SUBDIRLIST(all_sub_index ${ROOT_DIR})
foreach(child ${all_sub_index})
    list(APPEND all_include ${ROOT_DIR}/${child}/include)
    list(APPEND all_lib ${ROOT_DIR}/${child}/lib)
endforeach()

SET(FEEDER "feeder_parse_czce_history")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/QTSApiPub/include ${all_include})
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/QTSApiPub/lib/Linux64 ${all_lib})
aux_source_directory (./src/qts_feeder SRC_LIST)

# 可执行文件
add_executable(${FEEDER} ${SRC_LIST})
target_link_libraries(${FEEDER}
        feeder coral swordfish x stdc++fs yaml-cpp clickhouse-cpp-lib-static
        boost_date_time boost_filesystem boost_regex boost_system  boost_chrono boost_log boost_program_options boost_thread boost_iostreams z protobuf protobuf-lite sodium zmq ssl crypto iconv pthread dl)

add_executable(wal_reader ./src/wal_reader/main.cc)
target_link_libraries(wal_reader
        feeder coral swordfish x stdc++fs yaml-cpp clickhouse-cpp-lib-static
        boost_date_time boost_filesystem boost_regex boost_system  boost_chrono boost_log boost_program_options boost_thread boost_iostreams z protobuf protobuf-lite sodium zmq ssl crypto iconv pthread dl)


FILE(COPY Dockerfile image.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

FILE(GLOB LIBS_NAME "lib/QTSApiPub/lib/Linux64/*")
MESSAGE("复制库文件 ${LIBS_NAME}")
FILE(COPY ${LIBS_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

FILE(GLOB COMMON_LIB_NAME "${ROOT_DIR}/common_lib/lib/*so*")
MESSAGE("复制公共库文件 ${COMMON_LIB_NAME}")
FILE(COPY ${COMMON_LIB_NAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/image.sh "n=$1; v=`./$n -v`; ldd $n | grep /sys/lib/ | awk '{print $3}' | xargs -i cp -L -n {} .; docker build -t $n:$v .; rm -rf \${n}_$v.docker.tar.gz; docker save $n:$v > \${n}_$v.docker.tar && gzip \${n}_$v.docker.tar")

add_custom_target(image COMMAND sh image.sh ${FEEDER} DEPENDS ${FEEDER})
