# CMakeLists.txt for UNIX - rarcrack

cmake_minimum_required(VERSION 3.10)


project(rarcrack CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
# Compiler flags

add_compile_definitions(SILENT)

if (WIN32 AND MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  /wd4090 /wd4996")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4090 /wd4996 /std:c11")
    # Debug-Einstellungen für MSVC auf Windows
    set(CMAKE_CXX_FLAGS_DEBUG "/Od")
    set(CMAKE_C_FLAGS_DEBUG "/Od")
    
    # Release-Einstellungen für MSVC auf Windows
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /W1 /DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}  /O2 /W1 /DNDEBUG")
else()

set(CMAKE_CXX_STANDARD 11)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "-O0 -v -g -fno-omit-frame-pointer -pthread")
    set(CMAKE_C_FLAGS "-O0 -v -g -fno-omit-frame-pointer -pthread")
else()
    set(CMAKE_CXX_FLAGS "-O3 -march=native -Wno-switch -Wno-dangling-else -pthread -fomit-frame-pointer")
    set(CMAKE_C_FLAGS "-O3 -DRAR_SMP -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE  -pthread -fomit-frame-pointer")
endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # Equivalent to -fPIC
find_package(LibXml2 REQUIRED)
include_directories(${XML2_INCLUDE_DIRS})
link_directories(${XML2_LIBRARY_DIRS})
find_package(PThreads4W REQUIRED)
include_directories(${PTHREADS4W_INCLUDE_DIRS})

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
    find.cpp unpack.cpp headers.cpp threadpool.cpp rs16.cpp cmddata.cpp ui.cpp rarcrack.c)

# Target for unrar executable
add_executable(rarcrack ${OBJECTS} ${UNRAR_OBJ})
target_link_libraries(rarcrack  ${XML2_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(rarcrack PRIVATE PThreads4W::PThreads4W)
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

