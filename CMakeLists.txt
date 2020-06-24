cmake_minimum_required(VERSION 3.2.2)

################## ENVIRONMENT ##################
# GCC<5's bug:
# https://stackoverflow.com/questions/28775673/why-cant-i-move-stdofstream
set(GCC_HOME "/opt/compiler/gcc-8.2")
set(PKG_VENDOR "/home/Engineer/code/third-64")

# It seams that CMake would ignore the system directories, and our customized gcc
# installation also ignores them(run `gcc -v -xc++ -E /dev/null` to confirm).
# But the libboosts found by CMake are in /usr/lib64, so we have to specify the
# flags manually if those libboosts are used.

# Also notice to keep the header/lib dirs under GCC_HOME prefered.

set(OPT_SYS_LIBDIRS "${GCC_HOME}/lib64;${GCC_HOME}/lib")
set(OPT_SYS_INCDIRS "${GCC_HOME}/include/c++/8.2.0;\
${GCC_HOME}/include/c++/8.2.0/x86_64-pc-linux-gnu;\
${GCC_HOME}/include/c++/8.2.0/backward;\
${GCC_HOME}/lib/gcc/x86_64-pc-linux-gnu/8.2.0/include;\
${GCC_HOME}/lib/gcc/x86_64-pc-linux-gnu/8.2.0/include-fixed;\
${GCC_HOME}/include")

##################### MAIN ######################

project(top-pv)
add_definitions(-DVERSION=1.0.0 -DENABLE_DEBUG)

set(CMAKE_CXX_COMPILER "${GCC_HOME}/bin/g++")

set(CMAKE_CXX_FLAGS "-std=c++11 -pipe -g -O2 -Wall -W -Werror -fPIC -pthread")
set(CMAKE_EXE_LINKER_FLAGS "-lpthread")

# Embed the library paths into the artifact, since our customized gcc
# is not configured with ability of adding rpaths.
foreach(dir ${OPT_SYS_LIBDIRS})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath=${dir}")
endforeach(dir)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/output")

set(CMAKE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB PROJECT_SRC_FILES "${CMAKE_SOURCE_DIR}/*.cpp")

# CMake's bug:
# https://stackoverflow.com/questions/9948375/cmake-find-package-succeeds-but-returns-wrong-path
set(Boost_NO_BOOST_CMAKE ON)

set(BOOST_ROOT "${PKG_VENDOR}/boost")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
set(Boost_USE_STATIC_LIBS ON)
message("Try to find user installed boost libraries at: ${BOOST_ROOT}")
find_package(Boost COMPONENTS filesystem)

if(NOT Boost_FOUND)
    set(BOOST_ROOT "")
    set(BOOST_INCLUDEDIR "")
    set(BOOST_LIBRARYDIR "")
    set(Boost_USE_STATIC_LIBS "")

    find_package(Boost COMPONENTS filesystem REQUIRED)

    if (${Boost_INCLUDE_DIR} MATCHES "^/.*")
        # about -nostdc:
        # https://gcc.gnu.org/onlinedocs/gcc-4.3.2//gcc/Directory-Options.html
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostdinc++ -nostdinc")
        foreach(dir ${OPT_SYS_INCDIRS})
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${dir}")
        endforeach(dir)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${Boost_INCLUDE_DIR}")
    endif()

    if (${Boost_LIBRARY_DIR} MATCHES "^/.*")
        foreach(dir ${OPT_SYS_LIBDIRS})
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${dir}")
        endforeach(dir)
        set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} -L${Boost_LIBRARY_DIR} -Wl,-rpath=${Boost_LIBRARY_DIR}")
    endif()
endif()

message("Found boost libraries: ${Boost_LIBRARIES}")

add_executable(${PROJECT_NAME} ${PROJECT_SRC_FILES})

# -isystem
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
