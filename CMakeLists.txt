#########################################################################
# decenttree cmake build definition
# Copyright (c) 2012-2020 Bui Quang Minh, Lam-Tung Nguyen, James Barbetti
#########################################################################

cmake_minimum_required(VERSION 3.18.0 FATAL_ERROR)
#These tricks seem to be necessary to get GCC builds to work on Windows. 
#Which is ludicrous.  Out of the box the GCC compiler can't build even
#a trivial executable, unless you tell it want to link.  And CMake, when
#it is using the compiler, to see if it works, doesn't pass it the parameters
#that are needed, if the linker is to link.
set (CMAKE_C_COMPILER_WORKS 1)
set (CMAKE_CXX_COMPILER_WORKS 1)
set (CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(decenttree)

# Check if inside a conda environment
if(DEFINED ENV{CONDA_PREFIX})
    message(STATUS "Conda environment detected at $ENV{CONDA_PREFIX}")

    # Add conda's library directory to the search paths
    list(APPEND CMAKE_PREFIX_PATH "$ENV{CONDA_PREFIX}")
    list(APPEND CMAKE_LIBRARY_PATH "$ENV{CONDA_PREFIX}/lib")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# The version number.
set (decenttree_VERSION_MAJOR 1)
set (decenttree_VERSION_MINOR 0)
set (decenttree_VERSION_PATCH ".0")

if (CMAKE_C_COMPILER MATCHES "mpic")
    set(DECENTTREE_FLAGS "${DECENTTREE_FLAGS} mpi")
endif()

message("CMAKE_VERSION       : ${CMAKE_VERSION}")
message("DECENTTREE_FLAGS    : ${DECENTTREE_FLAGS}")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Build mode          : Release")
else()
    message("Build mode          : Debug")
endif()

if (CMAKE_GENERATOR MATCHES "Xcode")
    set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
endif()

##################################################################
# Detect target platforms
##################################################################
if (WIN32)
    message("Target OS           : Windows")
    # build as static binary to run on most machines
    if (DECENTTREE_FLAGS MATCHES "static")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    endif()
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
elseif (APPLE)
    message("Target OS           : Mac OS X")
    message("Target System Name    : ${CMAKE_HOST_SYSTEM_NAME}")
    message("Target System Version : ${CMAKE_HOST_SYSTEM_VERSION}")

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_GREATER_EQUAL 13)
            link_directories(/usr/local/opt/llvm/lib)
            find_library(CXXLIB c++ /usr/local/opt/llvm/lib)
        endif()
    endif()

    # to be compatible back to Mac OS X 10.7
    if (DECENTTREE_FLAGS MATCHES "oldmac")
        add_definitions("-mmacosx-version-min=10.5")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mmacosx-version-min=10.5")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        #Don't set --target parameter for Gnu C++ on MacOS
    elseif (CMAKE_HOST_SYSTEM_VERSION VERSION_LESS 20)
        #Don't do this for MacOS BigSur or later
        add_definitions("--target=x86_64-apple-macos10.7")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --target=x86_64-apple-macos10.7")
    else()
        add_definitions("--target=x86_64-apple-macos10.7")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --target=x86_64-apple-macos10.8")
    endif()
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
elseif (UNIX)
    message("Target OS           : Unix")
    # build as static binary to run on most machines
    if (NOT DECENTTREE_FLAGS MATCHES "static")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    endif()
else()
    # Note that IQ-TREE has NOT been tested on other platforms
    message("Target OS          : Unknown and untested yet")
endif()

##################################################################
# Setup compiler, currently supported GCC, CLANG, MSVC, and ICC
##################################################################

set(GCC "FALSE")            #  GNU compiler
set(CLANG "FALSE")          # Clang compiler
set(ICC "FALSE")            # Intel compiler
set(VCC "FALSE")            # MS Visual C Compiler, note that it is different from MSVC variable
set(CLANG_UNDER_VS "FALSE") # Clang compiler, used from inside Visual Studio
# using C++11 standard
if (CMAKE_CXX_COMPILER MATCHES "VISUAL STUDIO")
    set(CLANG_UNDER_VS "TRUE")
    include_directories("C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\Llvm\\lib\\clang\\10.0.0\\include")
    message("Compiler identified as CLANG under Visual Studio: Using CLANG header files")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(VCC "TRUE")
    message("Compiler identified as Visual Studio")
elseif (CMAKE_CXX_COMPILER MATCHES "clang")
    message("Compiler identified as Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif (CMAKE_CXX_COMPILER MATCHES "g\\+\\+")
    set (CMAKE_COMPILER_IS_GNUCXX "TRUE")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    message("Compiler identified as g++/gcc")
else()
    message("Using C++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()
message("CMAKE_CXX_COMPILER_ID was ${CMAKE_CXX_COMPILER_ID}")
message("CMAKE_CXX_COMPILER was ${CMAKE_CXX_COMPILER}")

if (CMAKE_COMPILER_IS_GNUCXX)
    message("Compiler            : GNU Compiler (gcc)")
    set(GCC "TRUE")

    set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} -O2 -g -ffunction-sections -fdata-sections")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -g -ffunction-sections -fdata-sections")
    if (APPLE)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,-dead_strip")
    else()
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections,--no-whole-archive")
    endif()

    if (NOT "${CMAKE_CXX_COMPILER_VERSION}" STREQUAL "")
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS GCC_MIN_VERSION)
            message(FATAL_ERROR "GCC version (${CMAKE_CXX_COMPILER_VERSION}) must be at least ${GCC_MIN_VERSION}!")
        endif()
    endif()

    if (WIN32)
        # disable AVX on 32-bitWindows due to memory alignment
        set(DECENTTREE_FLAGS "${DECENTTREE_FLAGS} noavx")
        message("WARNING: AVX is disabled on Windows as GCC does not properly suport memory alignment")
    endif()

elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Compiler            : Clang")
    set(CLANG "TRUE")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffunction-sections -fdata-sections")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -ffunction-sections -fdata-sections")
    if (APPLE)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,-dead_strip")
    else()
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")
    endif()

    # use libc++ per default in MacOS
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -v")
    if (APPLE)
        SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

    #remove -rdynamic for Clang under Linux
    if (UNIX AND DECENTTREE_FLAGS MATCHES "static")
        SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)
    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(VCC "TRUE")
    message("Compiler            : MS Visual C++ Compiler: ${CMAKE_CXX_COMPILER_ID}")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    message("Compiler            : Intel C++ Compiler (icc)")
    set(ICC "TRUE")
else()
    message("Compiler            : Unknown and untested yet")
endif()
message("Compiler path       : ${CMAKE_CXX_COMPILER}")
message("Compiler version    : ${CMAKE_CXX_COMPILER_VERSION}")

include_directories("${PROJECT_SOURCE_DIR}")

set(EXE_SUFFIX "")

if (MSVC)
    # MS Visual Studio environment
    message("Exporting MS Visual Studio projects...")
    if (CLANG_UNDER_VS)
        add_definitions(/D_UWIN)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /GX")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /I${EIGEN3_INCLUDE_DIR} /GX")
    endif()
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_definitions(/Ot /Oi)
        if (ICC)
            add_definitions(/O3)
        endif()
    endif()
endif()

# enable link time optimization
if (DECENTTREE_FLAGS MATCHES "lto")
    set(COMBINED_FLAGS "${COMBINED_FLAGS} -flto")
endif()

##################################################################
# detect 32 or 64 bit binary
##################################################################
set (BINARY32 "FALSE")
if(CMAKE_SIZEOF_VOID_P EQUAL 4 OR DECENTTREE_FLAGS MATCHES "m32")
    set(BINARY32 "TRUE")
    message("Target binary       : 32-bit")
    if (CMAKE_GENERATOR MATCHES "Win64")
        error("Both 32-bit and 64-bit mode cannot be specified")
    endif()
    if (GCC OR CLANG)
        set(COMBINED_FLAGS "${COMBINED_FLAGS} -m32")
    endif()
    add_definitions(-DBINARY32)
    if (WIN32)
        add_definitions(-DWIN32)
    endif()
else()
    message("Target binary       : 64-bit")
    if (CLANG_UNDER_VS)
        set(COMBINED_FLAGS "${COMBINED_FLAGS} -m64")
    endif()
    if (WIN32)
        add_definitions(-DWIN64)
        set(WIN64 "TRUE")
    endif()
endif()

if(DECENTTREE_FLAGS MATCHES "noavx")
    add_definitions(-D__NOAVX__)
endif()

##################################################################
# configure OpenMP/PThreads compilation
# change the executable name if compiled for OpenMP parallel version
##################################################################

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    message("OpenMP              : Yes")
else()
    message("OpenMP              : No")
    add_definitions(-D_OPENMP=0)
endif()

if (NOT DECENTTREE_FLAGS MATCHES "single")
    if (NOT CLANG_UNDER_VS)
        add_definitions(-D_USE_PTHREADS)
    endif()
    if (MSVC)
        if (NOT CLANG_UNDER_VS)
            if (CMAKE_BUILD_TYPE STREQUAL "Release")
                remove_definitions(/MD)
                add_compile_definitions(/MT)
            else()
                remove_definitions(/MDd)
                add_compile_definitions(/MTd)
            endif()
        endif()
    endif()

    if(CLANG AND APPLE)
        link_directories(${PROJECT_SOURCE_DIR}/libmac)
    elseif (WIN32 OR UNIX)
        if (BINARY32)
            link_directories(${PROJECT_SOURCE_DIR}/lib32)
        else()
            link_directories(${PROJECT_SOURCE_DIR}/lib)
        endif()
    endif()

    if (VCC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
    elseif (ICC)
        if (WIN32)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qopenmp")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -qopenmp")
        endif()
    elseif (GCC)
        if(OPENMP_FOUND)
            set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
            set (GCC_DIR "$ENV{GCC_DIR}")
            if ("${GCC_DIR}" STREQUAL "")
                if(WIN64)
                    set (GCC_DIR "C:\\msys64\\mingw64")
                else()
                endif(WIN64)
            endif()
            if ("${OpenMP_LINKER_FLAGS}" STREQUAL "")
                if ("${GCC_DIR}" STREQUAL "")
                    #So far, not necessary to fake OpenMP flags for GCC on Macintosh.
                else()
                    set (OpenMP_LINKER_FLAGS " -L${GCC_DIR}\\lib -lgomp")
                endif()
            endif()
            set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_LINKER_FLAGS}")
        else(OPENMP_FOUND)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -pthread")
        endif()
    elseif (CLANG)
        if (CLANG_UNDER_VS)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /DCLANG_UNDER_VS=1")
            set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   /openmp /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /DCLANG_UNDER_VS=1")

            set (LLVM_DIR "C:\\Projects\\LLVM_10")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LIBPATH:${LLVM_DIR}\\lib libomp.lib")
        else()
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
            if (APPLE)
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor")
                set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/bin")
            endif()
            if (APPLE OR WIN32 )
                set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lomp")
            endif()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -pthread")
        endif()
    endif()
else()
    message("OpenMP              : NONE")
endif()

##################################################################
# configure SSE/AVX/FMA instructions
##################################################################

SET(AVX_FLAGS "-D__SSE3 -D__AVX")
if (VCC)
    set(AVX_FLAGS "${AVX_FLAGS} /arch:AVX")
elseif (CLANG)
    set(AVX_FLAGS "${AVX_FLAGS} -mavx")
elseif (GCC)
    set(AVX_FLAGS "${AVX_FLAGS} -mavx -fabi-version=0")
elseif (ICC)
    if (WIN32)
        set(AVX_FLAGS "${AVX_FLAGS} /arch:avx")
    else()
        set(AVX_FLAGS "${AVX_FLAGS} -mavx")
    endif()
endif()

SET(SSE_FLAGS "-D__SSE3")
if (VCC)
    if (WIN64)
        set(SSE_FLAGS "${SSE_FLAGS} -D__SSE3__")
    else()
        set(SSE_FLAGS "${SSE_FLAGS} /arch:SSE2 -D__SSE3__")
    endif()
elseif (GCC OR CLANG)
    set(SSE_FLAGS "${SSE_FLAGS} -msse3")
elseif (ICC)
    if (WIN32)
        set(SSE_FLAGS "${SSE_FLAGS} /arch:sse3")
    else()
        set(SSE_FLAGS "${SSE_FLAGS} -msse3")
    endif()
endif()

SET(FMA_FLAGS "-D__SSE3 -D__AVX")
if (VCC)
    set(FMA_FLAGS "${FMA_FLAGS} /arch:AVX2")
elseif (CLANG)
    set(FMA_FLAGS "${FMA_FLAGS} -mavx -mfma")
elseif (GCC)
    set(FMA_FLAGS "${FMA_FLAGS} -mavx -fabi-version=0 -mfma")
elseif (ICC)
    if (WIN32)
        set(FMA_FLAGS "${FMA_FLAGS} /arch:core-avx2")
    else()
        set(FMA_FLAGS "${FMA_FLAGS} -march=core-avx2")
    endif()
endif()

SET(AVX512_FLAGS "-D__SSE3 -D__AVX")
if (NOT DECENTTREE_FLAGS MATCHES "no512")
    if (VCC)
        set(AVX512_FLAGS "${AVX512_FLAGS} /arch:AVX512")
    elseif (CLANG)
        set(AVX512_FLAGS "${AVX512_FLAGS} -mavx512f -mfma")
    elseif (GCC)
        set(AVX512_FLAGS "${AVX512_FLAGS} -mavx512f -mfma")
    elseif (ICC)
        if (WIN32)
            set(AVX512_FLAGS "${AVX512_FLAGS} /QxMIC-AVX512")
        else()
            set(AVX512_FLAGS "${AVX512_FLAGS} -xMIC-AVX512")
        endif()
    endif()
endif()

if (DECENTTREE_FLAGS MATCHES "fma")
    message("Vectorization : AVX+FMA")
    add_definitions(-D__SSE3 -D__AVX)
    set(COMBINED_FLAGS "${COMBINED_FLAGS} ${FMA_FLAGS}")
elseif (DECENTTREE_FLAGS MATCHES "avx")
    message("Vectorization : AVX")
    add_definitions(-D__SSE3 -D__AVX)
    set(COMBINED_FLAGS "${COMBINED_FLAGS} ${AVX_FLAGS}")
elseif (NOT DECENTTREE_FLAGS MATCHES "nosse")
    if (DECENTTREE_FLAGS MATCHES "KNL")
        message("Vectorization : SSE3/AVX/AVX2/AVX-512")
        add_definitions(-D__AVX512KNL)
    else()
        message("Vectorization : SSE3/AVX/AVX2")
    endif()
endif()

##################################################################
# Setup compiler flags
##################################################################

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMBINED_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMBINED_FLAGS}")
set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS} -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -fno-default-inline -fno-inline -O2 -fno-omit-frame-pointer -g")
set(CMAKE_C_FLAGS_PROFILE "${CMAKE_C_FLAGS} -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -O2 -fno-omit-frame-pointer -g")

if(CLANG AND DECENTTREE_FLAGS MATCHES "static")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -Wl,--allow-multiple-definition")
endif()

if (DECENTTREE_FLAGS MATCHES "libcxx")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

if (GCC)
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-inline-functions -fno-inline-functions-called-once -fno-default-inline -fno-inline")
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g -fno-inline-functions -fno-inline-functions-called-once -fno-default-inline -fno-inline")
    set(CMAKE_CXX_FLAGS_MEM "-g -O1")
    set(CMAKE_C_FLAGS_MEM "-g -O1")
elseif (CLANG AND NOT CLANG_UNDER_VS)
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-inline-functions -fno-inline")
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g -fno-inline-functions -fno-inline")
    set(CMAKE_CXX_FLAGS_MEM "-g -O1")
    set(CMAKE_C_FLAGS_MEM "-g -O1")
endif()

##################################################################
# check existence of a few basic functions
##################################################################
include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
check_function_exists (gettimeofday HAVE_GETTIMEOFDAY)
check_function_exists (getrusage HAVE_GETRUSAGE)
check_function_exists (GlobalMemoryStatusEx HAVE_GLOBALMEMORYSTATUSEX)
check_function_exists (strndup HAVE_STRNDUP)
check_function_exists (strtok_r HAVE_STRTOK_R)

find_package(Backtrace)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
        "${PROJECT_SOURCE_DIR}/decenttree_config.h.in"
        "${PROJECT_BINARY_DIR}/decenttree_config.h"
)

configure_file (
        "${PROJECT_SOURCE_DIR}/iqtree_config.h.in"
        "${PROJECT_BINARY_DIR}/iqtree_config.h"
)

# add the binary tree to the search path for include files
# so that we will find decenttree_config.h
include_directories("${PROJECT_BINARY_DIR}")

if(ZLIB_FOUND)
    message ("Using system zlib")
    include_directories(${ZLIB_INCLUDE_DIRS})
else(ZLIB_FOUND)
    message ("Using own zlib-1.2.7")
    include_directories("${PROJECT_BINARY_DIR}/zlib-1.2.7" "${PROJECT_SOURCE_DIR}/zlib-1.2.7")
    add_subdirectory(zlib-1.2.7)
endif(ZLIB_FOUND)

##################################################################
# subdirectories containing necessary libraries for the build
##################################################################

add_subdirectory(vectorclass)
LIST(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

add_compile_definitions(USE_PROGRESS_DISPLAY=1)
add_compile_definitions(USE_VECTORCLASS_LIBRARY=1)
add_compile_definitions(USE_GZSTREAM=1)
add_compile_definitions(DECENT_TREE=1)

message("CMake build type    : ${CMAKE_BUILD_TYPE}")
message("CMake source dir    : ${CMAKE_SOURCE_DIR}")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("C flags             : ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
    message("CXX flags           : ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("C flags             : ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
    message("CXX flags           : ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
elseif (CMAKE_BUILD_TYPE STREQUAL "Profile")
    message("C flags             : ${CMAKE_C_FLAGS_PROFILE} ")
    message("CXX flags           : ${CMAKE_CXX_FLAGS_PROFILE} ")
else()
    message("C flags             : ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
    message("CXX flags           : ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
endif()
message("LINKER flags        : ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
get_directory_property( COMPILER_DEFINITIONS DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS )
message("Compiler defns      : ${COMPILER_DEFINITIONS}")
message("Binary directory    : ${PROJECT_BINARY_DIR}")

##################################################################
# the main executable
##################################################################

add_executable(decenttree
        decenttree.cpp
        flatmatrix.cpp flatmatrix.h
        sequence.cpp   sequence.h
        starttree.cpp  starttree.h
        bionj.cpp
        bionj2.cpp     bionj2.h
        upgma.h        nj.h
        rapidnj.h      auctionmatrix.h
        clustertree.h  distancematrix.h
        stitchup.cpp

        utils/argument.cpp
        utils/hammingdistance.h    utils/heapsort.h
        utils/timeutil.h           utils/safe_io.h
        utils/parallel_mergesort.h utils/parallel_sort.h
        utils/gzstream.cpp         utils/gzstream.h
        utils/progress.cpp         utils/progress.h
        utils/operatingsystem.cpp  utils/operatingsystem.h
        utils/stringfunctions.cpp  utils/stringfunctions.h
        utils/vectortypes.cpp      utils/vectortypes.h)

if(ZLIB_FOUND)
    target_link_libraries(decenttree ${ZLIB_LIBRARIES})
else(ZLIB_FOUND)
    target_link_libraries(decenttree zlibstatic)
endif(ZLIB_FOUND)

if(Backtrace_FOUND)
    include_directories(${Backtrace_INCLUDE_DIR})
    target_link_libraries(decenttree ${Backtrace_LIBRARY})
endif(Backtrace_FOUND)

if (NOT DECENTTREE_FLAGS MATCHES "avx" AND NOT DECENTTREE_FLAGS MATCHES "fma")
    if (NOT DECENTTREE_FLAGS MATCHES "nosse")
        set_target_properties(decenttree vectorclass PROPERTIES COMPILE_FLAGS "${SSE_FLAGS}")
    endif()
endif()

##################################################################
# setup linking flags
##################################################################

# link special lib for WIN32
if (WIN32)
    if (MSVC)
        set(PLATFORM_LIB "ws2_32")
    endif()
else()
    if (NOT "${CXXLIB}" STREQUAL "")
        target_link_libraries(decenttree ${CXXLIB})
    endif()
endif()

if (DECENTTREE_FLAGS MATCHES "libcxx")
    set(STD_LIB "c++abi")
endif()

if(CLANG AND WIN32)
    if (BINARY32)
        target_link_libraries(decenttree ${PROJECT_SOURCE_DIR}/lib32/libiomp5md.dll)
    else()
        target_link_libraries(decenttree ${PROJECT_SOURCE_DIR}/lib/libiomp5md.dll)
    endif()
endif()

# basic linking libraries
# Find the shared OpenMP library
find_library(OpenMP_SHARED_LIBRARY NAMES omp libomp.so HINTS /usr/lib /usr/local/lib $ENV{CONDA_PREFIX}/lib)

# Check if the library was found
if(OpenMP_SHARED_LIBRARY)
    message(STATUS "Found OpenMP shared library: ${OpenMP_SHARED_LIBRARY}")
    target_link_libraries(decenttree ${OpenMP_SHARED_LIBRARY} vectorclass ${PLATFORM_LIB} ${STD_LIB})
else()
    message(FATAL_ERROR "Could not find a suitable shared OpenMP library")
endif()

# setup the executable name
##################################################################
set_target_properties(decenttree PROPERTIES OUTPUT_NAME "decenttree${EXE_SUFFIX}")

# strip the release build
if (NOT DECENTTREE_FLAGS MATCHES "no-strip" AND CMAKE_BUILD_TYPE STREQUAL "Release" AND (GCC OR CLANG) AND NOT APPLE)
    if (WIN32)
        ADD_CUSTOM_COMMAND(TARGET decenttree POST_BUILD COMMAND strip $<TARGET_FILE:decenttree>)
    elseif (NOT APPLE)
        ADD_CUSTOM_COMMAND(TARGET decenttree POST_BUILD COMMAND strip $<TARGET_FILE:decenttree>)
    endif()
endif()

if (MSVC)
    set (BINARY_DIR "${PROJECT_BINARY_DIR}/Release")
else()
    set (BINARY_DIR "${PROJECT_BINARY_DIR}")
endif()

if (WIN32)
    if(GCC)
        #Don't need a post-build target
    elseif(MSVC)
        #ADD_CUSTOM_COMMAND(TARGET decenttree POST_BUILD COMMAND copy "${BINARY_DIR}/decenttree${EXE_SUFFIX}.exe" "${BINARY_DIR}/decenttree${EXE_SUFFIX}-click.exe")
    else()
        ADD_CUSTOM_COMMAND(TARGET decenttree POST_BUILD COMMAND copy "decenttree${EXE_SUFFIX}.exe" "decenttree${EXE_SUFFIX}-click.exe")
    endif()
endif()

add_subdirectory(example)
add_subdirectory(pydecenttree)

##############################################################
# add the install targets
##############################################################
install (TARGETS decenttree DESTINATION bin)

if (WIN32 OR WIN64)
    if (MSVC)
        install (FILES "${BINARY_DIR}/decenttree${EXE_SUFFIX}-click.exe" DESTINATION bin)

        if (NOT DECENTTREE_FLAGS MATCHES "single")
            if (BINARY32)
                install(FILES  "${PROJECT_SOURCE_DIR}/lib32/pthreadVC2.dll" DESTINATION bin)
                install(FILES  "${PROJECT_SOURCE_DIR}/lib32/libiomp5md.dll" DESTINATION bin)
            else()
                install(FILES  "${PROJECT_SOURCE_DIR}/lib/pthreadVC2.dll" DESTINATION bin)
                install(FILES  "${PROJECT_SOURCE_DIR}/lib/libiomp5md.dll" DESTINATION bin)
            endif()
        endif()
    endif(MSVC)

    if (NOT DECENTTREE_FLAGS MATCHES "single" AND CLANG)
        if (BINARY32)
            install(FILES  "${PROJECT_SOURCE_DIR}/lib32/libiomp5md.dll" DESTINATION bin)
        else()
            install(FILES  "${PROJECT_SOURCE_DIR}/lib/libiomp5md.dll" DESTINATION bin)
        endif()
    endif()

endif()

##############################################################
# build a CPack driven installer package
##############################################################
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
        "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "${decenttree_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${decenttree_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${decenttree_VERSION_PATCH}")
if(WIN32 OR APPLE)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_SOURCE_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_SOURCE_GENERATOR "TGZ")
endif()

set(CPACK_SOURCE_IGNORE_FILES
        "/build.*/;/debug.*/;/examples/;/test_scripts/;/manual/;/.bzr/;~$;/\\\\.svn/;/\\\\.git/;/pllrepo/;${CPACK_SOURCE_IGNORE_FILES}")

set (SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    if (DECENTTREE_FLAGS MATCHES "oldmac")
        set (SYSTEM_NAME "MacOS10.5")
    else()
        set (SYSTEM_NAME "MacOSX")
    endif()
endif()

if (BINARY32)
    set (SYSTEM_NAME "${SYSTEM_NAME}32")
endif()

if (DECENTTREE_FLAGS MATCHES "KNL")
    set (SYSTEM_NAME "${SYSTEM_NAME}KNL")
endif()

set(CPACK_PACKAGE_FILE_NAME
        "${CMAKE_PROJECT_NAME}${EXE_SUFFIX}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}${CPACK_PACKAGE_VERSION_PATCH}-${SYSTEM_NAME}")

if (NOT APPLE)
    set(CPACK_STRIP_FILES TRUE)
endif()

include (CPack)

#add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)