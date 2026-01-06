# CMAKE configuration for HONOR simulation project
######################################################################
# @authors: Wojciech Borkowski, Andrzej Nowak
# @date: 2025-12-13 (last modifications)
#
cmake_minimum_required(VERSION 3.6)
project(honor_model_2)
set(CMAKE_CXX_STANDARD 11) # See: https://matgomes.com/setting-cpp-standard-cmake/

# Define WITHTOOLS if you want build helpers applications!
set(WITHTOOLS false)

if(WIN32)
  # set stuff for windows
  message( ${PROJECT_NAME} ": MS Windows platform assumed.\n\tInside C/C++ code automatic _MSC_VER variable will be used!" )
  set( WINDOWS true )
  #set( MSVC 1 ) #old macro used also somewhere in C code?
  #set( WINDOWS_ONLY_SRC  "${WBRTM}/BASE/wb_assert.c" )
  #add_compile_options( /W4 ) #  /WX
else()
  # set stuff for other systems
  message( ${PROJECT_NAME} ": Linux/unix platform with X11 assumed." )
  set( X11 true )
  #set( X_ONLY_SRC  "" )
  #add_compile_options( -Wall )
endif()

# SymShellLight library is required for compilation. 
if(EXISTS "${honor_model_2_SOURCE_DIR}/../SymShellLight")
    message("1:")
    get_filename_component(SYMSHELL "${honor_model_2_SOURCE_DIR}/../SymShellLight" ABSOLUTE)
elseif(EXISTS "${honor_model_2_SOURCE_DIR}/../symShellLight")
    message("2:")
    get_filename_component(SYMSHELL "${honor_model_2_SOURCE_DIR}/../symShellLight" ABSOLUTE)
elseif(EXISTS "${honor_model_2_SOURCE_DIR}/../../public/SymShellLight")
    message("3:")
    get_filename_component(SYMSHELL "${honor_model_2_SOURCE_DIR}/../../public/SymShellLight" ABSOLUTE)
elseif(EXISTS "${honor_model_2_SOURCE_DIR}/../../public/symShellLight")
    message("4:")
    get_filename_component(SYMSHELL "${honor_model_2_SOURCE_DIR}/../../public/symShellLight" ABSOLUTE)
elseif(EXISTS "${honor_model_2_SOURCE_DIR}/../../../public/SymShellLight")
    message("5:")
    get_filename_component(SYMSHELL "${honor_model_2_SOURCE_DIR}/../../public/SymShellLight" ABSOLUTE)
elseif(EXISTS "${honor_model_2_SOURCE_DIR}/../../../public/symShellLight")
    message("6:")
    get_filename_component(SYMSHELL "${honor_model_2_SOURCE_DIR}/../../public/symShellLight" ABSOLUTE)
else()
    message(FATAL_ERROR  ${PROJECT_NAME} ": SymShell sources not found!")
endif()

message(${PROJECT_NAME} ": SymShell used from ${SYMSHELL}.")

add_subdirectory( ${SYMSHELL} ${SYMSHELL} )

include_directories( ${SYMSHELL} )

if(true)
    add_executable( honor2svg
                    src/HonorAgent.hpp
                    src/HonorAgent.cpp
                    src/honor.cpp
            ../../public/SymShellLight/asserted.h
    )
    target_link_libraries( honor2svg
                    "-L${SYMSHELL}"
                    wbsyshLightSVG
                    rt
                    )
endif()

if(X11) # only for X11 LIB
    add_definitions( -DHAVE_X11 -DUSE_X11 )
    add_executable( honor2
                    src/HonorAgent.hpp
                    src/HonorAgent.cpp
                    src/honor.cpp
                   )
    target_link_libraries( honor2
                    "-L${SYMSHELL}"
                    wbsyshLight
                    X11 Xpm
                    rt
                    )
endif()

if(WINDOWS) # only for WINDOWS
    add_executable( honor2
                    src/HonorAgent.cpp
                    src/honor.cpp
                    src/symshmsc.rc
                    src/symshwin.ico
            ../../public/SymShellLight/asserted.h
    )
    target_link_libraries( honor2
                    "-L${SYMSHELL}"
                    wbsyshLight
                    )
endif(WINDOWS)

if(WITHTOOLS)
   add_subdirectory( mapGen/ )
endif()
