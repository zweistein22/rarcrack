# CMakeLists.txt for UNIX - rarcrack

cmake_minimum_required(VERSION 3.10)
project(rarcrack CXX)

# Compiler flags
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "-march=native  -Wno-switch -Wno-dangling-else  -pthread")
set(CMAKE_C_FLAGS "-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DRAR_SMP -pthread")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -v")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # Equivalent to -fPIC
find_package(PkgConfig REQUIRED)
pkg_check_modules(XML2 REQUIRED libxml-2.0)
include_directories(${XML2_INCLUDE_DIRS})
link_directories(${XML2_LIBRARY_DIRS})

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source files
set(UNRAR_OBJ filestr.cpp recvol.cpp rs.cpp scantree.cpp qopen.cpp)
set(LIB_OBJ filestr.cpp scantree.cpp dll.cpp qopen.cpp)
set(OBJECTS  rar.cpp strlist.cpp strfn.cpp pathfn.cpp smallfn.cpp global.cpp file.cpp 
    filefn.cpp filcreat.cpp archive.cpp arcread.cpp unicode.cpp system.cpp crypt.cpp 
    crc.cpp rawread.cpp encname.cpp resource.cpp match.cpp timefn.cpp rdwrfn.cpp 
    consio.cpp options.cpp errhnd.cpp rarvm.cpp secpassword.cpp rijndael.cpp getbits.cpp 
    sha1.cpp sha256.cpp blake2s.cpp hash.cpp extinfo.cpp extract.cpp volume.cpp list.cpp 
    find.cpp unpack.cpp headers.cpp threadpool.cpp rs16.cpp cmddata.cpp ui.cpp "rarcrack.c")

# Target for unrar executable
add_executable(rarcrack ${OBJECTS} ${UNRAR_OBJ})
target_link_libraries(rarcrack pthread)
target_link_libraries(rarcrack ${XML2_LIBRARIES})
add_custom_command(TARGET rarcrack POST_BUILD
    COMMAND ${CMAKE_STRIP} $<TARGET_FILE:rarcrack>
)
add_custom_command(
    TARGET rarcrack POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${rarcrack_SOURCE_DIR}/test.rar
            $<TARGET_FILE_DIR:rarcrack>
    COMMENT "Copying test.rar to the target directory."
)
# Installation rules
install(TARGETS rarcrack RUNTIME DESTINATION bin)

