# CMakeList.txt : CMake project for RarCrack, include source and define
# project specific logic here.
#
find_package(libxml2 CONFIG REQUIRED)
if(WIN32)
find_package(pthreads REQUIRED)
endif()
# Source files
set(UNRAR_OBJ filestr.cpp recvol.cpp rs.cpp scantree.cpp qopen.cpp)
set(OBJECTS rarcrack.cpp rar.cpp strlist.cpp strfn.cpp pathfn.cpp smallfn.cpp global.cpp file.cpp 
    filefn.cpp filcreat.cpp archive.cpp arcread.cpp unicode.cpp system.cpp crypt.cpp 
    crc.cpp rawread.cpp encname.cpp resource.cpp match.cpp timefn.cpp rdwrfn.cpp 
    consio.cpp options.cpp errhnd.cpp rarvm.cpp secpassword.cpp rijndael.cpp getbits.cpp 
    sha1.cpp sha256.cpp blake2s.cpp hash.cpp extinfo.cpp extract.cpp volume.cpp list.cpp 
    find.cpp unpack.cpp headers.cpp threadpool.cpp rs16.cpp cmddata.cpp ui.cpp)

if(WIN32)
    # Add isnt.cpp only if we're on a UNIX platform
    set(OBJECTS ${OBJECTS} isnt.cpp)
endif()
add_compile_definitions(SILENT)

if(UNIX)
 if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "-O3 -march=native -Wno-switch -Wno-dangling-else -pthread -fomit-frame-pointer")
    set(CMAKE_C_FLAGS "-O3 -DRAR_SMP -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE  -pthread -fomit-frame-pointer")
  endif()
endif()
# Add source to this project's executable.
add_executable (RarCrack ${OBJECTS} ${UNRAR_OBJ})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RarCrack PROPERTY CXX_STANDARD 11)
endif()

target_include_directories(RarCrack PRIVATE ${LIBXML2_INCLUDE_DIRS} ${Iconv_INCLUDE_DIR})
target_link_libraries(RarCrack PRIVATE  ${LIBXML2_LIBRARIES}  ${CMAKE_THREAD_LIBS_INIT})
if(WIN32)
target_include_directories(RarCrack PRIVATE ${PTHREADS4W_INCLUDE_DIRS})
target_link_libraries(RarCrack PRIVATE PThreads4W::PThreads4W)
endif()
# TODO: Add tests and install targets if needed.
add_custom_command(
    TARGET RarCrack POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${RarCrack_SOURCE_DIR}/RarCrack/test.rar
            ${RarCrack_BINARY_DIR}/RarCrack/test.rar
    COMMENT "Copying test.rar to the target directory."
)